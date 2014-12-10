//
//  lldb_module_memops.cpp
//  Novodb
//
//  Created by mike on 12/7/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//
#include "lldb_module_commands.h"

#include "yara.h"

#include <cstdlib>

namespace novo {

    // stealing from yara.c for now.
    void print_string(
                      uint8_t* data,
                      int length)
    {
        char* str = (char*) (data);
        
        for (int i = 0; i < length; i++)
        {
            if (str[i] >= 32 && str[i] <= 126)
                printf("%c", str[i]);
            else
                printf("\\x%02X", (uint8_t) str[i]);
        }
        
        printf("\n");
    }
    
    
    void print_hex_string(
                          uint8_t* data,
                          int length)
    {
        for (int i = 0; i < min(32, length); i++)
            printf("%02X ", (uint8_t) data[i]);
        
        if (length > 32)
            printf("...");
        
        printf("\n");
    }
    
typedef std::function< int (int message, void* message_data) > match_callback;
    
// a function which converts the call back to a lambda call.
int match_callback_function(int message, void* message_data, void* user_data) {
    match_callback cb = *(match_callback*)user_data;
    
    return cb(message, message_data);
}
    
void callback_function(int error_level, const char* file_name, int line_number, const char* message) {
    std::cout << "error: " << line_number << " " << message << std::endl;
}
    
void register_memops(RequestRouter& req_router, std::vector<LldbProcessSession>& sessions) {
    auto session_bounds = [&sessions](int sid) { return sid >= 0 && sid < sessions.size(); };
    
    req_router.register_path({"read", "memory"}, {
        RequestConstraint::has_int("session", session_bounds),
        RequestConstraint::exists({"address"}),
        RequestConstraint::has_int("count")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        int session_id = stoi(req.at("session"));
        addr_t addr = stoull(req.at("address"), 0, 16);
        size_t get_bytes = stol(req.at("count"));
        
        LldbProcessSession& session = sessions[session_id];
        SBError error;
        
        size_t read_bytes;
        array<unsigned char, 4096> mem_arr;
        
        if( (read_bytes = session.process.ReadMemory(addr, mem_arr.data(), min(mem_arr.size(), get_bytes), error)) != 0) {
            stringstream os;
            
            os << hex << setfill('0');  // set the stream to hex with 0 fill
            
            std::for_each(std::begin(mem_arr), std::end(mem_arr), [&os] (int i) {
                os << setw(2) << i;
            });
            
            output.put("address", to_string(addr));
            output.put("block", os.str());
            output.put("count", to_string(read_bytes));
            
            return ActionResponse::no_error();
        } else {
            return ActionResponse::error(string(error.GetCString()));
        }
    });
    
    req_router.register_path({"search", "memory", "yara"}, {
        RequestConstraint::has_int("session", session_bounds),
        RequestConstraint::exists({"address"}),
        RequestConstraint::has_int("length"),
        RequestConstraint::exists({"pattern"})
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        int session_id = stoi(req.at("session"));
        addr_t addr = stoull(req.at("address"), 0, 16);
        size_t get_bytes = stol(req.at("length"));
        string pattern = req.at("pattern");
        
        LldbProcessSession& session = sessions[session_id];
        SBError error;
        
        // first: read the memory
        size_t read_bytes;
        uint8_t* data = (uint8_t*)malloc(get_bytes);
        
        if(data == nullptr) {
            return ActionResponse::error("Unable to allocate memory");
        }
        
        if( (read_bytes = session.process.ReadMemory(addr, data, get_bytes, error)) <= 0) {
            free(data);
            return ActionResponse::error(string(error.GetCString()));
        }
        
        // second: set up rules (compile)
        YR_RULES* compiled_rules;
        YR_COMPILER* compiler;
        
        if(yr_compiler_create(&compiler) != ERROR_SUCCESS) {
            free(data);
            return ActionResponse::error("Unable to create Yara compiler");
        }
        
        yr_compiler_set_callback(compiler, callback_function);
        
        int comp_error = yr_compiler_add_string(compiler, pattern.c_str(), nullptr);
        if(comp_error > 0) {
            free(data);
            yr_compiler_destroy(compiler);
            
            cout << "errors: " << comp_error << endl;
            
            return ActionResponse::error("Errors compiling the pattern");
        }
        
        if(yr_compiler_get_rules(compiler, &compiled_rules) != ERROR_SUCCESS) {
            free(data);
            yr_compiler_destroy(compiler);
            
            return ActionResponse::error("Errors extracting compiled rules");
        }
        
        // third: scan read memory
        ptree matches;
        
        match_callback cb = [addr, &matches](int message, void* message_data) {
            // one or the other depending on message
            YR_RULE* rule = (YR_RULE*)message_data;
            YR_MODULE_IMPORT* import = (YR_MODULE_IMPORT*)message_data;
            
            switch(message) {
                case CALLBACK_MSG_RULE_MATCHING: {
                    ptree match_obj;
                    YR_STRING* yr_string;
                    
                    yr_rule_strings_foreach(rule, yr_string)
                    {
                        YR_MATCH* match;
                        
                        yr_string_matches_foreach(yr_string, match)
                        {
                            basic_string<uint8_t> matchdata(match->data, match->length);
                            stringstream os;
                            
                            os << hex << setfill('0');  // set the stream to hex with 0 fill
                            
                            std::for_each(std::begin(matchdata), std::end(matchdata), [&os] (int i) {
                                os << setw(2) << i;
                            });

                            match_obj.put("base", to_string(addr));
                            match_obj.put("offset", to_string(match->offset));
                            match_obj.put("identifier", string(yr_string->identifier));
                            match_obj.put("string", os.str());

                            matches.push_back(make_pair("", match_obj));
                        }
                    }
                    
                    break;
                }
                    
                case CALLBACK_MSG_RULE_NOT_MATCHING: {
                    break;
                }
                    
                case CALLBACK_MSG_SCAN_FINISHED: {
                    // cool!
                    break;
                }
                    
                case CALLBACK_MSG_IMPORT_MODULE: {
                    break;
                }
            }
            
            return CALLBACK_CONTINUE;
        };
        
        int scan_ret = yr_rules_scan_mem(compiled_rules, data, read_bytes, 0, match_callback_function, (void*)&cb, 0);
        
        if(scan_ret != ERROR_SUCCESS) {
            // handle scan errors;
        }
        
        if(!matches.empty()) {
            output.put_child("matches", matches);
        }
        
        // finally: do the cleanup.
        free(data);
        yr_rules_destroy(compiled_rules);
        yr_compiler_destroy(compiler);
        
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"write", "byte"}, {
        RequestConstraint::has_int("session", session_bounds),
        RequestConstraint::exists({"address"}),
        RequestConstraint::has_int("byte", 0, 0xFF)
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        int session_id = stoi(req.at("session"));
        addr_t addr = stoull(req.at("address"), 0, 16);
        long writebyte = stol(req.at("byte"));
        
        unsigned char byte = (unsigned char) writebyte;
        
        LldbProcessSession& session = sessions[session_id];
        SBError error;
        
        if(session.process.WriteMemory(addr, &byte, 1, error) != 1) {
            return ActionResponse::error(string(error.GetCString()));
        }
        
        return ActionResponse::no_error();
    });
}

}