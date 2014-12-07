
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

var wm_state = {
    x_offset: 20,
    y_offset: 100,
    last_x: 0,
    last_y: 0
};

function open_window(element, title) {
    wm_state.last_x = wm_state.x_offset + (wm_state.last_x + 20) % 400;
    wm_state.last_y = wm_state.y_offset + (wm_state.last_y + 20) % 400;

    var win = window.wm.createWindow.fromElement(element, {
        title: title,
        x: wm_state.last_x,
        y: wm_state.last_y,
        width: 400,
        height: 250
    });

    win.open();

    return win;
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