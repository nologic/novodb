//
//  Module.cpp
//  Novodb
//
//  Created by mike on 12/6/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "Module.h"

#include "include/cef_base.h"
#include "include/cef_browser.h"
#include "include/cef_url.h"
#include "include/cef_resource_handler.h"

#include "dbg_handler.h"

namespace novo {
    class ModuleSchemeHandler : public CefSchemeHandlerFactory {
    public:
        ModuleSchemeHandler(ModuleInterface& module) {
            module.registerRouter(this->req_router);
        }
        
        virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                                     CefRefPtr<CefFrame> frame,
                                                     const CefString& scheme_name,
                                                     CefRefPtr<CefRequest> request){
            
            CefRefPtr<CefResourceHandler> handler(new DbgResourceHandler(req_router));
            
            return handler;
        }
        
    private:
        RequestRouter req_router;
        
        IMPLEMENT_REFCOUNTING(ModuleSchemeHandler)
    };

    void register_module(ModuleInterface& module) {
        CefRefPtr<CefSchemeHandlerFactory> handler(new ModuleSchemeHandler(module));
        
        CefRegisterSchemeHandlerFactory(module.getName(), CefString(), handler);
    }
}