//
//  llvm_scheme_handler.cpp
//  Novodb
//
//  Created by mike on 9/13/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "llvm_scheme_handler.h"

#include <lldb/lldb-private.h>
#include <lldb/Target/Process.h>

#include <thread>
#include <array>
#include <iomanip>

#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace novo {
    void log_cb (const char *s, void *baton) {
        std::cout << s;
    }
    
    std::string to_string(lldb::addr_t address) {
        using namespace std;
        
        stringstream ss;
        ss << hex << address;
        
        return ss.str();
    }
    
    std::string state_type_to_str(lldb::StateType state) {
        switch(state) {
            case lldb::eStateInvalid:   return std::string("invalid");
            case lldb::eStateUnloaded:  return std::string("unloaded");
            case lldb::eStateConnected: return std::string("connected");
            case lldb::eStateAttaching: return std::string("attaching");
            case lldb::eStateLaunching: return std::string("launching");
            case lldb::eStateStopped:   return std::string("stopped");
            case lldb::eStateRunning:   return std::string("running");
            case lldb::eStateStepping:  return std::string("stepping");
            case lldb::eStateCrashed:   return std::string("crashed");
            case lldb::eStateDetached:  return std::string("detached");
            case lldb::eStateExited:    return std::string("exited");
            case lldb::eStateSuspended: return std::string("suspended");
            default: return std::string("Unknown");
        }
    }
    
    LlbdSchemeHandler::LlbdSchemeHandler() {
        BOOST_LOG_TRIVIAL(trace) << "LlbdSchemeHandler initializing";
        
        lldb::SBDebugger::Initialize();
        
        req_router.register_path({"version"}, [] ACTION_CALLBACK(req, output) {
            using namespace std;
            
            output.put("version", string(lldb::SBDebugger::GetVersionString()));
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"create", "target", "exe"}, [this] ACTION_CALLBACK(req, output) {
            using namespace lldb;
            using namespace std;
            
            stringstream ss;
            string exe_path(req.at("path"));
            
            SBDebugger debugger = SBDebugger::Create();
            
            char* val[3] = { "api", "all", NULL };
            debugger.EnableLog("lldb", (const char**)val);
            debugger.SetLoggingCallback(log_cb, NULL);
            debugger.HandleCommand("log enable -f /Users/mike/GitHub/Novodb/Build/Novodb/Build/Products/Debug/log.txt lldb api");
            
            debugger.SetAsync(false);
            
            if(!debugger.IsValid()) {
                return ActionResponse::error("Debugger not valid");
            }
            
            SBTarget target = debugger.CreateTargetWithFileAndArch(exe_path.c_str(), LLDB_ARCH_DEFAULT);
            
            if(!target.IsValid()) {
                return ActionResponse::error("Target invalid");
            }
            
            this->sessions.push_back(LldbProcessSession(target));
            
            output.put("session", to_string(this->sessions.size() - 1));
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"create", "target", "attach"}, [this] ACTION_CALLBACK(req, output) {
            return ActionResponse::error(501, "Not Implemented");
        });
        
        req_router.register_path({"create", "target", "remote"}, [this] ACTION_CALLBACK(req, output) {
            return ActionResponse::error(501, "Not Implemented");
        });
        
        req_router.register_path({"breakpoint", "set"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            
            string bp_symbol(req.at("symbol"));
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id >= this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            SBTarget& target = session.target;
            
            SBBreakpoint bp = target.BreakpointCreateByName(bp_symbol.c_str());
            
            if(bp.IsValid()) {
                output.put("msg", "Breakpoint set");
                
                return ActionResponse::no_error();
            } else {
                return ActionResponse::error(string("Breakpoint invalid"));
            }
        });

        req_router.register_path({"launch"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id > this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            
            SBLaunchInfo launch_info(NULL);
            SBError error;
            
            session.process = session.target.Launch(launch_info, error);
            
            if(session.process.IsValid()) {
                stringstream ss;
                
                output.put("proc_id", session.process.GetProcessID());
                
                return ActionResponse::no_error();
            } else {
                return ActionResponse::error(string("Process Invalid"));
            }
        }, true);
        
        req_router.register_path({"read", "symbols"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id > this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            
            SBSymbolContextList scontext = session.target.FindSymbols("m*");
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"list", "modules"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id > this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            
            auto mod_count = session.target.GetNumModules();
            
            ptree mods_list;
            
            for(decltype(mod_count) i = 0; i < mod_count; i++) {
                ptree pt, sections;
                
                SBModule mod = session.target.GetModuleAtIndex(i);
                
                pt.put("filename", string(mod.GetFileSpec().GetFilename()));
                pt.put("filepath", string(mod.GetFileSpec().GetDirectory()));
                
                auto num_sec = mod.GetNumSections();
                for(decltype(num_sec) s = 0; s < num_sec; s++) {
                    SBSection section = mod.GetSectionAtIndex(s);
                    
                    ptree pt_section;
                    pt_section.put("name", string(section.GetName()));
                    pt_section.put("address", to_string(section.GetLoadAddress(session.target)));
                    
                    sections.push_back(make_pair("", pt_section));
                }
                
                pt.put_child("sections", sections);
                
                mods_list.push_back(make_pair("", pt));
            }
            
            output.put_child("modules", mods_list);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"list", "registers"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id > this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            
            SBFrame frame = session.process.GetThreadAtIndex(0).GetFrameAtIndex(0);
            
            SBValueList reg_vals = frame.GetRegisters();
            ptree regvals;
            
            for(int i = 0; i < reg_vals.GetSize(); i++) {
                SBValue reg_val = reg_vals.GetValueAtIndex(i);
                ptree rval;
                
                rval.put("summary", string(reg_val.GetSummary()));
                rval.put("value", string(reg_val.GetValue()));
                
                regvals.push_back(make_pair("", rval));
            }
            
            output.put_child("registers", regvals);
            
            return ActionResponse::no_error();
        });

        req_router.register_path({"proc", "state"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id >= this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            StateType state = session.process.GetState();
            
            output.put("state", to_string(state));
            output.put("description", state_type_to_str(state));
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"list", "threads"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id >= this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            
            auto num_threads = session.process.GetNumThreads();
            
            ptree out_th;
            
            for(decltype(num_threads) i = 0; i < num_threads; i++) {
                SBThread thread = session.process.GetThreadAtIndex(i);
                SBStream status;
                
                const char* name = thread.GetName();
                
                out_th.put("tid", to_string(thread.GetThreadID()));
                
                if(name != NULL) {
                    out_th.put("name", string(thread.GetName()));
                }
                
                if(thread.GetStatus(status)) {
                    out_th.put("status", string(status.GetData()));
                }
            }
            
            output.put_child("threads", out_th);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"read", "memory"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            addr_t addr = stoull(req.at("address"), 0, 16);
            size_t get_bytes = stol(req.at("count"));
            string sep = req.at("sep");
            
            if(session_id < 0 || session_id >= this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            SBError error;
            
            size_t read_bytes;
            array<unsigned char, 4096> mem_arr;
            
            if( (read_bytes = session.process.ReadMemory(addr, mem_arr.data(), min(mem_arr.size(), get_bytes), error)) != 0) {
                stringstream os;

                os << hex << setfill('0');  // set the stream to hex with 0 fill
                
                std::for_each(std::begin(mem_arr), std::end(mem_arr), [&os, &sep] (int i) {
                    os << setw(2) << i << sep;
                });
                
                output.put("address", to_string(addr));
                output.put("block", os.str());
                output.put("count", to_string(read_bytes));
                
                return ActionResponse::no_error();
            } else {
                return ActionResponse::error(string(error.GetCString()));
            }
        });
        
        req_router.register_path({"event", "listen"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id >= this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            
            SBListener listen = session.target.GetDebugger().GetListener();
            SBEvent event;
            
            if(listen.WaitForEvent(60, event)) {
                SBStream desc;
                
                event.GetDescription(desc);
                
                output.put("valid", to_string(event.IsValid()));
                output.put("type", to_string(event.GetType()));
                output.put("flavor", string(event.GetDataFlavor()));
                output.put("broadcaster", string(event.GetBroadcasterClass()));
                output.put("description", string(desc.GetData(), desc.GetSize()));
                
                return ActionResponse::no_error();
            } else {
                return ActionResponse::error(string("Unable to wait for event"));
            }
        }, true);
        
        req_router.register_path({"event", "listen", "proc"}, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            
            if(session_id < 0 || session_id >= this->sessions.size()) {
                return ActionResponse::error(string("session out of range"));
            }
            
            LldbProcessSession& session = this->sessions[session_id];
            SBListener listener("Process Listener");
            
            session.process.GetBroadcaster().AddListener(listener, lldb::SBProcess::eBroadcastBitStateChanged);
            SBEvent event;
            
            /*if(listen.WaitForEvent(60, event)) {
                SBStream desc;
                
                event.GetDescription(desc);
                
                output.put("valid", to_string(event.IsValid()));
                output.put("type", to_string(event.GetType()));
                output.put("flavor", string(event.GetDataFlavor()));
                output.put("broadcaster", string(event.GetBroadcasterClass()));
                output.put("description", string(desc.GetData(), desc.GetSize()));
                
                return ActionResponse::no_error();
            } else {
                return ActionResponse::error(string("Unable to wait for event"));
            }*/
            
            return ActionResponse::no_error();
        }, true);

    }

}