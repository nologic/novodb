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
#include <boost/network/uri.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>

namespace novo {

std::atomic<uint32_t> ActionResponse::uniq_id = {0};
    
void RequestRouter::register_path(const std::vector<std::string>& path, const RequestConstraint::constraint_list& constraints,
                                  path_handler handler, handler_type type) {
    handler_entry entry(RequestPath(path), constraints, handler, type);
    
    prefix_list.push_back(entry);
    
    BOOST_LOG_TRIVIAL(info) << "registered " << std::get<0>(entry).toString();
}

void RequestRouter::unregister_path(const RequestPath& path) {
    auto found_pair = std::find_if(std::begin(prefix_list), std::end(prefix_list), [&path](const handler_entry& pair){
        auto check_path = std::get<0>(pair);
        
        return path == check_path;
    });
    
    if(found_pair != std::end(prefix_list)) {
        prefix_list.erase(found_pair);
    
        BOOST_LOG_TRIVIAL(info) << "Unregistered path " << path.toString();
    } else {
        BOOST_LOG_TRIVIAL(info) << "Unregister path not found " << path.toString();
    }
}

std::tuple<path_handler, handler_type> RequestRouter::find_handler(const ActionRequest& req) {
    using namespace std;
    
    BOOST_LOG_TRIVIAL(info) << "Request path " << req.get_path().toString();
    
    const RequestPath& req_path = req.get_path();
    
    auto found_pair = std::find_if(std::begin(prefix_list), std::end(prefix_list), [&req_path](const handler_entry& pair){
        auto check_path = get<0>(pair);
        
        return req_path == check_path;
    });

    if(found_pair != std::end(prefix_list)) {
        RequestConstraint::constraint_list constraints = get<1>(*found_pair);
        RequestConstraint::Valid valid = RequestConstraint::validate(constraints, req);
        
        if(!valid.is_valid()) {
            throw valid;
        }
        
        return make_tuple(get<2>(*found_pair), get<3>(*found_pair));
    } else {
        BOOST_LOG_TRIVIAL(warning) << "no path handler registered: " << req_path.toString();
        
        throw false;
    }
}

RequestPath make_path(const boost::network::uri::uri& url) {
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

ActionRequest::ActionRequest(std::string url_str, chunk_stage _stage) : stage(_stage) {
    using namespace boost::network::uri;
    using namespace boost;
    using namespace std;
    
    uri req_url(url_str);
    string url_query(req_url.query());
    
    this->path = make_path(req_url);
    
    vector<uri::string_type> query_params;
    if(url_query.size() > 0) {
        split(query_params, url_query, is_any_of("&"));
    }
    
    for_each(query_params.begin(), query_params.end(), [this](const string& param) {
        vector<uri::string_type> param_parts;
        split(param_parts, param, is_any_of("="));
        
        if(param_parts.size() > 1) {
            (*this)[param_parts[0]] = boost::network::uri::decoded(param_parts[1]);
        } else {
            (*this)[param_parts[0]] = "";
        }
    });
}

// RequestConstraint
namespace RequestConstraint {
    constraint exists(const std::vector<std::string>& keys) {
        return [keys] (const ActionRequest& req) {
            
            for(auto key : keys) {
                if(req.count(key) == 0) {
                    return Valid::aint("Key required: " + key);
                }
            }
            
            return Valid();
        };
    }

    constraint exists_or(const std::vector<std::string>& keys) {
        return [keys] (const ActionRequest& req) {
            
            for(auto key : keys) {
                if(req.count(key) > 0) {
                    return Valid();
                }
            }
            
            std::ostringstream ss;
            std::copy(keys.begin(), keys.end(), std::ostream_iterator<std::string>(ss, ", "));
            
            return Valid::aint("One of keys required: " + ss.str());
        };
    }
    
    constraint has_int(const std::string& key, int min, int max) {
        return has_int(key, [min, max](int value) {
            return value <= max && value >= min;
        });
    }
    
    constraint has_int(const std::string& key, int_bounds_check chfn) {
        return [key, chfn] (const ActionRequest& req) {
            if(req.count(key) == 0) {
                return Valid::aint("Key required: " + key);
            } else {
                int parsed;
                
                if((std::stringstream(req.at(key)) >> parsed).fail()) {
                    return Valid::aint("Value not int for: " + key);
                }
                
                if(!chfn(parsed)) {
                    return Valid::aint("Int value validation failed: " + key);
                }
                
                return Valid();
            }
        };
    }
    
    constraint matches(const std::string& key, const std::string& regex) {
        throw "Not implemented";
    }
    
    Valid validate(const RequestConstraint::constraint_list& constraints, const ActionRequest& req) {
        for(constraint con : constraints) {
            Valid val = con(req);
            
            if(!val.is_valid()) {
                return val;
            }
        }
        
        return Valid();
    }
}

}