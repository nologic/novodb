//
//  mail_web.cpp
//  Novodb
//
//  Created by mike on 1/18/15.
//  Copyright (c) 2015 Mikhail Sosonkin. All rights reserved.
//

#include <string>
#include <memory>
#include <thread>

#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <CivetServer.h>

#include "main_web.h"

namespace novo {
    
    const char * options[] = {
        "listening_ports", "4070", 0
    };
    
    std::shared_ptr<CivetServer> server;
    
    int main_web() {
        using namespace novo;
    
        server = std::shared_ptr<CivetServer>(new CivetServer(options));
        
        // start plugins
        util_module_main();
        lldb_module_main();
        
        while(true) {
            // let the server work.
            sleep(1);
        }
    
        return 0;
    }
    
    class UnknownHandler: public CivetHandler {
    public:

        bool handleGet(CivetServer *server, struct mg_connection *conn) {
            mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/javascript\r\n\r\n");
            
            return true;
        }
    };

    class ModuleHandler: public CivetHandler {
    public:
        ModuleHandler(ModuleInterface& module) {
            uri_skip_ind = module.getName().length() + 2;
            module.registerRouter(router);
        }
        
        bool handleGet(CivetServer *server, struct mg_connection *conn) {
            /* Handler may access the request info using mg_get_request_info */
            struct mg_request_info * req_info = mg_get_request_info(conn);
            
            BOOST_LOG_TRIVIAL(trace) << "Entering request " << req_info->uri;
            
            // dummy URL to adjust to request format spec.
            std::string url = "http://" + std::string(req_info->uri).substr(uri_skip_ind);
            
            if(req_info->query_string) {
                url += "?" + std::string(req_info->query_string);
            }

            ActionRequest a_req(url);
            ActionResponse response(ActionResponse::no_error());
            boost::property_tree::ptree output;
            std::string callback;
            
            if(a_req.contains_key("callback")) {
                callback.assign(a_req.at("callback"));
                
                try {
                    
                    auto handler_pair = router.find_handler(a_req);
                    auto handler = std::get<0>(handler_pair);
                    auto type = std::get<1>(handler_pair);
                    
                    auto proc_func = [this, &response, &output, handler, a_req]() {
                        response = handler(a_req, output);
                        
                        if(response.do_remove()) {
                            router.unregister_path(a_req.get_path());
                        }
                    };
                    
                    auto chunked_proc = [this, &response, handler, a_req, &output]() {
                        response = handler(a_req, output);
                        
                        output.put("output_path", response.get_path().toString());
                        this->router.register_path(response.get_path(), {}, response.get_output_handler());
                        
                        auto proc = response.get_processing_handler();
                        
                        // start the processing.
                        std::thread(proc).detach();
                    };
                    
                    switch(type) {
                        case PLAIN_NONBLOCK:
                        case PLAIN_BLOCK:
                            proc_func();
                            break;
                        
                        case CHUNKED_NONBLOCKING:
                            chunked_proc();
                            break;
                    }
                    
                } catch(RequestConstraint::Valid& not_valid){
                    // request is not valid, this should not happen unless of
                    // a programmer fail therefore accepting performance overhead
                    
                    response = ActionResponse::error(not_valid);
                } catch (bool not_found) {
                    // 'not_found' should only be false but shouldn't happen unless a typo occurs.
                    response = ActionResponse::error(404, "Handler not found");
                }
                
                BOOST_LOG_TRIVIAL(trace) << "Request Processed (" << req_info->uri << ")";
            } else {
                response = ActionResponse::error("'callback' parameter missing");
            }
            
            mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/javascript\r\n\r\n");

            boost::property_tree::ptree output_wrap;
            
            output_wrap.put("code", response.get_status());
            output_wrap.put("msg", response.get_message());
            
            if(output.size() > 0) {
                output_wrap.add_child("output", output);
            }
            
            std::stringstream ss;
            boost::property_tree::write_json(ss, output_wrap, false);
            
            std::string out_data = ss.str();
            output.clear();
            
            if(callback.empty()) {
                mg_printf(conn, "%s", out_data.c_str());
            } else {
                mg_printf(conn, "%s(%s);", callback.c_str(), out_data.c_str());
            }
            
            return true;
        }
        
    private:
        RequestRouter router;
        size_t uri_skip_ind;
    };

    
    /*
        Here to support Module.h
    */
    void register_module(ModuleInterface& module) {
        server->addHandler("/" + module.getName() + "/*", new ModuleHandler(module));
    }
    
}

/*
 Here to support {"list", "ui_plugins"} in Utils module.
 */
std::string app_path() {
    return std::string();
}
