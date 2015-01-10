//
//  lldb_module_commands1.cpp
//  Novodb
//
//  Created by mike on 12/6/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "lldb_module_commands.h"
#include "enum_to_string.h"

#include <boost/regex.hpp>

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
    
    
    
void register_commands(RequestRouter& req_router, LldbSessionMap& sessions) {
    BOOST_LOG_TRIVIAL(trace) << "Llbd Plugin initializing";
    
    req_router.register_path({"version"}, {}, [] ACTION_CALLBACK(req, output) {
        using namespace std;
        
        output.put("version", string(lldb::SBDebugger::GetVersionString()));
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"create", "target", "exe"}, {
        RequestConstraint::exists({"path"})
    }, [&sessions] ACTION_CALLBACK(req, output) {
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
        
        LldbProcessSession new_session(target);
        string new_id = sessions.add_session(new_session);
        
        output.put("session", new_id);
        
        return ActionResponse::no_error();
    }, PLAIN_NONBLOCK);
    
    req_router.register_path({"create", "target", "attach"}, {
        RequestConstraint::has_int("pid")
    }, [&sessions] ACTION_CALLBACK(req, output) {
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
            string new_id = sessions.add_session(session);
            
            output.put("session", new_id);
            output.put("current_tid", std::to_string(session.process.GetSelectedThread().GetThreadID()));
            output.put("triple", session.target.GetTriple());
            
            return ActionResponse::no_error();
        } else {
            return ActionResponse::error("Failed to attach");
        }
    }, PLAIN_NONBLOCK);
    
    req_router.register_path({"create", "target", "remote"}, {}, [] ACTION_CALLBACK(req, output) {
        return ActionResponse::error(501, "Not Implemented");
    });
    
    req_router.register_path({"add", "breakpoint"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::exists_or({"symbol", "address"})
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBTarget& target = session.target;
        
        SBBreakpoint bp;
        
        if(req.contains_key("address")) {
            addr_t bp_addr = std::stoll(req.at("address"));

            bp = target.BreakpointCreateByAddress(bp_addr);
        } else {
            string bp_symbol(req.at("symbol"));

            bp = target.BreakpointCreateByName(bp_symbol.c_str());
        }
        
        if(bp.IsValid()) {
            to_json(bp, output);
            
            return ActionResponse::no_error();
        } else {
            return ActionResponse::error(string("Breakpoint invalid"));
        }
    });
    
    req_router.register_path({"list", "breakpoint"}, {
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBTarget& target = session.target;
        ptree bp_list;
        
        for(uint32_t b = 0; b < target.GetNumBreakpoints(); b++) {
            SBBreakpoint bp = target.GetBreakpointAtIndex(b);
            ptree bp_out;
            
            to_json(bp, bp_out);
            
            bp_list.push_back(make_pair("", bp_out));
        }
        
        output.put_child("breakpoints", bp_list);
        
        return ActionResponse::no_error();
    });

    req_router.register_path({"launch"}, {
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
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
    }, PLAIN_NONBLOCK);
    
    req_router.register_path({"list", "symbols"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::has_int("module"),
        RequestConstraint::has_int("index"),
        RequestConstraint::has_int("count")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        int module_index = stoi(req.at("module"));
        size_t index_off = stol(req.at("index"));
        size_t count = stol(req.at("count"));
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBModule module = session.target.GetModuleAtIndex(module_index);
        size_t symbols = min( (index_off + count), module.GetNumSymbols() );
        ptree sym_list;
        
        for(size_t s = index_off; s < symbols; s++) {
            SBSymbol symbol = module.GetSymbolAtIndex(s);
            ptree sym_out;
            
            to_json(symbol, sym_out, &session.target);
            
            sym_list.push_back(make_pair("", sym_out));
        }
        
        output.put_child("symbols", sym_list);
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"search", "symbols"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::has_int("module"),
        RequestConstraint::has_int("index"),
        RequestConstraint::has_int("count"),
        RequestConstraint::has_int("max_matches"),
        RequestConstraint::exists({"regex"})
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        int module_index = stoi(req.at("module"));
        size_t index_off = stol(req.at("index"));
        size_t count = stol(req.at("count"));
        size_t max_matches = stol(req.at("max_matches"));
        
        boost::regex expression;
        try {
            expression.assign(req.at("regex"));
        } catch(boost::regex_error& err) {
            return ActionResponse::error(err.what());
        }
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBModule module = session.target.GetModuleAtIndex(module_index);
        size_t symbols = min( (index_off + count), module.GetNumSymbols() );
        ptree sym_list;
        size_t matches = 0;
        size_t last_index = 0;
        
        for(size_t s = index_off; s < symbols && matches < max_matches; s++) {
            using namespace boost;
            
            SBSymbol symbol = module.GetSymbolAtIndex(s);
            ptree sym_out;
            last_index = s;
            
            cmatch what;
            
            if(regex_match(symbol.GetName(), what, expression) ||
                 (symbol.GetMangledName() != NULL && regex_match(symbol.GetMangledName(), what, expression)) ) {

                to_json(symbol, sym_out, &session.target);
                sym_list.push_back(make_pair("", sym_out));
                
                matches++;
            }
        }
        
        output.put("last_index", last_index);
        output.put_child("symbols", sym_list);
        
        return ActionResponse::no_error();
    }, PLAIN_NONBLOCK);
    
    req_router.register_path({"list", "modules"}, {
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
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
            
            pt.put("index", std::to_string(i));
            pt.put("symbols", std::to_string(mod.GetNumSymbols()));
            
            auto num_sec = mod.GetNumSections();
            for(decltype(num_sec) s = 0; s < num_sec; s++) {
                SBSection section = mod.GetSectionAtIndex(s);
                
                ptree pt_section;
                pt_section.put("index", to_string(s));
                pt_section.put("name", string(section.GetName()));
                pt_section.put("address", to_string(section.GetLoadAddress(session.target)));
                pt_section.put("size", std::to_string(section.GetByteSize()));
                pt_section.put("type", section_type_to_string(section.GetSectionType()));
                
                sections.push_back(make_pair("", pt_section));
            }
            
            pt.put_child("sections", sections);
            
            mods_list.push_back(make_pair("", pt));
        }
        
        output.put_child("modules", mods_list);
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"list", "registers"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::has_int("thread"),
        RequestConstraint::has_int("frame")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        int thread_ind = stoi(req.at("thread"));
        int frame_ind  = stoi(req.at("frame"));
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
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
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        if(session.is_running) {
            StateType state = lldb::eStateRunning;
            
            output.put("state", to_string(state));
            output.put("description", state_type_to_string(state));
        } else {
            StateType state = session.process.GetState();
        
            output.put("state", to_string(state));
            output.put("description", state_type_to_string(state));
            output.put("thread", to_string(session.process.GetSelectedThread().GetThreadID()));
        }
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"list", "threads"}, {
        RequestConstraint::has_int("session"),
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        auto num_threads = session.process.GetNumThreads();
        auto cur_thread = session.process.GetSelectedThread();
        
        ptree out_threads;
        
        for(decltype(num_threads) i = 0; i < num_threads; i++) {
            ptree out_th;
            SBThread thread = session.process.GetThreadAtIndex(i);
            SBStream status;
            
            const char* name = thread.GetName();
            
            out_th.put("tid", std::to_string(thread.GetThreadID()));
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
        RequestConstraint::has_int("session"),
        RequestConstraint::has_int("thread")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        int thread_ind = stoi(req.at("thread"));
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
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
    
    req_router.register_path({"event", "listen"}, {
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
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
    }, PLAIN_NONBLOCK);
    
    req_router.register_path({"read", "instructions"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::exists({"address"}),
        RequestConstraint::has_int("count")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        addr_t addr = stoull(req.at("address"), 0, 16);
        size_t count = stol(req.at("count"));
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBInstructionList insts = session.target.ReadInstructions(session.target.ResolveLoadAddress(addr), (uint32_t)count);
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
        RequestConstraint::has_int("session"),
        RequestConstraint::has_int("tid")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        tid_t tid = stoi(req.at("tid"));
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBThread thread = session.process.GetThreadByID(tid);
        
        if(thread.IsValid()) {
            thread.StepInstruction(false);
        } else {
            return ActionResponse::error("Invalid thread selected");
        }
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"cmd", "continue"}, {
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        session.is_running = true;
        SBError error = session.process.Continue();
        session.is_running = false;
        
        if(error.Fail()) {
            ActionResponse::error(error.GetCString());
        }
        
        return ActionResponse::no_error();
    }, PLAIN_NONBLOCK);
    
    req_router.register_path({"cmd", "stop"}, {
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        if(session.is_running) {
            pause_process((int)session.process.GetProcessID());
        } else {
            SBError error = session.process.Stop();
        
            if(error.Fail()) {
                ActionResponse::error(error.GetCString());
            }
        }
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"cmd", "detach"}, {
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBError error = session.process.Detach();
        
        if(error.Fail()) {
            ActionResponse::error(error.GetCString());
        } else {
            sessions.erase(session_id);
        }
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"log", "start"}, {
        RequestConstraint::has_int("session")
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBDebugger debugger = session.target.GetDebugger();
        
        char* val[3] = { "api", "all", NULL };
        debugger.EnableLog("lldb", (const char**)val);
        debugger.SetLoggingCallback(log_cb, NULL);
        debugger.HandleCommand("log enable -f ./log.txt lldb api");
        
        return ActionResponse::no_error();
    });
    
    req_router.register_path({"cmd", "lldb"}, {
        RequestConstraint::has_int("session"),
        RequestConstraint::exists({"cmd"})
    }, [&sessions] ACTION_CALLBACK(req, output) {
        using namespace std;
        using namespace lldb;
        using namespace boost::property_tree;
        
        string session_id = req.at("session");
        LldbProcessSession& session = sessions.get_session(session_id);
        
        SBDebugger debugger = session.target.GetDebugger();
        
        lldb::SBCommandReturnObject result;
        
        ReturnStatus ret = debugger.GetCommandInterpreter().HandleCommand(req.at("cmd").c_str(), result);
        
        output.put("ret", std::to_string(ret));
        output.put("retstr", return_status_to_string(ret));
        output.put("result", string(result.GetOutput()));
        
        return ActionResponse::no_error();
    }, PLAIN_NONBLOCK);

}

}