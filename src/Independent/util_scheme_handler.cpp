//
//  util_scheme_hander.cpp
//  Novodb
//
//  Created by mike on 10/7/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "util_scheme_handler.h"
#include "platform_support.h"

#include <utility>
#include <string>
#include <array>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>

namespace novo {
    UtilSchemeHandler::UtilSchemeHandler() {
        BOOST_LOG_TRIVIAL(trace) << "UtilSchemeHandler initializing";
        
        req_router.register_path({"ls"}, {}, [] ACTION_CALLBACK(req, output) {
            using namespace std;
            using namespace boost::filesystem;
            
            string path_str = req.at("path");
            int maxcount = stoi(req.at("maxcount"));
            
            path ppath(path_str);
            
            if(!exists(ppath)) {
                return ActionResponse::error("Does not exist");
            }
            
            if(!is_directory(ppath)) {
                return ActionResponse::error("Not a directory");
            }
            
            int count = 0;
            boost::property_tree::ptree dirlist;
                
            for(directory_iterator dir_itr(ppath); dir_itr != directory_iterator(); dir_itr++, count++) {
                if(count >= maxcount) {
                    break;
                }
                    
                boost::property_tree::ptree dirent;
                    
                dirent.put("file", dir_itr->path().filename().string());
                    
                dirlist.push_back(make_pair("", dirent));
            }
                
            output.add_child("files", dirlist);
            
            return ActionResponse::no_error();
        });

        req_router.register_path({"list", "proc"}, {}, [this] ACTION_CALLBACK(req, output) {
            using namespace boost::property_tree;
            
            ptree proc_items;
            auto proc_list = get_process_listing();
            
            for(auto proc_tuple : proc_list) {
                ptree proc_item;
                
                proc_item.put("pid", std::to_string(std::get<0>(proc_tuple)));
                proc_item.put("debuggable", std::to_string(std::get<1>(proc_tuple)));
                proc_item.put("path", std::get<2>(proc_tuple));
                
                proc_items.push_back(make_pair("", proc_item));
            }
            
            output.put_child("processes", proc_items);
            
            return ActionResponse::no_error();
        });
        
        req_router.register_path({"list", "ui_plugins"}, {}, [this] ACTION_CALLBACK(req, output) {
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
                
                dirent.put("name", dir_itr->path().filename().string());
                
                dirlist.push_back(make_pair("", dirent));
            }
            
            output.add_child("plugins", dirlist);
            
            return ActionResponse::no_error();
        });
    }
}