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

#ifndef _OUT_OF_PROC_CLIENT_WRAPPER_H_
#define _OUT_OF_PROC_CLIENT_WRAPPER_H_

#include <OCApi.h>

namespace OC
{
    class OutOfProcClientWrapper : public IClientWrapper
    {
    public:
        OutOfProcClientWrapper(std::weak_ptr<std::mutex> csdkLock, PlatformConfig cfg) { }
        
        virtual OCStackResult ListenForResource(const std::string& serviceUrl, const std::string& resourceType, 
            std::function<void(std::shared_ptr<OCResource>)>& callback) {return OC_STACK_NOTIMPL;}

        virtual OCStackResult GetResourceAttributes(const std::string& host, const std::string& uri, 
            const QueryParamsMap& queryParams, std::function<void(const OCRepresentation, const int)>& callback){return OC_STACK_NOTIMPL;}
/*        
        virtual OCStackResult SetResourceAttributes(const std::string& host, const std::string& uri, 
            const AttributeMap& attributes, const QueryParamsMap& queryParams, std::function<void(const AttributeMap,const int)>& callback){return OC_STACK_NOTIMPL;}
 */       
        virtual OCStackResult SetResourceAttributes(const std::string& host, const std::string& uri, 
            const OCRepresentation& attributes, const QueryParamsMap& queryParams, std::function<void(const OCRepresentation,const int)>& callback){return OC_STACK_NOTIMPL;}

        virtual OCStackResult ObserveResource(ObserveType observeType, OCDoHandle* handle, const std::string& host, 
            const std::string& uri, const QueryParamsMap& queryParams, 
            std::function<void(const OCRepresentation&, const int&, const int&)>& callback){return OC_STACK_NOTIMPL;}
        
        virtual OCStackResult CancelObserveResource(OCDoHandle handle, const std::string& host, const std::string& uri){return OC_STACK_NOTIMPL;}

        virtual std::shared_ptr<OCResource> parseOCResource(IClientWrapper::Ptr clientWrapper, const std::string& host, 
            const boost::property_tree::ptree resourceNode) {return nullptr;}

        virtual OCStackResult subscribePresence(OCDoHandle* handle, const std::string& host,
            std::function<void(OCStackResult, const int&)> presenceHandler){return OC_STACK_NOTIMPL;}

        virtual OCStackResult unsubscribePresence(OCDoHandle handle){return OC_STACK_NOTIMPL;}
    };
}

#endif
