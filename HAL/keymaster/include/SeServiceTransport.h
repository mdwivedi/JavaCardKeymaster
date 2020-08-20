/*
 **
 ** Copyright 2020, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef __SE_SERVICE_TRANSPORT__
#define __SE_SERVICE_TRANSPORT__

#include "Transport.h"
#include <android/hardware/secure_element/1.2/ISecureElement.h>
#include <android/hardware/secure_element/1.1/ISecureElementHalCallback.h>
#include <hidl/HidlSupport.h>

namespace se_transport {

/**
 * SeServiceTransport is derived from ITransport. This class uses secure_element service to communicate with underlying secure
 * hardware or javacard.
 */
class SeServiceTransport : public ITransport, public ::android::hardware::hidl_death_recipient {

public:
    SeServiceTransport() : seConnected(false) {}

    /**
     * Gets the binder instance of ISEService, gets the reader corresponding to secure element, establishes a session
     * and opens a basic channel.
     */
	bool openConnection() override;
    /**
     * Transmists the data over the opened basic channel and receives the data back.
     */
    bool sendData(const uint8_t* inData, const size_t inLen, std::vector<uint8_t>& output) override;
    /**
     * Closes the connection.
     */
    bool closeConnection() override;
    /**
     * Returns the state of the connection status. Returns true if the connection is active, false if connection is
     * broken.
     */
    bool isConnected() override;

    virtual void serviceDied(uint64_t cookie, const android::wp<::android::hidl::base::V1_0::IBase>& who);

    void setConnected(bool connected) { seConnected = connected; }

private:
    ::android::sp<::android::hardware::secure_element::V1_0::ISecureElement> secureElement;
    bool seConnected;

};


}
#endif /* __SE_SERVICE_TRANSPORT__ */
