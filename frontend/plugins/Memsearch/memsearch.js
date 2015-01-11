load_plugin(function() {
    // these variable are "static" (ie. same across instances)
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Memsearch";

    function Plugin() {}

    Plugin.prototype.attach_ui = function(angular_module, div_container_jq) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/memsearch.html',
                link: function(scope, element, attrs) {
                    scope.base_container = $(element);
                }
            };
        }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile', 'DTOptionsBuilder', 'DTColumnBuilder',
            function ($scope, $http, $compile, DTOptionsBuilder, DTColumnBuilder) {
                _instance.set_session($scope.$parent.session);
                
                // regions
                $scope.dtOptions = 
                    DTOptionsBuilder.fromFnPromise(function() {
                        return session.listRegions(function(data) {
                            if(data.regions != "") {
                                return data.regions;
                            } else {
                                return [];
                            }
                        });
                    })
                    .withPaginationType('full_numbers');

                $scope.dtColumns = [
                    DTColumnBuilder.newColumn('type').withTitle('Type'),
                    DTColumnBuilder.newColumn('start').withTitle('Start'),
                    DTColumnBuilder.newColumn('end').withTitle('End'),
                    DTColumnBuilder.newColumn('cur_perm').withTitle('Perm'),
                    DTColumnBuilder.newColumn('max_perm').withTitle('Max'),
                    DTColumnBuilder.newColumn('sharing_mode').withTitle('SM'),
                    DTColumnBuilder.newColumn('descriptor').withTitle('Descriptor')
                ];
                // -regions


                $scope.search_results = [];

                var editor = undefined;

                setTimeout(function() {
                    editor = ace.edit("yara_editor");
                    editor.setTheme("ace/theme/xcode");
                    editor.getSession().setMode("ace/mode/w");
                }, 10);

                $scope.entryAddr = function(entry) {
                    return toHexString(parseInt(entry.base) + parseInt(entry.offset));
                }

                $scope.stringDisplay = function(entry) {
                    return entry.string.match(/.{1,2}/g).join(' ');
                }

                $scope.searching = false;

                $scope.yara_search = function(base_addr, search_length) {
                    if($scope.searching) {
                        log("Search in progress. Please wait.");
                        return;
                    }

                    search_pattern = editor.getSession().getValue();

                    $scope.search_results = [];

                    session.yaraSearch(base_addr, search_length, search_pattern, 256, function (matches) {
                        var retriever = setInterval(function() {
                            $scope.searching = true;
                            session.yaraSearchResults(matches.output_path, function(data) {
                                if(data.output != "") {
                                    data.output.forEach(function(match) {
                                        if('code' in match) {
                                            $scope.searching = false;
                                            log("search complete (" + $scope.search_results.length + " matches)");
                                        } else {
                                            $scope.search_results.push(match);
                                        }
                                    });
                                }
                            }, function(data) {
                                $scope.searching = false;
                                clearInterval(retriever);
                            });
                        }, 100);
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
                        session.yaraSearch(params[0], params[1], params[2], 256, function(matches) {
                            var retriever = setInterval(function() {
                                session.yaraSearchResults(matches.output_path, function(data) {
                                    if(data.output != "") {
                                        data.output.forEach(function(match) {
                                            if('code' in match) {
                                                log("search complete (" + match.code_str + ")");
                                            } else {
                                                log($scope.entryAddr(match) + ": " + $scope.stringDisplay(match));
                                            }
                                        });
                                    }
                                }, function(data) {
                                    clearInterval(retriever);
                                });
                            }, 100);
                        }, log);
                    }
                });

                register_command({
                    cmd: "regions",
                    complete: function(params) {
                        return ["(Show memory regions)"];
                    },

                    execute: function(params) {
                        session.listRegions(function(data) {
                            if(data.regions != "") {
                                data.regions.forEach(function(entry) {
                                    
                                });
                            } else {
                                log(data);
                            }
                        });
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