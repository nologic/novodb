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
                    $scope.register_output = data.registers.reduce(function(acc, regset) {
                        regset.values.forEach(function(reg) {
                            acc.push(reg);
                        });

                        return acc;
                    }, []);
                });
            };

            $scope.configRegisters = function(val, index) {
                // preconfigured user selected list of registers.
                var chosen = {
                    eax: true, rax: true, rip: true
                };

                return val.name in chosen;
            };

            $scope.$watch(session.get_stepCount, function(new_step) {
                $scope.readRegisters();
            });

            $scope.readRegisters();
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