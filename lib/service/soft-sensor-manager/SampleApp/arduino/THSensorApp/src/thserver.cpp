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

// Do not remove the include below
#include "Arduino.h"

#define dht11_pin 12 

#include "logger.h"
#include "ocstack.h"
#include <string.h>

#ifdef ARDUINOWIFI
// Arduino WiFi Shield
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#else
// Arduino Ethernet Shield
#include <EthernetServer.h>
#include <Ethernet.h>
#include <Dns.h>
#include <EthernetClient.h>
#include <util.h>
#include <EthernetUdp.h>
#include <Dhcp.h>
#endif

const char *getResult(OCStackResult result);

PROGMEM const char TAG[] = "ArduinoServer";

int g_THUnderObservation = 0;
void createTHResource();
typedef struct THRESOURCE {
    OCResourceHandle m_handle;
    int m_temp;
    int m_humid;
} THResource;

static THResource TH;

String g_responsePayloadPut = "{\"href\":\"\",\"rep\":{\"0\":\"temperature\",\"1\":\"int\",\"2\":\"0\",\"3\":\"humidity\",\"4\":\"int\",\"5\":\"0\"}}";
String g_responsePayloadGet = "{\"href\":\"\",\"rep\":{\"0\":\"temperature\",\"1\":\"int\",\"2\":\"0\",\"3\":\"humidity\",\"4\":\"int\",\"5\":\"0\"}}";

/// This is the port which Arduino Server will use for all unicast communication with it's peers
static uint16_t OC_WELL_KNOWN_PORT = 5683;

byte read_dht11_dat()
{
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++)
  {
    while (!digitalRead(dht11_pin));
    delayMicroseconds(30);
    if (digitalRead(dht11_pin) != 0 )
      bitSet(result, 7-i);
    while (digitalRead(dht11_pin));
  }
  return result;
}

#ifdef ARDUINOWIFI
// Arduino WiFi Shield
// Note : Arduino WiFi Shield currently does NOT support multicast and therefore
// this server will NOT be listening on 224.0.1.187 multicast address.

/// WiFi Shield firmware with Intel patches
static const char INTEL_WIFI_SHIELD_FW_VER[] = "1.2.0";

/// WiFi network info and credentials
char ssid[] = "SoftSensor_AP";
char pass[] = "1234567890";

int ConnectToNetwork()
{
    char *fwVersion;
    int status = WL_IDLE_STATUS;
    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD)
    {
        OC_LOG(ERROR, TAG, PCF("WiFi shield not present"));
        return -1;
    }

    // Verify that WiFi Shield is running the firmware with all UDP fixes
    fwVersion = WiFi.firmwareVersion();
    OC_LOG_V(INFO, TAG, "WiFi Shield Firmware version %s", fwVersion);
    if ( strncmp(fwVersion, INTEL_WIFI_SHIELD_FW_VER, sizeof(INTEL_WIFI_SHIELD_FW_VER)) !=0 )
    {
        OC_LOG(DEBUG, TAG, PCF("!!!!! Upgrade WiFi Shield Firmware version !!!!!!"));
        return -1;
    }

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED)
    {
        OC_LOG_V(INFO, TAG, "Attempting to connect to SSID: %s", ssid);
        status = WiFi.begin(ssid,pass);

        // wait 10 seconds for connection:
        delay(10000);
    }
    OC_LOG(DEBUG, TAG, PCF("Connected to wifi"));

    IPAddress ip = WiFi.localIP();
    OC_LOG_V(INFO, TAG, "IP Address:  %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return 0;
}
#else
// Arduino Ethernet Shield
int ConnectToNetwork()
{
    // Note: ****Update the MAC address here with your shield's MAC address****
    uint8_t ETHERNET_MAC[] = {0x90, 0xA2, 0xDA, 0x0E, 0xB8, 0xAC};
   
    uint8_t error = Ethernet.begin(ETHERNET_MAC);
    if (error  == 0)
    {
        OC_LOG_V(ERROR, TAG, "error is: %d", error);
        return -1;
    }
    OC_LOG_V(INFO, TAG, "IPAddress : %s", Serial.print(Ethernet.localIP()));
    return 0;
}
#endif //ARDUINOWIFI

// On Arduino Atmel boards with Harvard memory architecture, the stack grows
// downwards from the top and the heap grows upwards. This method will print
// the distance(in terms of bytes) between those two.
// See here for more details :
// http://www.atmel.com/webdoc/AVRLibcReferenceManual/malloc_1malloc_intro.html
void PrintArduinoMemoryStats()
{
#ifdef ARDUINO_AVR_MEGA2560
    //This var is declared in avr-libc/stdlib/malloc.c
    //It keeps the largest address not allocated for heap
    extern char *__brkval;
    //address of tmp gives us the current stack boundry
    int tmp;
    OC_LOG_V(INFO, TAG, "Unallocated Memory between heap and stack: %u",
             ((unsigned int)&tmp - (unsigned int)__brkval));
#endif
}

// This is the entity handler for the registered resource.
// This is invoked by OCStack whenever it recevies a request for this resource.
OCEntityHandlerResult OCEntityHandlerCb(OCEntityHandlerFlag flag, OCEntityHandlerRequest * entityHandlerRequest ) 
{
    OCEntityHandlerResult ehRet = OC_EH_OK;
    const char* typeOfMessage;

    switch (flag) {
    case OC_INIT_FLAG:
        typeOfMessage = "OC_INIT_FLAG";
        break;
    case OC_REQUEST_FLAG:
        typeOfMessage = "OC_REQUEST_FLAG";
        break;
    case OC_OBSERVE_FLAG:
        typeOfMessage = "OC_OBSERVE_FLAG";
        break;
    default:
        typeOfMessage = "UNKNOWN";
    }
    OC_LOG_V(INFO, TAG, "Receiving message type: %s", typeOfMessage);

    if(entityHandlerRequest && flag == OC_REQUEST_FLAG) 
    { 
        if(OC_REST_GET == entityHandlerRequest->method) 
        {
            int str_len = g_responsePayloadGet.length() + 1;
            char charBuf[str_len+1];

            g_responsePayloadGet.toCharArray(charBuf, str_len);

            if(strlen(charBuf) < entityHandlerRequest->resJSONPayloadLen)
            {
            strncpy((char *)entityHandlerRequest->resJSONPayload, charBuf, entityHandlerRequest->resJSONPayloadLen);
            }
            else
                ehRet = OC_EH_ERROR;
        }
        if(OC_REST_PUT == entityHandlerRequest->method) {

            int str_len1 = g_responsePayloadPut.length() + 1;
            char charBuf1[str_len1];

            g_responsePayloadPut.toCharArray(charBuf1, str_len1);

             if(strlen(charBuf1) < entityHandlerRequest->resJSONPayloadLen)
            {
            strncpy((char *)entityHandlerRequest->resJSONPayload, charBuf1, entityHandlerRequest->resJSONPayloadLen);
            }
             else
               ehRet = OC_EH_ERROR;
        }
    } else if (entityHandlerRequest && flag == OC_OBSERVE_FLAG) {
        g_THUnderObservation = 1;
    }

    return ehRet;
}

/* Json Generator */
String JsonGenerator(THResource TH){
   String a = "{\"href\":\"\",\"rep\":{\"0\":\"temperature\",\"1\":\"int\",\"2\":\"";
   String b = "\",\"3\":\"humidity\",\"4\":\"int\",\"5\":\"";
   String c = "\"}}";

   String ss;

    ss = a + TH.m_temp + b + TH.m_humid + c;
	return ss;
}

// This method is used to display 'Observe' functionality of OC Stack.
static uint8_t modCounter = 0;
void *ChangeTHRepresentation (void *param)
{
    (void)param;
    OCStackResult result = OC_STACK_ERROR;
    modCounter += 1;
    if(modCounter % 10 == 0)  // Matching the timing that the Linux Sample Server App uses for the same functionality.
    {

    byte dht11_dat[5];   
    byte i;// start condition
       
    digitalWrite(dht11_pin, LOW);
    delay(18);
    digitalWrite(dht11_pin, HIGH);
    delayMicroseconds(1);
    pinMode(dht11_pin, INPUT);
    delayMicroseconds(40);     

    if (digitalRead(dht11_pin))
    {
        Serial.println("dht11 start condition 1 not met"); // wait for DHT response signal: LOW
        delay(1000);
        return NULL;
    }
    delayMicroseconds(80);
    if (!digitalRead(dht11_pin))
    {
        Serial.println("dht11 start condition 2 not met");  //wair for second response signal:HIGH
        return NULL;
    }

    delayMicroseconds(80);// now ready for data reception
    for (i=0; i<5; i++)
    {
        dht11_dat[i] = read_dht11_dat();
    }  //recieved 40 bits data. Details are described in datasheet

    pinMode(dht11_pin, OUTPUT);
    digitalWrite(dht11_pin, HIGH);
    byte dht11_check_sum = dht11_dat[0]+dht11_dat[2];// check check_sum
    if(dht11_dat[4]!= dht11_check_sum)
    {
        Serial.println("DHT11 checksum error");
    }
    Serial.print("Current humdity = ");
    Serial.print(dht11_dat[0], DEC);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(dht11_dat[2], DEC);
    Serial.println("C  ");

 // delay(2000); //fresh time
    TH.m_humid = dht11_dat[0];
    TH.m_temp = dht11_dat[2];

    g_responsePayloadGet = JsonGenerator(TH);

        if (g_THUnderObservation)
        {
            OC_LOG_V(INFO, TAG, " =====> Notifying stack of new humid level %d\n", TH.m_humid);
            OC_LOG_V(INFO, TAG, " =====> Notifying stack of new temp level %d\n", TH.m_temp);

            result = OCNotifyObservers (TH.m_handle);

            if (OC_STACK_NO_OBSERVERS == result)
            {
                g_THUnderObservation = 0;
            }
        }
    }
    return NULL;
}



//The setup function is called once at startup of the sketch
void setup()
{
    pinMode(dht11_pin, OUTPUT);   
    digitalWrite(dht11_pin, HIGH);

    // Add your initialization code here
    OC_LOG_INIT();

    OC_LOG(DEBUG, TAG, PCF("OCServer is starting..."));
    uint16_t port = OC_WELL_KNOWN_PORT;
 
    // Connect to Ethernet or WiFi network
    if (ConnectToNetwork() != 0)
    {
        OC_LOG(ERROR, TAG, "Unable to connect to network");
        return;
    }

    // Initialize the OC Stack in Server mode
    if (OCInit(NULL, port, OC_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, PCF("OCStack init error"));
        return;
    }

    // Declare and create the example resource: TH
   createTHResource();

}

// The loop function is called in an endless loop
void loop()
{
    // This artificial delay is kept here to avoid endless spinning
    // of Arduino microcontroller. Modify it as per specfic application needs.
    delay(2000);

    // This call displays the amount of free SRAM available on Arduino
    PrintArduinoMemoryStats();

    if (OCProcess() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, PCF("OCStack process error"));
        return;
    }
     ChangeTHRepresentation(NULL);
}

void createTHResource() {
    TH.m_humid = 0;
    TH.m_temp = 0;

    OCStackResult res = OCCreateResource(&TH.m_handle,
                                         "SoftSensorManager.Sensor",
                                         "oc.mi.def",
                                         "/Thing_TempHumSensor1",
                                         OCEntityHandlerCb,
                                         OC_DISCOVERABLE|OC_OBSERVABLE);
    OC_LOG_V(INFO, TAG, "Created TH resource with result: %s", getResult(res));
}

const char *getResult(OCStackResult result) {
    switch (result) {
    case OC_STACK_OK:
        return "OC_STACK_OK";
    case OC_STACK_INVALID_URI:
        return "OC_STACK_INVALID_URI";
    case OC_STACK_INVALID_QUERY:
        return "OC_STACK_INVALID_QUERY";
    case OC_STACK_INVALID_IP:
        return "OC_STACK_INVALID_IP";
    case OC_STACK_INVALID_PORT:
        return "OC_STACK_INVALID_PORT";
    case OC_STACK_INVALID_CALLBACK:
        return "OC_STACK_INVALID_CALLBACK";
    case OC_STACK_INVALID_METHOD:
        return "OC_STACK_INVALID_METHOD";
    case OC_STACK_NO_MEMORY:
        return "OC_STACK_NO_MEMORY";
    case OC_STACK_COMM_ERROR:
        return "OC_STACK_COMM_ERROR";
    case OC_STACK_INVALID_PARAM:
        return "OC_STACK_INVALID_PARAM";
    case OC_STACK_NOTIMPL:
        return "OC_STACK_NOTIMPL";
    case OC_STACK_NO_RESOURCE:
        return "OC_STACK_NO_RESOURCE";
    case OC_STACK_RESOURCE_ERROR:
        return "OC_STACK_RESOURCE_ERROR";
    case OC_STACK_SLOW_RESOURCE:
        return "OC_STACK_SLOW_RESOURCE";
    case OC_STACK_NO_OBSERVERS:
        return "OC_STACK_NO_OBSERVERS";
    case OC_STACK_ERROR:
        return "OC_STACK_ERROR";
    default:
        return "UNKNOWN";
    }
}
