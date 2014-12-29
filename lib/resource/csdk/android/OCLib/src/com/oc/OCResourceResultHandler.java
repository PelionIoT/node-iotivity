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

package com.oc;

import java.io.Serializable;

/**
 * 
 * When a request or command is issued to a resource, an OCResourceResultHandler object must be
 * passed as a parameter to handle the callback(s).
 * 
 */
public interface OCResourceResultHandler extends Serializable {
  /**
   * 
   * The OC service calls this repeatedly until completed() or failed() is called.
   * 
   * @param serviceURI - The URI of the reporting service.
   * @param resources - The list of resources found on the remote service.
   */
  void onResourcesFound(String serviceURI, OCResource[] resources);

  /**
   * Called after all of the found resource(s) are reported.
   */
  void onCompleted();

  /**
   * Called if a failure occurred. No resources will be reported after this callback.
   * 
   * @param throwable - The error that caused this callback sequence to fail.
   */
  void onFailed(Throwable throwable);
}
