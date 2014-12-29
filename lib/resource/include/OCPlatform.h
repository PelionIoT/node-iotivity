//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/// @file OCPlatform.h

/// @brief  This file contains the declaration of classes and its members related to
///         OCPlatform.

#ifndef __OCPLATFORM_H
#define __OCPLATFORM_H

#include <map>

#include "OCApi.h"
#include "OCResource.h"
#include "OCPlatformHandler.h"
#include "WrapperFactory.h"
#include "OCResourceRequest.h"
#include "OCResourceResponse.h"
#include "OCRepresentation.h"

namespace OC
{
    /**
    *   @brief  Both server and client must initialize the core platform by instantiating OCPlatform.
    *           On successful initialization, an instance of the OCPlatform is returned.
    *           APIs in OCPlatform provide mechanism to register a resource and host the resource
    *           on the server, find resources on the network etc.
    */
    class OCPlatform
    {
    public:
        // typedef for handle to cancel presence info with
        typedef OCDoHandle OCPresenceHandle;

        /**
        * Constructor for OCPlatform. Constructs a new OCPlatform from a given PlatformConfig with
        * appropriate fields
        * @param config PlatformConfig struct which has details such as modeType (server/client/both),
        *               in-proc/out-of-proc etc.
        */
        OCPlatform(const PlatformConfig& config);

        /**
        * Virtual destructor
        */
        virtual ~OCPlatform(void);

        /**
        * API for notifying core that resource's attributes have changed.
        *
        * @param OCResourceHandle resource handle of the resource
        *
        * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
        * NOTE: This API is for server side only.
        * NOTE: OCResourceHandle is defined in ocstack.h.
        * NOTE: OCStackResult is defined in ocstack.h.
        */
        static OCStackResult notifyObservers(OCResourceHandle resourceHandle);

        /**
        * API for Service and Resource Discovery.
        * NOTE: This API applies to client side only.
        *
        * @param host - Host IP Address of a service to direct resource discovery query. If null or
        *        empty, performs multicast resource discovery query
        * @param resourceURI - name of the resource. If null or empty, performs search for all resource names
        * @param handler - Handles callbacks, success states and failure states.
        *
        *        Four modes of discovery defined as follows:
        *        (NULL/Empty, NULL/Empty) - Performs ALL service discovery AND ALL resource discovery.
        *        (NULL/Empty, Not Empty) - Performs query for a filtered/scoped/particular resource(s)
        *                                  from ALL services.
        *        (Not Empty, NULL/Empty) - Performs ALL resource discovery on a particular service.
        *        (Not Empty, Not Empty) - Performs query for a filtered/scoped/particular resource(s)
        *                                  from a particular service.
        *
        * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
        * NOTE: First parameter 'host' currently represents an IP address. This will change in future
        * and will refer to endpoint interface so that we can refer to other transports such as BTH etc.
        * NOTE: OCStackResult is defined in ocstack.h.
        */
        OCStackResult findResource(const std::string& host, const std::string& resourceURI,
            std::function<void(OCResource::Ptr)> resourceHandler);

        /**
        * This API registers a resource with the server
        * NOTE: This API applies to server side only.
        *
        * @param resourceHandle - Upon successful registration, resourceHandle will be filled
        * @param resourceURI - The URI of the resource. Example: "a/light". See NOTE below
        * @param resourceTypeName - The resource type. Example: "light"
        * @param resourceInterface - The resource interface (whether it is collection etc).
        * @param entityHandler - entity handler callback.
        * @param resourceProperty - indicates the property of the resource. Defined in ocstack.h.
        * setting resourceProperty as OC_DISCOVERABLE will allow Discovery of this resource
        * setting resourceProperty as OC_OBSERVABLE will allow observation
        * settings resourceProperty as OC_DISCOVERABLE | OC_OBSERVABLE will allow both discovery and observation
        *
        * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
        * NOTE: "a/light" is a relative URI.
        * Above relative URI will be prepended (by core) with a host IP + namespace "oc"
        * Therefore, fully qualified URI format would be //HostIP-Address/namespace/relativeURI"
        * Example, a relative URI: 'a/light' will result in a fully qualified URI: //192.168.1.1/oc/a/light"
        * First parameter can take a relative URI and core will take care of preparing the fully qualified URI
        * OR
        * first paramter can take fully qualified URI and core will take that as is for further operations
        * NOTE: OCStackResult is defined in ocstack.h.
        */
        OCStackResult registerResource(OCResourceHandle& resourceHandle,
                        std::string& resourceURI,
                        const std::string& resourceTypeName,
                        const std::string& resourceInterface,
                        std::function<void(const OCResourceRequest::Ptr, const OCResourceResponse::Ptr)> entityHandler,
                        uint8_t resourceProperty);

        /**
        * This API unregisters a resource with the server
        * NOTE: This API applies to server side only.
        *
        * @param resourceHandle - This is the resource handle which we which to unregister from the server
        *
        * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
        * NOTE: OCStackResult is defined in ocstack.h.
        */
        OCStackResult unregisterResource(const OCResourceHandle& resourceHandle) const;

        /**
        * Add a resource to a collection resource.
        *
        * @param collectionHandle - handle to the collection resource
        * @param addedResourceHandle - handle to resource to be added to the collection resource
        *
        * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.<br>
        * NOTE: OCStackResult is defined in ocstack.h. <br>
        * NOTE: bindResource must be used only after the both collection resource and 
        * resource to add under a collections are created and respective handles obtained<br>
        * <b>Example:</b> <br>
        * Step 1: registerResource(homeResourceHandle, "a/home", "home", Link_Interface, entityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 2: registerResource(kitchenResourceHandle, "a/kitchen", "kitchen", Link_Interface, entityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 3: bindResource(homeResourceHandle, kitchenResourceHandle);<br>
        * At the end of Step 3, resource "a/home" will contain a reference to "a/kitchen".<br> 
        */
        OCStackResult bindResource(const OCResourceHandle collectionHandle, const OCResourceHandle resourceHandle);

        /**
        * Add multiple resources to a collection resource.
        *
        * @param collectionHandle - handle to the collection resource
        * @param addedResourceHandleList reference to list of resource handles to be added to the collection resource
        *
        * @return OCStackResult return value of this API. Returns OC_STACK_OK if success. <br>
        * NOTE: OCStackResult is defined in ocstack.h. <br>
        * NOTE: bindResources must be used only after the both collection resource and 
        * list of resources to add under a collection are created and respective handles obtained <br>
        * <b> Example: </b> <br>
        * Step 1: registerResource(homeResourceHandle, "a/home", "home", Link_Interface, homeEntityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 2: registerResource(kitchenResourceHandle, "a/kitchen", "kitchen", Link_Interface, kitchenEntityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 3: registerResource(roomResourceHandle, "a/room", "room", Link_Interface, roomEntityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 4: std::vector<OCResourceHandle> rList; rList.push_back(kitchenResourceHandle); rList.push_back(roomResourceHandle);<br>
        * Step 5: bindResource(homeResourceHandle, rList);<br>
        * At the end of Step 5, resource "a/home" will contain a references to "a/kitchen" and "a/room" <br>
        */
        OCStackResult bindResources(const OCResourceHandle collectionHandle, const std::vector<OCResourceHandle>& addedResourceHandleList);

        /**
        * Unbind a resource from a collection resource.
        *
        * @param collectionHandle - handle to the collection resource
        * @param resourceHandle resource handle to be unbound from the collection resource
        *
        * @return OCStackResult return value of this API. Returns OC_STACK_OK if success. <br>
        * NOTE: OCStackResult is defined in ocstack.h.<br>
        * NOTE: unbindResource must be used only after the both collection resource and 
        * resource to unbind from a collection are created and respective handles obtained<br>
        * <b> Example </b> <br>
        * Step 1: registerResource(homeResourceHandle, "a/home", "home", Link_Interface, entityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 2: registerResource(kitchenResourceHandle, "a/kitchen", "kitchen", Link_Interface, entityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 3: bindResource(homeResourceHandle, kitchenResourceHandle);<br>
        * Step 4: unbindResource(homeResourceHandle, kitchenResourceHandle);<br>
        * At the end of Step 4, resource "a/home" will no longer reference "a/kitchen". <br>
        */
        OCStackResult unbindResource(const OCResourceHandle collectionHandle, const OCResourceHandle resourceHandle);

        /**
        * Unbind resources from a collection resource.
        *
        * @param collectionHandle - handle to the collection resource
        * @param resourceHandleList List of resource handles to be unbound from the collection resource
        *
        * @return OCStackResult return value of this API. Returns OC_STACK_OK if success. <br>
        * 
        * NOTE: OCStackResult is defined in ocstack.h.<br>
        * NOTE: unbindResources must be used only after the both collection resource and 
        * list of resources resource to unbind from a collection are created and respective handles obtained. <br>
        * <b>Example</b> <br>
        * Step 1: registerResource(homeResourceHandle, "a/home", "home", Link_Interface, homeEntityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 2: registerResource(kitchenResourceHandle, "a/kitchen", "kitchen", Link_Interface, kitchenEntityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 3: registerResource(roomResourceHandle, "a/room", "room", Link_Interface, roomEntityHandler, OC_DISCOVERABLE | OC_OBSERVABLE);<br>
        * Step 4: std::vector<OCResourceHandle> rList; rList.push_back(kitchenResourceHandle); rList.push_back(roomResourceHandle);<br>
        * Step 5: bindResource(homeResourceHandle, rList);<br>
        * Step 6: unbindResources(homeResourceHandle, rList);<br>
        * At the end of Step 6, resource "a/home" will no longer reference to "a/kitchen" and "a/room"<br>
        */
        OCStackResult unbindResources(const OCResourceHandle collectionHandle, const std::vector<OCResourceHandle>& resourceHandleList);

        /**
        * Binds a type to a particular resource
        * @param resourceHandle - handle to the resource
        * @param resourceTypeName - new typename to bind to the resource

        * @return OCStackResult - return value of the API. Returns OCSTACK_OK if success <br>
        */
        OCStackResult bindTypeToResource(const OCResourceHandle& resourceHandle,
                        const std::string& resourceTypeName) const;

        /**
        * Binds an interface to a particular resource
        * @param resourceHandle - handle to the resource
        * @param resourceTypeName - new interface  to bind to the resource

        * @return OCStackResult - return value of the API. Returns OCSTACK_OK if success <br>
        */
        OCStackResult bindInterfaceToResource(const OCResourceHandle& resourceHandle,
                        const std::string& resourceInterfaceName) const;

        public:
        /** 
        * Start or stop Presence announcements. 
        * 
        * @param announceDuration - Duration to keep presence duration active.
        * @return OCStackResult - Returns OCSTACK_OK if success <br>
        *
        * These apply only if a server instance is active.
        */
        OCStackResult startPresence(const unsigned int announceDurationSeconds);

        OCStackResult stopPresence();

        /**
        * subscribes to a server's presence change events.  By making this subscription,
        * every time a server adds/removes/alters a resource, starts or is intentionally
        * stopped (potentially more to be added later).
        *
        * @param presenceHandle - a handle object that can be used to identify this subscription
        *               request.  It can be used to unsubscribe from these events in the future. 
        *               It will be set upon successful return of this method.
        * @param host - The IP address/addressable name of the server to subscribe to.
        * @param presenceHandler - callback function that will receive notifications/subscription events
        *
        * @return OCStackResult - return value of the API.  Returns OCSTACK_OK if success <br>
        */
        OCStackResult subscribePresence(OCPresenceHandle& presenceHandle, const std::string& host, 
                        std::function<void(OCStackResult, const int&)> presenceHandler);

        /**
        * unsubscribes from a previously subscribed server's presence events. Note that
        * you may for a short time still receive events from the server since it may take time
        * for the unsubscribe to take effect.
        *
        * @param presenceHandle - the handle object provided by the subscribePresence call that identifies
        *               this subscription.
        *
        * @return OCStackResult - return value of the API.  Returns OCSTACK_OK if success <br>
        */
        OCStackResult unsubscribePresence(OCPresenceHandle presenceHandle);

        /**
        * Creates a resource proxy object so that get/put/observe functionality
        * can be used without discovering the object in advance.  Note that the
        * consumer of this method needs to provide all of the details required to
        * correctly contact and observe the object. If the consumer lacks any of 
        * this information, they should discover the resource object normally. 
        * Additionally, you can only create this object if OCPlatform was initialized
        * to be a Client or Client/Server.  Otherwise, this will return an empty
        * shared ptr.
        *
        * @param host - a string containing a resolvable host address of the server 
        *           holding the resource. Currently this should be in the format 
        *           coap://address:port, though in the future, we expect this to 
        *           change to //address:port
        *
        * @param uri - the rest of the resource's URI that will permit messages to be
        *           properly routed.  Example: /a/light
        *
        * @param isObservable - a boolean containing whether the resource supports observation
        *
        * @param resourceTypes - a collection of resource types implemented by the resource
        *
        * @param interfaces - a collection of interfaces that the resource supports/implements
        * @return OCResource::Ptr - a shared pointer to the new resource object
        */
        OCResource::Ptr constructResourceObject(const std::string& host, const std::string& uri,
                        bool isObservable, const std::vector<std::string>& resourceTypes,
                        const std::vector<std::string>& interfaces);

    private:
        PlatformConfig m_cfg;

    private:
        std::unique_ptr<WrapperFactory> m_WrapperInstance;
        IServerWrapper::Ptr m_server;
        IClientWrapper::Ptr m_client;
        std::shared_ptr<std::mutex> m_csdkLock;

    private:
        /**
        *  Private function to initalize the platfrom
        */
        void init(const PlatformConfig& config);
    };
}

#endif //__OCPLATFORM_H


