//
//  dbg_handler.h
//  Novodb
//
//  Created by mike on 9/10/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef __Novodb__dbg_handler__
#define __Novodb__dbg_handler__

#include "include/cef_app.h"

#include "request_router.h"

#include <atomic>

using namespace novo;

class DbgResourceHandler : public CefResourceHandler {
public:
    DbgResourceHandler(RequestRouter& _router) : router(_router),
                                                 bytes_sent(0),
                                                 response(ActionResponse::no_error()),
                                                 is_frozen(false),
                                                 request_id(request_counter.fetch_add(1)){
    }

    ~DbgResourceHandler() {
    }
    
    virtual void Cancel() OVERRIDE {
        
    }
    virtual bool CanGetCookie( const CefCookie& cookie ) OVERRIDE {
        return false;
    }
    virtual bool CanSetCookie( const CefCookie& cookie ) OVERRIDE {
        return false;
    }
    virtual void GetResponseHeaders( CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) OVERRIDE;
    virtual bool ProcessRequest( CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) OVERRIDE;
    virtual bool ReadResponse( void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) OVERRIDE;

private:
    void freeze_output();
    
private:
    ActionResponse response;
    std::string out_data;
    boost::property_tree::ptree output;
    bool is_frozen;
    
    int bytes_sent;
    RequestRouter& router;
    
    const uint32 request_id;
    static std::atomic<uint32> request_counter;
    
    IMPLEMENT_REFCOUNTING(DbgResourceHandler)
};

#endif /* defined(__Novodb__dbg_handler__) */
