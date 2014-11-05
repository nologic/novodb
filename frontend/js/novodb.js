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
                if (partial.indexOf('a') == 0) {
                    // doing the attach.

                    var splitPartial = partial.split(' ');

                    splitPartial.shift();

                    return $http.get('util://list/proc').then(function (data) {
                        return data.data.processes.map(function (proc) {
                            var slashIndex = proc.path.lastIndexOf('/');
                            proc.proc_name = (slashIndex > -1 ? proc.path.substring(slashIndex + 1) : proc.path);

                            return proc;
                        }).filter(function (testProc) {
                            return splitPartial.reduce(function (acc, testVal) {
                                return acc && ((testProc.pid + "").indexOf(testVal) > -1 || testProc.path.indexOf(testVal) > -1);
                            }, true);
                        });
                    });
                } else if (partial.indexOf('l') == 0) {
                    // load by path
                    var splitPartial = partial.split(' ');

                    splitPartial.shift();

                    if(splitPartial.length == 0) {
                        return [];
                    }

                    return $http.get('util://ls', {
                        method: "GET",
                        params: {
                            path: splitPartial[0],
                            maxcount: 100
                        }
                    }).then(function (data) {
                        console.info(JSON.stringify(data, null, "\t"));

                        return data.data.files.map(function (file) {
                            return {
                                proc_name: splitPartial[0] + (splitPartial[0] !== '/'?"/":"") + file.file
                            }
                        });
                    });
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
                output("Executing! attach");
            }
        });

        register_command({
            cmd: "load",
            description: "Load an executable",
            complete: function(params) {
                if(params.length == 0) {
                    return ["[path]"];
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
    $('#cmd_line').keydown(function(e){
        if(e.keyCode == 9) { 
            // tab completion
            var cmdText = $('#cmd_line').val().split(/\s+/); 
    
            var execCmd = commands.filter(function(cmd) {
                return cmd.cmd.indexOf(cmdText[0]) == 0;
            });

            if(execCmd.length > 1) {
                output(execCmd.map(function(cmd) {
                    return cmd.cmd;
                }).join(" "));
            } else if(execCmd.length == 1) {
                if(cmdText.length == 1) {
                    $('#cmd_line').val(execCmd[0].cmd + " ");
                }

                cmdText.shift();
                var compl = execCmd[0].complete(cmdText);

                if(compl != undefined) {
                    function proc_completes(arr) {
                        if(arr.length == 1) {
                            cmdText[cmdText.length - 1] = arr[0];

                            $('#cmd_line').val(execCmd[0].cmd + " " + cmdText.join(" "));
                        } else if(arr.length < 10) {
                            output(execCmd[0].cmd + ": " + arr.join(" "));
                        } else {
                            arr.forEach(function(c) {
                                output(c);
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
                output("Command '" + cmdText[0] + "' not found");
            }

            return e.preventDefault();
        } else if(e.keyCode == 13) {
            // execute command!
            var cmdText = $('#cmd_line').val().split(/\s+/);
    
            var execCmd = commands.filter(function(cmd) {
                return cmd.cmd.indexOf(cmdText[0]) == 0;
            });

            if(execCmd.length > 1) {
                output("Disambiguate: " + execCmd.map(function(cmd) {
                    return cmd.cmd;
                }).join(" "));
            } else if(execCmd.length == 1) {
                $('#cmd_line').val('');
                output(cmdText.join(' '));

                cmdText.shift();
                execCmd[0].execute(cmdText);
            } else {
                output("Command '" + cmdText[0] + "' not found");
            }

            return e.preventDefault();
        }

        return true;
    });

    log("Welcome to Novodb. Enjoy your debugging experience!");
});

