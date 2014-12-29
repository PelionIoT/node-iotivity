#include "light_resource.hpp"

namespace Intel { namespace OCDemo {

std::atomic<bool> LightResource::shutdown_flag(false);
std::thread LightResource::observe_thread;

void LightResource::setRepresentation(AttributeMap& attributeMap)
{
 cout << "\t\t\t" << "Received representation: " << endl;
 cout << "\t\t\t\t" << "power: " << attributeMap["power"][0] << endl;
 cout << "\t\t\t\t" << "state: " << attributeMap["state"][0] << endl;

 m_state = attributeMap["state"][0].compare("true") == 0;
 m_power = std::stoi(attributeMap["power"][0]);
}

void LightResource::getRepresentation(AttributeMap& attributeMap) const
{
 attributeMap["state"] = { (m_state ? "true" : "false") };
 attributeMap["power"] = { to_string(m_power) };
}

void LightResource::addType(const OC::OCPlatform& platform, const std::string& type) const
{
 OCStackResult result = platform.bindTypeToResource(m_resourceHandle, type);
 
 if(OC_STACK_OK != result)
  cout << "Binding TypeName to Resource was unsuccessful, result was " << result << '\n';
}

void LightResource::addInterface(const OC::OCPlatform& platform, const std::string& interface) const
{
 OCStackResult result = platform.bindInterfaceToResource(m_resourceHandle, interface);

 if(OC_STACK_OK != result)
  cout << "Binding TypeName to Resource was unsuccessful, result was " << result << '\n';
}

void LightResource::createResource(OC::OCPlatform& platform, const unsigned int resource_number)
{
 string resourceURI      { make_URI(resource_number) };
 string resourceTypeName { "core.light" };

 cout << "registering resource: " << resourceURI << '\n';
 cout << "registering type name \"" << resourceTypeName << "\".\n";
 // This will internally create and register the resource, binding the current instance's method as a callback:
 OCStackResult result = platform.registerResource(
                                            m_resourceHandle, resourceURI, resourceTypeName,
                                            DEFAULT_INTERFACE, 
                                            std::bind(&LightResource::entityHandler, this, std::placeholders::_1, std::placeholders::_2), 
                                            OC_DISCOVERABLE | OC_OBSERVABLE);
  if (OC_STACK_OK != result)
   std::cout << "Resource creation failed.\n";
}

void LightResource::observe_function()
{
 cerr << "Observation thread is spinning up.\n";

 while(!shutdown_flag)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));

    if(!m_observation)
     continue;

    m_power += 10;

    const auto result = OCPlatform::notifyObservers(getHandle());

    // Stop notifications when there are no more observers:
    if(OC_STACK_NO_OBSERVERS == result)
     {
        m_observation = 0;
     }
  }

 cerr << "Observation thread is shutting down.\n";
}

void LightResource::unregisterResource(OC::OCPlatform& platform)
{
    std::cout << "Unregistering light resource"<<std::endl;

    OCStackResult result = platform.unregisterResource(m_resourceHandle);

    if(result == OC_STACK_OK)
    {
        std::cout << "Resource unregistered."<<std::endl;
    }
    else
    {
        cerr << "Unregister resource failed: "<<std::endl;
    }
}

// This is just a sample implementation of entity handler.
// Entity handler can be implemented in several ways by the manufacturer
void LightResource::entityHandler(std::shared_ptr<OCResourceRequest> request, std::shared_ptr<OCResourceResponse> response)
{
    if(!request)
     {
        cerr << "entityHandler(): Received invalid request object.\n";
        return;
     }

    if(!response)
     {
        cerr << "entityHandler(): Received invalid response object.\n";
        return;
     }

    switch(request->getRequestHandlerFlag())   
    {
        default:
                cerr << "entityHandler(): invalid request flag\n";
                break;

        case RequestHandlerFlag::InitFlag:
                cerr << "entityHandler(): Initialization requested.\n";
                break;

        case RequestHandlerFlag::RequestFlag:
                dispatch_request(request->getRequestType(), request, response);
                break;

        case RequestHandlerFlag::ObserverFlag:
                handle_observe_event(request, response);
                break;
    }
}

void LightResource::dispatch_request(const std::string& request_type, std::shared_ptr<OCResourceRequest> request, std::shared_ptr<OCResourceResponse> response) 
{
 std::cout << "dispatch_request(): " << request_type << '\n';

 if("GET" == request_type)
  return handle_get_request(request, response);

 if("PUT" == request_type)
  return handle_put_request(request, response);

 if("POST" == request_type)
  return handle_post_request(request, response);

 if("DELETE" == request_type)
  return handle_delete_request(request, response);

 cerr << "entityHandler(): Invalid request type \"" << request_type << "\".\n";
}

void LightResource::handle_get_request(std::shared_ptr<OCResourceRequest> request, std::shared_ptr<OCResourceResponse> response) 
{
 cout << "handle_get_request():\n";

 const auto query_params_map = request->getQueryParameters();

 // ...do any processing of the query here...

 // Get a representation of the resource and send it back as a response:
 AttributeMap attribute_map;

 getRepresentation(attribute_map);

 response->setErrorCode(200);
 response->setResourceRepresentation(attribute_map);
}

void LightResource::handle_put_request(std::shared_ptr<OCResourceRequest> request, std::shared_ptr<OCResourceResponse> response) 
{
 // Here's how you would get any query parameters:
 const auto query_params_map = request->getQueryParameters();
 // ...do something with the query parameters (if there were any)...

 auto attribute_map = request->getAttributeRepresentation();

 setRepresentation(attribute_map);
 getRepresentation(attribute_map);  // in case we changed something

 if(!response)
  return;

 response->setErrorCode(200);
 response->setResourceRepresentation(attribute_map); 
}

void LightResource::handle_post_request(std::shared_ptr<OCResourceRequest> request, std::shared_ptr<OCResourceResponse> response)
{
 // ...demo-code...
 response->setErrorCode(200);

 auto attribute_map = request->getAttributeRepresentation();
 getRepresentation(attribute_map);
 response->setResourceRepresentation(attribute_map);
}

void LightResource::handle_delete_request(std::shared_ptr<OCResourceRequest> request, std::shared_ptr<OCResourceResponse> response) 
{
 // ...demo-code...
 response->setErrorCode(200);

 auto attribute_map = request->getAttributeRepresentation();
 getRepresentation(attribute_map);
 response->setResourceRepresentation(attribute_map);
}

// Set up observation in a separate thread:
void LightResource::handle_observe_event(std::shared_ptr<OCResourceRequest> request, std::shared_ptr<OCResourceResponse> response) 
{
 if(observe_thread.joinable())
  return; 

 observe_thread = thread(bind(&LightResource::observe_function, this));
 observe_thread.detach();
}



}} // namespace Intel::OCDemo

