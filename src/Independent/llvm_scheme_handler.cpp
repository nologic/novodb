//
//  llvm_scheme_handler.cpp
//  Novodb
//
//  Created by mike on 9/13/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "llvm_scheme_handler.h"
#include "json_serialize.h"
#include "platform_support.h"

#include <lldb/lldb-private.h>
#include <lldb/Target/Process.h>

#include <thread>
#include <array>
#include <iomanip>

#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>

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
        
        auto session_bounds = [this](int sid) { return sid >= 0 && sid < this->sessions.size(); };
        
        req_router.register_path({"version"}, {}, [] ACTION_CALLBACK(req, output) {
            using namespace std;
            
            output.put("version", string(lldb::SBDebugger::GetVersionString()));
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"create", "target", "exe"}, {
            RequestConstraint::exists({"path"})
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace lldb;
            using namespace std;
            
            stringstream ss;
            string exe_path(req.at("path"));
            
            SBDebugger debugger = SBDebugger::Create();
            
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
        }, true);
        
        req_router.register_path({"create", "target", "attach"}, {
            RequestConstraint::has_int("pid")
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace lldb;
            using namespace std;
            
            stringstream ss;
            lldb::pid_t pid = stoi(req.at("pid"));

            SBDebugger debugger = SBDebugger::Create();
            
            debugger.SetAsync(false);
            
            if(!debugger.IsValid()) {
                return ActionResponse::error("Debugger not valid");
            }
            
            SBTarget target = debugger.CreateTarget(nullptr);
            
            if(!target.IsValid()) {
                return ActionResponse::error("Target invalid");
            }
            
            SBAttachInfo attach_info(pid);
            SBError error;
            
            LldbProcessSession session(target.Attach(attach_info, error));
            
            if(error.Success()) {
                this->sessions.push_back(session);
            
                output.put("session", to_string(this->sessions.size() - 1));
            
                return ActionResponse::no_error();
            } else {
                return ActionResponse::error("Failed to attach");
            }
        }, true);
        
        req_router.register_path({"create", "target", "remote"}, {}, [this] ACTION_CALLBACK(req, output) {
            return ActionResponse::error(501, "Not Implemented");
        });
        
        req_router.register_path({"breakpoint", "set"}, {
            RequestConstraint::has_int("session", session_bounds),
            RequestConstraint::exists({"symbol"})
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            
            string bp_symbol(req.at("symbol"));
            
            int session_id = stoi(req.at("session"));
            
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

        req_router.register_path({"launch"}, {
            RequestConstraint::has_int("session", session_bounds)
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            
            int session_id = stoi(req.at("session"));
            
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
        
        req_router.register_path({"list", "symbols"}, {
            RequestConstraint::has_int("session", session_bounds),
            RequestConstraint::has_int("module")
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            int module_index = stoi(req.at("module"));
            
            LldbProcessSession& session = this->sessions[session_id];
            
            SBModule module = session.target.GetModuleAtIndex(module_index);
            auto symbols = module.GetNumSymbols();
            ptree sym_list;
            
            for(decltype(symbols) s = 0; s < symbols; s++) {
                SBSymbol symbol = module.GetSymbolAtIndex(s);
                ptree sym_out;
                
                sym_out.put("index", to_string(s));
                
                to_json(symbol, sym_out, &session.target);
                
                sym_list.push_back(make_pair("", sym_out));
            }
            
            output.put_child("symbols", sym_list);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"list", "modules"}, {
            RequestConstraint::has_int("session", session_bounds)
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            
            LldbProcessSession& session = this->sessions[session_id];
            
            auto mod_count = session.target.GetNumModules();
            
            ptree mods_list;
            
            for(decltype(mod_count) i = 0; i < mod_count; i++) {
                ptree pt, sections;
                
                SBModule mod = session.target.GetModuleAtIndex(i);
                
                vector< pair<string, const char*> > vals = {
                    make_pair(string("filename"), mod.GetFileSpec().GetFilename()),
                    make_pair(string("filepath"), mod.GetFileSpec().GetDirectory()),
                };
                
                for(auto p : vals) {
                    if(p.second != nullptr) {
                        pt.put(p.first, string(p.second));
                    }
                }
                
                auto num_sec = mod.GetNumSections();
                for(decltype(num_sec) s = 0; s < num_sec; s++) {
                    SBSection section = mod.GetSectionAtIndex(s);
                    
                    ptree pt_section;
                    pt_section.put("index", to_string(s));
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
        
        req_router.register_path({"list", "registers"}, {
            RequestConstraint::has_int("session", session_bounds),
            RequestConstraint::has_int("thread"),
            RequestConstraint::has_int("frame")
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            int thread_ind = stoi(req.at("thread"));
            int frame_ind  = stoi(req.at("frame"));
            
            LldbProcessSession& session = this->sessions[session_id];
            
            SBFrame frame = session.process.GetThreadAtIndex(thread_ind).GetFrameAtIndex(frame_ind);
            
            SBValueList reg_vals = frame.GetRegisters();
            ptree regvals;
            
            for(int i = 0; i < reg_vals.GetSize(); i++) {
                SBValue reg_val = reg_vals.GetValueAtIndex(i);
                ptree rval;
                
                vector< pair<string, const char*> > vals = {
                    make_pair(string("summary"), reg_val.GetSummary()),
                    make_pair(string("value"), reg_val.GetValue()),
                    make_pair(string("name"), reg_val.GetName())
                };
            
                for(auto p : vals) {
                    if(p.second != nullptr) {
                        rval.put(p.first, string(p.second));
                        cout << p.first << " " << string(p.second) << endl;
                    }
                }
                
                ptree child_values;
                uint32_t num_ch;
                if(reg_val.MightHaveChildren() && (num_ch = reg_val.GetNumChildren()) > 0) {
                    for(decltype(num_ch) c = 0; c < num_ch; c++) {
                        ptree child_pt;
                        SBValue regc_val(reg_val.GetChildAtIndex(c));
                        
                        vector< pair<string, const char*> > vals = {
                            make_pair(string("summary"), regc_val.GetSummary()),
                            make_pair(string("value"), regc_val.GetValue()),
                            make_pair(string("name"), regc_val.GetName())
                        };
                        
                        for(auto p : vals) {
                            if(p.second != nullptr) {
                                child_pt.put(p.first, string(p.second));
                            }
                        }
                        
                        child_values.push_back(make_pair("", child_pt));
                    }
                    
                    rval.put_child("values", child_values);
                }
                
                regvals.push_back(make_pair("", rval));
            }
            
            output.put_child("registers", regvals);
            
            return ActionResponse::no_error();
        });

        req_router.register_path({"proc", "state"}, {
            RequestConstraint::has_int("session", session_bounds)
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            
            int session_id = stoi(req.at("session"));
            
            LldbProcessSession& session = this->sessions[session_id];
            StateType state = session.process.GetState();
            
            output.put("state", to_string(state));
            output.put("description", state_type_to_str(state));
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"list", "threads"}, {
            RequestConstraint::has_int("session", session_bounds),
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            
            LldbProcessSession& session = this->sessions[session_id];
            
            auto num_threads = session.process.GetNumThreads();
            auto cur_thread = session.process.GetSelectedThread();
            
            ptree out_threads;
            
            for(decltype(num_threads) i = 0; i < num_threads; i++) {
                ptree out_th;
                SBThread thread = session.process.GetThreadAtIndex(i);
                SBStream status;
                
                const char* name = thread.GetName();
                
                out_th.put("tid", to_string(thread.GetThreadID()));
                out_th.put("index", std::to_string(i));
                
                if(name != NULL) {
                    out_th.put("name", string(thread.GetName()));
                }
                
                if(thread.GetStatus(status)) {
                    out_th.put("status", string(status.GetData()));
                }
                
                if(thread.GetThreadID() == cur_thread.GetThreadID()) {
                    out_th.put("current", "true");
                }
                
                out_threads.push_back(make_pair("", out_th));
            }
            
            output.put_child("threads", out_threads);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"list", "frames"}, {
            RequestConstraint::has_int("session", session_bounds),
            RequestConstraint::has_int("thread")
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            int thread_ind = stoi(req.at("thread"));
            
            LldbProcessSession& session = this->sessions[session_id];
            SBThread thread = session.process.GetThreadAtIndex(thread_ind);
            
            ptree frames_pt;
            auto frames = thread.GetNumFrames();
            for(decltype(frames) f = 0; f < frames; f++) {
                SBFrame frame = thread.GetFrameAtIndex(f);
                ptree frame_pt;
                
                const char* disasm = frame.Disassemble();
                
                if(disasm != nullptr) {
                    frame_pt.put("disasm", string(disasm));
                }
                
                frames_pt.push_back(make_pair("", frame_pt));
            }
            
            output.put_child("frames", frames_pt);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"read", "memory"}, {
            RequestConstraint::has_int("session", session_bounds),
            RequestConstraint::exists({"address"}),
            RequestConstraint::has_int("count")
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            addr_t addr = stoull(req.at("address"), 0, 16);
            size_t get_bytes = stol(req.at("count"));
            
            LldbProcessSession& session = this->sessions[session_id];
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
        
        req_router.register_path({"event", "listen"}, {
            RequestConstraint::has_int("session", session_bounds)
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            
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

        req_router.register_path({"read", "instructions"}, {
            RequestConstraint::has_int("session", session_bounds),
            RequestConstraint::exists({"address"}),
            RequestConstraint::has_int("count")
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            addr_t addr = stoull(req.at("address"), 0, 16);
            size_t count = stol(req.at("count"));
            
            LldbProcessSession& session = this->sessions[session_id];
            array<unsigned char, 4096> mem_arr;
            
            SBInstructionList insts = session.target.GetInstructions(addr, mem_arr.data(), min(mem_arr.size(), count));
            auto inst_count = insts.GetSize();
            
            ptree insts_out;
            
            for(decltype(inst_count) i = 0; i < inst_count; i++) {
                SBInstruction inst(insts.GetInstructionAtIndex((uint)i));
                
                ptree inst_out;
                SBStream inst_desc;
                
                if(inst.GetDescription(inst_desc)) {
                    inst_out.put("description", string(inst_desc.GetData(), inst_desc.GetSize()));
                }
                
                inst_out.put("mnem", string(inst.GetMnemonic(session.target)));
                inst_out.put("operands", string(inst.GetOperands(session.target)));
                inst_out.put("size", to_string(inst.GetByteSize()));
                inst_out.put("branch", to_string(inst.DoesBranch()));
                inst_out.put("valid", to_string(inst.IsValid()));
                inst_out.put("comment", string(inst.GetComment(session.target)));
                
                insts_out.push_back(make_pair("", inst_out));
            }
            
            output.add_child("instructions", insts_out);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"cmd", "step"}, {
            RequestConstraint::has_int("session", session_bounds),
            RequestConstraint::has_int("thread")
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            int thread_ind = stoi(req.at("thread"));
            
            LldbProcessSession& session = this->sessions[session_id];
            SBThread thread = session.process.GetThreadAtIndex(thread_ind);
            
            thread.StepInstruction(false);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"cmd", "resume"}, {
            RequestConstraint::has_int("session", session_bounds),
            RequestConstraint::has_int("thread")
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            int session_id = stoi(req.at("session"));
            int thread_ind = stoi(req.at("thread"));
            
            LldbProcessSession& session = this->sessions[session_id];
            SBThread thread = session.process.GetThreadAtIndex(thread_ind);
            
            thread.Resume();
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"log", "start"}, {
            RequestConstraint::has_int("session", session_bounds)
        }, [this] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace lldb;
            using namespace boost::property_tree;
            
            LldbProcessSession& session = this->sessions[stoi(req.at("session"))];
            
            SBDebugger debugger = session.target.GetDebugger();
            
            char* val[3] = { "api", "all", NULL };
            debugger.EnableLog("lldb", (const char**)val);
            debugger.SetLoggingCallback(log_cb, NULL);
            debugger.HandleCommand("log enable -f ./log.txt lldb api");
            
            return ActionResponse::no_error();
        });
    }

}