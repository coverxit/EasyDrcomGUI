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

#import "DrcomDealerU31.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <functional>
#include <ctime>
#include <fstream>
#include <cctype>

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

@implementation DrcomDealerU31
- (id) initWithNICName:(NSString *)_nicName userName:(NSString *)_userName passWord:(NSString *)_passWord IPAddress:(NSString *)_ip MacAddress:(NSString *)_mac
{
    if (self = [super initWithNICName:_nicName userName:_userName passWord:_passWord IPAddress:_ip MacAddress:_mac])
    {
        try
        {
            udp = std::shared_ptr<drcom_dealer_u31>(new drcom_dealer_u31(str_mac_to_vec([mac UTF8String]), [ip UTF8String], [userName UTF8String], [passWord UTF8String], "172.25.8.4", 61440, "EasyDrcom for HITwh", "v0.8 GUI for Mac OSX"));
        }
        catch (std::exception& ex)
        {
            U31_LOG_ERR(ex.what() << std::endl);
            throw easy_drcom_exception(ex.what()); // throw out to the loop
        }
    }
    return self;
}

- (void) dealloc
{
    udp.reset();
}

- (BOOL) isUDPObjectInit
{
    return udp != nullptr;
}

- (NSInteger) startRequest
{
    return udp->start_request();
}

- (NSInteger) sendLoginAuth
{
    return udp->send_login_auth();
}

- (NSInteger) sendLogoutAuth
{
    return udp->send_logout_auth();
}

- (NSInteger) sendAliveRequest
{
    return udp->send_alive_request();
}

- (NSInteger) sendAlivePacket1
{
    return udp->send_alive_pkt1();
}

- (NSInteger) sendAlivePacket2
{
    return udp->send_alive_pkt2();
}
@end
