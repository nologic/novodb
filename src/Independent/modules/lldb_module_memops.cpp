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
#include <memory>
#include <queue>
#include <algorithm>

namespace novo {
    using namespace std;
    using namespace lldb;
    using namespace boost::property_tree;

    namespace memops {
    
    std::string yr_error_retstr(int v) {
        switch(v) {
            case ERROR_INSUFICIENT_MEMORY:          return "ERROR_INSUFICIENT_MEMORY";
            case ERROR_COULD_NOT_ATTACH_TO_PROCESS: return "ERROR_COULD_NOT_ATTACH_TO_PROCESS";
            case ERROR_COULD_NOT_OPEN_FILE:         return "ERROR_COULD_NOT_OPEN_FILE";
            case ERROR_COULD_NOT_MAP_FILE:          return "ERROR_COULD_NOT_MAP_FILE";
            case ERROR_INVALID_FILE:                return "ERROR_INVALID_FILE";
            case ERROR_CORRUPT_FILE:                return "ERROR_CORRUPT_FILE";
            case ERROR_UNSUPPORTED_FILE_VERSION:    return "ERROR_UNSUPPORTED_FILE_VERSION";
            case ERROR_INVALID_REGULAR_EXPRESSION:  return "ERROR_INVALID_REGULAR_EXPRESSION";
            case ERROR_INVALID_HEX_STRING:          return "ERROR_INVALID_HEX_STRING";
            case ERROR_SYNTAX_ERROR:                return "ERROR_SYNTAX_ERROR";
            case ERROR_LOOP_NESTING_LIMIT_EXCEEDED: return "ERROR_LOOP_NESTING_LIMIT_EXCEEDED";
            case ERROR_DUPLICATED_LOOP_IDENTIFIER:  return "ERROR_DUPLICATED_LOOP_IDENTIFIER";
            case ERROR_DUPLICATED_IDENTIFIER:       return "ERROR_DUPLICATED_IDENTIFIER";
            case ERROR_DUPLICATED_TAG_IDENTIFIER:   return "ERROR_DUPLICATED_TAG_IDENTIFIER";
            case ERROR_DUPLICATED_META_IDENTIFIER:  return "ERROR_DUPLICATED_META_IDENTIFIER";
            case ERROR_DUPLICATED_STRING_IDENTIFIER: return "ERROR_DUPLICATED_STRING_IDENTIFIER";
            case ERROR_UNREFERENCED_STRING:         return "ERROR_UNREFERENCED_STRING";
            case ERROR_UNDEFINED_STRING:            return "ERROR_UNDEFINED_STRING";
            case ERROR_UNDEFINED_IDENTIFIER:        return "ERROR_UNDEFINED_IDENTIFIER";
            case ERROR_MISPLACED_ANONYMOUS_STRING:  return "ERROR_MISPLACED_ANONYMOUS_STRING";
            case ERROR_INCLUDES_CIRCULAR_REFERENCE: return "ERROR_INCLUDES_CIRCULAR_REFERENCE";
            case ERROR_INCLUDE_DEPTH_EXCEEDED:      return "ERROR_INCLUDE_DEPTH_EXCEEDED";
            case ERROR_WRONG_TYPE:                  return "ERROR_WRONG_TYPE";
            case ERROR_EXEC_STACK_OVERFLOW:         return "ERROR_EXEC_STACK_OVERFLOW";
            case ERROR_SCAN_TIMEOUT:                return "ERROR_SCAN_TIMEOUT";
            case ERROR_TOO_MANY_SCAN_THREADS:       return "ERROR_TOO_MANY_SCAN_THREADS";
            case ERROR_CALLBACK_ERROR:              return "ERROR_CALLBACK_ERROR";
            case ERROR_INVALID_ARGUMENT:            return "ERROR_INVALID_ARGUMENT";
            case ERROR_TOO_MANY_MATCHES:            return "ERROR_TOO_MANY_MATCHES";
            case ERROR_INTERNAL_FATAL_ERROR:        return "ERROR_INTERNAL_FATAL_ERROR";
            case ERROR_NESTED_FOR_OF_LOOP:          return "ERROR_NESTED_FOR_OF_LOOP";
            case ERROR_INVALID_FIELD_NAME:          return "ERROR_INVALID_FIELD_NAME";
            case ERROR_UNKNOWN_MODULE:              return "ERROR_UNKNOWN_MODULE";
            case ERROR_NOT_A_STRUCTURE:             return "ERROR_NOT_A_STRUCTURE";
            case ERROR_NOT_INDEXABLE:               return "ERROR_NOT_INDEXABLE";
            case ERROR_NOT_A_FUNCTION:              return "ERROR_NOT_A_FUNCTION";
            case ERROR_INVALID_FORMAT:              return "ERROR_INVALID_FORMAT";
            case ERROR_TOO_MANY_ARGUMENTS:          return "ERROR_TOO_MANY_ARGUMENTS";
            case ERROR_WRONG_ARGUMENTS:             return "ERROR_WRONG_ARGUMENTS";
            case ERROR_WRONG_RETURN_TYPE:           return "ERROR_WRONG_RETURN_TYPE";
            case ERROR_DUPLICATED_STRUCTURE_MEMBER: return "ERROR_DUPLICATED_STRUCTURE_MEMBER";
        }
        
        return "unknown";
    }
        
    enum chunked_done_code {
        DONE_NOT, DONE_SUCCESS, DONE_FAIL, DONE_MEMORY_UNREADABLE, DONE_ALLOCATION_ERROR
    };
    
    typedef std::function< int (int message, void* message_data) > match_callback;
    
    // a function which converts the call back to a lambda call.
    int match_callback_function(int message, void* message_data, void* user_data) {
        match_callback cb = *(match_callback*)user_data;
    
        return cb(message, message_data);
    }
    
    void callback_function(int error_level, const char* file_name, int line_number, const char* message) {
        std::cout << "error: " << line_number << " " << message << std::endl;
    }
    
    ActionResponse search_memory_yara(LldbProcessSession& session, addr_t addr, size_t get_bytes, string pattern,
                                      int max_matches, boost::property_tree::ptree& output) {
        // first: set up rules (compile)
        YR_RULES* compiled_rules;
        YR_COMPILER* compiler;
        
        if(yr_compiler_create(&compiler) != ERROR_SUCCESS) {
            return ActionResponse::error("Unable to create Yara compiler");
        }
        
        yr_compiler_set_callback(compiler, callback_function);
        
        int comp_error = yr_compiler_add_string(compiler, pattern.c_str(), nullptr);
        if(comp_error > 0) {
            yr_compiler_destroy(compiler);
            
            return ActionResponse::error("Errors compiling the pattern. Errors: " + to_string(comp_error));
        }
        
        if(yr_compiler_get_rules(compiler, &compiled_rules) != ERROR_SUCCESS) {
            yr_compiler_destroy(compiler);
            
            return ActionResponse::error("Errors extracting compiled rules");
        }
        
        // initialize chunking setup.
        shared_ptr<queue<ptree>> out_q(new queue<ptree>());
        shared_ptr<atomic<chunked_done_code>> done_code(new atomic<chunked_done_code>(DONE_NOT));
        shared_ptr<atomic<int64_t>> last_offset(new atomic<int64_t>(0));
        
        // protect the queue
        shared_ptr<std::mutex> q_mutex(new std::mutex());
        
        shared_ptr<int> total_matches(new int(0));
        
        auto output_func = [out_q, done_code, q_mutex] ACTION_CALLBACK(out_req, out_output) {
            int max_records = 256;
            
            if(out_req.count("max_records") > 0) {
                if((std::stringstream(out_req.at("max_records")) >> max_records).fail()) {
                    return ActionResponse::error("max_records has to be an int");
                }
            }
            
            ptree out_items;
            int count = 0;
            
            while(!out_q->empty() && count < max_records) {
                // let's empty out this queue
                std::lock_guard<std::mutex> lock(*q_mutex);
                
                out_items.push_back(make_pair("", out_q->front()));
                
                out_q->pop();
                count++;
            }
            
            out_output.put_child("output", out_items);
            
            if(*done_code != DONE_NOT) {
                return ActionResponse::no_error_remove();
            }
            
            return ActionResponse::no_error();
        };
        
        // this function is called when a match is found.
        match_callback match_cb = [addr, out_q, q_mutex, last_offset, max_matches, total_matches](int message, void* message_data) {
            // one or the other depending on message
            YR_RULE* rule = (YR_RULE*)message_data;
            
            if(message == CALLBACK_MSG_RULE_MATCHING){
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
                        
                        *last_offset = match->offset;
                        
                        match_obj.put("base", to_string(addr));
                        match_obj.put("offset", to_string(match->offset));
                        match_obj.put("identifier", string(yr_string->identifier));
                        match_obj.put("string", os.str());
                        
                        { // protected output
                            std::lock_guard<std::mutex> lock(*q_mutex);
                            out_q->push(match_obj);
                        }
                    }
                }
                
                (*total_matches)++;
            }
            
            if(*total_matches >= max_matches) {
                return CALLBACK_ABORT;
            }
            
            return CALLBACK_CONTINUE;
        };
        
        auto run_func = [&session, addr, get_bytes, pattern, compiler, compiled_rules, out_q, done_code, q_mutex, match_cb, last_offset]() {
            size_t to_read = get_bytes;
            size_t total_scanned = 0;
            size_t read_chunk = 1024*1024*1024; // 10MB at a time.
            SBError error;
        
            size_t allocated = min(to_read, read_chunk);
            uint8_t* data = (uint8_t*)malloc(allocated);
            
            if(data == nullptr) {
                done_code->store(DONE_ALLOCATION_ERROR);

                return;
            }

            bool keep_reading = true;
            
            while(keep_reading && *done_code == DONE_NOT) {
                // first: read the memory
                size_t read_bytes;
                size_t read_cycle = min(allocated, to_read);
        
                if( (read_bytes = session.process.ReadMemory(addr + total_scanned, data, read_cycle, error)) <= 0) {
                    done_code->store(DONE_MEMORY_UNREADABLE);
                } else {
                    // If we read as much as we wanted, then read no more.
                    keep_reading = ( (to_read - read_bytes) > 0);
                    
                    // second: scan read memory
                    int scan_ret = yr_rules_scan_mem(compiled_rules, data, read_bytes, 0, match_callback_function, (void*)&match_cb, 0);
        
                    // where do we resume in the next window
                    size_t over_lap_bytes = std::min((size_t)1024, (size_t)(read_bytes - (*last_offset + 1)));
                    
                    to_read -= (read_bytes - over_lap_bytes);
                    total_scanned += (read_bytes - over_lap_bytes);
                    
                    { // protected output.
                        std::lock_guard<std::mutex> lock(*q_mutex);
            
                        ptree final_obj;
                        final_obj.put("code", to_string(scan_ret));
                        final_obj.put("code_str", yr_error_retstr(scan_ret));
                        out_q->push(final_obj);
                    }
                }
                
            }
            
            // finally: do the cleanup.
            free(data);
            yr_rules_destroy(compiled_rules);
            yr_compiler_destroy(compiler);
            
            if(*done_code == DONE_NOT) {
                done_code->store(DONE_SUCCESS);
            }
        };
        
        return ActionResponse::chunked_response(output_func, run_func);
    }
    
    // implementation of /write/byte
    ActionResponse write_byte(LldbProcessSession& session, addr_t addr, long writebyte) {
        SBError error;
        
        unsigned char byte = (unsigned char) writebyte;
        
        if(session.process.WriteMemory(addr, &byte, 1, error) != 1) {
            return ActionResponse::error(string(error.GetCString()));
        }
        
        return ActionResponse::no_error();
    }
    
    // implementation of /read/memory
    ActionResponse read_memory(LldbProcessSession& session, addr_t addr, size_t get_bytes, boost::property_tree::ptree& output) {
        SBError error;
        
        size_t read_bytes;
        array<unsigned char, 4096> mem_arr;
        
        read_bytes = session.process.ReadMemory(addr, mem_arr.data(), min(mem_arr.size(), get_bytes), error);
        
        if( read_bytes != 0) {
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
            if(error.IsValid() && error.GetCString()) {
                return ActionResponse::error(string(error.GetCString()));
            } else {
                return ActionResponse::error("Memory unreadable: " + to_string(addr));
            }
        }
    }
        
    } // namespace memops
    
void register_memops(RequestRouter& req_router, LldbSessionMap& sessions) {
    req_router.register_path({"read", "memory"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::exists({"address"}),
        RequestConstraint::has_int("count")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        addr_t addr = stoull(req.at("address"), 0, 16);
        size_t get_bytes = stol(req.at("count"));
        
        return memops::read_memory(session, addr, get_bytes, output);
    });
    
    req_router.register_path({"search", "memory", "yara"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::exists({"address"}),
        RequestConstraint::has_int("length"),
        RequestConstraint::exists({"pattern"}),
        RequestConstraint::has_int({"max_matches"})
    }, [&sessions] ACTION_CALLBACK(req, output) {
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        addr_t addr = stoull(req.at("address"), 0, 16);
        size_t get_bytes = stol(req.at("length"));
        string pattern = req.at("pattern");
        int max_matches = stoi(req.at("max_matches"));
        
        return memops::search_memory_yara(session, addr, get_bytes, pattern, max_matches, output);
    }, CHUNKED_NONBLOCKING);
    
    req_router.register_path({"write", "byte"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::exists({"address"}),
        RequestConstraint::has_int("byte", 0, 0xFF)
    }, [&sessions] ACTION_CALLBACK(req, output) {
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        addr_t addr = stoull(req.at("address"), 0, 16);
        long writebyte = stol(req.at("byte"));
        
        return memops::write_byte(session, addr, writebyte);
    });
}

}