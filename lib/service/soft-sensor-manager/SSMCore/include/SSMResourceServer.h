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

#ifndef _SSMResourceServer_H_
#define _SSMResourceServer_H_

#include <functional>
#include "OCPlatform.h"
#include "OCApi.h"

class SSMResourceServer
{
public:
    SSMResourceServer();
    ~SSMResourceServer();

    int initializeManager(std::string &xmlDescription);
    int terminateManager();

    void entityHandler(std::shared_ptr< OC::OCResourceRequest > request,
            std::shared_ptr< OC::OCResourceResponse > response);

private:
    OC::OCPlatform *m_pPlatform;
    OCResourceHandle m_hSSMResource;
    int createResource();
};

#endif
