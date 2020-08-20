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
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <vector>
#include "SeServiceTransport.h"
#include <android-base/logging.h>

namespace se_transport {

using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Void;
using ::android::hardware::Return;
using ::android::sp;

/**
 * SecureElementCallback for version 1.1
 */
class SecureElementCallback_1_1 : public ::android::hardware::secure_element::V1_1::ISecureElementHalCallback {

    public:
        SecureElementCallback_1_1(const sp<SeServiceTransport> transport) : serviceTransport(transport) {}

        Return<void> onStateChange_1_1(bool connected, const hidl_string& /*debugReason*/) {
            serviceTransport->setConnected(connected);
            return Void();
        }

        Return<void> onStateChange(bool connected) {
            serviceTransport->setConnected(connected);
            return Void();
        }
    private:
        sp<SeServiceTransport> serviceTransport;
};

/**
 * SecureElementCallback for version 1.0
 */
class SecureElementCallback_1_0 : public ::android::hardware::secure_element::V1_0::ISecureElementHalCallback {

    public:
        SecureElementCallback_1_0(const sp<SeServiceTransport> transport) : serviceTransport(transport) {}

        Return<void> onStateChange(bool connected) {
            serviceTransport->setConnected(connected);
            return Void();
        }

    private:
        sp<SeServiceTransport> serviceTransport;
};

void SeServiceTransport::serviceDied(uint64_t /*cookie*/, const android::wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
    closeConnection();
}

bool SeServiceTransport::openConnection() {
    sp<::android::hardware::secure_element::V1_1::ISecureElement> secureElement_1_1;
    sp<::android::hardware::secure_element::V1_2::ISecureElement> secureElement_1_2;
    secureElement = secureElement_1_1 = secureElement_1_2 =
        ::android::hardware::secure_element::V1_2::ISecureElement::getService("eSE1");
    if(secureElement_1_2 == nullptr) {
        secureElement = secureElement_1_1 =
            ::android::hardware::secure_element::V1_1::ISecureElement::getService("eSE1");
        if(secureElement_1_1 == nullptr) {
            secureElement =
                ::android::hardware::secure_element::V1_0::ISecureElement::getService("eSE1");
            if(secureElement == nullptr) {
                LOG(ERROR) << "No Hal is provided for eSE1";
                return false;
            }
        } 
    }
    if(secureElement_1_2 != nullptr || secureElement_1_1 != nullptr) {
        //1.1 callback
        secureElement_1_1->init(new SecureElementCallback_1_1(this));
    } else {
        //1.0
        secureElement->init(new SecureElementCallback_1_0(this));
    }
    secureElement->linkToDeath(this, 0);
    return true;
}

bool SeServiceTransport::sendData(const uint8_t* inData, const size_t inLen, std::vector<uint8_t>& output) {
    if(!seConnected) {
        LOG(ERROR) << "venkat Connection not yet established";
        return false;
    }
    hidl_vec<uint8_t> data;
    data.resize(inLen);
    for(size_t i = 0; i < inLen; i++) {
        data[i] = inData[i];
    }
    LOG(ERROR) << "venkat SeServiceTransport::sendData call to transmit input len: "<<data.size();
    secureElement->transmit(data, [&] (const hidl_vec<uint8_t>& response) {
            LOG(ERROR) << "venkat SeServiceTransport::sendData recieved res len: " << output.size();
            output = response;
            });
    return true;
}

bool SeServiceTransport::closeConnection() {
    secureElement->unlinkToDeath(this);
    return true;
}

bool SeServiceTransport::isConnected() {
    return seConnected;
}

}
