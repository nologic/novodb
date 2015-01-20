//var url_prefix = "util://";
//var call_method = "GET";

var url_prefix = "http://localhost:4070/";
var call_method = "JSONP";

function toHexString(decNum) {
	return parseInt(decNum).toString(16);
}

function create_utils($http) {
    function Utils() {}

    function default_err(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function default_success(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function extract_data(func) {
        return function(resp){
            if(func == undefined) {
                return;
            } else if(typeof(func) == "function") {
                return func(resp.data);
            } else {
                // we actually have an array of functions to exec.
                func.forEach(function(f) {
                    if(f != undefined) {
                        f(resp.data);
                    }
                });
            }
        }
    }

    function url_get_passthrough(path, params, f_success, f_fail) {
        var url = undefined;

        if(call_method == "JSONP") {
            url = url_prefix + "util/" + path + "?callback=JSON_CALLBACK";
        } else {
            url = url_prefix + url;
        }

        return $http({
            url: url,
            method: call_method,
            params: params
        }).then(function (resp) {
            if (f_success == undefined) {
                f_success = default_success;
            }
            
            if(call_method == "JSONP") {
                // mask it to make it look like we did a regular GET.
                resp.status = resp.data.code;
                resp.statusText = resp.msg;
                resp.data = resp.data.output;
            }

            return f_success(resp);
        }, function (resp) {
            if (f_fail == undefined) {
                f_fail = default_err;
            }

            if(call_method == "JSONP") {
                resp.status = resp.data.code;
                resp.statusText = resp.msg;
                resp.data = resp.data.output;
            }

            return f_fail(resp);
        });
    }

    Utils.prototype.ls = function(path, startwith, maxcount, f_success, f_fail) {
    	return url_get_passthrough("ls", {
            path: path,
            startwith: startwith,
            maxcount: maxcount
        }, extract_data(f_success), f_fail);
    };

    Utils.prototype.proc_list = function(f_success, f_fail) {
    	return url_get_passthrough("list/proc", {}, extract_data(f_success), f_fail);
    };
    // // end backend functions.

    return new Utils();
}