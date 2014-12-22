load_plugin(function() {
    // these variable are "static" (ie. same across instances)
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Memsearch";

    function Plugin() {}

    Plugin.prototype.attach_ui = function(angular_module, div_container_jq) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/plugin.html',
                link: function(scope, element, attrs) {
                    scope.base_container = $(element);
                }
            };
        }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
            function ($scope, $http, $compile) {
                _instance.set_session($scope.$parent.session);
                
                $scope.search_results = [];

                $scope.entryAddr = function(entry) {
                    return toHexString(parseInt(entry.base) + parseInt(entry.offset));
                }

                $scope.yara_search = function(base_addr, search_length, search_pattern) {
                    search_pattern = "rule ExampleRule\n" +
                                    "{\n" +
                                    "   strings:\n" +
                                    "     $str = { " + search_pattern + " }\n" +
                                    "   condition:\n" + 
                                    "     $str\n" +
                                    "}";

                    $scope.search_results = [];

                    session.yaraSearch(base_addr, search_length, search_pattern, function (matches) {
                        var retriever = setInterval(function() {
                            session.yaraSearchResults(matches.output_path, function(data) {
                                if(data.output != "") {
                                    data.output.forEach(function(match) {
                                        if('code' in match) {
                                            log("search complete (" + match.code_str + ")");
                                        } else {
                                            $scope.search_results.push(match);
                                            log(match);
                                        }
                                    });
                                }
                            }, function(data) {
                                clearInterval(retriever);
                            });
                        }, 1000);
                    }, log);
                };

                register_command({
                    cmd: "search",
                    complete: function(params) {
                        return ["[address] [length] [Yara]"];
                    },

                    execute: function(params) {
                        params[2] = "rule ExampleRule\n" +
                                    "{\n" +
                                    "   strings:\n" +
                                    "     $str = { " + params[2] + " }\n" +
                                    "   condition:\n" + 
                                    "     $str\n" +
                                    "}";
                        session.yaraSearch(params[0], params[1], params[2], function(matches) {
                            var retriever = setInterval(function() {
                                session.yaraSearchResults(matches.output_path, function(data) {
                                    if(data.output != "") {
                                        data.output.forEach(function(match) {
                                            if('code' in match) {
                                                log("search complete (" + match.code_str + ")");
                                            } else {
                                                log(toHexString(parseInt(match.base) + parseInt(match.offset)) + 
                                                    " (" + match.identifier + ") " + match.string);
                                            }
                                        });
                                    }
                                }, function(data) {
                                    clearInterval(retriever);
                                });
                            }, 1000);
                        }, log);
                    }
                });
            }
        ]);
    };

    Plugin.prototype.set_session = function(new_session) {
        session = new_session;
    };

    Plugin.prototype.set_sessions = function(all_sessions) {

    };

    Plugin.prototype.get_plugin_name = function() {
        return pluginName;
    };

    _instance = new Plugin();

    return _instance;
}());