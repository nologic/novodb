load_plugin(function() {
    // these variable are "static" (ie. same across instances)
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Loader";

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
                

                $scope.load_processes = function() {
                    $http.get('util://list/proc').then(function (data) {
                        $scope.processes = data.data.processes.map(function(proc) {
                            // load by path
                            var slashIndex = proc.path.lastIndexOf('/');
                            
                            proc.name = proc.path.substring(slashIndex + 1, proc.path.length);
                            proc.path = proc.path.substring(0, slashIndex + 1);

                            return proc;
                        });
                    });
                };

                $scope.attach = function(proc) {
                    log("Attaching to " + proc.pid + ":" + proc.name + " ...");
                    session.attach(proc.pid, function() {
                        $('#controls_bar').show();

                        $scope.base_container.remove();
                        session.getProcState();

                        log("Attaced to " + proc.pid + ":" + proc.name);
                    });
                };

                $scope.load_processes();

                // register the loader commands:
                register_command({
                    cmd: "attach", 
                    description: "Attach to a process",
                    complete: function(params) {
                        if(params.length == 0) {
                            return ["[pid]", "[name]"];
                        } else {
                            var filterBy = params[0];

                            return $http.get('util://list/proc').then(function (data) {
                                return data.data.processes.map(function (proc) {
                                    var slashIndex = proc.path.lastIndexOf('/');
                                    var proc_name = (slashIndex > -1 ? proc.path.substring(slashIndex + 1) : proc.path);

                                    return proc.pid + ":" + proc_name;
                                }).filter(function (testVal) {
                                    return testVal.indexOf(filterBy) > -1;
                                });
                            });
                        }
                    },
                    execute: function(params) {
                        function attach_pid(pid) {
                            session.attach(pid, function() {
                                $('#controls_bar').show();

                                log("Attaced to " + $item.pid);
                            });
                        }

                        var parts = params[0].split(":");

                        if(parts.length == 1) {
                            if((/[0-9]+/).test(parts[0])) {
                                attach_pid(parts[0]);
                            } else {
                                $http.get('util://list/proc').then(function (data) {
                                    var procs = data.data.processes.map(function (proc) {
                                        var slashIndex = proc.path.lastIndexOf('/');
                                        var proc_name = (slashIndex > -1 ? proc.path.substring(slashIndex + 1) : proc.path);

                                        return proc.pid + ":" + proc_name;
                                    }).filter(function (testVal) {
                                        return testVal.indexOf(parts[0]) > -1;
                                    });

                                    if(procs.length == 0) {
                                        output("Process " + parts[0] + " not found");
                                    } else if(procs.length > 1) {
                                        output("Process name " + parts[0] + " is ambiguous");
                                    } else {
                                        attach_pid(procs[0].split(":")[0]);
                                    }
                                });
                            }
                        } else {
                            attach_pid(parts[0]);
                        }
                    }
                });

                register_command({
                    cmd: "load",
                    complete: function(params) {
                        if(params.length == 0) {
                            return ["[path start (/ or .) [run to symbol - default entry point]] (Load an executable)"];
                        } else {
                            // load by path
                            var slashIndex = params[0].lastIndexOf('/');
                            var prefix = "";
                            var postfix = "";

                            if(slashIndex < 0) {
                                return;
                            } 
                            
                            postfix = params[0].substring(slashIndex + 1, params[0].length);
                            prefix = params[0].substring(0, slashIndex + 1);
                            
                            return $http.get('util://ls', {
                                method: "GET",
                                params: {
                                    path: prefix,
                                    startwith: postfix,
                                    maxcount: 20
                                }
                            }).then(function (data) {
                                if('files' in data.data) {
                                    return data.data.files.map(function (file) {
                                        return prefix + file.file + (file.dir == "1"?"/":"");
                                    });
                                } else {
                                    return [];
                                }
                            });
                        }
                    },
                    execute: function(params) {
                        session.load(params[0], [], function() {
                            log("Loaded " + params[0]);

                            if(params[1] != undefined) {
                                session.setBreakpoint(params[1], function(data) {
                                    log("breakpoint set, launching");
                                    session.launch(function(data) {
                                        $('#controls_bar').show();

                                        log("Process launched");
                                    }, function(data) {
                                        log("Process launch failed");
                                    });
                                }, function(data) {
                                    log("failed to set breakpoint on " + params[1])
                                });
                            }
                        });
                    }
                });

                register_command({
                    cmd: "state",
                    complete: function(params) {
                        return ["(Get session state)"]
                    },

                    execute: function(params) {
                        session.getProcState(log, log);
                    }
                });

                register_command({
                    cmd: "launch",
                    complete: function(params) {
                        return ["[run to symbol (opt)] (Launch loaded executable)"]
                    },

                    execute: function(params) {
                        if(params[0] != undefined) {
                            session.setBreakpoint(params[1], function(data) {
                                log("breakpoint set, launching");
                                session.launch(function(data) {
                                    log("Process launched");
                                }, function(data) {
                                    log("Process launch failed");
                                });
                            }, function(data) {
                                log("failed to set breakpoint on " + params[1])
                            });
                        } else {
                            session.launch(function(data) {
                                log("Process launched");
                            }, function(data) {
                                log("Process launch failed");
                            });
                        }
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