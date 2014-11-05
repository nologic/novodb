load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Insview";

    function InsView() {}

    InsView.prototype.attach_ui = make_ui_func(pluginName, ['$scope', '$http', '$compile',
        function ($scope, $http, $compile) {
            _instance.set_session($scope.$parent.session);

            $scope.readInstructions = function (_address) {
                session.readInstructions(_address, 20, function(data) {
                    $scope.inst_addr = _address;
                    $scope.inst_output = data;
                });
            };

            $scope.$watch(session.get_currentPc, function(newVal) {
                $scope.readInstructions(newVal.value);
            });
        }
    ]);

    InsView.prototype.set_session = function(new_session) {
        session = new_session;
    };

    InsView.prototype.set_sessions = function(all_sessions) {

    };

    InsView.prototype.get_plugin_name = function() {
        return pluginName;
    };

    _instance = new InsView();

    return _instance;
}());