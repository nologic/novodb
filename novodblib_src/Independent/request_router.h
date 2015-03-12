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
#include <atomic>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

namespace novo {

    enum handler_type {
        PLAIN_BLOCK, PLAIN_NONBLOCK, CHUNKED_NONBLOCKING
    };

    enum chunk_stage {
        NOT_CHUNKED, MAKE_HANDLERS, START_PROCESSING
    };

    class RequestPath;
    class ActionRequest;
    class ActionResponse;

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
        constraint exists_or(const std::vector<std::string>& keys);
        constraint has_addr(const std::string& key);
        constraint has_int(const std::string& key, int min = INT_MIN, int max = INT_MAX);
        constraint has_int(const std::string& key, int_bounds_check chfn);
        constraint matches(const std::string& key, const std::string& regex);
        
        // validation
        Valid validate(const RequestConstraint::constraint_list& constraints, const ActionRequest& req);
        
    } // namespace RequestConstraint
    
    typedef RequestPath path_type;
    typedef std::function<ActionResponse (const ActionRequest& req, boost::property_tree::ptree& out_tree)> path_handler;
    typedef std::tuple<path_type, RequestConstraint::constraint_list, path_handler, handler_type> handler_entry;

    class RequestPath : public std::vector<std::string> {
    public:
        RequestPath(const std::vector<std::string>& other_vec) : std::vector<std::string>(other_vec) {
            
        }
        
        RequestPath(const RequestPath& other) : std::vector<std::string>(other) {
        }
        
        RequestPath() : std::vector<std::string>() {
            
        }
        
        std::string toString() const {
            using namespace std;
            
            std::ostringstream ss;
            copy(this->begin(), this->end(), ostream_iterator<string>(ss, "/"));
            
            string os(ss.str());
            
            return os.substr(0, os.size() - 1);
        }
        
        bool equals(const RequestPath& other) const {
            if(size() == other.size()) {
                return (*this == other);
            }
            
            return false;
        }
    };

    class ActionResponse {
    public:
        typedef std::function<void ()> proc_func;

        
        ActionResponse(const ActionResponse& other) : status_code(other.status_code), msg(other.msg),
        path(other.path), output_handler(other.output_handler), processing_handler(other.processing_handler),
        remove_self(other.remove_self) {
        }
        
        static ActionResponse error(const std::string& msg) {
            return ActionResponse(400, msg, false);
        }
        
        static ActionResponse error(const char* msg) {
            return ActionResponse(400, std::string(msg), false);
        }
        
        static ActionResponse error(int status_code, const char* msg) {
            return ActionResponse(status_code, std::string(msg), false);
        }
        
        static ActionResponse no_error() {
            return ActionResponse(200, "OK", false);
        }
        
        static ActionResponse no_error_remove() {
            return ActionResponse(200, "OK", true);
        }
        
        const std::string& get_message() const {
            return msg;
        }
        
        int get_status() {
            return status_code;
        }
        
        static ActionResponse chunked_response(path_handler _handler, proc_func _proc) {
            std::string track_id = std::to_string(uniq_id.fetch_add(1));
            RequestPath path({"progress", track_id});
            
            return ActionResponse(path, _handler, _proc);
        }
        
        const RequestPath& get_path() const {
            return path;
        }
        
        path_handler get_output_handler() const {
            return output_handler;
        }
        
        proc_func get_processing_handler() const {
            return processing_handler;
        }
        
        bool do_remove() const {
            return remove_self;
        }
        
    private:
        friend class ChunkedResponse;
        
        ActionResponse(int code, const std::string& _msg, bool _remove_self) : status_code(code), msg(_msg), remove_self(_remove_self) {}
        
        ActionResponse(RequestPath& _path, path_handler _output_handler, proc_func _processing_handler) :
        path(_path), output_handler(_output_handler), processing_handler(_processing_handler), status_code(200), msg("OK"), remove_self(false){
        }
        
        std::string msg;
        int status_code;
        
        // to handle chunking
        RequestPath path;
        
        path_handler output_handler;
        proc_func processing_handler;
        
        // self removal
        bool remove_self;
        
        static std::atomic<uint32_t> uniq_id;
    };

    class ActionRequest : public std::map<std::string, std::string> {
    public:
        ActionRequest(std::string url_str, chunk_stage _stage = NOT_CHUNKED);
        
        const RequestPath& get_path() const {
            return path;
        }
        
        const chunk_stage get_stage() const {
            return stage;
        }
        
        void set_stage(chunk_stage _stage) {
            stage = _stage;
        }
        
        bool contains_key(const std::string& key) const {
            return this->count(key) > 0;
        }
    private:
        RequestPath path;
        
        // for support of chunked request handlers
        chunk_stage stage;
    };

    class RequestRouter {
    public:
        RequestRouter() {}
        
        void reguster_path(handler_entry& entry);
        void register_path(const std::vector<std::string>& path, const RequestConstraint::constraint_list& constraints,
                           path_handler handler, handler_type type = PLAIN_BLOCK);
        void unregister_path(const RequestPath& path);
        std::tuple<path_handler, handler_type> find_handler(const ActionRequest& req);
        
    private:
        std::vector<handler_entry> prefix_list;
    };
    
} // namespace novo

#endif /* defined(__Novodb__request_router__) */
