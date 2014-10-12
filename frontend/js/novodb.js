//function format_mem

var novo = angular.module('novodb', ['ui.bootstrap']);

novo.controller("dbg", ['$scope', '$http',
	function($scope, $http) {
        var session = null;

        db.create_session(function(_session) {
            session = _session;
        });

        $scope.targetExe = function(path) {
            db.load(path, [], session, function(_session) {
                $scope.listSymbols(_session.id);
            });
        };

        $scope.attachTarget = function(pid) {
            pid = pid.split(" ");
            pid.shift();

            db.attach(pid[0], session, function(_session) {
                $scope.listSymbols(_session.id);
            });
        };

		$scope.launchTarget = function() {
            db.launch(session);
		};

		$scope.getThreads = function() {
			db.getThreads(session, function(data) {
				$scope.thread_list = JSON.stringify(data);
			});
		};

		$scope.setBreakpoint = function(session_id, symbol) {
            db.setBreakpoint(symbol, session, function(data) {
				$scope.bp_output = JSON.stringify(data);
			});
		};

		$scope.getModules = function() {
			db.getModules(session, function(data) {
				$scope.module_output = data;
			});
		};

		$scope.readMemory = function(_address) {
			db.readMemory(_address, 4096, ' ', session, function(data) {
				$scope.memory_output = JSON.stringify(data, null, "\t");
			});
		};

		$scope.procState = function() {
			db.getProcState(session, function(data) {
                data.session = session.id;

                $scope.proc_state = data;
			});
		};

		$scope.readRegisters = function(session_id) {
			$http.get('dbg-llvm://list/registers', {
                method: "GET",
                params: {
                    session: session_id,
                    thread: 0,
                    frame: 0
                }
            }).success(function(data) {
				$scope.register_output = data;
			});
		};

		$scope.readInstructions = function(session_id, _address) {
			$http.get('dbg-llvm://read/instructions', {
				method: "GET",
				params: {
					session: session_id,
					count: 4096,
					address: _address
				}
			}).success(function(data) {
                $scope.inst_output = data;
			});
		};

		$scope.listSymbols = function() {
            db.getSymbols(session, 0, function(data) {
				$scope.symbols_output = data;
			});
		};

        $scope.getFrames = function(thread_ind) {
            db.getFrames(thread_ind, session, function(data) {
                $scope.frames_list = JSON.stringify(data, null, "\t");
            });
        };

        $scope.onSelectPart = function($item, $model, $label) {
            $scope.attachAutocomplete = $item.pid + " " + $item.path;

            $scope.attachTarget($item.pid);
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