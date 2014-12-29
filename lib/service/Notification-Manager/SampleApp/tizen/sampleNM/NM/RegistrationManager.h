//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

#ifndef REGISTRATIONMANAGER_H_
#define REGISTRATIONMANAGER_H_

#include "NotificationManager.h"

using namespace OC;

class VirtualRepresentation;

class RegistrationManager {

private:
	static RegistrationManager *instance;


public:
	RegistrationManager();
	~RegistrationManager();

	static RegistrationManager *getInstance();

	bool registerNMResource(VirtualRepresentation &resourceObject, std::shared_ptr<OCResource> resource);


	int addResource ();
	int removeResource ();
	int updateResource ();
	bool unregisterResource ();

};

#endif /* REGISTRATIONMANAGER_H_ */
