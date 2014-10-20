load_plugin(function() {
    var _instance = undefined;
    var session = undefined;

    function MemView() {}

    MemView.prototype.attach_ui = function(angular_module) {
        angular_module.directive('ndbPluginMemview', ['$compile', function ($compile) {
            /*return {
             restrict: 'E',
             scope: { 'plugin': '@' },
             link: function(scope, element) {
             var template = '<h1>hello</h1>',
             compiled = $compile(template)(scope);

             element.append(compiled);
             }
             }*/

            return {
                templateUrl: 'plugins/memview/plugin.html'
            };
        }]).controller("ndbPluginMemview", ['$scope', '$http', '$compile',
            function ($scope, $http, $compile) {
                _instance.set_session($scope.$parent.session);

                $scope.readMemory = function (_address) {
                    session.readMemory(_address, 4096, function (data) {
                        $scope.memory_output = data;
                    });
                };
            }
        ]);
    };

    MemView.prototype.set_session = function(new_session) {
        session = new_session;
    };

    MemView.prototype.get_plugin_name = function() {
        return "Memview";
    }

    _instance = new MemView();

    return _instance;
}());