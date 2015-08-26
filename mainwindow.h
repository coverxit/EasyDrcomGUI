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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>
#include <QCloseEvent>
#include <QMutex>
#include <QWaitCondition>
#include <QSharedPointer>

//#include <curl/curl.h>
#include "PcapHelper.hpp"
#include "CancellableThread.h"

#define CURRENT_VERSION "1.2"

namespace Ui {
class MainWindow;
}

DECLARE_THREAD_OBJECT(ThreadConnectTimeT)
DECLARE_THREAD_OBJECT(ThreadStuDistConnectT)
DECLARE_THREAD_OBJECT(ThreadWorkDistConnectT)
DECLARE_THREAD_OBJECT(ThreadStuDistKeepAliveT)
DECLARE_THREAD_OBJECT(ThreadWorkDistKeepAliveT)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef ThreadConnectTimeT<MainWindow> ThreadConnectTime;
    typedef ThreadStuDistConnectT<MainWindow> ThreadStuDistConnect;
    typedef ThreadWorkDistConnectT<MainWindow> ThreadWorkDistConnect;
    typedef ThreadStuDistKeepAliveT<MainWindow> ThreadStuDistKeepAlive;
    typedef ThreadWorkDistKeepAliveT<MainWindow> ThreadWorkDistKeepAlive;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbSupport_clicked();
    void on_rbStudent_clicked();
    void on_pbConnect_clicked();
    void on_pbViewLog_clicked();
    void on_rbWorkplace_clicked();

    void on_pbAboutQt_clicked();

protected:
    void closeEvent(QCloseEvent *);

public:
    Ui::MainWindow *ui;

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
        AuthErrorCodeReserved5 = 0x1c,
    };

    int m_iConnectMode;
    unsigned int m_uiOnlineTime;
    QString m_szAppDir, m_szLogPath, m_szConfPath;
    std::map<unsigned int, QString> m_mapAuthError;
    QStringList m_nicList;

    CancellableThread *m_thConnectTime, *m_thConnectJob;

    PcapHelper m_pcapHelper;

    QString m_szStoredNIC, m_szStoredUserName, m_szStoredPassWord, m_szStoredIP, m_szStoredMAC;

    bool m_bKeepAliveFail;
    QWaitCondition m_condKeepAliveFirstTry, m_condKeepAlive;
    QMutex m_mutexKeepAliveFirstTry, m_mutexKeepAlive;

    //CURL *m_CURL;

// functions
    void ResetOnlineTime();
    void GatewayNotificate(QString string, QString type);
    //bool CheckIsOnline();
};

#endif // MAINWINDOW_H
