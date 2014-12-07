//
//  request_router.h
//  Novodb
//
//  Created by mike on 9/13/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef __Novodb__request_router__
#define __Novodb__request_router__

#include <vector>
#include <functional>
#include <tuple>
#include <map>
#include <mutex>
#include <sstream>

#include <boost/network/uri.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "include/cef_base.h"
#include "include/cef_app.h"

class ActionResponse {
public:
    ActionResponse(const ActionResponse& other) : status_code(other.status_code), msg(other.msg) {
        
    }
    
    static ActionResponse error(const std::string& msg) {
        return ActionResponse(400, msg);
    }
    
    static ActionResponse error(const char* msg) {
        return ActionResponse(400, std::string(msg));
    }
    
    static ActionResponse error(int status_code, const char* msg) {
        return ActionResponse(status_code, std::string(msg));
    }
    
    static ActionResponse no_error() {
        return ActionResponse(200, "OK");
    }
    
    const std::string& get_message() const {
        return msg;
    }
    
    int get_status() {
        return status_code;
    }
    
private:
    ActionResponse(int code, const std::string& _msg) : status_code(code), msg(_msg){}
    
    std::string msg;
    int status_code;
};

class RequestPath : public std::vector<std::string> {
public:
    static RequestPath make_path(const boost::network::uri::uri& url);
    
    RequestPath(const std::vector<std::string>& other_vec) : std::vector<std::string>(other_vec) {
        
    }
    
    RequestPath(const RequestPath& other) : std::vector<std::string>(other) {
        
    }
    
    RequestPath() {
    
    }
    
    std::string toString() const {
        std::string ss;
        
        for_each(this->begin(), this->end(), [&ss](const std::string part) {
           ss += (part + "/");
        });
        
        return ss;
    }
    
    bool equals(const RequestPath& other) const {
        if(size() == other.size()) {
            return (*this == other);
        }
        
        return false;
    }
    
private:
    
    IMPLEMENT_REFCOUNTING(RequestRouter);
};

class ActionRequest : public std::map<std::string, std::string> {
public:
    ActionRequest(CefRefPtr<CefRequest> _request);
    
    const CefRequest& cef_request() const {
        return *request;
    }
    
    const RequestPath& get_path() const {
        return path;
    }
private:
    CefRefPtr<CefRequest> request;
    RequestPath path;
    
    IMPLEMENT_REFCOUNTING(RequestRouter);
};

namespace RequestConstraint {

    class Valid : public std::string {
    public:
        Valid() : _is_valid(true) {}
        Valid(const std::string& msg) : std::string(msg), _is_valid(false) {}
        Valid(const Valid& other) : std::string(other), _is_valid(other._is_valid) {}
    
        static Valid aint(const std::string& msg) {
            return Valid(msg);
        }
    
        bool is_valid() const {
            return _is_valid;
        }
    
    private:
        bool _is_valid;
    };

    typedef std::function< Valid (const ActionRequest& req) > constraint;
    typedef std::function< bool (int value) > int_bounds_check;
    typedef std::vector<constraint> constraint_list;
    
    // the constraint validators
    constraint exists(const std::vector<std::string>& keys);
    constraint has_int(const std::string& key, int min = INT_MIN, int max = INT_MAX);
    constraint has_int(const std::string& key, int_bounds_check chfn);
    constraint matches(const std::string& key, const std::string& regex);
    
    // validation
    Valid validate(const RequestConstraint::constraint_list& constraints, const ActionRequest& req);
    
} // namespace RequestConstraint

class RequestRouter {
public:
    typedef RequestPath path_type;
    typedef std::function<ActionResponse (const ActionRequest& req, boost::property_tree::ptree& out_tree)> path_handler;
    typedef std::tuple<path_type, RequestConstraint::constraint_list, path_handler, bool> handler_entry;
    
    RequestRouter() {}
    
    void reguster_path(handler_entry& entry);
    void register_path(const std::vector<std::string>& path, const RequestConstraint::constraint_list& constraints, path_handler handler, bool blocking = false);
    void unregister_path(const std::string& path);
    std::tuple<path_handler, bool> find_handler(const ActionRequest& req);
    
private:
    std::vector<handler_entry> prefix_list;
    
    IMPLEMENT_REFCOUNTING(RequestRouter);
};

#endif /* defined(__Novodb__request_router__) */
