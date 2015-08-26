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

#pragma once

#include <IPHlpApi.h>

#include <stdint.h>
#include <sstream>

#include "../EasyDrcomCore/easy_drcom_exception.hpp"
#include "../EasyDrcomCore/utils.h"
#include "../EasyDrcomCore/get_nic_addr.h"

class PcapHelper
{
public:
	void GetNICList(std::map<CString, CString>& out)
	{
		pcap_if_t			*adapters;
		char				errbuf[PCAP_ERRBUF_SIZE];
		IP_ADAPTER_INFO		adapterInfo[16]; // Allocate information
		DWORD dwBufLen		= sizeof(adapterInfo); // Save memory size of buffer
		std::vector<std::pair<CString, CString>> adapterArray;

		out.clear();
		if (GetAdaptersInfo(adapterInfo, &dwBufLen) == ERROR_SUCCESS)
		{
			for (auto ptr = adapterInfo; ptr != NULL; ptr = ptr->Next)
				adapterArray.push_back(std::make_pair(CString(ptr->AdapterName), CString(ptr->Description)));
		}
		else
		{
			m_szLastError = "PcapHelper::GetNICList: GetAdaptersInfo failed";
			return;
		}

		if (pcap_findalldevs(&adapters, errbuf) == -1)
		{
			m_szLastError = errbuf;
			return;
		}

		for (pcap_if_t *nic = adapters; nic != NULL; nic = nic->next)
		{
			if (nic->flags != PCAP_IF_LOOPBACK && nic->name)
				for (auto adapter : adapterArray)
					if (-1 != CString(nic->name).Find(adapter.first))
						out.insert(std::make_pair(adapter.second, CString(nic->name)));
		}

		pcap_freealldevs(adapters);

		/*IP_ADAPTER_INFO		adapterInfo[16]; // Allocate information
		DWORD dwBufLen = sizeof(adapterInfo); // Save memory size of buffer

		out.clear();

		if (GetAdaptersInfo(adapterInfo, &dwBufLen) == ERROR_SUCCESS)
		{
			for (auto ptr = adapterInfo; ptr != NULL; ptr = ptr->Next)
			{
				CString name = _T("\\Device\\NPF_");
				name.Append(CString(ptr->AdapterName));
				out.insert(std::make_pair(CString(ptr->Description), name));
			}
		}
		else
			m_szLastError = "PcapHelper::GetNICList: GetAdaptersInfo failed";
			*/
	}

	CString GetIPAddressByNIC(CString nic)
	{
		try 
		{
			CT2CA szAnsiNIC(nic);
			return get_ip_address(std::string(szAnsiNIC)).c_str();
		}
		catch (easy_drcom_exception& ex) 
		{
			m_szLastError = ex.what();
			return CString();
		}
	}

	CString GetMACAddressByNIC(CString nic)
	{
		try 
		{
			CT2CA szAnsiNIC(nic);
			auto mac = get_mac_address(std::string(szAnsiNIC));
			return hex_to_str(&mac[0], 6, ':').c_str();
		}
		catch (easy_drcom_exception& ex) 
		{
			m_szLastError = ex.what();
			return CString();
		}
	}

	bool ModifyRouteTable()
	{
		PMIB_IPFORWARDTABLE pIpForwardTable = nullptr;
		PMIB_IPFORWARDROW pRow = nullptr;
		DWORD dwSize = 0;
		DWORD dwStatus = 0;
		DWORD gatewayIP = 0x040819AC;  // this is in host order Ip Address AA.BB.CC.DD is DDCCBBAA
		bool ret = true;

		// Find out how big our buffer needs to be.
		dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, FALSE);
		if (dwStatus == ERROR_INSUFFICIENT_BUFFER) {
			// Allocate the memory for the table
			pIpForwardTable = new MIB_IPFORWARDTABLE[dwSize];
			// Now get the table.
			dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, FALSE);
		}

		if (dwStatus != ERROR_SUCCESS) {
			if (dwStatus == ERROR_NO_DATA)
				m_szLastError = _T("GetIpForwardTable: ERROR_NO_DATA: No data is available.");
			else
			{
				LPTSTR lpMsgBuf = NULL;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, dwStatus, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 0, NULL);

				m_szLastError.Format(_T("GetIpForwardTable: 0x%X: %s"), dwStatus, lpMsgBuf);
				LocalFree(lpMsgBuf);
			}

			if (pIpForwardTable) delete pIpForwardTable;
			return false;
		}

		// Search for the row in the table we want. The default gateway has a destination of 0.0.0.0.
		for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++) {
			if (pIpForwardTable->table[i].dwForwardDest == 0) {
				// We have found the default gateway.
				if (!pRow) {
					// Allocate some memory to store the row in; this is easier than filling
					// in the row structure ourselves, and we can be sure we change only the
					// gateway address.
					pRow = new MIB_IPFORWARDROW;
					
					// Copy the row
					memcpy(pRow, &(pIpForwardTable->table[i]), sizeof (MIB_IPFORWARDROW));
				}
			}
			else if (pIpForwardTable->table[i].dwForwardDest == gatewayIP)
			{
				dwStatus = DeleteIpForwardEntry(&(pIpForwardTable->table[i]));
				if (dwStatus != ERROR_SUCCESS)
				{
					LPTSTR lpMsgBuf = NULL;
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, dwStatus, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 0, NULL);

					m_szLastError.Format(_T("DeleteIpForwardEntry: 0x%X: %s"), dwStatus, lpMsgBuf);
					LocalFree(lpMsgBuf);
				}
			}
		}
		
		if (pRow == nullptr) // the wire may be disconnected
		{
			ret = false;
			m_szLastError = _T("获取路由表失败，请检查网线是否插好、网卡是否已启用！");
		}
		else
		{
			// Set the dest field to our new ip - all the other properties of the route will
			// be the same as they were previously.
			pRow->dwForwardDest = gatewayIP;
			pRow->dwForwardMask = 0xFFFFFFFF; // 255.255.255.255

			// Create a new route entry for the gateway.
			dwStatus = CreateIpForwardEntry(pRow);

			if (dwStatus == NO_ERROR) ret = true;
			else
			{
				if (dwStatus == ERROR_INVALID_PARAMETER)
					m_szLastError = _T("GetIpForwardTable: ERROR_INVALID_PARAMETER: Invalid parameter.");
				else
				{
					LPTSTR lpMsgBuf = NULL;
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, dwStatus, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 0, NULL);

					m_szLastError.Format(_T("GetIpForwardTable: 0x%X: %s"), dwStatus, lpMsgBuf);
					LocalFree(lpMsgBuf);
				}

				ret = false;
			}
		}
		
		// Free resources
		if (pIpForwardTable) delete pIpForwardTable;
		if (pRow) delete pRow;
		return ret;
	}

public:
	CString m_szLastError;
};