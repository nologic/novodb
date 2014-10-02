//
//  llvm_scheme_handler.h
//  Novodb
//
//  Created by mike on 9/13/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef __Novodb__llvm_scheme_handler__
#define __Novodb__llvm_scheme_handler__

#include <iostream>
#include <sstream>

#include "include/cef_base.h"
#include "include/cef_browser.h"
#include "include/cef_url.h"
#include "include/cef_resource_handler.h"

#include "lldb/API/LLDB.h"

#include "dbg_handler.h"

#define __STDC_LIMIT_MACROS

namespace novo {

class LldbProcessSession {
public:
    LldbProcessSession(const lldb::SBTarget& _target) : target(_target) {}
    LldbProcessSession(const lldb::SBProcess& _process) : process(_process), target(_process.GetTarget()) {}

    lldb::SBProcess process;
    lldb::SBTarget target;
    
private:
    IMPLEMENT_REFCOUNTING(LldbProcessSession)
};
    
class LlbdSchemeHandler : public CefSchemeHandlerFactory {
public:
    LlbdSchemeHandler();
    
    virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                                 CefRefPtr<CefFrame> frame,
                                                 const CefString& scheme_name,
                                                 CefRefPtr<CefRequest> request){
        
        CefRefPtr<CefResourceHandler> handler(new DbgResourceHandler(req_router));
        
        return handler;
    }
    
private:
    RequestRouter req_router;
    
    std::vector<LldbProcessSession> sessions;
    
    IMPLEMENT_REFCOUNTING(LlbdSchemeHandler)
};

static void install_llvm_scheme() {
    CefRefPtr<CefSchemeHandlerFactory> handler(new LlbdSchemeHandler());
    
    CefRegisterSchemeHandlerFactory("dbg-llvm", CefString(), handler);
}
   
} // namespace novo

#endif /* defined(__Novodb__llvm_scheme_handler__) */
