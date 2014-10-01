//function format_mem

var novo = angular.module('novodb', []);

novo.controller("dbg", ['$scope', '$http', 
	function($scope, $http) {
        $scope.targetExe = function(path) {
			$http.get('dbg-llvm://create/target/exe?path=' + path).success(function(data) {
				$scope.session_id = data.session;

                $scope.listSymbols(data.session);
			});
		};

		$scope.launchTarget = function(session_id) {
			$http.get("dbg-llvm://launch?session=" + session_id).success(function(data) {
				$scope.gen_output = JSON.stringify(data);
			});
		};

		$scope.getThreads = function(session_id) {
			$http.get('dbg-llvm://list/threads?session=' + session_id).success(function(data) {
				$scope.thread_list = JSON.stringify(data);
			});
		};

		$scope.setBreakpoint = function(session_id, symbol) {
			$http.get('dbg-llvm://breakpoint/set?session=' + session_id + "&symbol=" + symbol).success(function(data) {
				$scope.bp_output = JSON.stringify(data);
			});
		};

		$scope.listenEvent = function(session_id) {
			$http.get('dbg-llvm://event/listen?session=' + session_id).success(function(data) {
				$scope.event_output = JSON.stringify(data);
			});
		};

		$scope.getModules = function(session_id) {
			$http.get('dbg-llvm://list/modules?session=' + session_id).success(function(data) {
				$scope.module_output = JSON.stringify(data, null, "\t");
			});
		};

		$scope.readMemory = function(session_id, _address) {
			$http.get('dbg-llvm://read/memory', {
				method: "GET",
				params: {
					session: session_id,
					count: 4096,
					address: _address,
					sep: ' '
				}
			}).success(function(data) {
				$scope.memory_output = JSON.stringify(data, null, "\t");
			});
		};

		$scope.procState = function(session_id) {
			$http.get('dbg-llvm://proc/state?session=' + session_id).success(function(data) {
                data.session = session_id;
				$scope.proc_state = JSON.stringify(data, null, "\t");
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
				$scope.register_output = JSON.stringify(data, null, "\t");
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
                $scope.inst_output = data.instructions.map(function(inst) {
                    return inst.description;
                }).join("\n");
			});
		};

		$scope.listSymbols = function(session_id) {
			$http.get('dbg-llvm://list/symbols', {
				method: "GET",
				params: {
					session: session_id,
					module: 0,
				}
			}).success(function(data) {
				$scope.symbols_output = JSON.stringify(data, null, "\t");
			});
		};

        $scope.getFrames = function(session_id, thread_ind) {
            $http.get('dbg-llvm://list/frames', {
                method: "GET",
                params: {
                    session: session_id,
                    thread: thread_ind
                }
            }).success(function(data) {
                $scope.frames_list = JSON.stringify(data, null, "\t");
            });
        };

        $scope.getProcesses = function() {
            $http.get('dbg-llvm://list/proc').success(function(data) {
                $scope.proc_output = data.processes.map(function(proc){
                    return proc.pid + " " + proc.path;
                }).join("\n");
            });
        };

        setInterval(function() {
            if($scope.session_id !== undefined) {
                $scope.procState($scope.session_id);
            }
        }, 1000);
	}
]);