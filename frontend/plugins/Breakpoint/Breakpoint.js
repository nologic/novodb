load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Breakpoint";

    function Plugin() {}

    Plugin.prototype.attach_ui = make_ui_func(pluginName, ['$scope', '$http', '$compile',
        function ($scope, $http, $compile) {
            _instance.set_session($scope.$parent.session);

            $scope.setBreakpoint = function(session_id, symbol) {
                session.setBreakpoint(symbol, function(data) {
                    $scope.bp_output = JSON.stringify(data);
                });
            };
        }
    ]);

    Plugin.prototype.set_session = function(new_session) {
        session = new_session;
    };

    Plugin.prototype.set_sessions = function(all_sessions) {

    };

    Plugin.prototype.get_plugin_name = function() {
        return pluginName;
    };

    _instance = new Plugin();

    return _instance;
}());