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

#define DECLARE_THREAD_OBJECT(theClass)				\
	template <typename T>							\
	class theClass : public CThread {				\
	private:										\
		T* dlg;										\
		LPVOID param;								\
	public:											\
		theClass(T* dlg, LPVOID param)				\
			: dlg(dlg), param(param) {}				\
		DWORD Run();								\
	};

DECLARE_THREAD_OBJECT(CThreadConnectTimeT)
DECLARE_THREAD_OBJECT(CThreadStuDistConnectT)
DECLARE_THREAD_OBJECT(CThreadWorkDistConnectT)
DECLARE_THREAD_OBJECT(CThreadStuDistKeepAliveT)
DECLARE_THREAD_OBJECT(CThreadWorkDistKeepAliveT)

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>, public CWinDataExchange<CMainDlg>,
				 public CTrayIconImpl<CMainDlg>, public CMessageFilter, public CIdleHandler
{
public:
	typedef CThreadConnectTimeT<CMainDlg> CThreadConnectTime;
	typedef CThreadStuDistConnectT<CMainDlg> CThreadStuDistConnect;
	typedef CThreadWorkDistConnectT<CMainDlg> CThreadWorkDistConnect;
	typedef CThreadStuDistKeepAliveT<CMainDlg> CThreadStuDistKeepAlive;
	typedef CThreadWorkDistKeepAliveT<CMainDlg> CThreadWorkDistKeepAlive;

public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_BRING_TO_FRONT, OnBringToFront)

		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BTN_SUPPORT, OnSupportClicked)
		COMMAND_ID_HANDLER(IDC_VIEW_LOG, OnViewLogClicked)

		COMMAND_HANDLER(IDC_RADIO_STUDENT, BN_CLICKED, OnModeChanged)
		COMMAND_HANDLER(IDC_RADIO_WORKPLACE, BN_CLICKED, OnModeChanged)

		COMMAND_ID_HANDLER(IDC_BTN_CONNECT, OnConnectClicked)

		COMMAND_ID_HANDLER(ID_POPUP_SHOW_WINDOW, OnPopupShowWindowClicked)
		COMMAND_ID_HANDLER(ID_POPUP_EXIT, OnPopupExitClicked)

		CHAIN_MSG_MAP(CTrayIconImpl<CMainDlg>)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CMainDlg)
		DDX_CONTROL_HANDLE(IDC_BUILD, m_lblBuild)
		DDX_CONTROL(IDC_VIEW_LOG, m_btnViewLog)
		DDX_CONTROL_HANDLE(IDC_RADIO_STUDENT, m_rbStudent)
		DDX_CONTROL_HANDLE(IDC_RADIO_WORKPLACE, m_rbWorkplace)
		DDX_CONTROL_HANDLE(IDC_COMBO_NIC, m_ltbNIC)
		DDX_CONTROL_HANDLE(IDC_EDIT_USERNAME, m_txtUserName)
		DDX_CONTROL_HANDLE(IDC_EDIT_PASSWORD, m_txtPassWord)
		DDX_CONTROL_HANDLE(IDC_BTN_CONNECT, m_btnConnect)
		DDX_CONTROL_HANDLE(IDC_LBL_STATUS, m_lblStatus)
		DDX_CONTROL_HANDLE(IDC_LBL_ONLINE_TIME, m_lblOnlineTime)
		DDX_CONTROL_HANDLE(IDC_LBL_IP_ADDR, m_lblIP)
		DDX_CONTROL_HANDLE(IDC_LBL_MAC_ADDR, m_lblMAC)
	END_DDX_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBringToFront(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnSupportClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewLogClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnModeChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnConnectClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnPopupShowWindowClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPopupExitClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);

// control variables
public:
	CStatic m_lblBuild;
	CHyperLink m_btnViewLog;
	CButton m_rbStudent, m_rbWorkplace;
	CComboBox m_ltbNIC;
	CEdit m_txtUserName, m_txtPassWord;
	CButton m_btnConnect;
	CStatic m_lblStatus, m_lblOnlineTime, m_lblIP, m_lblMAC;

// other variables
	enum
	{ 
		ConnectModeStudentDistrict = 0, 
		ConnectModeWorkDistrict = 1 
	};

	enum
	{
		AuthErrorCodeCheckMAC = 0x01,
		AuthErrorCodeServerBusy = 0x02,
		AuthErrorCodeWrongPass = 0x03,
		AuthErrorCodeNotEnough = 0x04,
		AuthErrorCodeFreezeUp = 0x05,
		AuthErrorCodeNotOnThisIP = 0x07,
		AuthErrorCodeNotOnThisMac = 0x0B,
		AuthErrorCodeTooMuchIP = 0x14,
		AuthErrorCodeUpdateClient = 0x15,
		AuthErrorCodeNotOnThisIPMAC = 0x16,
		AuthErrorCodeMustUseDHCP = 0x17,
		AuthErrorCodeReserved1 = 0x18,
		AuthErrorCodeReserved2 = 0x19,
		AuthErrorCodeReserved3 = 0x1A,
		AuthErrorCodeReserved4 = 0x1B,
		AuthErrorCodeReserved5 = 0x1C,
	};
	
	int m_iConnectMode;
	unsigned int m_uiOnlineTime;
	CString m_szExeDir, m_szLogPath, m_szConfPath;
	CFont m_fontBigNormal, m_fontBigBold;
	std::map<unsigned int, CString> m_mapAuthError;

	CThread *m_thConnectTime, *m_thConnectJob;

	PcapHelper m_pcapHelper;
	std::map<CString, CString> m_mapNIC;

	CString m_szStoredNIC, m_szStoredNICDescription, m_szStoredUserName, m_szStoredPassWord, m_szStoredIP, m_szStoredMAC;

	bool m_bKeepAliveFail;
	CEvent m_evtKeepAliveFirstTry, m_evtKeepAlive;

	//CURL *m_CURL;

// functions
	void ResetOnlineTime();
	void GatewayNotificate(CString string, CString type);
	//bool CheckIsOnline();
};
