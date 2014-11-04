load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Memview";

    function MemView() {}

    MemView.prototype.attach_ui = make_ui_func(pluginName, ['$scope', '$http', '$compile',
        function ($scope, $http, $compile) {
            console.info("Memview attach_ui");

            _instance.set_session($scope.$parent.session);

            log("config: " + JSON.stringify($scope.$parent.params));

            $scope.readMemory = function (_address) {
                session.readMemory(_address, 4096, function (data) {
                    $scope.memory_output = data;
                });
            };

            $scope.$watch(session.get_stepCount, function(newVal) {
                $scope.readMemory($scope.read_mem_addr);
            });
        }
    ]);

    MemView.prototype.set_session = function(new_session) {
        session = new_session;
    };

    MemView.prototype.set_sessions = function(all_sessions) {

    };

    MemView.prototype.get_plugin_name = function() {
        return pluginName;
    };

    _instance = new MemView();

    return _instance;
}());