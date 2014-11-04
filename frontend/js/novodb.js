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

$( document ).ready(function() {
    /*document.onkeyup = function(e) {
        if(e.ctrlKey && e.keyCode == 82) {
            // ctrl+r reload page
            location.reload();
        }
    };*/

    log("Welcome to Novodb. Enjoy your debugging experience!");
});

