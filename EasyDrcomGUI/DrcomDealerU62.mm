/**
 * Copyright (C) 2015 Shindo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "DrcomDealerU62.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <functional>
#include <ctime>
#include <fstream>
#include <cctype>

#include <thread>

#if defined(__APPLE__) || defined(LINUX) || defined(linux)
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#endif

#define EASYDRCOM_DEBUG

#include "EasyDrcomCore/easy_drcom_exception.hpp"
#include "EasyDrcomCore/log.hpp"
#include "EasyDrcomCore/utils.h"
#include "EasyDrcomCore/drcom_dealer.hpp"
#include "EasyDrcomCore/eap_dealer.hpp"

@implementation DrcomDealerU62
std::vector<uint8_t> broadcast_mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
std::vector<uint8_t> nearest_mac = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x03 };

- (id) initWithNICName: (NSString*) _nicName userName:(NSString *)_userName passWord:(NSString *)_passWord IPAddress:(NSString *)_ip MacAddress:(NSString *)_mac
{
    if (self = [super initWithNICName:_nicName userName:_userName passWord:_passWord IPAddress:_ip MacAddress:_mac])
    {
        try {
            eap = std::shared_ptr<eap_dealer>(new eap_dealer([nicName UTF8String], str_mac_to_vec([mac UTF8String]), [ip UTF8String], [userName UTF8String], [passWord UTF8String]));
        } catch (std::exception& ex) {
            EAP_LOG_ERR(ex.what() << std::endl);
            throw easy_drcom_exception(ex.what()); // throw out to the loop
        }
        
        try {
            udp = std::shared_ptr<drcom_dealer_u62>(new drcom_dealer_u62(str_mac_to_vec([mac UTF8String]), [ip UTF8String], [userName UTF8String], [passWord UTF8String], "172.25.8.4", 61440, "EasyDrcomGUI", "for Mac OSX"));
        } catch (std::exception& ex) {
            U62_LOG_ERR(ex.what() << std::endl);
            throw easy_drcom_exception(ex.what()); // throw out to the loop
        }
    }
    return self;
}

- (void) dealloc
{
    eap.reset();
    udp.reset();
}

- (BOOL) isEAPObjectInit
{
    return eap != nullptr;
}

- (BOOL) isUDPObjectInit
{
    return udp != nullptr;
}

- (NSInteger) logOff
{
    return eap->logoff(nearest_mac);
}

-(NSInteger) start
{
    return eap->start(broadcast_mac);
}

-(NSInteger) responseIdentity
{
    return eap->response_identity(broadcast_mac);
}

-(NSInteger) responseMD5Challenge
{
    return eap->response_md5_challenge(broadcast_mac);
}

-(NSInteger) sendAlivePacket1
{
    return udp->send_alive_pkt1();
}

-(NSInteger) sendAlivePacket2
{
    return udp->send_alive_pkt2();
}

-(NSString*) getNotification
{
    return [NSString stringWithUTF8String:eap->getNotification().c_str()];
}
@end
