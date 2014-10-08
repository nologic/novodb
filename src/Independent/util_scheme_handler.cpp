//
//  util_scheme_hander.cpp
//  Novodb
//
//  Created by mike on 10/7/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "util_scheme_handler.h"

#include <utility>
#include <string>
#include <array>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>

namespace novo {
    UtilSchemeHandler::UtilSchemeHandler() {
        BOOST_LOG_TRIVIAL(trace) << "UtilSchemeHandler initializing";
        
        req_router.register_path({"ls"}, [] ACTION_CALLBACK(req, output) {
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

    }
}