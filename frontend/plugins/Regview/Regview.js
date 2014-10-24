load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Regview";

    function MemView() {}

    MemView.prototype.attach_ui = make_ui_func(pluginName, ['$scope', '$http', '$compile',
        function ($scope, $http, $compile) {
            _instance.set_session($scope.$parent.session);

            $scope.readRegisters = function() {
                session.readRegisters(0, 0, function(data) {
                    $scope.register_output = data;
                });
            };
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

    MemView.prototype.refresh = function() {

    };

    _instance = new MemView();

    return _instance;
}());