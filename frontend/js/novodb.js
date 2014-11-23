if (!String.prototype.startsWith) {
  Object.defineProperty(String.prototype, 'startsWith', {
    enumerable: false,
    configurable: false,
    writable: false,
    value: function (searchString, position) {
      position = position || 0;
      return this.lastIndexOf(searchString, position) === position;
    }
  });
}

String.prototype.replaceAt = function(index, character) {
    return this.substr(0, index) + character + this.substr(index+character.length);
}

function load_js_file(filename){
    var fileref = document.createElement('script');

    fileref.setAttribute("type","text/javascript");
    fileref.setAttribute("src", filename);

    if (typeof fileref != "undefined") {
        document.getElementsByTagName("head")[0].appendChild(fileref);
    }
}

window.novo = angular.module('novodb', ['ui.bootstrap']);

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

		$scope.getThreads = function() {
			session.getThreads(function(resp) {
				$scope.thread_list = JSON.stringify(resp);
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

        $scope.plugins = [];
        $scope.instantiatePlugin = function(pluginName, params) {
            $scope.plugins.push({
                name: pluginName,
                session: session,
                params: params
            });
        };

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
                        return data.data.files.map(function (file) {
                            return prefix + file.file + (file.dir == "1"?"/":"");
                        });
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

        $scope.instantiatePlugin('Loader');
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

