(function() {
    var pluginName = "Loader";

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/loader.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile', 'DTOptionsBuilder', 'DTColumnBuilder',
                function ($scope, $http, $compile, DTOptionsBuilder, DTColumnBuilder) {
                    var session = $scope.$parent.session;

                    $scope.dtOptions = 
                        DTOptionsBuilder.fromFnPromise(function() {
                            return utils.proc_list(function (data) {
                                return data.processes.map(function(proc) {
                                    // load by path
                                    var slashIndex = proc.path.lastIndexOf('/');
                                    
                                    proc.name = proc.path.substring(slashIndex + 1, proc.path.length);
                                    proc.path = proc.path.substring(0, slashIndex + 1);

                                    return proc;
                                });
                            });
                        })
                        .withPaginationType('full_numbers')
                        .withOption('rowCallback', function(nRow, aData, iDisplayIndex, iDisplayIndexFull) {
                            // Unbind first in order to avoid any duplicate handler (see https://github.com/l-lin/angular-datatables/issues/87)
                            $('td', nRow).unbind('dblclick');
                            $('td', nRow).bind('dblclick', function() {
                                $scope.$apply(function() {
                                    $scope.attach(aData);
                                });
                            });
                            return nRow;
                        });

                    $scope.dtColumns = [
                        DTColumnBuilder.newColumn('pid').withTitle('PID'),
                        DTColumnBuilder.newColumn('name').withTitle('Process'),
                        DTColumnBuilder.newColumn('path').withTitle('Path').notVisible()
                    ];

                    function in_process() {
                        $scope.$parent.closePlugin();
                        session.getProcState();
                    }

                    $scope.attach = function(proc) {
                        log("Attaching to " + proc.pid + ":" + proc.name + " ...");
                        session.attach(proc.pid, function(data) {
                            log(data);
                            in_process();

                            log("Attached to " + proc.pid + ":" + proc.name);
                        });
                    };

                    $scope.connect = function(hostport) {
                        var url = "connect://" + hostport;

                        session.connect(url, function(data) {
                            log(data);

                            in_process();

                            log("connected to " + hostport);
                        }, function(data){
                            log(data);
                            log("unable to connect to " + hostport);
                        });
                    };

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
                                
                                return utils.ls(prefix, postfix, 20, function (data) {
                                    if('files' in data) {
                                        return data.files.map(function (file) {
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
                            return ["(Get session state)"];
                        },

                        execute: function(params) {
                            session.getProcState(log, log);
                        }
                    });

                    register_command({
                        cmd: "threads",
                        complete: function(params) {
                            return ["(Get list of threads)"];
                        },

                        execute: function(params) {
                            session.getThreads(log, log);
                        }
                    });

                    register_command({
                        cmd: "stop",
                        complete: function(params) {
                            return ["(stop running process)"];
                        },

                        execute: function(params) {
                            session.stop_proc(log, log);
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

                    register_command({
                        cmd: "sessions",
                        complete: function(params) {
                            return ["(Get session state)"];
                        },

                        execute: function(params) {
                            session.listSessions(log, log);
                        }
                    });

                }
            ]);
        },

        constructor: function() {
            return function() {
                function Plugin() {}

                Plugin.prototype.get_name = function() {
                    return pluginName;
                };

                Plugin.prototype.get_descriptor = function() {
                    return "Process loader";
                };

                return new Plugin();
            }
        }
    };

    register_plugin(pluginSpec);
})();