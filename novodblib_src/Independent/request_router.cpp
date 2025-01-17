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

RequestPath make_path(std::string url) {
    using namespace boost;
    using namespace std;

    // remove [http]://
    string::size_type prot_loc = url.find("://");
    if(prot_loc != string::npos) {
        url = url.substr(prot_loc + 3);
    }
    
    // remove ? and beyond
    string::size_type query_loc = url.find("?");
    if(query_loc != string::npos) {
        url = url.substr(0, query_loc);
    }
    
    vector<string> path_vec;
    
    if(url.size() > 0) {
        split(path_vec, url, is_any_of("/"), token_compress_on);
    }
    
    return RequestPath(path_vec);
}

// ActionRequest class

ActionRequest::ActionRequest(std::string url_str, chunk_stage _stage) : stage(_stage) {
    using namespace boost;
    using namespace std;
    
    this->path = make_path(url_str);
    
    vector<string> query_params;
    string::size_type query_loc = url_str.find("?");
    
    if(query_loc != string::npos) {
        string url_query(url_str.substr(query_loc + 1));
        
        split(query_params, url_query, is_any_of("&"));
    }
    
    for_each(query_params.begin(), query_params.end(), [this](const string& param) {
        vector<string> param_parts;
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
    
    constraint has_addr(const std::string& key) {
        return [&key](const ActionRequest& req) {
            if(req.count(key) == 0) {
                return Valid::aint("Key required: " + key);
            } else {
                try {
                    std::stoull(req.at(key), 0, 16);
                    
                    return Valid();
                } catch(std::invalid_argument& ex) {
                    return Valid::aint("Value not address for: " + key + "(" + std::string(ex.what()) + ")");
                }
            }
        };
    }
    
    constraint has_int(const std::string& key, int_bounds_check chfn) {
        return [&key, chfn] (const ActionRequest& req) {
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