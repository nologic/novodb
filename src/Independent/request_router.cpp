//
//  request_router.cpp
//  Novodb
//
//  Created by mike on 9/13/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "request_router.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/log/trivial.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>

void RequestRouter::register_path(const std::vector<std::string>& path, path_handler handler, bool blocking) {
    handler_entry entry(RequestPath(path), handler, blocking);
    
    prefix_list.push_back(entry);
    
    BOOST_LOG_TRIVIAL(info) << "registered " << std::get<0>(entry).toString();
}

void RequestRouter::unregister_path(const std::string& path) {
    RequestPath r_path(RequestPath::make_path("dbg://" + path));
    
    std::remove_if(std::begin(prefix_list), std::end(prefix_list), [&r_path](RequestRouter::handler_entry pair){
        return std::get<0>(pair) == r_path;
    });
}

std::tuple<RequestRouter::path_handler, bool> RequestRouter::find_handler(const ActionRequest& req) {
    using namespace std;
    
    BOOST_LOG_TRIVIAL(info) << "Request path " << req.get_path().toString();
    
    const RequestPath& req_path = req.get_path();
    
    auto found_pair = std::find_if(std::begin(prefix_list), std::end(prefix_list), [&req_path](const handler_entry& pair){
        auto check_path = get<0>(pair);
        
        return req_path == check_path;
    });

    if(found_pair != std::end(prefix_list)) {
        return make_tuple(get<1>(*found_pair), get<2>(*found_pair));
    } else {
        BOOST_LOG_TRIVIAL(warning) << "no path handler registered: " << req_path.toString();
        
        throw false;
    }
}

RequestPath RequestPath::make_path(const boost::network::uri::uri& url) {
    using namespace boost::network::uri;
    using namespace boost;
    using namespace std;
    
    vector<string> path_vec;
    string path(url.host());
    path.append(url.path());
    
    if(path.size() > 0) {
        split(path_vec, path, is_any_of("/"), token_compress_on);
    }
    
    return RequestPath(path_vec);
}

// ActionRequest class

ActionRequest::ActionRequest(CefRefPtr<CefRequest> _request) : request(_request) {
    using namespace boost::network::uri;
    using namespace boost;
    using namespace std;
    
    uri req_url(_request->GetURL().ToString());
    string url_query(req_url.query());
    
    this->path = RequestPath::make_path(req_url);
    
    vector<uri::string_type> query_params;
    if(url_query.size() > 0) {
        split(query_params, url_query, is_any_of("&"));
    }
    
    for_each(query_params.begin(), query_params.end(), [this](const string& param) {
        vector<uri::string_type> param_parts;
        split(param_parts, param, is_any_of("="));
        
        std::cerr << param_parts[0] << " = " << param_parts[1] << std::endl;
        
        if(param_parts.size() > 1) {
            (*this)[param_parts[0]] = param_parts[1];
        } else {
            (*this)[param_parts[0]] = "";
        }
    });
}