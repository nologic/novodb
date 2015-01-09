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

    function url_get_passthrough(url, params, f_success, f_fail) {
        return $http.get(url, {
            method: "GET",
            params: params
        }).then(function (resp) {
            if (f_success == undefined) {
                f_success = default_success;
            }

            return f_success(resp);
        }, function (resp) {
            if (f_fail == undefined) {
                f_fail = default_err;
            }

            return f_fail(resp);
        });
    }

    Utils.prototype.ls = function(path, startwith, maxcount, f_success, f_fail) {
    	return url_get_passthrough("util://ls", {
            path: path,
            startwith: startwith,
            maxcount: maxcount
        }, extract_data(f_success), f_fail);
    };

    Utils.prototype.proc_list = function(f_success, f_fail) {
    	return url_get_passthrough("util://list/proc", {}, extract_data(f_success), f_fail);
    };
    // // end backend functions.

    return new Utils();
}