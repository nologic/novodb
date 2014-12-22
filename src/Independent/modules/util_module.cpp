//
//  util_scheme_hander.cpp
//  Novodb
//
//  Created by mike on 10/7/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "platform_support.h"

#include <utility>
#include <string>
#include <array>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>

#include "Module.h"

namespace novo {
    class util_module : public ModuleInterface {
        virtual std::string getName() {
            return "util";
        }
        
        virtual void registerRouter(RequestRouter& req_router);
        
        virtual CefRefPtr<SessionState> createSession() {
            return CefRefPtr<SessionState>(new SessionState());
        }
    };
    
    void util_module_main() {
        util_module module;
        
        register_module(module);
    }
    
    void util_module_unload() {
        
    }
    
    void util_module::registerRouter(RequestRouter& req_router) {
        BOOST_LOG_TRIVIAL(trace) << "UtilSchemeHandler initializing";
        
        req_router.register_path({"ls"}, {
            RequestConstraint::exists({"startwith"}),
            RequestConstraint::exists({"path"}),
            RequestConstraint::has_int("maxcount")
        }, [] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace boost::filesystem;
            
            string startwith;
            string path_str = req.at("path");
            int maxcount = stoi(req.at("maxcount"));
            
            if(req.find("startwith") != std::end(req)) {
                startwith = req.at("startwith");
            }
            
            path ppath(path_str);
            
            if(!exists(ppath)) {
                return ActionResponse::error("Does not exist");
            }
            
            if(!is_directory(ppath)) {
                return ActionResponse::error("Not a directory");
            }
            
            int count = 0;
            boost::property_tree::ptree dirlist;
                
            for(directory_iterator dir_itr(ppath); dir_itr != directory_iterator() && count < maxcount; dir_itr++) {
                boost::property_tree::ptree dirent;
                
                path p_look_path = dir_itr->path();
                string look_path = p_look_path.filename().string();
                
                if(look_path.find(startwith) == 0) {
                    count++;
                    dirent.put("file", look_path);
                    dirent.put("dir", to_string(is_directory(p_look_path)));
                    
                    dirlist.push_back(make_pair("", dirent));
                }
            }
            
            if(dirlist.size() > 0) {
                output.add_child("files", dirlist);
            }
            
            return ActionResponse::no_error();
        });

        req_router.register_path({"list", "proc"}, {}, [] ACTION_CALLBACK(req, output) {
            using namespace boost::property_tree;
            
            ptree proc_items;
            auto proc_list = get_process_listing();
            
            for(auto proc_tuple : proc_list) {
                ptree proc_item;
                
                proc_item.put("pid", std::to_string(std::get<0>(proc_tuple)));
                proc_item.put("path", std::get<1>(proc_tuple));
                
                proc_items.push_back(make_pair("", proc_item));
            }
            
            output.put_child("processes", proc_items);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"list", "ui_plugins"}, {}, [] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace boost::filesystem;
            
            path plugins_path(path(app_path()) / path("frontend/plugins"));
            
            if(!exists(plugins_path)) {
                return ActionResponse::error("Does not exist " + plugins_path.string());
            }
            
            if(!is_directory(plugins_path)) {
                return ActionResponse::error("Not a directory" + plugins_path.string());
            }
            
            boost::property_tree::ptree dirlist;
            
            for(directory_iterator dir_itr(plugins_path); dir_itr != directory_iterator(); dir_itr++) {
                boost::property_tree::ptree dirent;
                
                if(is_directory(*dir_itr)) {
                    dirent.put("name", dir_itr->path().filename().string());
                    dirlist.push_back(make_pair("", dirent));
                }
            }
            
            output.add_child("plugins", dirlist);
            
            return ActionResponse::no_error();
        });
    }
}