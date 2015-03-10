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

function dispatch_event(type, params, plugin_type, plugin_instance) {
    var event = new Event(type);
    event.params = params;

    document.dispatchEvent(event);
}

function listen_event(type, fcn) {
    document.addEventListener(type, fcn);
}

var apis = {};

function register_api(name, functions) {
    apis[name] = functions;
}

function get_api(name) {
    return apis[name];
}