#if !defined(AFX_THREAD_H__20000927_7992_09B2_A0EF_0080AD509054__INCLUDED_)
#define AFX_THREAD_H__20000927_7992_09B2_A0EF_0080AD509054__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Thread - wrapper for Thread API
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#if defined(_MT) || defined(_DLL)
   // Due to the nature of the multithreaded C runtime lib we
   // need to use _beginthreadex() and _endthreadex() instead
   // of CreateThread() and ExitThread(). See Q104641.
   #include <process.h>
#endif // _MT

#ifndef ATLTRY
   #define ATLTRY(x) (x)
#endif // ATLTRY


/////////////////////////////////////////////////////////////////////////////
// CThreadT

template< bool t_bManaged >
class CThreadT
{
public:
   HANDLE m_hThread;       // Handle to thread
   DWORD m_dwThreadID;     // Thread ID
   bool m_bSuspended;      // Thread currently suspended?

   CThreadT(HANDLE hThread = NULL) : m_hThread(hThread), m_dwThreadID(0), m_bSuspended(false)
   {
   }

   ~CThreadT()
   {
      if( t_bManaged ) Release();
   }

   // Operations

   BOOL Create(LPTHREAD_START_ROUTINE pThreadProc, LPVOID pParam = NULL, int iPriority = THREAD_PRIORITY_NORMAL)
   {
      _ASSERTE(m_hThread==NULL);
      _ASSERTE(pThreadProc);
#if defined(_MT) || defined(_DLL)
      m_hThread = (HANDLE) _beginthreadex(NULL, 0, (UINT (WINAPI*)(void*)) pThreadProc, pParam, CREATE_SUSPENDED, (UINT*) &m_dwThreadID);
#else
      m_hThread = ::CreateThread(NULL, 0, pThreadProc, pParam, CREATE_SUSPENDED, &m_dwThreadID);
#endif // _MT
      if( m_hThread == NULL ) return FALSE;
      if( iPriority != THREAD_PRIORITY_NORMAL ) {
         if( !::SetThreadPriority(m_hThread, iPriority) ) {
            _ASSERTE(!"Couldn't set thread priority");
         }
      }
      return ::ResumeThread(m_hThread) != (DWORD) -1;
   }

   BOOL Release()
   {
      if( m_hThread == NULL ) return TRUE;
      if( ::CloseHandle(m_hThread) == FALSE ) return FALSE;
      m_hThread = NULL;
      return TRUE;
   }

   void Attach(HANDLE hThread)
   {
      _ASSERTE(m_hThread==NULL);
      m_hThread = hThread;
   }

   HANDLE Detach()
   {
      HANDLE hThread = m_hThread;
      m_hThread = NULL;
      return hThread;
   }

   BOOL SetPriority(int iPriority) const
   {
      _ASSERTE(m_hThread);
      return ::SetThreadPriority(m_hThread, iPriority);
   }

   int GetPriority() const
   {
      _ASSERTE(m_hThread);
      return ::GetThreadPriority(m_hThread);
   }

   BOOL Suspend()
   {
      _ASSERTE(m_hThread);
      if( m_bSuspended ) return TRUE;
      if( ::SuspendThread(m_hThread) == (DWORD) -1 ) return FALSE;
      m_bSuspended = true;
      return TRUE;
   }

   BOOL Resume()
   {
      _ASSERTE(m_hThread);
      if( !m_bSuspended ) return TRUE;
      if( ::ResumeThread(m_hThread) == (DWORD) -1 ) return FALSE;
      m_bSuspended = false;
      return TRUE;
   }

   BOOL IsSuspended() const
   {
      _ASSERTE(m_hThread);
      return m_bSuspended == true;
   }

   BOOL IsRunning() const
   {
      if( m_hThread == NULL ) return FALSE;
      DWORD dwCode = 0;
      ::GetExitCodeThread(m_hThread, &dwCode);
      return dwCode == STILL_ACTIVE;
   }

   BOOL WaitForThread(DWORD dwTimeout = INFINITE) const
   {
      _ASSERTE(m_hThread);
      return ::WaitForSingleObject(m_hThread, dwTimeout) == WAIT_OBJECT_0;
   }

   BOOL Terminate(DWORD dwExitCode = 0) const
   {
      // See Q254956 why calling this could be a bad idea!
      _ASSERTE(m_hThread);
      return ::TerminateThread(m_hThread, dwExitCode);
   }

   DWORD GetThreadID() const
   {
      return m_dwThreadID;
   }

   BOOL GetExitCode(DWORD* pExitCode) const
   {
      _ASSERTE(m_hThread);
      _ASSERTE(pExitCode);
      return ::GetExitCodeThread(m_hThread, pExitCode);
   }

#if(WINVER >= 0x0500)

   BOOL GetThreadTimes(LPFILETIME lpCreationTime, LPFILETIME lpExitTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime) const
   {
      _ASSERTE(m_hThread);
      _ASSERTE(lpExitTime!=NULL && lpKernelTime!=NULL && lpUserTime!=NULL);
      return ::GetThreadTimes(m_hThread, lpCreationTime, lpExitTime, lpKernelTime, lpUserTime);
   }

#endif // WINVER

#if(WINVER >= 0x0501)

   BOOL SetThreadAffinityMask(DWORD dwThreadMask)
   {
      _ASSERTE(m_hThread);
      return ::SetThreadAffinityMask(m_hThread, dwThreadMask) != 0;
   }

   BOOL SetThreadIdealProcessor(DWORD dwIdealProcessor)
   {
      _ASSERTE(m_hThread);
      return ::SetThreadIdealProcessor(m_hThread, dwIdealProcessor) != (DWORD) -1;
   }

   DWORD GetThreadIdealProcessor() const
   {
      _ASSERTE(m_hThread);
      return ::SetThreadIdealProcessor(m_hThread, MAXIMUM_PROCESSORS);
   }

#endif // WINVER

   operator HANDLE() const 
   { 
      return m_hThread; 
   }
};

/*typedef CThreadT<false> CThreadHandle;
typedef CThreadT<true> CThread;*/


/////////////////////////////////////////////////////////////////////////////
// Thread Stop policy

class CThreadStopAtBool
{
public:
   volatile bool m_bStopped;
   CThreadStopAtBool() : m_bStopped(false) { };
   BOOL _ClearAbort()        { m_bStopped = false; return TRUE; };
   BOOL _Abort()             { m_bStopped = true; return TRUE; };
   BOOL _IsAborted() const   { return m_bStopped == true; };
};

class CThreadStopAtEvent
{
public:
   HANDLE m_hStopEvent;
   CThreadStopAtEvent()      { m_hStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL); };
   ~CThreadStopAtEvent()     { ::CloseHandle(m_hStopEvent); };
   BOOL _ClearAbort()        { return ::ResetEvent(m_hStopEvent); };
   BOOL _Abort()             { return ::SetEvent(m_hStopEvent); };
   BOOL _IsAborted() const   { return ::WaitForSingleObject(m_hStopEvent, 0) != WAIT_TIMEOUT; };
};


/////////////////////////////////////////////////////////////////////////////
// CThreadImpl 

template< typename T, typename TStopPolicy = CThreadStopAtBool >
class CThreadImpl : public CThreadT<true>, public TStopPolicy
{
public:
   bool m_bAutoDelete;     // Thread class will delete itself upon thread exit?
   bool m_bAutoCleanup;    // Thread class will wait for thread completion upon scope exit?

   CThreadImpl() : m_bAutoDelete(false), m_bAutoCleanup(true)
   {
   }

   virtual ~CThreadImpl()
   {
      if( m_bAutoCleanup ) Stop();
   }

   // Operations

   BOOL Start()
   {
      if( !_ClearAbort() ) return FALSE;
      if( !Create(ThreadProc, (LPVOID) static_cast<T*>(this)) ) return FALSE;
      return TRUE;
   }

   void Stop()
   {
      if( !Abort() ) return;
      WaitForThread();
      Release();
   }

   BOOL Abort()
   {
      if( m_hThread == NULL ) return FALSE;
      if( !_Abort() ) return FALSE;
      if( m_bSuspended ) Resume();
      return TRUE;
   }

   BOOL IsAborted() const
   {
      _ASSERTE(m_hThread);
      return _IsAborted();
   }

   void SetAutoClean(bool bAutoClean = true)
   {
      m_bAutoCleanup = bAutoClean;
   }

   void SetDeleteOnExit(bool bAutoDelete = true)
   {
      m_bAutoDelete = bAutoDelete;
      m_bAutoCleanup = !bAutoDelete;
   }

   // Static members

   static DWORD WINAPI ThreadProc(LPVOID pData)
   {
      T* pThis = static_cast<T*>(pData);
      DWORD dwRet = 0;
      ATLTRY( dwRet = pThis->Run() );
      pThis->m_dwThreadID = 0;
      if( pThis->m_bAutoDelete ) delete pThis;
#if defined(_MT) || defined(_DLL)
      _endthreadex(dwRet);
#else
      ::ExitThread(dwRet);
#endif // _MT
      return dwRet;
   }

   // Overridables
   virtual DWORD Run() = 0;
   /*{
      _ASSERTE(false); // must override this
      //
      // Sample thread loop...
      //
      //  while( !IsAborted() ) {
      //    ...
      //  }
      //
      return 0;
   }*/
};

class CThread : public CThreadImpl<CThread>
{
public:
	virtual DWORD Run() = 0;
};

/////////////////////////////////////////////////////////////////////////////
// CMessageThreadImpl

template< typename T >
class CMessageThreadImpl : public CThreadImpl<T, CThreadStopAtEvent>
{
public:
   HANDLE m_hStartEvent;

   // Operations

   BOOL Start()
   {
      m_hStartEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
      BOOL bRes = CThreadImpl<T, CThreadStopAtEvent>::Start();
      if( bRes ) ::WaitForSingleObject(m_hStartEvent, INFINITE);
      ::CloseHandle(m_hStartEvent);
      return bRes;
   }

   BOOL PostQuitMessage()
   {
      _ASSERTE(m_dwThreadID);
      return ::PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0L);
   }

   BOOL PostMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L)
   {
      _ASSERTE(m_dwThreadID);
      return ::PostThreadMessage(m_dwThreadID, uMsg, wParam, lParam);
   }

   // Overridables

   void OnInitQueue()
   {
      ::SetEvent(m_hStartEvent);
   }

   void OnCloseQueue()
   {
   }

   void ProcessWindowMessage(LPVOID /*pReserved*/, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& /*lResult*/, DWORD /*dwReserved*/)
   {
      _ASSERTE(false); // must override this; perhaps implement through an ATL message map
   }

   // Implementation

   DWORD Run()
   {
      T* pT = static_cast<T*>(this); pT;
      MSG msg = { 0 };
      ::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
      pT->OnInitQueue();
      while( true ) {
         DWORD dwRes = ::MsgWaitForMultipleObjectsEx(1, &m_hStopEvent, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
         if( dwRes != WAIT_OBJECT_0 + 1 ) break;
         int iRet = (int) ::GetMessage(&msg, NULL, 0, 0);
         if( iRet <= 0 ) break;
         LRESULT lResult = 0;
         pT->ProcessWindowMessage(NULL, msg.message, msg.wParam, msg.lParam, lResult, 0L);
      }
      pT->OnCloseQueue();
      return 0;
   }
};


/////////////////////////////////////////////////////////////////////////////
// CEvent

class CEvent
{
public:
   HANDLE m_hEvent;

   CEvent(HANDLE hEvent = INVALID_HANDLE_VALUE) : m_hEvent(hEvent)
   { 
   }

   ~CEvent()
   {
      Close();
   }

   BOOL Create(LPCTSTR pstrName = NULL, BOOL bManualReset = FALSE, BOOL bInitialState = FALSE, LPSECURITY_ATTRIBUTES pEventAttributes = NULL)
   {
      _ASSERTE(pstrName==NULL || !::IsBadStringPtr(pstrName,(UINT)-1));
      _ASSERTE(m_hEvent==INVALID_HANDLE_VALUE);
      m_hEvent = ::CreateEvent(pEventAttributes, bManualReset, bInitialState, pstrName);
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return m_hEvent != INVALID_HANDLE_VALUE;
   }

   BOOL Open(LPCTSTR pstrName, DWORD dwDesiredAccess = EVENT_ALL_ACCESS, BOOL bInheritHandle = TRUE)
   {
      _ASSERTE(!::IsBadStringPtr(pstrName,(UINT)-1));
      _ASSERTE(m_hEvent==INVALID_HANDLE_VALUE);
      m_hEvent = ::OpenEvent(dwDesiredAccess, bInheritHandle, pstrName);
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return m_hEvent != INVALID_HANDLE_VALUE;
   }

   BOOL IsOpen() const
   {
      return m_hEvent != INVALID_HANDLE_VALUE;
   }

   void Close()
   {
      if( m_hEvent == INVALID_HANDLE_VALUE ) return;
      ::CloseHandle(m_hEvent);
      m_hEvent = INVALID_HANDLE_VALUE;
   }

   void Attach(HANDLE hEvent)
   {
      _ASSERTE(m_hEvent==INVALID_HANDLE_VALUE);
      m_hEvent= hEvent;
   }  

   HANDLE Detach()
   {
      HANDLE hEvent = m_hEvent;
      m_hEvent = INVALID_HANDLE_VALUE;
      return hEvent;
   }

   BOOL ResetEvent()
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::ResetEvent(m_hEvent);
   }

   BOOL SetEvent()
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::SetEvent(m_hEvent);
   }

   BOOL PulseEvent()
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::PulseEvent(m_hEvent);
   }

   BOOL IsSignalled() const
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::WaitForSingleObject(m_hEvent, 0) == WAIT_OBJECT_0;
   }

   BOOL WaitForEvent(DWORD dwTimeout = INFINITE)
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0;
   }

   operator HANDLE() const 
   { 
      return m_hEvent; 
   }
};


/////////////////////////////////////////////////////////////////////////////
// CCriticalSection

#ifndef __ATLBASE_H__

class CCriticalSection
{
public:
   CRITICAL_SECTION m_sec;

   void Init() 
   {
      ::InitializeCriticalSection(&m_sec);
   }

   void Term() 
   {
      ::DeleteCriticalSection(&m_sec);
   }

   void Lock() 
   {
      ::EnterCriticalSection(&m_sec);
   }

   void Unlock() 
   {
      ::LeaveCriticalSection(&m_sec);
   }
};

class CAutoCriticalSection
{
public:
   CRITICAL_SECTION m_sec;

   CAutoCriticalSection() 
   {
      ::InitializeCriticalSection(&m_sec);
   }

   ~CAutoCriticalSection() 
   {
      ::DeleteCriticalSection(&m_sec);
   }

   void Lock() 
   {
      ::EnterCriticalSection(&m_sec);
   }

   void Unlock() 
   {
      ::LeaveCriticalSection(&m_sec);
   }
};

#endif // __ATLBASE_H__


#endif // !defined(AFX_THREAD_H__20000927_7992_09B2_A0EF_0080AD509054__INCLUDED_)

