//
//  llvm_scheme_handler.cpp
//  Novodb
//
//  Created by mike on 9/13/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include <map>

#include "lldb_module_commands.h"

#include "yara.h"

namespace novo {
    
    class lldb_module : public ModuleInterface {
        virtual std::string getName() {
            return "dbg-lldb";
        }
        
        virtual void registerRouter(RequestRouter& req_router) {
            register_commands(req_router, sessions);
            register_memops(req_router, sessions);
        }
        
        virtual CefRefPtr<SessionState> createSession() {
            return CefRefPtr<SessionState>(new SessionState());
        }
        
    private:
        LldbSessionMap sessions;
    };
    
    lldb_module _lldb_module;
    
    void lldb_module_main() {
        lldb::SBDebugger::Initialize();
        yr_initialize();
        
        register_module(_lldb_module);
    }
    
    void lldb_module_unload() {
        yr_finalize();
    }
}