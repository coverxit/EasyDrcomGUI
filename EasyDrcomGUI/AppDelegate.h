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

#import <Cocoa/Cocoa.h>
//#import <curl/curl.h>
#import "PcapBridge.h"
#import "DrcomDealerU31.h"
#import "DrcomDealerU62.h"

enum
{
    ConnectionModeStudentDistrict = 0,
    ConnectionModeWorkDistrict = 1,
};
typedef NSInteger ConnectionMode;

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
typedef NSUInteger AuthErrorCode;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
{
    NSDictionary *udpErrorDict;
    
    NSString *logPath;
    
    NSThread* connectTimer;
    NSInteger connectTime;
    PcapBridge* bridge;

    NSString *storedNIC, *storedUserName, *storedPassWord, *storedIP, *storedMAC;
    NSThread *connectJob;
    
    BOOL keepAliveFail;
    NSCondition *keepAliveCondition;
    NSCondition *keepAliveFirstTry;
    
    ConnectionMode connectMode;
    //CURL *curl;
}

@property (strong) id activity;

@property (assign) IBOutlet NSWindow *window;

@property (nonatomic, retain) IBOutlet NSMatrix *selectedMode;
@property (nonatomic, retain) IBOutlet NSPopUpButton *nicList;
@property (nonatomic, retain) IBOutlet NSTextField *userName;
@property (nonatomic, retain) IBOutlet NSTextField *passWord;
@property (nonatomic, retain) IBOutlet NSButton *btnConnect;
//@property (nonatomic, retain) IBOutlet NSProgressIndicator *connectIndicator;

@property (nonatomic, retain) IBOutlet NSBox *boxStatus;
@property (nonatomic, retain) IBOutlet NSTextField *lblStatus;
@property (nonatomic, retain) IBOutlet NSTextField *lblOnlineTime;
@property (nonatomic, retain) IBOutlet NSTextField *lblIPAddr;
@property (nonatomic, retain) IBOutlet NSTextField *lblMacAddr;
@property (nonatomic, retain) IBOutlet NSTextField *lblBuild;

- (IBAction) connectClicked:(id)sender;
- (IBAction) modeChanged:(id)sender;
- (IBAction) viewLogClicked:(id)sender;
- (IBAction) supportClicked:(id)sender;


- (void) alertFetchNICListFailed:(NSAlert*)alert returnCode:(NSInteger)returnCode contextInfo:(void*)contextInfo;
- (void) alertQuitApplication:(NSAlert*)alert returnCode:(NSInteger)returnCode contextInfo:(void*)contextInfo;

- (void) updateOnlineTime;

- (void) connectionJobForWorkDistrict;
- (void) connectionJobForStudentDistrict;

- (void) keepAliveForWorkDistrictWithDealer: (DrcomDealerU62*)dealer;
- (void) keepAliveForStudentDistrictWithDealer: (DrcomDealerU31*)dealer;

- (void) resetOnlineTime;
- (void) notificateWithString:(NSString*) string notificateType:(NSString*) type;

//- (BOOL) checkIsOnline;
@end
