
// let's not use this any more!
function make_ui_func(pluginName, controllerArray) {
    return function(angular_module) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/plugin.html'
            };
        }]).controller("ndbPlugin" + pluginName, controllerArray);
    }
}

function load_plugin(plugin_spec) {
    if(window.plugin_specs == undefined) {
        window.plugin_specs = [];
    }

    plugin_specs.push(plugin_spec);

    plugin_spec.attach_ui(novo);

    console.info("loaded plugin: " + plugin_spec.get_plugin_name());
}

var EVENT = {
    ipchange: "ip-change"
};

function dispatch_event(type, params, plugin_type, plugin_instance) {
    var event = new Event(type);
    event.params = params;

    document.dispatchEvent(event);
}

function listen_event(type, fcn) {
    document.addEventListener(type, fcn);
}