function make_ui_func(pluginName, controllerArray) {
    return function(angular_module) {
        angular_module.compileProvider.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/plugin.html'
            };
        }]);

        angular_module.controllerProvider.register("ndbPlugin" + pluginName, controllerArray);
    }
}

function load_plugin(plugin_spec) {
    plugin_specs.push(plugin_spec);

    plugin_spec.attach_ui(novo);

    log("loaded plugin: " + plugin_spec.get_plugin_name());
}