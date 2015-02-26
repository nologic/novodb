(function() {
    var pluginName = "Regview";

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/regview.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
                function ($scope, $http, $compile) {
                    var session = $scope.$parent.session;

                    // defaults
                    $scope.show_defaults = {
                        'x86_64': ['rip', 'rax', 'rbx', 'rcx', 'rdx', 'rdi', 'rsi', 'rbp', 'rsp'],
                        'armv7s': ['pc', 'sp']
                    };

                    $scope.reg_props = {};
                    $scope.showregs = [];

                    $scope.arch = session.get_architecture();
                    if($scope.arch in $scope.show_defaults) {
                        $scope.showregs = $scope.show_defaults[$scope.arch];
                    } else {
                        log("no register detaults for architecture: " + $scope.arch);
                    }

                    $scope.readRegisters = function() {
                        session.readRegisters(0, 0, function(data) {
                            $scope.reg_props = data.registers.reduce(function(acc, regset) {
                                regset.values.reduce(function(acc2, reg) {
                                    acc2[reg.name] = reg.value;

                                    return acc2;
                                }, acc);

                                return acc;
                            }, {});

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

                    var rem = $scope.$watch(session.get_stepCount, function(new_step) {
                        $scope.readRegisters();
                    });

                    session.add_destroy_listener(rem);

                    $scope.readRegisters();
                }
            ]).filter('selectedRegs', function() {
                return function(regs, tags) {
                    if(regs == undefined) {
                        return [];
                    } else {
                        return regs.filter(function(reg) {
                            if (tags.indexOf(reg.name) != -1) {
                                return false;
                            }

                            return true;

                        });
                    }
                };
            });
        },

        constructor: function() {
            return function() {
                function Plugin() {}

                Plugin.prototype.get_name = function() {
                    return pluginName;
                };

                Plugin.prototype.get_descriptor = function() {
                    return "Register Viewer";
                };

                return new Plugin();
            }
        }
    };

    register_plugin(pluginSpec);
})();