var novo = angular.module('novodb', ['ui.bootstrap']);

novo.controller("dbg", ['$scope', '$http',
	function($scope, $http) {
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
	}
]);

document.onkeyup = function(e) {
    if(e.ctrlKey && e.keyCode == 82) {
        // ctrl+r reload page
        location.reload();
    }
}

$( document ).ready(function() {
    log("Welcome to Novodb. Enjoy your debugging experience!");
});