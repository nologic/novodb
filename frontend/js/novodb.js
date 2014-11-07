function load_js_file(filename){
    var fileref = document.createElement('script');

    fileref.setAttribute("type","text/javascript");
    fileref.setAttribute("src", filename);

    if (typeof fileref != "undefined") {
        document.getElementsByTagName("head")[0].appendChild(fileref);
    }
}

var novo = angular.module('novodb', ['ui.bootstrap']);

novo.config(function(/*$routeProvider, */$controllerProvider, $compileProvider, $filterProvider, $provide) {
    novo.controllerProvider = $controllerProvider;
    novo.compileProvider    = $compileProvider;
    //novo.routeProvider      = $routeProvider;
    novo.filterProvider     = $filterProvider;
    novo.provide            = $provide;

    // Register routes with the $routeProvider
}).controller("dbg", ['$scope', '$http', '$compile',
	function($scope, $http, $compile) {
        var session = create_ndb_session($http);

        $scope.targetExe = function(path) {
            session.load(path, [], function() {
                $scope.listSymbols();
            });
        };

        $scope.attachTarget = function(pid) {
            pid = pid.split(" ");
            pid.shift();

            session.attach(pid[0], function() {
                $scope.listSymbols();
                $scope.instantiatePlugin('Regview');
                log("Attaced to " + pid[0]);
            });
        };

		$scope.launchTarget = function() {
            session.launch();
		};

		$scope.getThreads = function() {
			session.getThreads(function(resp) {
				$scope.thread_list = JSON.stringify(resp);
			});
		};

		$scope.setBreakpoint = function(session_id, symbol) {
            session.setBreakpoint(symbol, function(data) {
				$scope.bp_output = JSON.stringify(data);
			});
		};

		$scope.getModules = function() {
			session.getModules(function(data) {
				$scope.module_output = data;
			});
		};

		$scope.readMemory = function(_address) {
			session.readMemory(_address, 4096, function(data) {
				$scope.memory_output = JSON.stringify(data, null, "\t");
			});
		};

		$scope.procState = function() {
			session.getProcState(function(data) {
                $scope.proc_state = data;
			});
		};

		$scope.readRegisters = function() {
            session.readRegisters(0, 0, function(data) {
				$scope.register_output = data;
			});
		};

		$scope.readInstructions = function(_address) {
			session.readInstructions(_address, 4096, function(data) {
                $scope.inst_output = data;
			});
		};

		$scope.listSymbols = function() {
            session.getSymbols(0, function(data) {
				$scope.symbols_output = data;
			});
		};

        $scope.getFrames = function(thread_ind) {
            session.getFrames(thread_ind, function(data) {
                $scope.frames_list = JSON.stringify(data, null, "\t");
            });
        };

        $scope.onSelectPart = function($item, $model, $label) {
            $scope.attachAutocomplete = $item.pid + " " + $item.path;

            session.attach($item.pid, function() {
                $scope.listSymbols();
                $scope.getThreads();
                $scope.instantiatePlugin('Regview');
                $scope.instantiatePlugin('Controller');
                log("Attaced to " + $item.pid);
            });
        };

        $scope.plugins = [];
        $scope.instantiatePlugin = function(pluginName, params) {
            $scope.plugins.push({
                name: pluginName,
                session: session,
                params: params
            });
        };

        $scope.get_attach_data = function(partial) {
            if(partial != undefined) {
                if (partial.indexOf('l') == 0) {
                    
                }
            }

            return ['load', 'attach'];
        };

        setInterval(function() {
            if($scope.session_id !== undefined) {
                $scope.procState($scope.session_id);
            }
        }, 1000);

        $http.get("util://list/ui_plugins", {
            method: "GET"
        }).then(function (resp) {
            console.info("plugins", resp);

            resp.data.plugins.forEach(function(p) {
                load_js_file("plugins/" + p.name + "/" + p.name + ".js");
            });
        }, function (resp) {
            console.info("fail plugins: ", resp);

            log("Failed to load UI plugins");
        });

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
                        $scope.listSymbols();
                        $scope.getThreads();
                        $scope.instantiatePlugin('Regview');
                        $scope.instantiatePlugin('Controller');
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
            description: "Load an executable",
            complete: function(params) {
                if(params.length == 0) {
                    return ["[path start / | .]"];
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
                        return data.data.files.map(function (file) {
                            return prefix + file.file;
                        });
                    });
                }
            },
            execute: function(params) {
                output("Executing! load");
            }
        });
	}
]).directive("ndbPluginsContainer", function($compile) {
    return {
        scope:{
            ndbPluginsContainer:"=" //import referenced model to our directives scope
        },
        link: function(scope, elem, attr, ctrl) {
            scope.$watch('ndbPluginsContainer', function(){ // watch for when model changes
                while(scope.ndbPluginsContainer.length > 0) {
                    var d = scope.ndbPluginsContainer.shift();

                    if (d == undefined) {
                        return;
                    }

                    log("Instantiating: " + d);

                    var s = scope.$new(); //create a new scope
                    angular.extend(s, d); //copy data onto it

                    var template = '<div class="col-md-4 column" ndb-Plugin-' + d.name + ' ng-controller="ndbPlugin' + d.name + '"></div>';
                    elem.append($compile(template)(s)); // compile template & append
                }
            }, true); //look deep into object
        }
    };
});

var commands = [];

function register_command(cmd) {
    commands.push(cmd);
}

$( document ).ready(function() {
    $('#footer_terminal').terminal(function (command, term) {
        // execute command!
        var cmdText = $.terminal.splitCommand(command);

        var execCmd = commands.filter(function(cmd) {
            return cmd.cmd == cmdText.name;
        });

        if(execCmd.length > 1) {
            term.echo("Disambiguate: " + execCmd.map(function(cmd) {
                return cmd.cmd;
            }).join(" "));
        } else if(execCmd.length == 1) {
            var execStr = execCmd[0].execute(cmdText.args);

            if(execStr != undefined) {
                term.echo(execStr);
            }
        } else {
            term.echo("Command '" + cmdText.name + "' not found");
        }
    }, {
        keydown: function(event, term) {
            if (event.keyCode == 9) {
                // tab completion
                var cmdText = $.terminal.splitCommand(term.get_command()); 
        
                var execCmd = commands.filter(function(cmd) {
                    return cmd.cmd.indexOf(cmdText.name) == 0;
                });

                if(execCmd.length > 1) {
                    term.echo(execCmd.map(function(cmd) {
                        return cmd.cmd;
                    }).join(" "));
                } else if(execCmd.length == 1) {
                    if(cmdText.args.length == 0) {
                        term.set_command(execCmd[0].cmd + " ");
                    }

                    var compl = execCmd[0].complete(cmdText.args);

                    if(compl != undefined) {
                        function proc_completes(arr) {
                            if(cmdText.args.length == 0){
                                // print out the "help"
                                term.echo(execCmd[0].cmd + ": " + arr.join(" "));
                            } else if(arr.length == 1) {
                                cmdText.args[cmdText.args.length - 1] = arr[0];

                                term.set_command(execCmd[0].cmd + " " + cmdText.args.join(" "));
                            } else if(arr.length < 10) {
                                term.echo(execCmd[0].cmd + ":");
                                term.echo(arr.join(" "));
                            } else {
                                arr.forEach(function(c) {
                                    term.echo(" " + c);
                                });
                            }
                        }

                        if(typeof compl.then === 'function') {
                            compl.then(proc_completes);
                        } else {
                            proc_completes(compl);
                        }
                    }
                } else {
                    term.echo("Command '" + cmdText.name + "' not found");
                }

                // Tells the terminal to not handle the tab key
                return false;
            }

            //return true;
        }
    });

    log("Welcome to Novodb. Enjoy your debugging experience!");
});

