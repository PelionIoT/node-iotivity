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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <ocstack.h>
#include <logger.h>

#define TAG PCF("ocserver")

int gQuitFlag = 0;
OCStackResult createLEDResource();

typedef struct LEDRESOURCE{
    OCResourceHandle handle;
    bool power;
} LEDResource;

static LEDResource LED;

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}

int main() {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    if (OCInit((char *) addr, port, OC_SERVER) != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /*
     * Declare and create the example resource: LED
     */
    if(createLEDResource() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack cannot create resource...");
    }

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

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}

OCStackResult createLEDResource() {
    LED.power = false;
    OCStackResult res = OCCreateResource(&LED.handle,
                    "core.led",
                    "core.rw",
                    "/a/led",
                    0,
                    OC_DISCOVERABLE|OC_OBSERVABLE);
    return res;
}
