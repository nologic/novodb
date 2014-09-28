//
//  dbg_handler.cpp
//  Novodb
//
//  Created by mike on 9/10/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include <iostream>
#include <algorithm>
#include <thread>

#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "dbg_handler.h"
#include "include/cef_url.h"
#include "lldb/API/LLDB.h"

void DbgResourceHandler::GetResponseHeaders( CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) {
    response->SetStatus(this->response.get_status());
    response->SetMimeType(CefString("application/json"));
    response->SetStatusText(CefString(this->response.get_message()));
    response_length = out_data.size();
    
    return;
}

bool DbgResourceHandler::ProcessRequest( CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) {
    BOOST_LOG_TRIVIAL(trace) << "Entering request " << request->GetURL().ToString();
    
    ActionRequest a_req(request);
    
    try {
        
        auto handler_pair = router.find_handler(a_req);
        auto handler = std::get<0>(handler_pair);
        
        auto proc_func = [this, callback, handler, a_req]() {
            this->response = handler(a_req, this->output);
            
            std::stringstream ss;
            boost::property_tree::write_json(ss, this->output, false);
            
            this->out_data = ss.str();
            this->output.clear();
            
            callback->Continue();
        };
        
        if(std::get<1>(handler_pair)) {
            // this will be handled as non-blocking (i.e handler takes long time)
            std::thread(proc_func).detach();
            
        } else {
            // This will be handled as blocking.
            proc_func();
        }
        
    } catch (bool not_found) {
        // should only be false but shouldn't happen unless a typo occurs.
        this->response = ActionResponse::error(404, "Handler not found");
    }
    
    BOOST_LOG_TRIVIAL(trace) << "Processed request " << request->GetURL().ToString();

    return true;
}

bool DbgResourceHandler::ReadResponse( void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) {
    // making a lot of assumtions here!
    // to be redone.
    int read_count = std::min((int)out_data.size(), bytes_to_read);
    memcpy(data_out, out_data.data(), read_count);
    
    bytes_read = read_count;
    
    std::cout << "bytes_read " << bytes_read << " " << out_data << std::endl;

    callback->Continue();
    
    if(!sent) {
        sent = true;
        return true;
    } else {
        return false;
    }
}
