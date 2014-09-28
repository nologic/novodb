//function format_mem

var novo = angular.module('novodb', []);

novo.controller("dbg", ['$scope', '$http', 
	function($scope, $http) {
		$scope.targetExe = function() {
			$http.get('dbg-llvm://create/target/exe?path=/Users/mike/cef/sample/hellow/a.out').success(function(data) {
				$scope.session_id = data.session;
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
				$scope.gen_output = JSON.stringify(data);
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
				$scope.proc_state = JSON.stringify(data, null, "\t");
			});
		};
	}
]);