var commands = [];

function register_command(cmd) {
    commands.push(cmd);
}

$( document ).ready(function() {
    $('[data-toggle-tp="tooltip"]').tooltip({
        animated: 'fade',
    });

    $('#footer_terminal').terminal(function (command, term) {
        // execute command!
        var cmdText = $.terminal.split_command(command);

        var execCmd = commands.filter(function(cmd) {
            return cmd.cmd == cmdText.name;
        });

        if(execCmd.length > 1) {
            term.echo("Disambiguate: " + execCmd.map(function(cmd) {
                return cmd.cmd;
            }).join(" "));
        } else if(execCmd.length == 1) {
            var execStr = execCmd[0].execute(cmdText.args, window.getCurrentSession());

            if(execStr != undefined) {
                term.echo(execStr);
            }
        } else {
            term.echo("Command '" + cmdText.name + "' not found");
        }
    }, {
            greetings: "[[i;red;]Welcome to Novodb. Enjoy your debugging experience!]",
            exit: false,
            clear: false,
            completion: function(term, command, callback) {
                // tab completion
                var cmdLine = term.get_command();
                var cmdText = $.terminal.split_command(cmdLine);
        
                var execCmd = commands.filter(function(cmd) {
                    return cmd.cmd.indexOf(cmdText.name) == 0;
                });

                if(execCmd.length > 1) {
                    callback(execCmd.map(function(cmd) {
                        return cmd.cmd;
                    }));
                } else if(execCmd.length == 1 && cmdText.name.length != execCmd[0].cmd.length) {
                    // we only have one option, show it and it's quick help.
                    callback([execCmd[0].cmd]);
                    term.echo(execCmd[0].cmd + " " + execCmd[0].quick_help);
                } else if(execCmd.length == 1) {
                    if(cmdLine.charAt(cmdLine.length - 1) === " ") {
                        cmdText.args.push("");
                    }

                    var compl = execCmd[0].complete(cmdLine, cmdText.args, term.cmd().position(), window.getCurrentSession());
                
                    if(typeof compl.then === 'function') {
                        compl.then(callback);
                    } else {
                        callback(compl);
                    }
                }
            }
        });

    window.output = function(txt) {
        $('#footer_terminal').terminal().echo(txt);
    }

    angular.element('#dbg').scope().jQueryLoaded();

    console.info(window.cef_embed);

    if(window.cef_embed == undefined) {
        $('#connect_screen').modal('show');
    }
});

function getStackTrace() {
  var obj = {};
  Error.captureStackTrace(obj, getStackTrace);
  return obj.stack;
};