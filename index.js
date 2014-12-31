
var util = require('util');
var COAP = require('./lib/node-coap');
var _ = require('./lib/underscore.js');
var ordererdTable = require('./lib/orderedTable.js');
var lookupTreeRegex = require('./lib/lookupTreeRegex');

var defaultOpts = {
    port: 5683,        // default COAP / OIC port
    debugOutput: false, // debug output to console 
    errorOutput: false, // output errors?
    debugOut : function() {
        var args = Array.prototype.slice.call(arguments);
        args.unshift('<OIC debug> ');
        console.log.apply(console,args);
    },
    errorOut: function() {
        var args = Array.prototype.slice.call(arguments);
        args.unshift('<OIC error> ');
        console.error.apply(console,args);
    }

}



/**
 * Instance of an OIC Stack
 */
var OIC = function(stackopts) {
    var stack = this;
	var Resources = new ordererdTable(); // list of OICResources
	var hrefTree = new lookupTreeRegex();

	var Interfaces = []; // so we can bind multicast to an address... b/c we listen on multicast

    var opts = stackopts || {};
    _.defaults(opts, defaultOpts);

    var dOut = function() { if(opts.debugOutput) opts.debugOut.apply(undefined,arguments); }
    var eOut = function() { if(opts.errorOutput) opts.errorOut.apply(undefined,arguments); }

    var OICResourceDefaults = {
        _uri: "NOT_SET",
        _rt:  "NOT_SET",
        _if:  "NOT_SET",
        props: 0
    };


	/**
	 * Equates to the OICResource object in C library. Analagous to the OCCreateResource() in csdk/stack/src/ocstack.c
	 * @param {[type]} owner The owning OIC stack instance
	 */
	var OICResource = function(owner,typename,interfacename,uri,handler,flags) {
        var resource = this;
        var defaultResourceCB = function() {
            dOut("default resource handler: " + this.props._uri);
        }

		var context = null;
		var sequenceNum = null;
        this.handlerCB = defaultResourceCB;
        if(handler) this.handlerCB = handler; 

        this.props = {
            _uri: uri,
            _rt:  typename,
            _if:  interfacename,
            flags: flags
        }
        _.defaults(this.props,OICResourceDefaults);

	// FROM:  csdk/stack/include/internal/ocstackinternal.h
	// 
    // struct rsrc_t *next; // Points to next resource in list
    // // Relative path on the device; will be combined with base url to create fully qualified path
    // char *uri;
    // OCResourceType *rsrcType; // Resource type(s); linked list
    // OCResourceInterface *rsrcInterface; // Resource interface(s); linked list
    // OCAttribute *rsrcAttributes; // Resource interface(s); linked list
    // // Array of pointers to resources; can be used to represent a container of resources
    // // (i.e. hierarchies of resources) or for reference resources (i.e. for a resource collection)
    // struct rsrc_t *rsrcResources[MAX_CONTAINED_RESOURCES];
    // //struct rsrc_t *rsrcResources;
    // // Pointer to function that handles the entity bound to the resource.
    // // This handler has to be explicitly defined by the programmer
    // OCEntityHandler entityHandler;
    // // Properties on the resource – defines meta information on the resource
    // OCResourceProperty resourceProperties ; /* ACTIVE, DISCOVERABLE etc */
    // // Pointer to an opaque object where app/user specific data can be placed with the resource;
    // // this could be information for the entity handler between invocations
    // void *context;
    // // NOTE: Methods supported by this resource should be based on the interface targeted
    // // i.e. look into the interface structure based on the query request Can be removed here; place holder for the note above
    // /* method_t methods; */
    // // Sequence number for observable resources. Per the CoAP standard it is a 24 bit value.
    // uint32_t sequenceNum;


        /** 
         * Make a CoaP
         */
        this.toPayloadObj = function() {
            //Example payload:
            //{"oc":
            //  //[ 
            //  { "href":"/a/led", // start here
            //        "prop":{
            //           "rt":["core.led"],
            //           "if":["core.rw"],
            //           "obs":1
            //          }
            //       }             // end here
            // ]}
            
            var ret = {
                href : resource.props._uri,
                prop : {
                    "rt" : [ resource.props._rt ],
                    "if" : [ resource.props._if ],
                    obs: (resource.props.flags & OIC.OC_OBSERVABLE) ? 1 : 0
                }
            };

            return ret;
        }
	}

    /**
     * Returns a handle
     * @param  {[type]} typename      [description]
     * @param  {[type]} interfacename [description]
     * @param  {string|regex} uri     A String or Regex for a URI which           
     * @param  {[type]} handlerCB     [description]
     * @param  {[type]} props         [description]
     * @return {[type]}               [description]
     */
    this.newResource = function(typename,interfacename,uri,handlerCB,props) {
        var res = new OICResource(stack,typename,interfacename,uri,handlerCB,props);
        var h = Resources.add(res);
        hrefTree.add(uri,h); // TODO make this more efficient for larger sets by splitting into multiple strings.
    };

    /**
     * remove a resource from the stack
     * @param  {[type]} handle The handle of the resource (not the resource itself)
     * @return {[type]}        [description]
     */
    this.removeResource = function(handle) {
        Resources.remove(handle);
    };





    // Protocol functions
    // 
    var getResponse_DiscoverResources = function() {
        var container = {
            oc: []
        };

        Resources.forEach(function(r){
            container.oc.push(r.toPayloadObj());
        });

        return JSON.stringify(container);
    }

    var parsePUTPayload = function(buf) {
        dOut("raw payload <"+ util.inspect(buf)+">");
        var s = buf.toString();
        dOut("PUT parse: <"+s+">");
        var ret = null;
        if(s) {
//            s = s + "";
//            s = s.replace(/[\r\t\n]/g,"");
              s = s.replace(/\\n/g, "\\n")  // preserve these if they are really there
               .replace(/\\'/g, "\\'")
               .replace(/\\"/g, '\\"')
               .replace(/\\&/g, "\\&")
               .replace(/\\r/g, "\\r")
               .replace(/\\t/g, "\\t")
               .replace(/\\b/g, "\\b")
               .replace(/\\f/g, "\\f");
            s = s.replace(/[\u0000-\u0019]+/g,""); // remove non-printable and other non-valid JSON chars which seems to be coming from the Intel Android app
//            s = s.replace(/(['"])?([a-zA-Z0-9_]+)(['"])?:/g, '"$2":');
//            s = s.slice(0);
            dOut("cleaned: <"+s+">");
            try {
                ret = JSON.parse(s);

//                ret = JSON.parse('{"oc":[{"rep":{"state":"true"}}]}');

            } catch (e) {
                eOut(" JSON parse error: " + util.inspect(e));
                return null;
            }
            if(ret && ret.oc && ret.oc[0]) {
                return ret.oc[0];
            } else {
                eOut(" Malformed OIC payload.");
                return null;
            }
        } else 
            return null;
    }

	var server;

    // public:



    /**
     * Starts the stack
     * @return {[type]} [description]
     */
	this.startServer = function() {
		server = COAP.createServer({
            addMembership: [ '224.0.1.187' ]
        });

		server.on('request', function(req, res) {

            console.dir(arguments);
            console.log("-- Request for: " + req.url);
            if(req.url == OIC.OC_WELL_KNOWN_URI_str) {
                res.end(getResponse_DiscoverResources());
            } else {
                dOut("got: " +req.method);
                var container = {
                    oc:[]
                };

                var h = hrefTree.lookup(req.url);
                var r = null;
                if(h) {
                    dOut("Resource found: " + req.url);
                    r = Resources.lookup(h);
                } else {
                    dOut("Resource not found: " + req.url);
                }
                if(req.method == 'GET') {
                    if(r) {
                        var retobj = {href: req.url, rep: {} };
                        var result = r.handlerCB.call(r,req.method,null,req);
                        if(result !== undefined) {
                            dOut("have result from cb: " + util.inspect(result));
                            retobj.rep = result;
                        }
                        container.oc.push(retobj);
                    } else {
                        // else {
                        //   TODO - need to return a 'not available'
                        // }
                    }
                    var json_res = JSON.stringify(container);
                    dOut("resp: " + json_res);
                    res.end(json_res);                    
                } else if(req.method == 'PUT') {
                    if(r) {
                        (function(cb,method,req,res,resource){
                            var payload = parsePUTPayload(req.payload);
                            if(!payload) {
                                return;
                            }
                            dOut(" PUT payload: " + util.inspect(payload));
                            var responseCB = function(result) {
                                var retobj = {
                                    //href: req.url,  // we don't do this in the response for PUT (at least according to Intel's demo app)
                                    rep: {} };
                                if(result !== undefined) {
                                    dOut("have result from responseCB: " + util.inspect(result));
                                    retobj.rep = result;
                                }
                                container.oc.push(retobj);
                                var json_res = JSON.stringify(container);
                                dOut("resp: " + json_res);
                                res.end(json_res);                    
                            }
                            cb.call(resource,method,payload,req,responseCB);
                        })(r.handlerCB,req.method,req,res,r);
                    } else {
                        var json_res = JSON.stringify(container);
                        dOut("resp: " + json_res);
                        res.end(json_res);                                            
                    }
                } else {
                    eOut("Unsupported method <"+req.method+">");
                }

            }


//			res.end('Hello Node ' + req.url.split('/')[1] + '\n')
		});

       	// the default CoAP port is 5683
       	server.listen(function() {
       		var req = COAP.request('coap://[::1]/Matteo');

       		req.on('response', function(res) {
       			res.pipe(process.stdout);
       			res.on('end', function() {
       				process.exit(0);
       			});
       		});

       		req.end();
       	});
    };

    
    this.stopServer = function() {

    };


};

// FROM: csdk/stack/include/ocstack.h
	
// typedef enum {
//     OC_WELL_KNOWN_URI= 0,       // "/oc/core"
//     OC_DEVICE_URI,              // "/oc/core/d"
//     OC_RESOURCE_TYPES_URI,      // "/oc/core/d/type"
//     #ifdef WITH_PRESENCE
//     OC_PRESENCE,                // "/oc/presence"
//     #endif
//     OC_MAX_VIRTUAL_RESOURCES    // Max items in the list
// } OCVirtualResources;

// /**
//  * Standard RESTful HTTP Methods
//  */
// typedef enum {
//     OC_REST_NOMETHOD    = 0,
//     OC_REST_GET         = (1 << 0),     // Read
//     OC_REST_PUT         = (1 << 1),     // Write
//     OC_REST_POST        = (1 << 2),     // Update
//     OC_REST_DELETE      = (1 << 3),     // Delete
//     OC_REST_OBSERVE     = (1 << 4),     // Register observe request for most up date notifications ONLY.
//     OC_REST_OBSERVE_ALL = (1 << 5),     // Register observe request for all notifications, including stale notifications.
//     #ifdef WITH_PRESENCE
//     OC_REST_PRESENCE    = (1 << 6)      // Subscribe for all presence notifications of a particular resource.
//     #endif
// } OCMethod;

// /**
//  * Host Mode of Operation
//  */
// typedef enum {
//     OC_CLIENT = 0,
//     OC_SERVER,
//     OC_CLIENT_SERVER
// } OCMode;

// extern OCMode myStackMode;
// /**
//  * Quality of Service
//  */
// typedef enum {
//     OC_NON_CONFIRMABLE = 0,
//     OC_CONFIRMABLE
// } OCQualityOfService;

// /**
//  * Resource Properties
//  */
// typedef enum {
//     OC_ACTIVE       = (1 << 0),  // set == initialized; unset == deleted
//     OC_DISCOVERABLE = (1 << 1),  // Discovery of this resource allowed
//     OC_OBSERVABLE   = (1 << 2),  // Observe allowed
//     OC_SLOW         = (1 << 3)   // Expect delay in processing requests. Send ACK to request and send response later
// } OCResourceProperty;

// /**
//  * Declares Stack Results & Errors
//  */
// typedef enum {
//     OC_STACK_OK = 0,
//     OC_STACK_INVALID_URI,
//     OC_STACK_INVALID_QUERY,
//     OC_STACK_INVALID_IP,
//     OC_STACK_INVALID_PORT,
//     OC_STACK_INVALID_CALLBACK,
//     OC_STACK_INVALID_METHOD,
//     OC_STACK_INVALID_PARAM,
//     OC_STACK_INVALID_OBSERVE_PARAM,
//     OC_STACK_NO_MEMORY,
//     OC_STACK_COMM_ERROR,
//     OC_STACK_NOTIMPL,
//     OC_STACK_NO_RESOURCE, /* resource not found*/
//     OC_STACK_RESOURCE_ERROR, /*ex: not supported method or interface*/
//     OC_STACK_SLOW_RESOURCE,
//     OC_STACK_NO_OBSERVERS, /* resource has no registered observers */
//     OC_STACK_OBSERVER_NOT_FOUND,
//     OC_STACK_OBSERVER_NOT_ADDED,
//     OC_STACK_OBSERVER_NOT_REMOVED,
//     #ifdef WITH_PRESENCE
//     OC_STACK_PRESENCE_NO_UPDATE,
//     OC_STACK_PRESENCE_STOPPED,
//     OC_STACK_PRESENCE__NOT_HANDLE,
//     #endif
//     OC_STACK_ERROR
// } OCStackResult;

// /**
//  * Incoming requests handled by the server. Requests are passed in as a parameter to the @ref OCEntityHandler callback API.
//  * @brief The @ref OCEntityHandler callback API must be implemented in the application in order to receive these requests.
//  */
// typedef struct {
//     // Associated resource
//     OCResourceHandle resource;
//     // resource query send by client
//     unsigned char * query;
//     // the REST method retrieved from received request PDU
//     OCMethod method;
//     // reqJSON is retrieved from the payload of the received request PDU
//     unsigned const char * reqJSONPayload;
//     // resJSON is allocated in the stack and will be used by entity handler to fill in its response
//     unsigned char * resJSONPayload;
//     // Length of maximum allowed response
//     uint16_t resJSONPayloadLen;
// }OCEntityHandlerRequest;

// /**
//  * Response from queries to remote servers. Queries are made by calling the @ref OCDoResource API.
//  */
// typedef struct {
//     // the is the result of our stack, OCStackResult should contain coap/other error codes;
//     OCStackResult result;
//     // Address of remote server
//     OCDevAddr * addr;
//     // If associated with observe, this will represent the sequence of notifications from server.
//     uint32_t sequenceNumber;
//     // resJSONPayload is retrieved from the payload of the received request PDU
//     unsigned  const char * resJSONPayload;
// }OCClientResponse;

// typedef enum {
//     OC_INIT_FLAG    = (1 << 0),
//     OC_REQUEST_FLAG = (1 << 1),
//     OC_OBSERVE_FLAG = (1 << 2)
// } OCEntityHandlerFlag; //entity_handler_flag_t ;

// // possible returned values from client application
// typedef enum {
//     OC_STACK_DELETE_TRANSACTION = 0,
//     OC_STACK_KEEP_TRANSACTION
// } OCStackApplicationResult;

// //-----------------------------------------------------------------------------
// // Callback function definitions
// //-----------------------------------------------------------------------------

// /**
//  * Client applications implement this callback to consume responses received from Servers.
//  */
// typedef OCStackApplicationResult (* OCClientResponseHandler)(void *context, OCDoHandle handle, OCClientResponse * clientResponse);


// /*
//  * This info is passed from application to OC Stack when initiating a request to Server
//  */
// typedef struct {
//     void *context;
//     OCClientResponseHandler cb;
// } OCCallbackData;

// /**
//  * Possible returned values from entity handler
//  */
// typedef enum {
//     OC_EH_OK = 0,
//     OC_EH_ERROR
// } OCEntityHandlerResult;


// FROM: csdk/stack/include/ocstackconfig.h

// #ifndef OCSTACK_CONFIG_H_
// #define OCSTACK_CONFIG_H_

// // This file contains all the variables which can be configured/modified as
// // per platform or specific product usage scenarios.


// /**
//  * Maximum length of the response supported by Server for any REST request.
//  */
// #ifdef WITH_ARDUINO
// #define MAX_RESPONSE_LENGTH (256)
// #else
// #define MAX_RESPONSE_LENGTH (1024)
// #endif

// /**
//  * Maximum length of the request supported by Client/Server for any REST request.
//  */
// #ifdef WITH_ARDUINO
// #define MAX_REQUEST_LENGTH (256)
// #else
// #define MAX_REQUEST_LENGTH (1024)
// #endif

// /**
//  * Maximum length of the URI supported by client/server while processing
//  * REST requests/responses.
//  */
// #define MAX_URI_LENGTH (64)

// /**
//  * Maximum length of the query supported by client/server while processing
//  * REST requests/responses.
//  */
// #define MAX_QUERY_LENGTH (64)

// /**
//  * Maximum number of resources which can be contained inside collection
//  * resource.
//  */
// #define MAX_CONTAINED_RESOURCES  (5)

// #endif //OCSTACK_CONFIG_H_
// #



// OIC.OCVirtualResources = {
OIC.OC_WELL_KNOWN_URI =0;       // "/oc/core"
OIC.OC_DEVICE_URI = 1;              // "/oc/core/d"
OIC.OC_RESOURCE_TYPES_URI = 2;      // "/oc/core/d/type"
//    #ifdef WITH_PRESENCE
OIC.OC_PRESENCE = 3;                // "/oc/presence"
//    #endif
OIC.OC_MAX_VIRTUAL_RESOURCES = 4    // Max items in the list
// };

OIC.OC_WELL_KNOWN_URI_str = "/oc/core";
OIC.OC_DEVICE_URI_str = "/oc/core/d";
OIC.OC_RESOURCE_TYPES_URI_str = "/oc/core/d/type";
OIC.OC_PRESENCE_str = "/oc/presence";



// OIC.OCMethod = {
OIC.OC_REST_NOMETHOD    = 0;
OIC.OC_REST_GET         = (1 << 0);     // Read
OIC.OC_REST_PUT         = (1 << 1);     // Write
OIC.OC_REST_POST        = (1 << 2);     // Update
OIC.OC_REST_DELETE      = (1 << 3);     // Delete
OIC.OC_REST_OBSERVE     = (1 << 4);     // Register observe request for most up date notifications ONLY.
OIC.OC_REST_OBSERVE_ALL = (1 << 5);     // Register observe request for all notifications; including stale notifications.
//    #ifdef WITH_PRESENCE
OIC.OC_REST_PRESENCE    = (1 << 6);      // Subscribe for all presence notifications of a particular resource.
//    #endif
// };

//OIC.OCMode = {
OIC.OC_CLIENT = 0;
OIC.OC_SERVER = 1;
OIC.OC_CLIENT_SERVER = 2;
//};

//OIC.OCQualityOfService = {
OIC.OC_NON_CONFIRMABLE = 0;
OIC.OC_CONFIRMABLE = 1;
//};


//OIC.OCResourceProperty = {
OIC.OC_ACTIVE       = (1 << 0);  // set == initialized; unset == deleted
OIC.OC_DISCOVERABLE = (1 << 1);  // Discovery of this resource allowed
OIC.OC_OBSERVABLE   = (1 << 2);  // Observe allowed
OIC.OC_SLOW         = (1 << 3);   // Expect delay in processing requests. Send ACK to request and send response later
//};

//OIC.StackResult = {
OIC.OC_STACK_OK= 0;
OIC.OC_STACK_INVALID_URI= 1;
OIC.OC_STACK_INVALID_QUERY= 2;
OIC.OC_STACK_INVALID_IP= 3;
OIC.OC_STACK_INVALID_PORT= 4;
OIC.OC_STACK_INVALID_CALLBACK= 5;
OIC.OC_STACK_INVALID_METHOD= 6;
OIC.OC_STACK_INVALID_PARAM= 7;
OIC.OC_STACK_INVALID_OBSERVE_PARAM= 8;
OIC.OC_STACK_NO_MEMORY= 9;
OIC.OC_STACK_COMM_ERROR= 10;
OIC.OC_STACK_NOTIMPL= 11;
OIC.OC_STACK_NO_RESOURCE= 12; /* resource not found*/
OIC.OC_STACK_RESOURCE_ERROR= 13; /*ex: not supported method or interface*/
OIC.OC_STACK_SLOW_RESOURCE= 14;
OIC.OC_STACK_NO_OBSERVERS= 15; /* resource has no registered observers */
OIC.OC_STACK_OBSERVER_NOT_FOUND= 16;
OIC.OC_STACK_OBSERVER_NOT_ADDED= 17;
OIC.OC_STACK_OBSERVER_NOT_REMOVED= 18;
//    #ifdef WITH_PRESENCE                 // FIXME - we need to deal with this... is this turned on or not?
OIC.OC_STACK_PRESENCE_NO_UPDATE= 19;
OIC.OC_STACK_PRESENCE_STOPPED= 20;
OIC.OC_STACK_PRESENCE_DO_NOT_HANDLE= 21;
//    #endif
OIC.OC_STACK_ERROR= 22;
//};

//OIC.OCEntityHandlerFlag = {
OIC.OC_INIT_FLAG    = (1 << 0);
OIC.OC_REQUEST_FLAG = (1 << 1);
OIC.OC_OBSERVE_FLAG = (1 << 2);
//};

//OIC.OCStackApplicationResult = {
OIC.OC_STACK_DELETE_TRANSACTION = 0;
OIC.OC_STACK_KEEP_TRANSACTION = 1;
//};

//OIC.OCEntityHandlerResult = {
OIC.OC_EH_OK = 0;
OIC.OC_EH_ERROR = 1;
//};


// some more constants=
OIC.MAX_RESPONSE_LENGTH = 1024;
OIC.MAX_RESPONSE_LENGTH_ARDUINO = 256;

OIC.MAX_REQUEST_LENGTH = 1024;
OIC.MAX_REQUEST_LENGTH_ARDUINO = 256;

OIC.MAX_URI_LENGTH = 64;
OIC.MAX_QUERY_LENGTH = 64;

OIC.MAX_CONTAINED_RESOURCES = 5;


// export to node
module.exports = OIC;

