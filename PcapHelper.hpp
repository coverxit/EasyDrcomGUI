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

#ifndef PCAPHELPER_HPP
#define PCAPHELPER_HPP

#include <QStringList>

#include <stdint.h>
#include <vector>
#include <pcap.h>

#include "EasyDrcomCore/easy_drcom_exception.hpp"
#include "EasyDrcomCore/utils.h"
#include "EasyDrcomCore/get_nic_addr.h"

class PcapHelper
{
public:
    QStringList GetNICList()
    {
        QStringList     ret;
        pcap_if_t       *adapters;
        char            errbuf[PCAP_ERRBUF_SIZE];

        if (pcap_findalldevs(&adapters, errbuf) == -1)
        {
            m_szLastError = QString(errbuf);
            return ret;
        }

        for (pcap_if_t *nic = adapters; nic != NULL; nic = nic->next)
        {
            if (nic->flags != PCAP_IF_LOOPBACK && nic->name)
                ret.append(QString(nic->name));
        }

        pcap_freealldevs(adapters);
        return ret;
    }

    QString GetIPAddressByNIC(QString nic)
    {
        try
        {
            return QString(get_ip_address(nic.toStdString()).c_str());
        }
        catch (easy_drcom_exception& ex)
        {
            m_szLastError = ex.what();
            return QString();
        }
    }

    QString GetMACAddressByNIC(QString nic)
    {
        try
        {
            auto mac = get_mac_address(nic.toStdString());
            return QString(hex_to_str(&mac[0], 6, ':').c_str());
        }
        catch (easy_drcom_exception& ex)
        {
            m_szLastError = ex.what();
            return QString();
        }
    }

public:
    QString m_szLastError;
};

#endif // PCAPHELPER_HPP
