/*

Plugin JS template:
(function() {
    var pluginName = "Loader"; // Must start with upper and continue with lower cases.

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/loader.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile', 'DTOptionsBuilder', 'DTColumnBuilder',
                function ($scope, $http, $compile, DTOptionsBuilder, DTColumnBuilder) {
                    var session = $scope.$parent.session;

                    // do general angular stuff on per instance basis.
                }
            ]);
        },

        constructor: function() {
            return function() {
                function Plugin() {}

                Plugin.prototype.get_name = function() {
                    return pluginName;
                };

                Plugin.prototype.get_descriptor = function() {
                    return "Process loader";
                };

                return new Plugin();
            }
        }
    };

    register_plugin(pluginSpec);
})();

*/

function load_plugin(plugin_spec) {
    if(window.plugin_specs == undefined) {
        window.plugin_specs = [];
    }

    plugin_specs.push(plugin_spec);

    plugin_spec.attach_ui(novo);

    console.info("loaded plugin: " + plugin_spec.get_plugin_name());
}

function register_plugin(spec) {
    if(window.loaded_plugins == undefined) {
        window.loaded_plugins = {};
    }

    if(spec.name in window.loaded_plugins) {
        console.info("Not loaded; dumplicate name: " + spec.name);
    } else {
        window.loaded_plugins[spec.name] = spec;
        spec.staticInit(window.novo);
    }
}

var EVENT = {
    ipchange: "ip-change"
};

var PLUGIN_TYPE = {
    console: "console",
    windowed: "windowed"
};

function dispatch_event(type, params, plugin_type, plugin_instance) {
    var event = new Event(type);
    event.params = params;

    document.dispatchEvent(event);
}

function listen_event(type, fcn) {
    document.addEventListener(type, fcn);
}