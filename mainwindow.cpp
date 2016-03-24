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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QUrl>
#include <QTimer>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>

#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <functional>
#include <ctime>
#include <fstream>
#include <cctype>
#include <stdint.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/ethernet.h>

#define EASYDRCOM_DEBUG

#include "EasyDrcomCore/easy_drcom_exception.hpp"
#include "EasyDrcomCore/log.hpp"
#include "EasyDrcomCore/utils.h"
#include "EasyDrcomCore/drcom_dealer.hpp"
#include "EasyDrcomCore/eap_dealer.hpp"

std::ofstream log_stream;

/*size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t nSegSize = size * nmemb;
    char *szTemp = new char[nSegSize + 1];
    memcpy(szTemp, buffer, nSegSize);
    *((std::string*)userp) += szTemp;

    delete[] szTemp;
    return nSegSize;
}*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // open log stream
    m_szAppDir = QApplication::applicationDirPath();
    m_szLogPath = "/var/log/EasyDrcom.log";
    m_szConfPath = "/etc/EasyDrcom.conf";
    log_stream.open(m_szLogPath.toStdString());

    // some var init works
    m_uiOnlineTime = 0;
    //m_CURL = nullptr;
    m_thConnectJob = nullptr;
    m_thConnectTime = nullptr;
    m_bKeepAliveFail = false;

    // set version
    SYS_LOG_INFO("EasyDrcomGUI For Linux, v" CURRENT_VERSION << std::endl);
    SYS_LOG_INFO("Code by Shindo, build on " __DATE__ " " __TIME__ "." << std::endl);
    ui->lblBuild->setText(QString("Code by Shindo, build on " __DATE__ " " __TIME__ "."));

    // fetch NIC list
    SYS_LOG_INFO("Attempt to load NIC list...");
    m_nicList = m_pcapHelper.GetNICList();
    if (m_nicList.empty())
    {
        LOG_APPEND("Failed!" << std::endl);
        if (!m_pcapHelper.m_szLastError.isEmpty())
        {
            SYS_LOG_DBG("lastError = " << m_pcapHelper.m_szLastError.toStdString() << std::endl);
            QMessageBox::critical(NULL, QString("获取网卡信息失败！"), m_pcapHelper.m_szLastError);
        }
        else
        {
            QMessageBox::critical(NULL, QString("获取网卡信息失败！"), QString("EasyDrcomGUI未能在您的计算机上找到任何网卡。\n请检查网卡是否被禁用！"));
            SYS_LOG_DBG("EasyDrcomGUI未能在您的计算机上找到任何网卡。请检查网卡是否被禁用！" << std::endl);
        }

        QTimer::singleShot(0, this, SLOT(close()));
        return;
    }
    LOG_APPEND("OK." << std::endl);
    ui->cbNIC->addItems(m_nicList);

    // load settings
    QSettings settings(m_szConfPath, QSettings::NativeFormat);
    m_iConnectMode = settings.value("ConnectMode", ConnectModeStudentDistrict).toInt();

    // select connect mode
    switch (m_iConnectMode)
    {
    case ConnectModeStudentDistrict:
        ui->rbStudent->setChecked(true);
        break;

    case ConnectModeWorkDistrict:
        ui->rbWorkplace->setChecked(true);
        break;

    default:
        m_iConnectMode = ConnectModeStudentDistrict;
        ui->rbStudent->setChecked(true);
        break;
    }

    if (m_iConnectMode == ConnectModeStudentDistrict)
    {
        m_szStoredNIC = settings.value("StuDist.NIC", "").toString();
        m_szStoredUserName = settings.value("StuDist.UserName", "").toString();
        m_szStoredPassWord = settings.value("StuDist.PassWord", "").toString();
    }
    else
    {
        m_szStoredNIC = settings.value("WorkDist.NIC", "").toString();
        m_szStoredUserName = settings.value("WorkDist.UserName", "").toString();
        m_szStoredPassWord = settings.value("WorkDist.PassWord", "").toString();
    }

    // print info, we'll print it later
    /*SYS_LOG_DBG("config.General.ConnectMode = " << m_iConnectMode << ", config.General.NIC = " << m_szStoredNIC.toStdString() << std::endl);
    SYS_LOG_DBG("config.General.UserName = " << m_szStoredUserName.toStdString() << ", config.General.PassWord = " << m_szStoredPassWord.toStdString() << std::endl);
    */

    // dispatch config to ui
    if (!m_szStoredNIC.isEmpty())
    {
        if (ui->cbNIC->findText(m_szStoredNIC) != -1)
            ui->cbNIC->setCurrentText(m_szStoredNIC);
    }
    if (!m_szStoredUserName.isEmpty()) ui->leUserName->setText(m_szStoredUserName);
    if (!m_szStoredPassWord.isEmpty()) ui->lePassWord->setText(m_szStoredPassWord);

    // fix for ubuntu's focus mask
    ui->pbConnect->setFocus();

    // init error dict
    m_mapAuthError.insert({
            { AuthErrorCodeCheckMAC, QString("账户正在被使用！") },
            { AuthErrorCodeServerBusy, QString("服务器繁忙，请稍后尝试！") },
            { AuthErrorCodeWrongPass, QString("账户或密码错误！") },
            { AuthErrorCodeNotEnough, QString("本账户的累计时间或流量已超出限制！") },
            { AuthErrorCodeFreezeUp, QString("本账户暂停使用！") },
            { AuthErrorCodeNotOnThisIP, QString("IP地址不匹配，本账户只能在指定的IP地址上使用！") },
            { AuthErrorCodeNotOnThisMac, QString("MAC地址不匹配，本账户只能在指定的IP和MAC地址上使用！") },
            { AuthErrorCodeTooMuchIP, QString("本账户登录的IP地址太多！") },
            { AuthErrorCodeUpdateClient, QString("客户端版本号不正确！") },
            { AuthErrorCodeNotOnThisIPMAC, QString("本账户只能在指定的MAC地址和IP地址上使用！") },
            { AuthErrorCodeMustUseDHCP, QString("你的PC设置了静态IP，请改为动态获取方式(DHCP)，然后重新登录！") },
            { AuthErrorCodeReserved1, QString("AuthErrorCode24") },
            { AuthErrorCodeReserved2, QString("AuthErrorCode25") },
            { AuthErrorCodeReserved3, QString("AuthErrorCode26") },
            { AuthErrorCodeReserved4, QString("AuthErrorCode27") },
            { AuthErrorCodeReserved5, QString("AuthErrorCode28") }
        });

    // we've dropped the online check
    /*m_CURL = curl_easy_init();
    curl_easy_setopt(m_CURL, CURLOPT_URL, "http://172.25.8.4");
    curl_easy_setopt(m_CURL, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(m_CURL, CURLOPT_TIMEOUT, 1L); // 1s for every action*/

    SYS_LOG_INFO("EasyDrcomGUI is ready." << std::endl);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    auto ret = QMessageBox::question(this, QString("确定要退出吗？"), QString("一旦退出EasyDrcomGUI，您可能会失去网络连接。"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
        SYS_LOG_INFO("EasyDrcomGUI quit." << std::endl);
        //curl_easy_cleanup(m_CURL);

        event->accept();
    }
    else
        event->ignore();
}

void MainWindow::on_pbSupport_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/coverxit/EasyDrcom"));
}

void MainWindow::on_pbViewLog_clicked()
{
    log_stream.flush();
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_szLogPath));
}

void MainWindow::on_rbStudent_clicked()
{
    if (ui->rbStudent->isChecked())
    {
        m_iConnectMode = ConnectModeStudentDistrict;

        QSettings settings(m_szConfPath, QSettings::NativeFormat);
        m_szStoredNIC = settings.value("StuDist.NIC", "").toString();
        m_szStoredUserName = settings.value("StuDist.UserName", "").toString();
        m_szStoredPassWord = settings.value("StuDist.PassWord", "").toString();

        // dispatch config to ui
        if (!m_szStoredNIC.isEmpty())
        {
            if (ui->cbNIC->findText(m_szStoredNIC) != -1)
                ui->cbNIC->setCurrentText(m_szStoredNIC);
        }
        if (!m_szStoredUserName.isEmpty()) ui->leUserName->setText(m_szStoredUserName);
        if (!m_szStoredPassWord.isEmpty()) ui->lePassWord->setText(m_szStoredPassWord);
    }
}

void MainWindow::on_rbWorkplace_clicked()
{
    if (ui->rbWorkplace->isChecked())
    {
        m_iConnectMode = ConnectModeWorkDistrict;

        QSettings settings(m_szConfPath, QSettings::NativeFormat);
        m_szStoredNIC = settings.value("WorkDist.NIC", "").toString();
        m_szStoredUserName = settings.value("WorkDist.UserName", "").toString();
        m_szStoredPassWord = settings.value("WorkDist.PassWord", "").toString();

        // dispatch config to ui
        if (!m_szStoredNIC.isEmpty())
        {
            if (ui->cbNIC->findText(m_szStoredNIC) != -1)
                ui->cbNIC->setCurrentText(m_szStoredNIC);
        }
        if (!m_szStoredUserName.isEmpty()) ui->leUserName->setText(m_szStoredUserName);
        if (!m_szStoredPassWord.isEmpty()) ui->lePassWord->setText(m_szStoredPassWord);
    }
}

void MainWindow::on_pbAboutQt_clicked()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_pbConnect_clicked()
{
    QString btnValue = ui->pbConnect->text();
    if (btnValue.compare("连接") == 0)
    {
        m_szStoredNIC = ui->cbNIC->currentText();
        m_szStoredUserName = ui->leUserName->text();
        m_szStoredPassWord = ui->lePassWord->text();

        if (m_szStoredUserName.isEmpty() || m_szStoredPassWord.isEmpty())
        {
            QMessageBox::critical(NULL, "EasyDrcomGUI", "账户或密码不能为空！");
            return;
        }

        SYS_LOG_INFO("Prepare to authenticate..." << std::endl);

        // save settings
        QSettings settings(m_szConfPath, QSettings::NativeFormat);
        settings.setValue("ConnectMode", m_iConnectMode);

        if (m_iConnectMode == ConnectModeStudentDistrict)
        {
            settings.setValue("StuDist.NIC", m_szStoredNIC);
            settings.setValue("StuDist.UserName", m_szStoredUserName);
            settings.setValue("StuDist.PassWord", m_szStoredPassWord);
        }
        else
        {
            settings.setValue("WorkDist.NIC", m_szStoredNIC);
            settings.setValue("WorkDist.UserName", m_szStoredUserName);
            settings.setValue("WorkDist.PassWord", m_szStoredPassWord);
        }

        SYS_LOG_DBG("ConnectMode = " << m_iConnectMode << ", NIC = " << m_szStoredNIC.toStdString() << std::endl);
        SYS_LOG_DBG("UserName = " << m_szStoredUserName.toStdString() << ", PassWord = " << m_szStoredPassWord.toStdString() << std::endl);

        // get ip & mac
        SYS_LOG_INFO("Attempt to fetch IP & MAC...");
        m_szStoredIP = m_pcapHelper.GetIPAddressByNIC(m_szStoredNIC);
        m_szStoredMAC = m_pcapHelper.GetMACAddressByNIC(m_szStoredNIC);
        if (m_szStoredIP.isEmpty() || m_szStoredMAC.isEmpty())
        {
            LOG_APPEND("Failed!" << std::endl);
            SYS_LOG_ERR("lastError = " << m_pcapHelper.m_szLastError.toStdString() << std::endl);
            QMessageBox::critical(NULL, "获取网卡信息失败！", m_pcapHelper.m_szLastError);
            return;
        }
        LOG_APPEND("OK." << std::endl);

        ui->lblIP->setText(m_szStoredIP);
        ui->lblMAC->setText(m_szStoredMAC);
        SYS_LOG_DBG("IP = " << m_szStoredIP.toStdString() << ", MAC = " << m_szStoredMAC.toStdString() << std::endl);
        SYS_LOG_INFO("Preparation done." << std::endl);

        switch (m_iConnectMode)
        {
        case ConnectModeStudentDistrict:
            m_thConnectJob = new ThreadStuDistConnect(this, nullptr);
            break;

        case ConnectModeWorkDistrict:
            m_thConnectJob = new ThreadWorkDistConnect(this, nullptr);
            break;

        default:
            m_thConnectJob = new ThreadStuDistConnect(this, nullptr);
            break;
        }

        ui->lblStatus->setText("准备连接中……");
        ui->lblOnlineTime->setText("00:00:00");

        ui->rbStudent->setEnabled(false);
        ui->rbWorkplace->setEnabled(false);
        ui->cbNIC->setEnabled(false);
        ui->leUserName->setEnabled(false);
        ui->lePassWord->setEnabled(false);
        ui->pbConnect->setEnabled(false);

        m_thConnectJob->setAutoDelete();
        m_thConnectJob->start();
    }
    else if (btnValue.compare("断开") == 0)
    {
        ui->pbConnect->setEnabled(false);
        m_thConnectJob->cancel();

        m_mutexKeepAlive.lock();
        m_condKeepAlive.wakeAll();
        m_mutexKeepAlive.unlock();
    }
}

void MainWindow::ResetOnlineTime()
{
    if (m_thConnectTime)
    {
        m_thConnectTime->cancel();
        m_thConnectTime = nullptr;
    }

    m_uiOnlineTime = 0;
    ui->lblOnlineTime->setText("00:00:00");
}

template<>
void MainWindow::ThreadConnectTime::run()
{
    while (!isCancelled())
    {
        int connectTime = dlg->m_uiOnlineTime++;
        int seconds = connectTime % 60;
        int minutes = (connectTime / 60) % 60;
        int hours = connectTime / 3600;

        QString stringTime;
        stringTime.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
        dlg->ui->lblOnlineTime->setText(stringTime);

        sleep(1); //
    }
}

void MainWindow::GatewayNotificate(QString string, QString type)
{
    SYS_LOG_INFO("Gateway notificate - " << type.toStdString() << ": " << string.toStdString() << std::endl);

    QMessageBox::information(this, QString("网关通知 - %1").arg(type), string);
}

template<>
void MainWindow::ThreadStuDistKeepAlive::run()
{
    bool firstTry = true;
    drcom_dealer_u62 *dealer = reinterpret_cast<drcom_dealer_u62*>(param);

    try
    {
        // first try
        dlg->ui->lblStatus->setText("发送心跳包中……");
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

        dlg->ui->lblStatus->setText("已连接上");
        dlg->m_mutexKeepAliveFirstTry.lock();
        dlg->m_condKeepAliveFirstTry.wakeAll();
        dlg->m_mutexKeepAliveFirstTry.unlock();

        firstTry = false;
        while (!isCancelled())
        {
            sleep(20); // 20s for alive
            if (isCancelled()) break;

            dlg->ui->lblStatus->setText("发送心跳包中……");
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

            dlg->ui->lblStatus->setText("已连接上");
        }
    }
    catch (std::exception)
    {
        goto udp_fail;
    }
    return;

udp_fail:
    if (isCancelled()) return;

    dlg->ui->lblStatus->setText("发送心跳包失败！");
    QMutex& mutex = firstTry ? dlg->m_mutexKeepAliveFirstTry : dlg->m_mutexKeepAlive;
    QWaitCondition& cond = firstTry? dlg->m_condKeepAliveFirstTry : dlg->m_condKeepAlive;

    dlg->m_bKeepAliveFail = true;
    mutex.lock();
    cond.wakeOne();
    mutex.unlock();
}

template<>
void MainWindow::ThreadStuDistConnect::run()
{
    std::vector<uint8_t> broadcast_mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    std::vector<uint8_t> nearest_mac = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x03 };

    CancellableThread *threadKeepAlive = nullptr;
    drcom_dealer_u62* udp = nullptr;
    eap_dealer* eap = nullptr;
    bool firstTry = true;

    while (!isCancelled()) // auto-redial
    {
        dlg->m_bKeepAliveFail = false;

        try
        {
           eap = new eap_dealer(dlg->m_szStoredNIC.toStdString(), str_mac_to_vec(dlg->m_szStoredMAC.toStdString()),
                                dlg->m_szStoredIP.toStdString(), dlg->m_szStoredUserName.toStdString(), dlg->m_szStoredPassWord.toStdString());
        }
        catch (std::exception &ex)
        {
            EAP_LOG_ERR(ex.what() << std::endl);
            goto eap_fail;
        }

        try
        {
            udp = new drcom_dealer_u62(str_mac_to_vec(dlg->m_szStoredMAC.toStdString()), dlg->m_szStoredIP.toStdString(),
                                       dlg->m_szStoredUserName.toStdString(), dlg->m_szStoredPassWord.toStdString(), "172.25.8.4", 61440, "EasyDrcomGUI", "for Linux, Core v0.9");
        }
        catch (std::exception &ex)
        {
            U62_LOG_ERR(ex.what() << std::endl);
            goto eap_fail;
        }

        dlg->ui->lblStatus->setText("802.1X清理中……");
        eap->logoff(nearest_mac);
        eap->logoff(nearest_mac);

        try
        {
            dlg->ui->lblStatus->setText("802.1X认证中……");
            if (eap->start(broadcast_mac)) goto eap_fail;
            if (eap->response_identity(broadcast_mac)) goto eap_fail;

            switch (eap->response_md5_challenge(broadcast_mac))
            {
            // success
            case 0:
                threadKeepAlive = new MainWindow::ThreadStuDistKeepAlive(dlg, (void*) udp);
                threadKeepAlive->setAutoDelete();
                threadKeepAlive->start();
                break;

            // notifications
            case 1:
                dlg->GatewayNotificate(QString::fromStdString(eap->getNotification()), "EAP");
                goto interrupt;

            case 2:
                dlg->GatewayNotificate("账户不存在！", "EAP");
                goto interrupt;

            case 3:
                dlg->GatewayNotificate("账户或密码错误！", "EAP");
                goto interrupt;

            case 4:
                dlg->GatewayNotificate("您账号绑定的网卡MAC地址与现有网卡MAC地址不匹配！\n\n提示：在学生区使用时，帐号不需要加'o'。\n参考代码：Mac,IP,NASip,PORTerr(2)!", "EAP");
                goto interrupt;

            case 5:
                dlg->GatewayNotificate("您账号绑定的网卡MAC地址与现有网卡MAC地址不匹配！\n\n参考代码：Mac,IP,NASip,PORTerr(11)!", "EAP");
                goto interrupt;

            case 6:
                dlg->GatewayNotificate("您的账号已在线！\n\n参考代码：In use !", "EAP");
                goto interrupt;

            case 7:
                dlg->GatewayNotificate("您的账号已暂停使用！\n\n提示：您的账号可能已欠费停机。\n参考代码：Authentication Fail ErrCode=05", "EAP");
                goto interrupt;

            default: // other errors
                goto eap_fail;
            }
        }
        catch (std::exception)
        {
            goto eap_fail;
        }

        dlg->m_mutexKeepAliveFirstTry.lock();
        dlg->m_condKeepAliveFirstTry.wait(&dlg->m_mutexKeepAliveFirstTry);
        dlg->m_mutexKeepAliveFirstTry.unlock();

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
                dlg->m_thConnectTime = new MainWindow::ThreadConnectTime(dlg, nullptr);
                dlg->m_thConnectTime->setAutoDelete();
                dlg->m_thConnectTime->start();

                firstTry = false;
            }

            dlg->ui->lblStatus->setText("已连接上");
            dlg->ui->pbConnect->setText("断开");
            dlg->ui->pbConnect->setEnabled(true);
        }

        dlg->m_mutexKeepAlive.lock();
        while (!dlg->m_bKeepAliveFail && !isCancelled())
            dlg->m_condKeepAlive.wait(&dlg->m_mutexKeepAlive);
        dlg->m_mutexKeepAlive.unlock();

        if (dlg->m_bKeepAliveFail && !isCancelled()) goto eap_fail;
        else continue;

    eap_fail:
        if (firstTry) // first try failed
        {
            dlg->ui->lblStatus->setText("802.1X认证失败！");
            goto firstFail;
        }
        else
        {
            if (isCancelled()) break;

            dlg->ui->lblStatus->setText("连接丢失，5秒后重试。");
            sleep(5);

            if (eap) delete eap;
            if (udp) delete udp;
            eap = nullptr;
            udp = nullptr;
        }
    }

    // cancelled
    if (threadKeepAlive && threadKeepAlive->isRunning())
        threadKeepAlive->cancel();

    dlg->ui->lblStatus->setText("802.1X注销中……");
    if (eap) eap->logoff(nearest_mac);

interrupt:
    dlg->ui->lblStatus->setText("已断开");

firstFail:
    dlg->ResetOnlineTime();
    dlg->ui->rbStudent->setEnabled(true);
    dlg->ui->rbWorkplace->setEnabled(true);
    dlg->ui->cbNIC->setEnabled(true);
    dlg->ui->leUserName->setEnabled(true);
    dlg->ui->lePassWord->setEnabled(true);
    dlg->ui->pbConnect->setEnabled(true);

    dlg->ui->lblIP->setText("-");
    dlg->ui->lblMAC->setText("-");
    dlg->ui->pbConnect->setText("连接");

    dlg->m_thConnectJob = nullptr;

    if (eap) delete eap;
    if (udp) delete udp;
}

template<>
void MainWindow::ThreadWorkDistKeepAlive::run()
{
    bool firstTry = true;
    drcom_dealer_u31 *dealer = reinterpret_cast<drcom_dealer_u31*>(param);

    try
    {
        // first try
        dlg->ui->lblStatus->setText("发送心跳包中……");
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

        dlg->ui->lblStatus->setText("已连接上");
        dlg->m_mutexKeepAliveFirstTry.lock();
        dlg->m_condKeepAliveFirstTry.wakeAll();
        dlg->m_mutexKeepAliveFirstTry.unlock();

        firstTry = false;
        while (!isCancelled())
        {
            sleep(20); // 20s for alive
            if (isCancelled()) break;

            dlg->ui->lblStatus->setText("发送心跳包中……");
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

            dlg->ui->lblStatus->setText("已连接上");
        }
    }
    catch (std::exception)
    {
        goto udp_fail;
    }
    return;

udp_fail:
    if (isCancelled()) return;

    dlg->ui->lblStatus->setText("发送心跳包失败！");
    QMutex& mutex = firstTry ? dlg->m_mutexKeepAliveFirstTry : dlg->m_mutexKeepAlive;
    QWaitCondition& cond = firstTry? dlg->m_condKeepAliveFirstTry : dlg->m_condKeepAlive;

    dlg->m_bKeepAliveFail = true;
    mutex.lock();
    cond.wakeOne();
    mutex.unlock();
}

template<>
void MainWindow::ThreadWorkDistConnect::run()
{
    CancellableThread *threadKeepAlive = nullptr;
    drcom_dealer_u31* udp = nullptr;
    bool firstTry = true;

    while (!isCancelled()) // auto-redial
    {
        dlg->m_bKeepAliveFail = false;

        try
        {
            udp = new drcom_dealer_u31(str_mac_to_vec(dlg->m_szStoredMAC.toStdString()), dlg->m_szStoredIP.toStdString(),
                                       dlg->m_szStoredUserName.toStdString(), dlg->m_szStoredPassWord.toStdString(), "172.25.8.4", 61440, "EasyDrcomGUI", "for Linux, Core v0.9");
        }
        catch (std::exception &ex)
        {
            U31_LOG_ERR(ex.what() << std::endl);
            goto udp_fail;
        }

        try
        {
            dlg->ui->lblStatus->setText("UDP认证中……");
            if (udp->start_request()) goto udp_fail;

            int ret = udp->send_login_auth();
            if (ret == 0) // success
            {
                threadKeepAlive = new MainWindow::ThreadWorkDistKeepAlive(dlg, (void*) udp);
                threadKeepAlive->setAutoDelete();
                threadKeepAlive->start();
            }
            else
            {
                if (ret < 0) // must be error
                    goto udp_fail;

                if (dlg->m_mapAuthError.find(ret) == dlg->m_mapAuthError.end()) // not specific error
                {
                    dlg->GatewayNotificate(QString("Unknown auth error code: %1").arg(ret), "UDP");
                    goto interrupt;
                }
                else
                {
                    dlg->GatewayNotificate(dlg->m_mapAuthError[ret], "UDP");
                    goto interrupt;
                }
            }
        }
        catch (std::exception)
        {
            goto udp_fail;
        }

        dlg->m_mutexKeepAliveFirstTry.lock();
        dlg->m_condKeepAliveFirstTry.wait(&dlg->m_mutexKeepAliveFirstTry);
        dlg->m_mutexKeepAliveFirstTry.unlock();

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
                dlg->m_thConnectTime = new MainWindow::ThreadConnectTime(dlg, nullptr);
                dlg->m_thConnectTime->setAutoDelete();
                dlg->m_thConnectTime->start();

                firstTry = false;
            }

            dlg->ui->lblStatus->setText("已连接上");
            dlg->ui->pbConnect->setText("断开");
            dlg->ui->pbConnect->setEnabled(true);
        }

        dlg->m_mutexKeepAlive.lock();
        while (!dlg->m_bKeepAliveFail && !isCancelled())
            dlg->m_condKeepAlive.wait(&dlg->m_mutexKeepAlive);
        dlg->m_mutexKeepAlive.unlock();

        if (dlg->m_bKeepAliveFail && !isCancelled()) goto udp_fail;
        else continue;

    udp_fail:
        if (firstTry) // first try failed
        {
            dlg->ui->lblStatus->setText("UDP认证失败！");
            goto firstFail;
        }
        else
        {
            if (isCancelled()) break;

            dlg->ui->lblStatus->setText("连接丢失，5秒后重试。");
            sleep(5);

            if (udp) delete udp;
            udp = nullptr;
        }
    }

    // cancelled
    if (threadKeepAlive && threadKeepAlive->isRunning())
        threadKeepAlive->cancel();

    dlg->ui->lblStatus->setText("UDP注销中……");
    try
    {
        if (udp)
        {
            udp->send_alive_request();
            udp->start_request();
            udp->send_logout_auth();
        }
    }
    catch (std::exception&)
    {
        // fuck it, but we don't mangage it :P
    }

interrupt:
    dlg->ui->lblStatus->setText("已断开");

firstFail:
    dlg->ResetOnlineTime();
    dlg->ui->rbStudent->setEnabled(true);
    dlg->ui->rbWorkplace->setEnabled(true);
    dlg->ui->cbNIC->setEnabled(true);
    dlg->ui->leUserName->setEnabled(true);
    dlg->ui->lePassWord->setEnabled(true);
    dlg->ui->pbConnect->setEnabled(true);

    dlg->ui->lblIP->setText("-");
    dlg->ui->lblMAC->setText("-");
    dlg->ui->pbConnect->setText("连接");

    dlg->m_thConnectJob = nullptr;

    if (udp) delete udp;
}

/*bool MainWindow::CheckIsOnline()
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
