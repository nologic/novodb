//
//  Module.h
//  Novodb
//
//  Created by mike on 12/6/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef Module_h
#define Module_h

#include <string>
#include <vector>

#include "include/cef_base.h"

#include "request_router.h"

#define ACTION_CALLBACK(req, output) (const ActionRequest& req, boost::property_tree::ptree& output)

namespace novo {
    class RouterInterface {
        
    };
    
    class SessionState {
    private:
        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(SessionState);
    };
    
    class ModuleInterface {
    public:
        virtual std::string             getName       () = 0;
        virtual void                    registerRouter(RequestRouter& router) = 0;
        virtual CefRefPtr<SessionState> createSession() = 0;
    };
    
    // global module functions
    void register_module(ModuleInterface& module);
}

namespace novo {
    // list of module init function, to be made dynamic
    void util_module_main();
    void lldb_module_main();
}

#endif