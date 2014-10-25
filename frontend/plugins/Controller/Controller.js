load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Controller";

    function Plugin() {}

    Plugin.prototype.attach_ui = make_ui_func(pluginName, ['$scope', '$http', '$compile',
        function ($scope, $http, $compile) {
            _instance.set_session($scope.$parent.session);

            $scope.step = function (thread_num) {
                session.step(thread_num, function(data){
                    dispatch_event(EVENT.ipchange);
                });
            };

            $scope.stepOver = function (thread_num) {
                log("this one doesn't work")
            };

            $scope.resume = function (thread_num) {
                session.resume(thread_num, function(data){
                    // refresh all.
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