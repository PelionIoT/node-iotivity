/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/
/*
 * ThingResourceServer.h
 */

#ifndef THINGRESOURCESERVER_H_
#define THINGRESOURCESERVER_H_

#include <functional>
#include <pthread.h>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;

#include <string>
#include <cstdlib>

#define COAP_IP					"134.134.161.33"
#define COAP_PORT				56838
#define COAP_MODE				ModeType::Server
#define COAP_SRVTYPE		ServiceType::InProc

#define COAP_TYPE_NAME			"SoftSensorManager.Sensor"

// Forward declaring the entityHandler

class TemphumidResource
{
public:
    /// Access this property from a TB client
    int m_humid;
    int m_temp;
    string m_resourceUri;
    OCResourceHandle m_resourceHandle;

public:
    /// Constructor
    TemphumidResource() :
            m_humid(0), m_temp(0), m_resourceUri("/Thing_TempHumSensor1"), m_resourceHandle(NULL)
    {
    }

    ~TemphumidResource()
    {
    }

    void registerResource(OC::OCPlatform& platform);

    OCResourceHandle getHandle();

    void setRepresentation(AttributeMap& attributeMap);

    void getRepresentation(AttributeMap& attributeMap);
};

#endif /* THINGRESOURCESERVER_H_ */
