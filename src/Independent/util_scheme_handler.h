//
//  util_scheme_hander.h
//  Novodb
//
//  Created by mike on 10/7/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef __Novodb__util_scheme_hander__
#define __Novodb__util_scheme_hander__

#include <stdio.h>
#include <iostream>
#include <sstream>

#include "include/cef_base.h"
#include "include/cef_browser.h"
#include "include/cef_url.h"
#include "include/cef_resource_handler.h"

#include "dbg_handler.h"

namespace novo {
    
    class UtilSchemeHandler : public CefSchemeHandlerFactory {
    public:
        UtilSchemeHandler();
        
        virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                                     CefRefPtr<CefFrame> frame,
                                                     const CefString& scheme_name,
                                                     CefRefPtr<CefRequest> request){
            
            CefRefPtr<CefResourceHandler> handler(new DbgResourceHandler(req_router));
            
            return handler;
        }
        
    private:
        RequestRouter req_router;

        IMPLEMENT_REFCOUNTING(LlbdSchemeHandler)
    };
    
    static void install_util_scheme() {
        CefRefPtr<CefSchemeHandlerFactory> handler(new UtilSchemeHandler());
        
        CefRegisterSchemeHandlerFactory("util", CefString(), handler);
    }
    
} // namespace novo

#endif /* defined(__Novodb__util_scheme_hander__) */
