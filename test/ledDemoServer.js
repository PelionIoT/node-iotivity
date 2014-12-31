// Test OIC stack

// this is the equivalent to the 
// csdk/stack/samples/linux/SimpleClientServer/ledDemoServer.ccp program

var OIC = require('../index.js');

var util = require('util');

var stack = new OIC(	{
		debugOutput: true,
		errorOutput: true
	});



var BogusLed = function() {
	var ledState = false; // aka off

	var uri = "/a/led";
	var typename = "core.led";
	var interfacename = "core.rw";
	var props = OIC.OC_DISCOVERABLE | OIC.OC_OBSERVABLE;

	var handlerCB = function(method,payload,req,responseCB) {
		console.log(" ledDemoServer: handler");
		if(method == 'GET') {
			return { state: ledState };
		} else if (method == 'PUT') {
			if(payload && payload.rep && payload.rep.state) {
				ledState = payload.rep.state;
				console.log(" **** LED: " + uri + " now <"+ledState+">");
				responseCB( {state: ledState } );
			} else {
				console.log(" **** LED: " + uri + " uknown payload.");
				responseCB( {state: ledState } );
			}

//			console.log("payload for LED: " + util.inspect(payload));
		}
	}

	stack.newResource(typename,interfacename,uri,handlerCB,props);
}


var led = new BogusLed();

stack.startServer(function(err){
	if(!err)
		console.log("stack started.");
	else
		console.error("Error starting stack: " + util.inspect(err));
});

