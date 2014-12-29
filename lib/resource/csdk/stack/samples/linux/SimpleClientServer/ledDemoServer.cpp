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


//************************LED Demo App*****************************************
//
// This Server app provides the functionality of creating a light resource.
// once light resource is created , user can swiitch ON/OFF the light by sending putrequest to this app.
// It also provides the functionality to get the current state of light.
//*****************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "ocstack.h"
#include "logger.h"
#include "cJSON.h"
#include <iostream>


#include <fcntl.h>

//#define USING_EDISON_BOARD true

#ifdef USING_EDISON_BOARD
#include "mraa.h"
static int iopin;
mraa_gpio_context gpio;
mraa_result_t r = MRAA_SUCCESS;
#endif


const char *getResult(OCStackResult result);

#define TAG PCF("ocserver")

static int gObserveNotifyType = 3;

int gQuitFlag = 0;
int gLEDUnderObservation = 0;

typedef struct LEDRESOURCE{
    OCResourceHandle handle;
    bool state;
} LEDResource;

static LEDResource LED;
int createLEDResource (char *uri, LEDResource *ledResource);

//const char respPLGet_light2_ON[] = "{\"href\":\"/123_seymour_street/parlor/light2\",\"rep\":{\"state\":\"on\"}}";
//const char respPLGet_light2_OFF[] = "{\"href\":\"/123_seymour_street/parlor/light2\",\"rep\":{\"state\":\"off\"}}";

char *gResourceUri= (char *)"/a/led";

static uint16_t OC_WELL_KNOWN_PORT = 5683;

#ifdef USING_EDISON_BOARD
mraa_result_t gpio_set(int pin, int level) 
{	
	if (gpio != NULL) 
	{
		mraa_gpio_write(gpio, level);
		return MRAA_SUCCESS;
	} 
	else
	{
		return MRAA_ERROR_INVALID_RESOURCE;
	}
}
mraa_result_t gpio_get(int pin, int* level) 
{
	if (gpio != NULL) 
	{
		*level = mraa_gpio_read(gpio);
		return MRAA_SUCCESS;
	} 
	else
	{
		return MRAA_ERROR_INVALID_RESOURCE;
	}
}
#endif

bool getPutRequestState(OCEntityHandlerRequest *ehRequest)
{
	bool state = false;
	 if(OC_REST_PUT == ehRequest->method)
    {
        /*cJSON *putJson = cJSON_Parse((char *)ehRequest->reqJSONPayload);
        state = ( !strcmp(cJSON_GetObjectItem(putJson,"state")->valuestring , "true") ? true:false);
        cJSON_Delete(putJson);*/
        
	  cJSON *putJson = cJSON_Parse((char *)ehRequest->reqJSONPayload); 
	  OC_LOG_V (INFO, TAG, "###PUT getPutRequestState %s:",cJSON_Print(putJson));       
	  cJSON *OC = cJSON_GetObjectItem(putJson,"oc");     
	  cJSON *subitem = cJSON_GetArrayItem(OC,0);
	  cJSON *rep = cJSON_GetObjectItem(subitem,"rep"); 
	  bool power = ( !strcmp(cJSON_GetObjectItem(rep,"state")->valuestring , "true") ? true:false); 
		if(power)
		{
		 state = true;
		}
		else
		{
		 state = false;
		}
		
		cJSON_Delete(putJson);
    }
    return state;
}

char* constructJsonResponse (OCEntityHandlerRequest *ehRequest)
{
    cJSON *json = cJSON_CreateObject();
    cJSON *format;
    char *jsonResponse;
    LEDResource *currLEDResource = &LED;

	OC_LOG_V (INFO, TAG, "###constructJsonResponse");
    if (ehRequest->resource == LED.handle)
    {
		
        gResourceUri = (char *) "a/led";
    }
      
    if(OC_REST_PUT == ehRequest->method)
    {
	  OC_LOG_V (INFO, TAG, "###constructJsonResponse: for PUT");
      cJSON *putJson = cJSON_Parse((char *)ehRequest->reqJSONPayload); 
      OC_LOG_V (INFO, TAG, "###PUT ConstructJsonResponse: for putJson %s:",cJSON_Print(putJson));       
      cJSON *OC = cJSON_GetObjectItem(putJson,"oc");     
      cJSON *subitem = cJSON_GetArrayItem(OC,0);
      cJSON *rep = cJSON_GetObjectItem(subitem,"rep"); 
      bool power = ( !strcmp(cJSON_GetObjectItem(rep,"state")->valuestring , "true") ? true:false); 
    
        if(power)
        {
         currLEDResource->state = true;
	    }
	    else
	    {
			currLEDResource->state = false;
		}
         OC_LOG_V (INFO, TAG, "###PUT ConstructJsonResponse: got value of power"); 
        cJSON_Delete(putJson);
        
    }

    cJSON_AddStringToObject(json,"href",gResourceUri);
    cJSON_AddItemToObject(json, "rep", format=cJSON_CreateObject());
    cJSON_AddStringToObject(format, "state", (char *) (currLEDResource->state ? "true":"false"));
   
    jsonResponse = cJSON_Print(json);
    cJSON_Delete(json);
     OC_LOG_V (INFO, TAG, "###json response %s:",jsonResponse);

    return jsonResponse;
}


void ProcessGetRequest (OCEntityHandlerRequest *ehRequest)
{
    char *getResp;
    int state = 0;
    OC_LOG_V (INFO, TAG, "###Got Get request %s",ehRequest->reqJSONPayload);
   
    if (ehRequest->resource == LED.handle)
    {
	// read from libMRAA	
	#if USING_EDISON_BOARD	
	    r = gpio_get(iopin,&state);
	    if (r != MRAA_SUCCESS) 
	    {
			mraa_result_print(r);
		} 
		else 
		{
			OC_LOG_V (INFO, TAG, "###MRAA Read Success, state is %d",state);
		}
	#endif
		
		if(state == 0)
		{
			LED.state = false;
			
	    }
	    else
	    {
			LED.state = true;
		}
		getResp = constructJsonResponse(ehRequest);
	
    }
    
    if (ehRequest->resJSONPayloadLen > strlen ((char *)getResp))
    {
        strncpy((char *)ehRequest->resJSONPayload, getResp,
                strlen((char *)getResp));
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                ehRequest->resJSONPayloadLen);
    }
    free(getResp);
}

void ProcessPutRequest (OCEntityHandlerRequest *ehRequest)
{
    char *putResp;
    OC_LOG_V (INFO, TAG, "###Got put request %s",ehRequest->reqJSONPayload);
    putResp = constructJsonResponse(ehRequest);
    if (ehRequest->resource == LED.handle)
    {
		bool ledstate = getPutRequestState(ehRequest);
       
	    if(ledstate)
	    {					
			#if USING_EDISON_BOARD
		    r = gpio_set(iopin, 1);
			if (r != MRAA_SUCCESS) {
			mraa_result_print(r);
			} else {
			OC_LOG_V (INFO, TAG, "###MRAA SUCCESS in ON");
			}
		    #endif 
		    
	      OC_LOG_V (INFO, TAG, "###Switch Light2 on");
	    }
	    else
	    {			
		#if USING_EDISON_BOARD		  
		   r = gpio_set(iopin, 0);
			if (r != MRAA_SUCCESS) {
			mraa_result_print(r);
			} else {
			OC_LOG_V (INFO, TAG, "###MRAA SUCCESS in OFF");
			}
		 #endif 		    
            OC_LOG_V (INFO, TAG, "###Switch Light2  Off");
	    }
	    
	    LED.state = ledstate;
	    
    }
    if (ehRequest->resJSONPayloadLen > strlen ((char *)putResp))
    {
        strncpy((char *)ehRequest->resJSONPayload, putResp,
                strlen((char *)putResp));
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                ehRequest->resJSONPayloadLen);
    } 
    
    free(putResp);
}


OCEntityHandlerResult
OCEntityHandlerCb (OCEntityHandlerFlag flag,
                   OCEntityHandlerRequest *entityHandlerRequest)
{
    OC_LOG_V (INFO, TAG, "Inside entity handler - flags: 0x%x", flag);
    if (flag & OC_INIT_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_INIT_FLAG");
    }
    if (flag & OC_REQUEST_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        if (entityHandlerRequest)
        {
            if (OC_REST_GET == entityHandlerRequest->method)
            {
                OC_LOG (INFO, TAG, "Received OC_REST_GET from client");
                ProcessGetRequest (entityHandlerRequest);
            }
            else if (OC_REST_PUT == entityHandlerRequest->method)
            {
                OC_LOG (INFO, TAG, "Received OC_REST_PUT from client");
                ProcessPutRequest (entityHandlerRequest);
            }
            else
            {
                OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                            entityHandlerRequest->method);
            }
        }
    }
   

    return OC_EH_OK;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}

int main(int argc, char* argv[])
{
    uint8_t addr[20] = {0};
    uint8_t* paddr = NULL;
    uint16_t port = OC_WELL_KNOWN_PORT;
    uint8_t ifname[] = "eth0";
    int opt;

    OC_LOG(DEBUG, TAG, "LED Demo Server is starting...");
    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    if ( OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr,
                               sizeof(addr)) == ERR_SUCCESS)
    {
        OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
        paddr = addr;
    }

    if (OCInit((char *) paddr, port, OC_SERVER) != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }
  //############################################################# 
  #ifdef USING_EDISON_BOARD
		 mraa_result_t r = MRAA_SUCCESS;
		iopin = 13;
		gpio = mraa_gpio_init(iopin);
		if (gpio == NULL)
		 {
		  OC_LOG(INFO, TAG, "Error MRAA...");
	    }
	   else
	   {
		   OC_LOG(INFO, TAG, "Success MRAA");
	   }
	   r = mraa_gpio_dir(gpio, MRAA_GPIO_OUT);
		if (r != MRAA_SUCCESS) {
		mraa_result_print(r);
		}
		
		#endif
//###########################################################
    /*
     * Declare and create the example resource: LED
     */
    createLEDResource("/a/led", &LED);
  
    // Break from loop with Ctrl-C
    OC_LOG(INFO, TAG, "Entering ocserver main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag) {
        if (OCProcess() != OC_STACK_OK) {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(1);
    }

#ifdef USING_EDISON_BOARD
    
	mraa_gpio_close(gpio);

#endif
    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");
    if (OCStop() != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}

int createLEDResource (char *uri, LEDResource *ledResource)
{
    if (!uri)
    {
        OC_LOG(ERROR, TAG, "Resource URI cannot be NULL");
        return -1;
    }

    ledResource->state = "false";
    OCStackResult res = OCCreateResource(&(ledResource->handle),
            "core.led",
            "core.rw",
            uri,
            OCEntityHandlerCb,
            OC_DISCOVERABLE|OC_OBSERVABLE);
            
    OC_LOG_V(INFO, TAG, "Created LED resource with result: %s", getResult(res));

    return 0;
}
