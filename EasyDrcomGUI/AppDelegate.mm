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

#define EASYDRCOM_DEBUG

#import "AppDelegate.h"
#import "STPrivilegedTask.h"
#import "NSObject+extend.h"
#import "EasyDrcomCore/log.hpp"

#import <exception>
#import <iostream>
#import <fstream>
#import <sstream>

std::ofstream log_stream;

/*size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t nSegSize = size * nmemb;
    char *szTemp = new char[nSegSize+1];
    memcpy(szTemp, buffer, nSegSize);
    *((std::string*)userp) += szTemp;
    
    delete[] szTemp;
    return nSegSize;
}*/

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    [self.window setDelegate:self];
    [self.btnConnect setBezelStyle:NSRoundedBezelStyle];
    [self.window setDefaultButtonCell:[self.btnConnect cell]];
    
    // open log
    logPath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/EasyDrcomGUI.log"];
    log_stream.open([logPath UTF8String]);
    
    NSString *version = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    [self.window setTitle:[NSString stringWithFormat:@"EasyDrcomGUI For OSX, v%@", version]];
    [self.lblBuild setStringValue: [NSString stringWithFormat:@"Code by Shindo, build on %s %s.", __DATE__, __TIME__]];
    
    SYS_LOG_INFO("EasyDrcomGUI For OSX, v" << [version UTF8String] << std::endl);
    SYS_LOG_INFO("Code by Shindo, build on " __DATE__ " " __TIME__ << std::endl);
    
    // init
    keepAliveCondition = [[NSCondition alloc] init];
    keepAliveFirstTry = [[NSCondition alloc] init];
    connectMode = ConnectionModeStudentDistrict;
    //curl = nullptr;
    
    // load nics
    SYS_LOG_INFO("First attempt to load NIC list...");
    bridge = [[PcapBridge alloc] init];
    NSArray *list = [bridge getNICList];
    if ([list count] == 0)
    {
        // hide window
        [self.window orderOut:self];
        LOG_APPEND("Failed!" << std::endl);
        if ([bridge lastError] != nil)
            SYS_LOG_DBG("lastError = " << [[bridge lastError] UTF8String]);
        
        SYS_LOG_INFO("Try to `chmod 666 /dev/bpf*`...");
        // try to grant access to /dev/bpf*
        NSArray *args = [NSArray arrayWithObjects:@"-c", @"chmod 666 /dev/bpf*", nil];
        STPrivilegedTask *task = [[STPrivilegedTask alloc] initWithLaunchPath:@"/bin/sh" arguments:args];
        // NSLog(@"%@", [task description]);
        
        int result = [task launch];
        if (result != noErr) // error occured
        {
            LOG_APPEND("Failed!" << std::endl);
            SYS_LOG_ERR("Failed to init, quit." << std::endl);
            [self.window makeKeyAndOrderFront:self];
            [NSApp activateIgnoringOtherApps:YES];
            NSError *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:nil];
            NSAlert *alert = [NSAlert alertWithMessageText:@"获取网卡信息失败！" defaultButton:@"好" alternateButton:nil otherButton:nil informativeTextWithFormat:@"请在终端中执行如下命令后再运行EasyDrcomGUI：\n\tsudo chmod 666 /dev/bpf*\n\n错误信息：\n%@", [error localizedFailureReason] ?: [error localizedDescription]];
            [alert beginSheetModalForWindow:self.window modalDelegate:self didEndSelector:@selector(alertFetchNICListFailed:returnCode:contextInfo:) contextInfo:nil];
            return;
        }
        
        // just wait for chmod
        [task waitUntilExit];
        LOG_APPEND("OK." << std::endl);
        SYS_LOG_INFO("Second attempt to load NIC list...");
        list = [bridge getNICList];
        if ([list count] == 0) // still failed
        {
            LOG_APPEND("Failed!" << std::endl);
            if ([bridge lastError] != nil)
                SYS_LOG_DBG("lastError = " << [[bridge lastError] UTF8String]);
            SYS_LOG_ERR("Failed to init, quit." << std::endl);
            [self.window makeKeyAndOrderFront:self];
            [NSApp activateIgnoringOtherApps:YES];
            NSAlert *alert = [NSAlert alertWithMessageText:@"获取网卡信息失败！" defaultButton:@"好" alternateButton:nil otherButton:nil informativeTextWithFormat:@"%@", bridge.lastError ?: @"请在终端中执行如下命令后再运行EasyDrcomGUI：\n\tsudo chmod 666 /dev/bpf*"];
            [alert beginSheetModalForWindow:self.window modalDelegate:self didEndSelector:@selector(alertFetchNICListFailed:returnCode:contextInfo:) contextInfo:nil];
            return;
        }
        LOG_APPEND("OK." << std::endl);
    }
    else
        LOG_APPEND("OK." << std::endl);
    
    [self.window makeKeyAndOrderFront:self];
    [NSApp activateIgnoringOtherApps:YES];
    [self.nicList addItemsWithTitles: list];
    
    // load settings
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    NSInteger storedMode = [[userDefaults objectForKey:@"storedMode"] integerValue];
    
    switch (storedMode)
    {
        case ConnectionModeStudentDistrict:
        case ConnectionModeWorkDistrict:
            connectMode = storedMode;
            break;
            
        default:
            connectMode = ConnectionModeStudentDistrict;
            break;
    }
    [self.selectedMode selectCellWithTag: connectMode];
    
    if (connectMode == ConnectionModeStudentDistrict)
    {
        storedNIC = [userDefaults objectForKey:@"stuDist.storedNIC"];
        storedUserName = [userDefaults objectForKey:@"stuDist.storedUserName"];
        storedPassWord = [userDefaults objectForKey:@"stuDist.storedPassWord"];
    }
    else // work dist
    {
        storedNIC = [userDefaults objectForKey:@"workDist.storedNIC"];
        storedUserName = [userDefaults objectForKey:@"workDist.storedUserName"];
        storedPassWord = [userDefaults objectForKey:@"workDist.storedPassWord"];
    }
        
    // print info, not here, we will print it when connect
    /*SYS_LOG_DBG("config.storedMode = " << storedMode);
    if (storedNIC != nil) { LOG_APPEND(", config.storedNIC = " << [storedNIC UTF8String]); }
    LOG_APPEND(std::endl);
    
    if (storedUserName != nil) { SYS_LOG_DBG("config.storedUserName = " << [storedUserName UTF8String] << std::endl); }
    if (storedPassWord != nil) { SYS_LOG_DBG("config.storedPassWord = " << [storedPassWord UTF8String] << std::endl); }*/
    
    if (storedNIC != nil)
    {
        if ([self.nicList itemWithTitle:storedNIC] != nil)
            [self.nicList selectItemWithTitle:storedNIC];
    }
    if (storedUserName != nil) [self.userName setStringValue:storedUserName];
    if (storedPassWord != nil) [self.passWord setStringValue:storedPassWord];
    
    // init errorDict
    NSMutableDictionary *udpErrors = [[NSMutableDictionary alloc] init];
    [udpErrors setObject:@"账户正在被使用！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeCheckMAC]];
    [udpErrors setObject:@"服务器繁忙，请稍后尝试！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeServerBusy]];
    [udpErrors setObject:@"账户或密码错误！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeWrongPass]];
    [udpErrors setObject:@"本账户的累计时间或流量已超出限制！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeNotEnough]];
    [udpErrors setObject:@"本账户暂停使用！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeFreezeUp]];
    [udpErrors setObject:@"IP地址不匹配，本账户只能在指定的IP地址上使用！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeNotOnThisIP]];
    [udpErrors setObject:@"MAC地址不匹配，本账户只能在指定的IP和MAC地址上使用！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeNotOnThisMac]];
    [udpErrors setObject:@"本账户登录的IP地址太多！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeTooMuchIP]];
    [udpErrors setObject:@"客户端版本号不正确！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeUpdateClient]];
    [udpErrors setObject:@"本账户只能在指定的MAC地址和IP地址上使用！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeNotOnThisIPMAC]];
    [udpErrors setObject:@"你的PC设置了静态IP，请改为动态获取方式(DHCP)，然后重新登录！" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeMustUseDHCP]];
    [udpErrors setObject:@"AuthErrorCode24" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeReserved1]];
    [udpErrors setObject:@"AuthErrorCode25" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeReserved2]];
    [udpErrors setObject:@"AuthErrorCode26" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeReserved3]];
    [udpErrors setObject:@"AuthErrorCode27" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeReserved4]];
    [udpErrors setObject:@"AuthErrorCode28" forKey:[NSNumber numberWithUnsignedInteger:AuthErrorCodeReserved5]];
    
    udpErrorDict = [NSDictionary dictionaryWithDictionary:udpErrors];
    
    // we've dropped the online check
    /*curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://172.25.8.4");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L); // 1s for every action*/
    
    SYS_LOG_INFO("EasyDrcomGUI is ready." << std::endl);
}

- (void) alertFetchNICListFailed:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    [[NSApplication sharedApplication] terminate:self];
}

- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication *)sender
{
    if (connectJob)
    {
        NSAlert *alert = [NSAlert alertWithMessageText:@"确定要退出吗？" defaultButton:@"否" alternateButton:@"是" otherButton:nil informativeTextWithFormat:@"一旦退出EasyDrcomGUI，您将会在稍后失去网络连接。"];
        [alert beginSheetModalForWindow:self.window modalDelegate:self didEndSelector:@selector(alertQuitApplication: returnCode:contextInfo:) contextInfo:nil];
        return NSTerminateLater;
    }
    else
    {
        SYS_LOG_INFO("EasyDrcomGUI quit." << std::endl);
        
        /*if (curl != nullptr)
            curl_easy_cleanup(curl);*/
        return NSTerminateNow;
    }
}

- (void) alertQuitApplication:(NSAlert*)alert returnCode:(NSInteger)returnCode contextInfo:(void*)contextInfo
{
    if (returnCode != NSAlertDefaultReturn)
        [[NSApplication sharedApplication] replyToApplicationShouldTerminate:YES];
    else
        [[NSApplication sharedApplication] replyToApplicationShouldTerminate:NO];
}

- (BOOL) windowShouldClose:(id)sender
{
    [[NSRunningApplication currentApplication] hide];
    return NO;
}

- (IBAction) connectClicked:(id)sender
{
    NSString *btnValue = [self.btnConnect title];
    if ([btnValue isEqualToString:@"连接"])
    {
        storedNIC = [[self.nicList selectedItem] title];
        storedUserName = [self.userName stringValue];
        storedPassWord = [self.passWord stringValue];
      
        if (storedUserName.length == 0 || storedPassWord.length == 0)
        {
            NSAlert *alert = [NSAlert alertWithMessageText:@"错误" defaultButton:@"好" alternateButton:nil otherButton:nil informativeTextWithFormat:@"账户或密码不能为空！"];
            [alert beginSheetModalForWindow:self.window modalDelegate:nil didEndSelector:nil contextInfo:nil];
            return;
        }
        
        SYS_LOG_INFO("Prepare to authenticate..." << std::endl);
        
        // save settings
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        [userDefaults setObject:[NSNumber numberWithInteger:connectMode] forKey:@"storedMode"];
        
        if (connectMode == ConnectionModeStudentDistrict)
        {
            [userDefaults setObject:[[self.nicList selectedItem] title] forKey:@"stuDist.storedNIC"];
            [userDefaults setObject:[self.userName stringValue] forKey:@"stuDist.storedUserName"];
            [userDefaults setObject:[self.passWord stringValue] forKey:@"stuDist.storedPassWord"];
        }
        else // work dist
        {
            [userDefaults setObject:[[self.nicList selectedItem] title] forKey:@"workDist.storedNIC"];
            [userDefaults setObject:[self.userName stringValue] forKey:@"workDist.storedUserName"];
            [userDefaults setObject:[self.passWord stringValue] forKey:@"workDist.storedPassWord"];
        }
        
        SYS_LOG_DBG("ConnectMode = " << connectMode << ", NIC = " << [storedNIC UTF8String] << std::endl);
        SYS_LOG_DBG("UserName = " << [storedUserName UTF8String] << ", PassWord = " << [storedPassWord UTF8String] << std::endl);
        
        // get ip & mac
        SYS_LOG_INFO("Attempt to fetch IP & MAC...");
        storedIP = [bridge getIPAddressWithNICName:storedNIC];
        storedMAC = [bridge getMACAddressWithNICName:storedNIC];
        if (storedIP == nil || storedMAC == nil)
        {
            LOG_APPEND("Failed!" << std::endl);
            if ([bridge lastError] != nil)
            {
                SYS_LOG_ERR("lastError = " << [[bridge lastError] UTF8String] << std::endl);
            }
            
            NSAlert *alert = [NSAlert alertWithMessageText:@"获取网卡信息失败！" defaultButton:@"好" alternateButton:nil otherButton:nil informativeTextWithFormat:@"%@", [bridge lastError] ?: @"Unknown error occured."];
            [alert beginSheetModalForWindow:self.window modalDelegate:nil didEndSelector:nil contextInfo:nil];
            return;
        }
        LOG_APPEND("OK." << std::endl);
        
        [self.lblIPAddr setStringValue:storedIP];
        [self.lblMacAddr setStringValue:storedMAC];
        SYS_LOG_DBG("IP = " << [storedIP UTF8String] << ", MAC = " << [storedMAC UTF8String] << std::endl);
        SYS_LOG_INFO("Preparation done." << std::endl);
        
        switch (connectMode)
        {
            case ConnectionModeStudentDistrict: // 学生区
            {
                connectJob = [[NSThread alloc] initWithTarget:self selector:@selector(connectionJobForStudentDistrict) object:nil];
                break;
            }
                
            case ConnectionModeWorkDistrict:
            {
                connectJob = [[NSThread alloc] initWithTarget:self selector:@selector(connectionJobForWorkDistrict) object:nil];
                break;
            }
                
            default:
            {
                connectJob = [[NSThread alloc] initWithTarget:self selector:@selector(connectionJobForStudentDistrict) object:nil];
                break;
            }
        }
        
        //[self.connectIndicator setHidden:NO];
        //[self.connectIndicator startAnimation:sender];

        [self.lblStatus setStringValue: @"准备连接中……"];
        [self.lblOnlineTime setStringValue: @"00:00:00"];
        
        [self.selectedMode setEnabled:NO];
        [self.nicList setEnabled:NO];
        [self.userName setEnabled:NO];
        [self.passWord setEnabled:NO];
        [self.btnConnect setEnabled:NO];
        
        [connectJob start];
    }
    else if ([btnValue isEqualToString:@"断开"])
    {
        [self.btnConnect setEnabled:NO];
        //[self.connectIndicator setHidden:NO];
        //[self.connectIndicator startAnimation:sender];
        
        [connectJob cancel];
        [keepAliveCondition lock];
        [keepAliveCondition signal];
        [keepAliveCondition unlock];
    }
}

- (void) resetOnlineTime
{
    if (connectTimer != nil)
    {
        [connectTimer cancel];
        connectTimer = nil;
    }
        
    connectTime = 0;
    [self.lblOnlineTime setStringValue:@"00:00:00"];
}

- (IBAction) modeChanged:(id)sender
{
    NSButtonCell *selCell = [sender selectedCell];
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    
    // 0: 学生区，1: 办公区
    switch ([selCell tag])
    {
        case 0:
            connectMode = ConnectionModeStudentDistrict;
            
            storedNIC = [userDefaults objectForKey:@"stuDist.storedNIC"];
            storedUserName = [userDefaults objectForKey:@"stuDist.storedUserName"];
            storedPassWord = [userDefaults objectForKey:@"stuDist.storedPassWord"];
            break;
            
        case 1:
            connectMode = ConnectionModeWorkDistrict;
            
            storedNIC = [userDefaults objectForKey:@"workDist.storedNIC"];
            storedUserName = [userDefaults objectForKey:@"workDist.storedUserName"];
            storedPassWord = [userDefaults objectForKey:@"workDist.storedPassWord"];
            break;
            
        default:
            NSLog(@"Unexpected selected cell = %ld in `modeChanged`!", [selCell tag]);
            break;
    }
    
    if (storedNIC != nil)
    {
        if ([self.nicList itemWithTitle:storedNIC] != nil)
            [self.nicList selectItemWithTitle:storedNIC];
    }
    if (storedUserName != nil) [self.userName setStringValue:storedUserName];
    if (storedPassWord != nil) [self.passWord setStringValue:storedPassWord];
}

- (void) updateOnlineTime
{
    while ([[NSThread currentThread] isCancelled] == NO)
    {
        ++connectTime;
        
        NSInteger seconds = connectTime % 60;
        NSInteger minutes = (connectTime / 60) % 60;
        NSInteger hours = connectTime / 3600;
        
        [self.lblOnlineTime setStringValue: [NSString stringWithFormat:@"%02ld:%02ld:%02ld", hours, minutes, (long)seconds]];
        
        [NSThread sleepForTimeInterval:1.0f];
    }
}

- (void) notificateWithString:(NSString *)string notificateType:(NSString *)type
{
    SYS_LOG_INFO("Gateway notificate - " << [type UTF8String] << ": " << [string UTF8String] << std::endl);
    NSString *title = [NSString stringWithFormat:@"网关通知 - %@", type];
    NSAlert *alert = [NSAlert alertWithMessageText:title defaultButton:@"好" alternateButton:nil otherButton:nil informativeTextWithFormat:@"%@", string];
    [alert beginSheetModalForWindow:self.window modalDelegate:nil didEndSelector:nil contextInfo:nil];
}

- (void) keepAliveForStudentDistrictWithDealer: (DrcomDealerU62*) dealer
{
    BOOL firstTry = YES;
    try {
        // first try
        [self.lblStatus setStringValue:@"发送心跳包中……"];
        if ([dealer sendAlivePacket1]) goto udp_fail;
        if ([dealer sendAlivePacket2]) goto udp_fail;
        
        /*if ([self checkIsOnline] == YES)
        {
            U62_LOG_INFO("checkIsOnline succeeded" << std::endl);
        }
        else // WTF!!
        {
            U62_LOG_ERR("checkIsOnline failed." << std::endl);
            goto udp_fail;
        }*/
        
        [self.lblStatus setStringValue:@"已连接上"];
        [keepAliveFirstTry lock];
        [keepAliveFirstTry signal];
        [keepAliveFirstTry unlock];
        
        firstTry = NO;
        while (![[NSThread currentThread] isCancelled])
        {
            [NSThread sleepForTimeInterval:20.0f]; // 20s for alive
            if ([[NSThread currentThread] isCancelled]) break;
            
            [self.lblStatus setStringValue:@"发送心跳包中……"];
            if ([dealer sendAlivePacket1]) goto udp_fail;
            if ([dealer sendAlivePacket2]) goto udp_fail;
            
            /*if ([self checkIsOnline] == YES)
            {
                U62_LOG_INFO("checkIsOnline succeeded" << std::endl);
            }
            else // WTF!!
            {
                U62_LOG_ERR("checkIsOnline failed." << std::endl);
                goto udp_fail;
            }*/
            
            [self.lblStatus setStringValue:@"已连接上"];
        }
    }
    catch (std::exception&) {
        goto udp_fail;
    }
    return;

udp_fail:
    if ([[NSThread currentThread] isCancelled]) return;
    
    [self.lblStatus setStringValue:@"发送心跳包失败！"];
    NSCondition *cond = firstTry ? keepAliveFirstTry : keepAliveCondition;
    
    [cond lock];
    keepAliveFail = YES;
    [cond signal];
    [cond unlock];
}

- (void) connectionJobForStudentDistrict
{
    NSThread *threadKeepAlive = nil;
    DrcomDealerU62 *dealer = nil;
    BOOL firstTry = YES;
    
    while (NO == [[NSThread currentThread] isCancelled]) // auto-redial
    {
        keepAliveFail = NO;
        try {
            dealer = [[DrcomDealerU62 alloc] initWithNICName:storedNIC userName:storedUserName passWord:storedPassWord IPAddress:storedIP MacAddress:storedMAC];
        } catch (std::exception&) {
            goto eap_fail;
        }
        
        [self.lblStatus setStringValue:@"802.1X清理中……"];
        [dealer logOff];
        [dealer logOff];
        
        try {
            [self.lblStatus setStringValue:@"802.1X认证中……"];
            if ([dealer start]) goto eap_fail;
            if ([dealer responseIdentity]) goto eap_fail;
            
            switch([dealer responseMD5Challenge])
            {
                // success
                case 0:
                    threadKeepAlive = [[NSThread alloc] initWithTarget:self selector:@selector(keepAliveForStudentDistrictWithDealer:) object:dealer];
                    [threadKeepAlive start];
                    break;
                    
                // notifications
                case 1:
                    [self performSelectorOnMainThread:@selector(notificateWithString: notificateType:) withObject:[dealer getNotification] withObject:@"EAP" waitUntilDone:NO];
                    goto interrupt;
                    
                case 2:
                    [self performSelectorOnMainThread:@selector(notificateWithString: notificateType:) withObject:@"账户不存在！" withObject:@"EAP" waitUntilDone:NO];
                    goto interrupt;
                
                case 3:
                    [self performSelectorOnMainThread:@selector(notificateWithString: notificateType:) withObject:@"账户或密码错误！" withObject:@"EAP" waitUntilDone:NO];
                    goto interrupt;
                    
                case 4:
                    [self performSelectorOnMainThread:@selector(notificateWithString: notificateType:) withObject:@"您账号绑定的网卡MAC地址与现有网卡MAC地址不匹配！\n\n提示：在学生区使用时，帐号不需要加'o'。\n参考代码：Mac,IP,NASip,PORTerr(2)!" withObject:@"EAP" waitUntilDone:NO];
                    goto interrupt;
                    
                case 5:
                    [self performSelectorOnMainThread:@selector(notificateWithString: notificateType:) withObject:@"您账号绑定的网卡MAC地址与现有网卡MAC地址不匹配！\n\n参考代码：Mac,IP,NASip,PORTerr(11)!" withObject:@"EAP" waitUntilDone:NO];
                    goto interrupt;
                    
                case 6:
                    [self performSelectorOnMainThread:@selector(notificateWithString: notificateType:) withObject:@"您的账号已在线！\n\n参考代码：In use !" withObject:@"EAP" waitUntilDone:NO];
                    goto interrupt;
                    
                case 7:
                    [self performSelectorOnMainThread:@selector(notificateWithString: notificateType:) withObject:@"您的账号已暂停使用！\n\n提示：您的账号可能已欠费停机。\n参考代码：Authentication Fail ErrCode=05" withObject:@"EAP" waitUntilDone:NO];
                    goto interrupt;
        
                default: // other errors
                    goto eap_fail;
            }
        } catch (std::exception&) {
            goto eap_fail;
        }
        
        [keepAliveFirstTry lock];
        [keepAliveFirstTry wait];
        [keepAliveFirstTry unlock];
        if (keepAliveFail)
        {
            if (firstTry) goto firstFail; // first try failed
            else goto eap_fail;
        }
        else
        {
            if (firstTry)
            {
                [self resetOnlineTime];
                connectTimer = [[NSThread alloc] initWithTarget:self selector:@selector(updateOnlineTime) object:nil];
                [connectTimer start];
                
                firstTry = NO;
            }
            
            // prevent app nap
            if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)])
                self.activity = [[NSProcessInfo processInfo] beginActivityWithOptions:0x00FFFFFF reason:@"receiving gateway keep-alive packets"];
            
            [self.lblStatus setStringValue:@"已连接上"];
            [self.btnConnect setEnabled:YES];
            [self.btnConnect setTitle:@"断开"];
            //[self.connectIndicator setHidden:YES];
        }
        
        [keepAliveCondition lock];
        while (!keepAliveFail && ![connectJob isCancelled])
            [keepAliveCondition wait];
        [keepAliveCondition unlock];
        
        if (keepAliveFail && ![connectJob isCancelled]) goto eap_fail;
        else continue;
        
    eap_fail:
        if (firstTry) // first try failed
        {
            [self.lblStatus setStringValue:@"802.1X认证失败！"];
            goto firstFail;
        }
        else
        {
            if ([[NSThread currentThread] isCancelled]) break;
            
            [self.lblStatus setStringValue:@"连接丢失，5秒后重试。"];
            [NSThread sleepForTimeInterval:5.0f];
            dealer = nil;
        }
    }
    
    // canceled
    if (threadKeepAlive != nil && [threadKeepAlive isExecuting])
        [threadKeepAlive cancel];
    
    if (self.activity)
        [[NSProcessInfo processInfo] endActivity:self.activity];
    self.activity = nil;
    
    [self.lblStatus setStringValue:@"802.1X注销中……"];
    if ([dealer isEAPObjectInit]) [dealer logOff];

interrupt:
    [self.lblStatus setStringValue:@"已断开"];

firstFail:
    [self resetOnlineTime];
    [self.selectedMode setEnabled:YES];
    [self.nicList setEnabled:YES];
    [self.userName setEnabled:YES];
    [self.passWord setEnabled:YES];
    [self.btnConnect setEnabled:YES];
    
    [self.lblIPAddr setStringValue:@"-"];
    [self.lblMacAddr setStringValue:@"-"];
    [self.btnConnect setTitle:@"连接"];
    //[self.connectIndicator setHidden:YES];
    
    connectJob = nil;
}

- (IBAction) viewLogClicked:(id)sender {
    log_stream.flush();
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    [workspace openFile:logPath];
}

- (IBAction) supportClicked:(id)sender {
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    [workspace openURL:[NSURL URLWithString:@"https://github.com/coverxit/EasyDrcom"]];
}

- (void) keepAliveForWorkDistrictWithDealer: (DrcomDealerU31*)dealer
{
    BOOL firstTry = YES;
    try {
        // first try
        [self.lblStatus setStringValue:@"发送心跳包中……"];
        if ([dealer sendAliveRequest]) goto udp_fail;
        if ([dealer sendAlivePacket1]) goto udp_fail;
        if ([dealer sendAlivePacket2]) goto udp_fail;
        
        /*if ([self checkIsOnline] == YES)
        {
            U31_LOG_INFO("checkIsOnline succeeded." << std::endl);
        }
        else // WHAT THE FUCK!!
        {
            U31_LOG_ERR("checkIsOnline failed." << std::endl);
            goto udp_fail;
        }*/
        
        [self.lblStatus setStringValue:@"已连接上"];
        [keepAliveFirstTry lock];
        [keepAliveFirstTry signal];
        [keepAliveFirstTry unlock];
        
        firstTry = NO;
        while (![[NSThread currentThread] isCancelled])
        {
            [NSThread sleepForTimeInterval:20.0f]; // 20s for alive
            if ([[NSThread currentThread] isCancelled]) break;
            
            [self.lblStatus setStringValue:@"发送心跳包中……"];
            if ([dealer sendAliveRequest]) goto udp_fail;
            if ([dealer sendAlivePacket1]) goto udp_fail;
            if ([dealer sendAlivePacket2]) goto udp_fail;
            
            /*if ([self checkIsOnline] == YES)
            {
                U31_LOG_INFO("checkIsOnline succeeded." << std::endl);
            }
            else // WHAT THE FUCK!!
            {
                U31_LOG_ERR("checkIsOnline failed." << std::endl);
                goto udp_fail;
            }*/
            
            [self.lblStatus setStringValue:@"已连接上"];
        }
    }
    catch (std::exception&) {
        goto udp_fail;
    }
    return;
    
udp_fail:
    if ([[NSThread currentThread] isCancelled]) return;
    
    [self.lblStatus setStringValue:@"发送心跳包失败！"];
    NSCondition *cond = firstTry ? keepAliveFirstTry : keepAliveCondition;
    
    [cond lock];
    keepAliveFail = YES;
    [cond signal];
    [cond unlock];
}

- (void) connectionJobForWorkDistrict
{
    NSThread *threadKeepAlive = nil;
    DrcomDealerU31 *dealer = nil;
    BOOL firstTry = YES;
    
    while (NO == [[NSThread currentThread] isCancelled]) // auto-redial
    {
        keepAliveFail = NO;
        try {
            dealer = [[DrcomDealerU31 alloc] initWithNICName:storedNIC userName:storedUserName passWord:storedPassWord IPAddress:storedIP MacAddress:storedMAC];
        } catch (std::exception&) {
            goto udp_fail;
        }
     
        try {
            [self.lblStatus setStringValue:@"UDP认证中……"];
            if ([dealer startRequest]) goto udp_fail;
            
            NSInteger ret = [dealer sendLoginAuth];
            if (ret == 0) // success
            {
                threadKeepAlive = [[NSThread alloc] initWithTarget:self selector:@selector(keepAliveForWorkDistrictWithDealer:) object:dealer];
                [threadKeepAlive start];
            }
            else
            {
                if (ret < 0) // must be error
                    goto udp_fail;
                
                NSString *error = [udpErrorDict objectForKey:[NSNumber numberWithInteger:ret]];
                if (error == nil) // not specific error
                    error = [NSString stringWithFormat:@"Unknown auth error code: %ld", ret];
                
                [self performSelectorOnMainThread:@selector(notificateWithString: notificateType:) withObject:error withObject:@"UDP" waitUntilDone:NO];
                goto interrupt;
            }
        } catch (std::exception&) {
            goto udp_fail;
        }
        
        
        [keepAliveFirstTry lock];
        [keepAliveFirstTry wait];
        [keepAliveFirstTry unlock];
        if (keepAliveFail)
        {
            if (firstTry) goto firstFail; // first try failed
            else goto udp_fail;
        }
        else
        {
            if (firstTry)
            {
                [self resetOnlineTime];
                connectTimer = [[NSThread alloc] initWithTarget:self selector:@selector(updateOnlineTime) object:nil];
                [connectTimer start];
                
                firstTry = NO;
            }
            
            // prevent app nap
            if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)])
                self.activity = [[NSProcessInfo processInfo] beginActivityWithOptions:0x00FFFFFF reason:@"receiving gateway keep-alive packets"];
            
            [self.lblStatus setStringValue:@"已连接上"];
            [self.btnConnect setEnabled:YES];
            [self.btnConnect setTitle:@"断开"];
            //[self.connectIndicator setHidden:YES];
        }
        
        [keepAliveCondition lock];
        while (!keepAliveFail && ![connectJob isCancelled])
            [keepAliveCondition wait];
        [keepAliveCondition unlock];
        
        if (keepAliveFail && ![connectJob isCancelled]) goto udp_fail;
        else continue;
        
    udp_fail:
        if (firstTry) // first try failed
        {
            [self.lblStatus setStringValue:@"UDP认证失败！"];
            goto firstFail;
        }
        else
        {
            if ([[NSThread currentThread] isCancelled]) break;
            
            [self.lblStatus setStringValue:@"连接丢失，5秒后重试。"];
            [NSThread sleepForTimeInterval:5.0f];
            dealer = nil;
        }
    }
    
    // canceled
    if (threadKeepAlive != nil && [threadKeepAlive isExecuting])
        [threadKeepAlive cancel];
    
    if (self.activity)
        [[NSProcessInfo processInfo] endActivity:self.activity];
    self.activity = nil;
    
    [self.lblStatus setStringValue:@"UDP注销中……"];
    try {
        if ([dealer isUDPObjectInit])
        {
            [dealer sendAliveRequest];
            [dealer startRequest];
            [dealer sendLogoutAuth];
        }
    } catch (std::exception&) {
        // fuck it, but we don't mangage it :P
    }
    
interrupt:
    [self.lblStatus setStringValue:@"已断开"];
    
firstFail:
    [self resetOnlineTime];
    [self.selectedMode setEnabled:YES];
    [self.nicList setEnabled:YES];
    [self.userName setEnabled:YES];
    [self.passWord setEnabled:YES];
    [self.btnConnect setEnabled:YES];
    
    [self.lblIPAddr setStringValue:@"-"];
    [self.lblMacAddr setStringValue:@"-"];
    [self.btnConnect setTitle:@"连接"];
    //[self.connectIndicator setHidden:YES];
    
    connectJob = nil;
}

/*- (BOOL) checkIsOnline
{
    BOOL ret = YES;
    std::string strRetData;
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strRetData);
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK)
        ret = NO;
    else
    {
        if (strRetData.find("Logout") == -1) // seems we're not online.
            ret = NO;
    }
    
    return ret;
}*/

@end
