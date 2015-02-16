# Novodb - Deep inspection beyond debugging #

Novodb is a platform for building inspection user interfaces. It is particulary aimed at imporving binary analysis. Novodb leverages LLDB to build innovative debugging tools.

There are several major components:

* Novodbweb is a webserver that provides a RESTful interface to LLDB and yara libraries. The server can be used by either Novodbui or the frontend on http://novodb.org
* Nobodbui is a Chrome Embedded Frame work UI that provides a native type debugging UI for LLDB. It uses the same framework that is available on novodb.org. The JavaScript UI support custom plugins for building new analysis tools.
* Novodblib is an underlying library that supports the other binary components mentioned above. The library supports a plugin architecture where developers can build extensions to support other inspection tools. 

## Use cases ##
We aim to support the following use cases.

### Vulnerability analysis

* Ability to overlay partially defined data structures over the memory. 
* Run simulations and provide recording of traces.
* Multiple target debugging

### Capture the flag ###

* Match common patterns of challenges
* Cooler looking UI for browny points at the table

### Memory Forensics ###

* Dynamically analyze what is happening on the host.

## Extensibility ##

To support extensions the architecture support plugins in the following spaces:

* GUI - make it look as you wish with access to the same underlying functionality
* Debugger - provide additional fundamental functionality (i.e. remote agents, etc)
* Utils - provide platform functions (i.e. process listing, etc).
* Analytics - build new analysis tools.