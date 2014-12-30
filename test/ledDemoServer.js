// Test OIC stack

// this is the equivalent to the 
// csdk/stack/samples/linux/SimpleClientServer/ledDemoServer.ccp program

var OIC = require('../index.js');

var util = require('util');

var stack = new OIC();

var uri = "/a/led";
var typename = "core.led";
var interfacename = "core.rw";
var props = OIC.OC_DISCOVERABLE | OIC.OC_OBSERVABLE;
var handlerCB = function(resource) {

}



stack.newResource(typename,interfacename,uri,handlerCB,props);


stack.startServer(function(err){
	if(!err)
		console.log("stack started.");
	else
		console.error("Error starting stack: " + util.inspect(err));
});

