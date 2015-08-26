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

#import "PcapBridge.h"

#include <pcap.h>
#include <vector>
#include <sstream>
#include <map>
#include <tuple>

#if defined(__APPLE__) || defined(LINUX) || defined(linux)
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#endif

#include "EasyDrcomCore/easy_drcom_exception.hpp"
#include "EasyDrcomCore/utils.h"
#include "EasyDrcomCore/get_nic_addr.h"

@implementation PcapBridge
-(NSArray*) getNICList
{
    NSMutableArray          *ret = [[NSMutableArray alloc] init];
    pcap_if_t               *adapters;
    char                    errbuf[PCAP_ERRBUF_SIZE];
    
    if (pcap_findalldevs(&adapters, errbuf) == -1)
    {
        self.lastError = [NSString stringWithUTF8String:errbuf];
        return nil;
    }
	
    for (pcap_if_t *nic = adapters; nic != NULL; nic = nic->next)
    {
        if (nic->flags != PCAP_IF_LOOPBACK && nic->name)
            [ret addObject:[NSString stringWithUTF8String:nic->name]];
    }
    
    pcap_freealldevs(adapters);
    return [NSArray arrayWithArray:ret];
}

-(NSString*) getIPAddressWithNICName:(NSString *)name
{
    try {
        auto ip = get_ip_address([name UTF8String]);
        return [NSString stringWithUTF8String:ip.c_str()];
    } catch (easy_drcom_exception& ex) {
        self.lastError = [NSString stringWithUTF8String:ex.what()];
        return nil;
    }
}

-(NSString*) getMACAddressWithNICName:(NSString *)name
{
    try {
        auto mac = get_mac_address([name UTF8String]);
        return [NSString stringWithUTF8String:hex_to_str(&mac[0], 6, ':').c_str()];
    } catch (easy_drcom_exception& ex) {
        self.lastError = [NSString stringWithUTF8String:ex.what()];
        return nil;
    }
}@end
