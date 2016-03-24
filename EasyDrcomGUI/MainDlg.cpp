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

#include "stdafx.h"
#include "resource.h"

//#include <curl/curl.h>

#include "PcapHelper.hpp"
#include "MainDlg.h"

#include <iostream>
#include <fstream>
#include <functional>

#define EASYDRCOM_DEBUG

#include "../EasyDrcomCore/log.hpp"
#include "../EasyDrcomCore/utils.h"
#include "../EasyDrcomCore/drcom_dealer.hpp"
#include "../EasyDrcomCore/eap_dealer.hpp"

std::ofstream log_stream;

/*
// for curl
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t nSegSize = size * nmemb;
	char *szTemp = new char[nSegSize + 1];
	memcpy(szTemp, buffer, nSegSize);
	*((std::string*)userp) += szTemp;

	delete[] szTemp;
	return nSegSize;
}
*/

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	UIUpdateChildWindows();
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	// open log stream
	CString exePath;
	DWORD pathLength = GetModuleFileName(NULL, exePath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	exePath.ReleaseBuffer(pathLength);
	m_szExeDir = exePath.Left(exePath.ReverseFind(_T('\\')));
	m_szLogPath = m_szExeDir + _T("\\EasyDrcom.log");
	m_szConfPath = m_szExeDir + _T("\\EasyDrcom.conf");
	log_stream.open(m_szLogPath);

	// some var init works
	m_uiOnlineTime = 0;
	m_thConnectTime = nullptr;
	m_thConnectJob = nullptr;
	//m_CURL = nullptr;
	m_bKeepAliveFail = false;

	// get version
	CResource resVersion;
	resVersion.Load(RT_VERSION, VS_VERSION_INFO);

	LPVOID pVersion = LocalAlloc(LMEM_FIXED, resVersion.GetSize());
	CopyMemory(pVersion, resVersion.Lock(), resVersion.GetSize());
	resVersion.Release();

	UINT uLen;
	VS_FIXEDFILEINFO *lpFfi;
	VerQueryValue(pVersion, _T("\\"), (LPVOID*)&lpFfi, &uLen);

	CString version;
	version.Format(_T("EasyDrcomGUI For Win32, v%d.%d"), HIWORD(lpFfi->dwFileVersionMS), LOWORD(lpFfi->dwFileVersionMS));

	SYS_LOG_INFO(std::string(CT2CA(version)) << std::endl);
	SYS_LOG_INFO("Code by Shindo, build on " __DATE__ " " __TIME__ << std::endl);

	// fetch nic list
	SYS_LOG_INFO("Attempt to load NIC list...");
	m_pcapHelper.GetNICList(m_mapNIC);
	if (m_mapNIC.empty()) // no nic found
	{
        LOG_APPEND("Failed!" << std::endl);
		if (!m_pcapHelper.m_szLastError.IsEmpty()) // we have error
		{
			CT2CA ansiError(m_pcapHelper.m_szLastError);
			SYS_LOG_DBG("lastError = " << std::string(ansiError) << std::endl);
			MessageBox(m_pcapHelper.m_szLastError, _T("获取网卡信息失败！"), MB_ICONERROR);
		}
		else
		{
			MessageBox(_T("EasyDrcomGUI未能在您的计算机上找到任何网卡。\n请检查网卡是否被禁用！"), _T("获取网卡信息失败！"), MB_ICONERROR);
			SYS_LOG_DBG("EasyDrcomGUI未能在您的计算机上找到任何网卡。请检查网卡是否被禁用！" << std::endl);
		}

		CloseDialog(0);
		return FALSE;
	}
	LOG_APPEND("OK." << std::endl);

	// create font
	m_fontBigNormal.CreateFont(-14, 0, 0, 0, FW_NORMAL, NULL, NULL, NULL, DEFAULT_CHARSET, NULL, NULL, NULL, NULL, _T("MS Shell Dlg"));
	m_fontBigBold.CreateFont(-14, 0, 0, 0, FW_BOLD, NULL, NULL, NULL, DEFAULT_CHARSET, NULL, NULL, NULL, NULL, _T("MS Shell Dlg"));
	
	// init error dict
	m_mapAuthError.insert({
		{ AuthErrorCodeCheckMAC, _T("账户正在被使用！") },
		{ AuthErrorCodeServerBusy, _T("服务器繁忙，请稍后尝试！") },
		{ AuthErrorCodeWrongPass, _T("账户或密码错误！") },
		{ AuthErrorCodeNotEnough, _T("本账户的累计时间或流量已超出限制！") },
		{ AuthErrorCodeFreezeUp, _T("本账户暂停使用！") },
		{ AuthErrorCodeNotOnThisIP, _T("IP地址不匹配，本账户只能在指定的IP地址上使用！") },
		{ AuthErrorCodeNotOnThisMac, _T("MAC地址不匹配，本账户只能在指定的IP和MAC地址上使用！") },
		{ AuthErrorCodeTooMuchIP, _T("本账户登录的IP地址太多！") },
		{ AuthErrorCodeUpdateClient, _T("客户端版本号不正确！") },
		{ AuthErrorCodeNotOnThisIPMAC, _T("本账户只能在指定的MAC地址和IP地址上使用！") },
		{ AuthErrorCodeMustUseDHCP, _T("你的PC设置了静态IP，请改为动态获取方式(DHCP)，然后重新登录！") },
		{ AuthErrorCodeReserved1, _T("AuthErrorCode24") },
		{ AuthErrorCodeReserved2, _T("AuthErrorCode25") },
		{ AuthErrorCodeReserved3, _T("AuthErrorCode26") },
		{ AuthErrorCodeReserved4, _T("AuthErrorCode27") },
		{ AuthErrorCodeReserved5, _T("AuthErrorCode28") }
	});

	// we need auto-reset, nonsignaled event
	m_evtKeepAlive.Create();
	m_evtKeepAliveFirstTry.Create();

	// load configs
	m_iConnectMode = GetPrivateProfileInt(_T("General"), _T("ConnectMode"), ConnectModeStudentDistrict, m_szConfPath);
	if (m_iConnectMode < ConnectModeStudentDistrict || m_iConnectMode > ConnectModeWorkDistrict)
		m_iConnectMode = ConnectModeStudentDistrict;

	if (m_iConnectMode == ConnectModeStudentDistrict)
	{
		DWORD dwLen;
		dwLen = GetPrivateProfileString(_T("General"), _T("StuDist.NIC"), NULL, m_szStoredNIC.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredNIC.ReleaseBuffer(dwLen);

		dwLen = GetPrivateProfileString(_T("General"), _T("StuDist.UserName"), NULL, m_szStoredUserName.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredUserName.ReleaseBuffer(dwLen);

		dwLen = GetPrivateProfileString(_T("General"), _T("StuDist.PassWord"), NULL, m_szStoredPassWord.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredPassWord.ReleaseBuffer(dwLen);
	}
	else
	{
		DWORD dwLen;
		dwLen = GetPrivateProfileString(_T("General"), _T("WorkDist.NIC"), NULL, m_szStoredNIC.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredNIC.ReleaseBuffer(dwLen);

		dwLen = GetPrivateProfileString(_T("General"), _T("WorkDist.UserName"), NULL, m_szStoredUserName.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredUserName.ReleaseBuffer(dwLen);

		dwLen = GetPrivateProfileString(_T("General"), _T("WorkDist.PassWord"), NULL, m_szStoredPassWord.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredPassWord.ReleaseBuffer(dwLen);
	}
	
	// we'll print later in connect
	/*CT2CA ansiStoredNIC(m_szStoredNIC);
	CT2CA ansiStoredUserName(m_szStoredUserName);
	CT2CA ansiStoredPassWord(m_szStoredPassWord);
	SYS_LOG_DBG("config.General.ConnectMode = " << m_iConnectMode << ", config.General.NIC = " << std::string(ansiStoredNIC) << std::endl);
	SYS_LOG_DBG("config.General.UserName = " << std::string(ansiStoredUserName) << ", config.General.PassWord = " <<  std::string(ansiStoredPassWord) << std::endl);*/

	// bind controls
	DoDataExchange(FALSE);

	// some controls init work
	CStatic(GetDlgItem(IDC_LBL_STATUS_BEFORE)).SetFont(m_fontBigNormal); 
	m_lblStatus.SetFont(m_fontBigBold);

	// add nic to combobox & find select
	bool foundNIC = false;
	for (auto item : m_mapNIC)
	{
		m_ltbNIC.AddString(item.first);

		if (m_szStoredNIC.Compare(item.second) == 0)
		{
			foundNIC = true;
			m_szStoredNICDescription = item.first;
		}
	}

	// select stored nic
	if (foundNIC) m_ltbNIC.SelectString(-1, m_szStoredNICDescription);
	else m_ltbNIC.SetCurSel(0);

	// select connect mode
	switch (m_iConnectMode)
	{
	case ConnectModeStudentDistrict:
		m_rbStudent.SetCheck(BST_CHECKED);
		break;

	case ConnectModeWorkDistrict:
		m_rbWorkplace.SetCheck(BST_CHECKED);
		break;

	default:
		m_iConnectMode = ConnectModeStudentDistrict;
		m_rbStudent.SetCheck(BST_CHECKED);
		break;
	}

	// init user info with stored
	m_txtUserName.SetWindowText(m_szStoredUserName);
	m_txtPassWord.SetWindowText(m_szStoredPassWord);

	// set version info for title
	CString& title = version;
	SetWindowText(title);
	LocalFree(pVersion);

	// set build info
	m_lblBuild.SetWindowText(CString("Code by Shindo, build on " __DATE__ " " __TIME__));

	// set "view log" button style to command button
	m_btnViewLog.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON);

	// set up tray icon
	InstallIcon(title, hIconSmall, IDR_POPUP);

	// we've dropped the online check now
	/*m_CURL = curl_easy_init();
	curl_easy_setopt(m_CURL, CURLOPT_URL, "http://172.25.8.4");
	curl_easy_setopt(m_CURL, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(m_CURL, CURLOPT_TIMEOUT, 1L); // 1s for every action*/

	SYS_LOG_INFO("EasyDrcomGUI is ready." << std::endl);
	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT CMainDlg::OnBringToFront(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	::ShowWindow(m_hWnd, SW_SHOW);
	::SetForegroundWindow(m_hWnd);
	::SetActiveWindow(m_hWnd);
	return 0;
}

// also called by the 'X' button.
LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!m_thConnectJob)
	{
		CloseDialog(0);
		return 0;
	}

	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CMainDlg::OnSupportClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, _T("open"), _T("https://github.com/coverxit/EasyDrcom"), NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

LRESULT CMainDlg::OnViewLogClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	log_stream.flush();
	ShellExecute(NULL, _T("open"), m_szLogPath, NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

LRESULT CMainDlg::OnModeChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_rbStudent.GetCheck() == BST_CHECKED)
		m_iConnectMode = ConnectModeStudentDistrict;
	else if (m_rbWorkplace.GetCheck() == BST_CHECKED)
		m_iConnectMode = ConnectModeWorkDistrict;
	else // wtf?!
		m_iConnectMode = ConnectModeStudentDistrict;

	// load user info
	if (m_iConnectMode == ConnectModeStudentDistrict)
	{
		DWORD dwLen;
		dwLen = GetPrivateProfileString(_T("General"), _T("StuDist.NIC"), NULL, m_szStoredNIC.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredNIC.ReleaseBuffer(dwLen);

		dwLen = GetPrivateProfileString(_T("General"), _T("StuDist.UserName"), NULL, m_szStoredUserName.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredUserName.ReleaseBuffer(dwLen);

		dwLen = GetPrivateProfileString(_T("General"), _T("StuDist.PassWord"), NULL, m_szStoredPassWord.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredPassWord.ReleaseBuffer(dwLen);
	}
	else
	{
		DWORD dwLen;
		dwLen = GetPrivateProfileString(_T("General"), _T("WorkDist.NIC"), NULL, m_szStoredNIC.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredNIC.ReleaseBuffer(dwLen);

		dwLen = GetPrivateProfileString(_T("General"), _T("WorkDist.UserName"), NULL, m_szStoredUserName.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredUserName.ReleaseBuffer(dwLen);

		dwLen = GetPrivateProfileString(_T("General"), _T("WorkDist.PassWord"), NULL, m_szStoredPassWord.GetBufferSetLength(MAX_PATH + 1), MAX_PATH, m_szConfPath);
		m_szStoredPassWord.ReleaseBuffer(dwLen);
	}

	// find nic
	bool foundNIC = false;
	for (auto item : m_mapNIC)
	{
		if (m_szStoredNIC.Compare(item.second) == 0)
		{
			foundNIC = true;
			m_szStoredNICDescription = item.first;
		}
	}

	// select stored nic
	if (foundNIC) m_ltbNIC.SelectString(-1, m_szStoredNICDescription);
	else m_ltbNIC.SetCurSel(0);

	// put user info with stored
	m_txtUserName.SetWindowText(m_szStoredUserName);
	m_txtPassWord.SetWindowText(m_szStoredPassWord);

	return 0;
}

LRESULT CMainDlg::OnConnectClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString btnValue;
	m_btnConnect.GetWindowText(btnValue);

	if (btnValue.Compare(_T("连接")) == 0)
	{
		m_ltbNIC.GetLBText(m_ltbNIC.GetCurSel(), m_szStoredNICDescription);
		m_szStoredNIC = m_mapNIC[m_szStoredNICDescription];

		m_txtUserName.GetWindowText(m_szStoredUserName);
		m_txtPassWord.GetWindowText(m_szStoredPassWord);
		
		if (m_szStoredUserName.IsEmpty() || m_szStoredPassWord.IsEmpty())
		{
			MessageBox(_T("账户或密码不能为空！"), _T("EasyDrcomGUI"), MB_ICONERROR);
			return 0;
		}

		SYS_LOG_INFO("Prepare to authenticate..." << std::endl);

		// save settings
		CString connectMode;
		connectMode.Format(_T("%d"), m_iConnectMode);
		WritePrivateProfileString(_T("General"), _T("ConnectMode"), connectMode, m_szConfPath);

		if (m_iConnectMode == ConnectModeStudentDistrict)
		{
			WritePrivateProfileString(_T("General"), _T("StuDist.NIC"), m_szStoredNIC, m_szConfPath);
			WritePrivateProfileString(_T("General"), _T("StuDist.UserName"), m_szStoredUserName, m_szConfPath);
			WritePrivateProfileString(_T("General"), _T("StuDist.PassWord"), m_szStoredPassWord, m_szConfPath);
		}
		else
		{
			WritePrivateProfileString(_T("General"), _T("WorkDist.NIC"), m_szStoredNIC, m_szConfPath);
			WritePrivateProfileString(_T("General"), _T("WorkDist.UserName"), m_szStoredUserName, m_szConfPath);
			WritePrivateProfileString(_T("General"), _T("WorkDist.PassWord"), m_szStoredPassWord, m_szConfPath);
		}
		

		CT2CA ansiNIC(m_szStoredNIC), ansiNICDesc(m_szStoredNICDescription);
		CT2CA ansiUserName(m_szStoredUserName), ansiPassWord(m_szStoredPassWord);

		SYS_LOG_DBG("ConnectMode = " << m_iConnectMode << ", NIC = " << std::string(ansiNIC) << std::endl);
		SYS_LOG_DBG("NIC Desc = " << std::string(ansiNICDesc) << std::endl);
		SYS_LOG_DBG("UserName = " << std::string(ansiUserName) << ", PassWord = " << std::string(ansiPassWord) << std::endl);

		// get ip & mac
		SYS_LOG_INFO("Attempt to fetch IP & MAC...");
		m_szStoredIP = m_pcapHelper.GetIPAddressByNIC(m_szStoredNIC);
		m_szStoredMAC = m_pcapHelper.GetMACAddressByNIC(m_szStoredNIC);
		if (m_szStoredIP.IsEmpty() || m_szStoredMAC.IsEmpty())
		{
			LOG_APPEND("Failed!" << std::endl);
			CT2CA ansiError(m_pcapHelper.m_szLastError);
			SYS_LOG_ERR("lastError = " << std::string(ansiError) << std::endl);
			MessageBox(m_pcapHelper.m_szLastError, _T("获取网卡信息失败！"), MB_ICONERROR);
			return 0;
		}
		LOG_APPEND("OK." << std::endl);

		m_lblIP.SetWindowText(m_szStoredIP);
		m_lblMAC.SetWindowTextW(m_szStoredMAC);

		CT2CA ansiIP(m_szStoredIP), ansiMAC(m_szStoredMAC);
		SYS_LOG_DBG("IP = " << std::string(ansiIP) << ", MAC = " << std::string(ansiMAC) << std::endl);
		SYS_LOG_INFO("Preparation done." << std::endl);

		switch (m_iConnectMode)
		{
		case ConnectModeStudentDistrict:
			m_thConnectJob = new CThreadStuDistConnect(this, nullptr);
			break;

		case ConnectModeWorkDistrict:
			m_thConnectJob = new CThreadWorkDistConnect(this, nullptr);
			break;

		default:
			m_thConnectJob = new CThreadStuDistConnect(this, nullptr);
			break;
		}

		m_lblStatus.SetWindowText(_T("准备连接中……"));
		m_lblOnlineTime.SetWindowText(_T("00:00:00"));

		m_rbStudent.EnableWindow(FALSE);
		m_rbWorkplace.EnableWindow(FALSE);
		m_ltbNIC.EnableWindow(FALSE);
		m_txtUserName.EnableWindow(FALSE);
		m_txtPassWord.EnableWindow(FALSE);
		m_btnConnect.EnableWindow(FALSE);

		m_thConnectJob->SetDeleteOnExit();
		m_thConnectJob->Start();
	}
	else if (btnValue.Compare(_T("断开")) == 0)
	{
		m_btnConnect.EnableWindow(FALSE);
		m_thConnectJob->Abort();
		m_evtKeepAlive.SetEvent();
	}

	return 0;
}

void CMainDlg::CloseDialog(int nVal = 0)
{
	SYS_LOG_INFO("EasyDrcomGUI quit." << std::endl);

	/*if (m_CURL != nullptr)
		curl_easy_cleanup(m_CURL);*/

	DestroyWindow();
	::PostQuitMessage(nVal);
}

void CMainDlg::ResetOnlineTime()
{
	if (m_thConnectTime)
	{
		m_thConnectTime->Abort();
		m_thConnectTime = nullptr;
	}

	m_uiOnlineTime = 0;
	m_lblOnlineTime.SetWindowText(_T("00:00:00"));
}

LRESULT CMainDlg::OnPopupShowWindowClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CMainDlg::OnPopupExitClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_thConnectJob)
	{
		if (IDNO == MessageBox(_T("一旦退出EasyDrcomGUI，您将会在稍后失去网络连接。"), _T("确定要退出吗？"), MB_ICONEXCLAMATION | MB_YESNO))
			return 0;
	}

	CloseDialog(0);
	return 0;
}

DWORD CMainDlg::CThreadConnectTime::Run()
{
	while (!IsAborted())
	{
		int connectTime = dlg->m_uiOnlineTime++;
		int seconds = connectTime % 60;
		int minutes = (connectTime / 60) % 60;
		int hours = connectTime / 3600;

		CString stringTime;
		stringTime.Format(_T("%02d:%02d:%02d"), hours, minutes, seconds);
		dlg->m_lblOnlineTime.SetWindowText(stringTime);

		Sleep(1000);
	}

	return 0;
}

void CMainDlg::GatewayNotificate(CString string, CString type)
{
	CT2CA ansiString(string);
	CT2CA ansiType(type);
	SYS_LOG_INFO("Gateway notificate - " << std::string(ansiType) << ": " << std::string(ansiString) << std::endl);

	CString title;
	title.Format(_T("网关通知 - %s"), type);
	MessageBox(string, title, MB_ICONINFORMATION);
}

DWORD CMainDlg::CThreadStuDistKeepAlive::Run()
{
	bool firstTry = true;
	drcom_dealer_u62 *dealer = reinterpret_cast<drcom_dealer_u62*>(param);

	try 
	{
		// first try
		dlg->m_lblStatus.SetWindowText(_T("发送心跳包中……"));
		if (dealer->send_alive_pkt1()) goto udp_fail;
		if (dealer->send_alive_pkt2()) goto udp_fail;

		/*if (dlg->CheckIsOnline())
		{
			U62_LOG_INFO("checkIsOnline succeeded." << std::endl);
		}
		else // WHAT THE FUCK!!
		{
			U62_LOG_ERR("checkIsOnline failed." << std::endl);
			goto udp_fail;
		}*/

		dlg->m_lblStatus.SetWindowText(_T("已连接上"));
		dlg->m_evtKeepAliveFirstTry.SetEvent();

		firstTry = false;
		while (!IsAborted())
		{
			Sleep(20000); // 20s for alive
			if (IsAborted()) break;

			dlg->m_lblStatus.SetWindowText(_T("发送心跳包中……"));
			if (dealer->send_alive_pkt1()) goto udp_fail;
			if (dealer->send_alive_pkt2()) goto udp_fail;

			/*if (dlg->CheckIsOnline())
			{
				U62_LOG_INFO("checkIsOnline succeeded." << std::endl);
			}
			else // WHAT THE FUCK!!
			{
				U62_LOG_ERR("checkIsOnline failed." << std::endl);
				goto udp_fail;
			}*/

			dlg->m_lblStatus.SetWindowText(_T("已连接上"));
		}
	}
	catch (std::exception&) 
	{
		goto udp_fail;
	}
	return 0;

udp_fail:
	if (IsAborted()) return 0;

	dlg->m_lblStatus.SetWindowText(_T("发送心跳包失败！"));
	CEvent& event = firstTry ? dlg->m_evtKeepAliveFirstTry : dlg->m_evtKeepAlive;
	
	dlg->m_bKeepAliveFail = true;
	event.SetEvent();
	return 0;
}

DWORD CMainDlg::CThreadStuDistConnect::Run()
{
	std::vector<uint8_t> broadcast_mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	std::vector<uint8_t> nearest_mac = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x03 };

	CThread *threadKeepAlive = nullptr;
	drcom_dealer_u62* udp = nullptr;
	eap_dealer* eap = nullptr;
	bool firstTry = true;

	while (!IsAborted()) // auto-redial
	{
		dlg->m_bKeepAliveFail = false;

		// modify route table
		dlg->m_lblStatus.SetWindowText(_T("刷新路由表中……"));
		if (!dlg->m_pcapHelper.ModifyRouteTable())
		{
			CT2CA printString(dlg->m_pcapHelper.m_szLastError);
			SYS_LOG_ERR("Failed to modify route table: " << std::string(printString) << std::endl);

			if (!firstTry) // not first try, keep retry
				goto eap_fail;
			else
			{
				CString content;
				content.Format(_T("错误信息：\n%s"), dlg->m_pcapHelper.m_szLastError);
				MessageBox(dlg->m_hWnd, content, _T("刷新路由表失败！"), MB_ICONERROR);
				goto interrupt;
			}
		}

#define CSTRING_TO_STD(x) std::string(CT2CA(x))
		try {
			eap = new eap_dealer(CSTRING_TO_STD(dlg->m_szStoredNIC), str_mac_to_vec(CSTRING_TO_STD(dlg->m_szStoredMAC)), 
				CSTRING_TO_STD(dlg->m_szStoredIP), CSTRING_TO_STD(dlg->m_szStoredUserName), CSTRING_TO_STD(dlg->m_szStoredPassWord));	
		}
		catch (std::exception& ex) {
			EAP_LOG_ERR(ex.what() << std::endl);
			goto eap_fail;
		}

		try {
			udp = new drcom_dealer_u62(str_mac_to_vec(CSTRING_TO_STD(dlg->m_szStoredMAC)), CSTRING_TO_STD(dlg->m_szStoredIP),
				CSTRING_TO_STD(dlg->m_szStoredUserName), CSTRING_TO_STD(dlg->m_szStoredPassWord), "172.25.8.4", 61440, "EasyDrcomGUI", "for Win32, Core v0.9");
		}
		catch (std::exception& ex) {
			U62_LOG_ERR(ex.what() << std::endl);
			goto eap_fail;
		}
#undef CSTRING_TO_STD

		dlg->m_lblStatus.SetWindowText(_T("802.1X清理中……"));
		eap->logoff(nearest_mac);
		eap->logoff(nearest_mac);

		try {
			dlg->m_lblStatus.SetWindowText(_T("802.1X认证中……"));
			if (eap->start(broadcast_mac)) goto eap_fail;
			if (eap->response_identity(broadcast_mac)) goto eap_fail;

			switch (eap->response_md5_challenge(broadcast_mac))
			{
				// success
			case 0:
				threadKeepAlive = new CMainDlg::CThreadStuDistKeepAlive(dlg, (LPVOID) udp);
				threadKeepAlive->SetDeleteOnExit();
				threadKeepAlive->Start();
				break;

				// notifications
			case 1:
				dlg->GatewayNotificate(eap->getNotification().c_str(), _T("EAP"));
				goto interrupt;

			case 2:
				dlg->GatewayNotificate(_T("账户不存在！"), _T("EAP"));
				goto interrupt;

			case 3:
				dlg->GatewayNotificate(_T("账户或密码错误！"), _T("EAP"));
				goto interrupt;

			case 4:
				dlg->GatewayNotificate(_T("您账号绑定的网卡MAC地址与现有网卡MAC地址不匹配！\n\n提示：在学生区使用时，帐号不需要加'o'。\n参考代码：Mac,IP,NASip,PORTerr(2)!"), _T("EAP"));
				goto interrupt;

			case 5:
				dlg->GatewayNotificate(_T("您账号绑定的网卡MAC地址与现有网卡MAC地址不匹配！\n\n参考代码：Mac,IP,NASip,PORTerr(11)!"), _T("EAP"));
				goto interrupt;

			case 6:
				dlg->GatewayNotificate(_T("您的账号已在线！\n\n参考代码：In use !"), _T("EAP"));
				goto interrupt;

			case 7:
				dlg->GatewayNotificate(_T("您的账号已暂停使用！\n\n提示：您的账号可能已欠费停机。\n参考代码：Authentication Fail ErrCode=05"), _T("EAP"));
				goto interrupt;

			default: // other errors
				goto eap_fail;
			}
		}
		catch (std::exception&) {
			goto eap_fail;
		}

		dlg->m_evtKeepAliveFirstTry.WaitForEvent();
		if (dlg->m_bKeepAliveFail)
		{
			if (firstTry) goto firstFail; // first try failed
			else goto eap_fail;
		}
		else
		{
			if (firstTry)
			{
				dlg->ResetOnlineTime();
				dlg->m_thConnectTime = new CMainDlg::CThreadConnectTime(dlg, nullptr);
				dlg->m_thConnectTime->SetDeleteOnExit();
				dlg->m_thConnectTime->Start();

				firstTry = false;
			}

			dlg->m_lblStatus.SetWindowText(_T("已连接上"));
			dlg->m_btnConnect.SetWindowText(_T("断开"));
			dlg->m_btnConnect.EnableWindow();
		}

		while (!dlg->m_bKeepAliveFail && !IsAborted())
			dlg->m_evtKeepAlive.WaitForEvent();

		if (dlg->m_bKeepAliveFail && !IsAborted()) goto eap_fail;
		else continue;

	eap_fail:
		if (firstTry) // first try failed
		{
			dlg->m_lblStatus.SetWindowText(_T("802.1X认证失败！"));
			goto firstFail;
		}
		else
		{
			if (IsAborted()) break;

			dlg->m_lblStatus.SetWindowText(_T("连接丢失，5秒后重试。"));
			Sleep(5000);

			if (eap) delete eap;
			if (udp) delete udp;
			eap = nullptr; 
			udp = nullptr;
		}
	}

	// canceled
	if (threadKeepAlive != nullptr && threadKeepAlive->IsRunning())
		threadKeepAlive->Abort();

	dlg->m_lblStatus.SetWindowText(_T("802.1X注销中……"));
	if (eap) eap->logoff(nearest_mac);

interrupt:
	dlg->m_lblStatus.SetWindowText(_T("已断开"));

firstFail:
	dlg->ResetOnlineTime();
	dlg->m_rbStudent.EnableWindow();
	dlg->m_rbWorkplace.EnableWindow();
	dlg->m_ltbNIC.EnableWindow();
	dlg->m_txtUserName.EnableWindow();
	dlg->m_txtPassWord.EnableWindow();
	dlg->m_btnConnect.EnableWindow();

	dlg->m_lblIP.SetWindowText(_T("-"));
	dlg->m_lblMAC.SetWindowText(_T("-"));
	dlg->m_btnConnect.SetWindowText(_T("连接"));

	dlg->m_thConnectJob = nullptr;

	if (eap) delete eap;
	if (udp) delete udp;
	return 0;
}

DWORD CMainDlg::CThreadWorkDistKeepAlive::Run()
{
	bool firstTry = true;
	drcom_dealer_u31 *dealer = reinterpret_cast<drcom_dealer_u31*>(param);

	try
	{
		// first try
		dlg->m_lblStatus.SetWindowText(_T("发送心跳包中……"));
		if (dealer->send_alive_request()) goto udp_fail;
		if (dealer->send_alive_pkt1()) goto udp_fail;
		if (dealer->send_alive_pkt2()) goto udp_fail;

		/*if (dlg->CheckIsOnline())
		{
			U31_LOG_INFO("checkIsOnline succeeded." << std::endl);
		}
		else  // WHAT THE FUCK!!
		{
			U31_LOG_ERR("checkIsOnline failed." << std::endl);
			goto udp_fail;
		}*/

		dlg->m_lblStatus.SetWindowText(_T("已连接上"));
		dlg->m_evtKeepAliveFirstTry.SetEvent();

		firstTry = false;
		while (!IsAborted())
		{
			Sleep(10000); // 10s for alive
			if (IsAborted()) break;

			dlg->m_lblStatus.SetWindowText(_T("发送心跳包中……"));
			if (dealer->send_alive_request()) goto udp_fail;
			if (dealer->send_alive_pkt1()) goto udp_fail;
			if (dealer->send_alive_pkt2()) goto udp_fail;

			/*if (dlg->CheckIsOnline())
			{
				U31_LOG_INFO("checkIsOnline succeeded." << std::endl);
			}
			else // WHAT THE FUCK!!
			{
				U31_LOG_ERR("checkIsOnline failed." << std::endl);
				goto udp_fail;
			}*/

			dlg->m_lblStatus.SetWindowText(_T("已连接上"));
		}
	}
	catch (std::exception&)
	{
		goto udp_fail;
	}
	return 0;

udp_fail:
	if (IsAborted()) return 0;

	dlg->m_lblStatus.SetWindowText(_T("发送心跳包失败！"));
	CEvent& event = firstTry ? dlg->m_evtKeepAliveFirstTry : dlg->m_evtKeepAlive;

	dlg->m_bKeepAliveFail = true;
	event.SetEvent();
	return 0;
}

DWORD CMainDlg::CThreadWorkDistConnect::Run()
{
	CThread *threadKeepAlive = nullptr;
	drcom_dealer_u31* udp = nullptr;
	bool firstTry = true;

	while (!IsAborted()) // auto-redial
	{
		dlg->m_bKeepAliveFail = false;

		// modify route table
		dlg->m_lblStatus.SetWindowText(_T("刷新路由表中……"));
		if (!dlg->m_pcapHelper.ModifyRouteTable())
		{
#define CSTRING_TO_STD(x) std::string(CT2CA(x))
			SYS_LOG_ERR("Failed to modify route table: " << CSTRING_TO_STD(dlg->m_pcapHelper.m_szLastError) << std::endl);
#undef CSTRING_TO_STD

			if (!firstTry) // not first try, keep retry
				goto udp_fail;
			else
			{
				CString content;
				content.Format(_T("错误信息：\n%s"), dlg->m_pcapHelper.m_szLastError);
				MessageBox(dlg->m_hWnd, content, _T("刷新路由表失败！"), MB_ICONERROR);
				goto interrupt;
			}
		}

		try {
#define CSTRING_TO_STD(x) std::string(CT2CA(x))
			udp = new drcom_dealer_u31(str_mac_to_vec(CSTRING_TO_STD(dlg->m_szStoredMAC)), CSTRING_TO_STD(dlg->m_szStoredIP),
				CSTRING_TO_STD(dlg->m_szStoredUserName), CSTRING_TO_STD(dlg->m_szStoredPassWord), "172.25.8.4", 61440, "EasyDrcomGUI", "for Win32, Core v0.9");
#undef CSTRING_TO_STD
		}
		catch (std::exception&) {
			goto udp_fail;
		}

		try {
			dlg->m_lblStatus.SetWindowText(_T("UDP认证中……"));
			if (udp->start_request()) goto udp_fail;

			int ret = udp->send_login_auth();
			if (ret == 0) // success
			{
				threadKeepAlive = new CMainDlg::CThreadWorkDistKeepAlive(dlg, (LPVOID) udp);
				threadKeepAlive->SetDeleteOnExit();
				threadKeepAlive->Start();
			}
			else
			{
				if (ret < 0) // must be error
					goto udp_fail;

				if (dlg->m_mapAuthError.find(ret) == dlg->m_mapAuthError.end()) // not specific error
				{
					CString error;
					error.Format(_T("Unknown auth error code: %d"), ret);
					dlg->GatewayNotificate(error, _T("UDP"));
					goto interrupt;
				}
				else
				{
					dlg->GatewayNotificate(dlg->m_mapAuthError[ret], _T("UDP"));
					goto interrupt;
				}
			}
		}
		catch (std::exception&) {
			goto udp_fail;
		}

		dlg->m_evtKeepAliveFirstTry.WaitForEvent();
		if (dlg->m_bKeepAliveFail)
		{
			if (firstTry) goto firstFail; // first try failed
			else goto udp_fail;
		}
		else
		{
			if (firstTry) 
			{
				dlg->ResetOnlineTime();

				dlg->m_thConnectTime = new CMainDlg::CThreadConnectTime(dlg, nullptr);
				dlg->m_thConnectTime->SetDeleteOnExit();
				dlg->m_thConnectTime->Start();

				firstTry = false;
			}
			
			dlg->m_lblStatus.SetWindowText(_T("已连接上"));
			dlg->m_btnConnect.SetWindowText(_T("断开"));
			dlg->m_btnConnect.EnableWindow();
		}

		while (!dlg->m_bKeepAliveFail && !IsAborted())
			dlg->m_evtKeepAlive.WaitForEvent();

		if (dlg->m_bKeepAliveFail && !IsAborted()) goto udp_fail;
		else continue;

	udp_fail:
		if (firstTry) // first try failed
		{
			dlg->m_lblStatus.SetWindowText(_T("UDP认证失败！"));
			goto firstFail;
		}
		else
		{
			if (IsAborted()) break;

			dlg->m_lblStatus.SetWindowText(_T("连接丢失，5秒后重试。"));
			Sleep(5000);

			if (udp) delete udp;
			udp = nullptr;
		}
	}

	// canceled
	if (threadKeepAlive != nullptr && threadKeepAlive->IsRunning())
		threadKeepAlive->Abort();

	dlg->m_lblStatus.SetWindowText(_T("UDP注销中……"));
	try {
		if (udp)
		{
			udp->send_alive_request();
			udp->start_request();
			udp->send_logout_auth();
		}
	}
	catch (std::exception&) {
		// fuck it, but we don't mangage it :P
	}

interrupt:
	dlg->m_lblStatus.SetWindowText(_T("已断开"));

firstFail:
	dlg->ResetOnlineTime();
	dlg->m_rbStudent.EnableWindow();
	dlg->m_rbWorkplace.EnableWindow();
	dlg->m_ltbNIC.EnableWindow();
	dlg->m_txtUserName.EnableWindow();
	dlg->m_txtPassWord.EnableWindow();
	dlg->m_btnConnect.EnableWindow();

	dlg->m_lblIP.SetWindowText(_T("-"));
	dlg->m_lblMAC.SetWindowText(_T("-"));
	dlg->m_btnConnect.SetWindowText(_T("连接"));

	dlg->m_thConnectJob = nullptr;

	if (udp) delete udp;
	return 0;
}

/*bool CMainDlg::CheckIsOnline()
{
	bool ret = true;
	std::string strRetData;

	curl_easy_setopt(m_CURL, CURLOPT_WRITEDATA, &strRetData);
	CURLcode res = curl_easy_perform(m_CURL);

	if (res != CURLE_OK)
		ret = false;
	else
	{
		if (strRetData.find("Logout") == -1) // seems we're not online.
			ret = false;
	}

	return ret;
}*/