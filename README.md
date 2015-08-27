# EasyDrcomGUI
GUI Wrapper for **[EasyDrcom](https://github.com/coverxit/EasyDrcom)**, which is a 3rd Party Dr.COM Client for HITwh.

## Description
**本分支（master）用于存放针对各平台的 EasyDrcomGUI 发行版。若您需要获取源代码，请根据所需平台移步至对应分支：**

* Windows：[Win32分支](https://github.com/coverxit/EasyDrcomGUI/tree/Win32)
* Mac OS：[MacOS分支](https://github.com/coverxit/EasyDrcomGUI/tree/MacOS)
* Linux：[Linux分支](https://github.com/coverxit/EasyDrcomGUI/tree/Linux)

## Current Version
* Windows：**v1.6**，[Download](https://github.com/coverxit/EasyDrcomGUI/blob/master/Windows/EasyDrcomGUI_v1.6_For_Windows_XP_SP3_or_higher.zip?raw=true)
* Mac OS：**v1.6**，[Download](https://github.com/coverxit/EasyDrcomGUI/blob/master/Mac%20OS/EasyDrcomGUI_v1.6_For_OSX_10.9_or_higher.zip?raw=true)
* Linux (64-bit)：**v1.2**，[Download](https://github.com/coverxit/EasyDrcomGUI/blob/master/Linux%20(64-bit)/EasyDrcomGUI_v1.2_For_Ubuntu_14.04_or_higher.zip?raw=true)

## Introduction
EasyDrcomGUI 是 EasyDrcom（获取详情请[点击这里](http://bbs.ghtt.net/thread-1984091-1-1.html)）的衍生作品，是 EasyDrcom 的图形界面版，可用于学生区、教学区、家属区，可运行在 Windows、Mac OSX 和 Linux 等主流操作系统上，Windows、Mac OSX 和 Linux 版均已发布。

EasyDrcomGUI 开发的初衷是为了提供**另一种途径(alternative way)**，从而更加自由地使用校园网。

**关于 EasyDrcomGUI 更详细的使用说明，请参见 [http://bbs.ghtt.net/thread-2013765-1-1.html](http://bbs.ghtt.net/thread-2013765-1-1.html)**

## Advantages
相比我校原有的客户端，EasyDrcomGUI有以下优点：

* 学生区和教学区采用**统一客户端**，避免了使用两个客户端的繁琐。
* EasyDrcomGUI 默认开启**断线重连**功能，尽力提高用户体验。
* 您将**不会再遇到**诸如“发现您正在使用以下代理软件：Windows(vista)系统自带共享代理。”、“发现您修改了网卡MAC地址。”等错误，因为 EasyDrcomGUI 开发的初衷（也是 EasyDrcom 的初衷）是为了**更加自由地使用校园网**。
* 您将**几乎不会再遇到**“无法选择网卡”、“获取用户属性超时！请检查防火墙配置允许UDP61440端口！”、“客户端页面无法自动显示，通知栏出现两个客户端图标（一个彩色一个黑白）”等错误。

此外，EasyDrcomGUI 在不同操作系统上的发行版均使用**原生代码**编写，以最大程度上保证兼容性。

## Dependencies
不同平台的 EasyDrcomGUI 依赖于不同的库，使用前请确认您已经安装依赖库。
 
* Windows版：[WinPcap](https://github.com/coverxit/EasyDrcomGUI/blob/master/Windows/WinPcap_4_1_3.exe?raw=true), 对于Windows 10用户：[Win10Pcap](https://github.com/coverxit/EasyDrcomGUI/blob/master/Windows/Win10Pcap-v10.1-5001.msi?raw=true)
* Mac OS版：libpcap（已内建于Mac OS）
* Linux (64-bit)版：
	* libc6
	* libqt5gui5
	* libpcap0.8（已静态链接）

## Supported OS
<table>
	<tbody>
		<tr>
			<td><strong>Windows</strong></td><td><strong>Mac OS X</strong></td><td><strong>Linux (64-bit)</strong></td>
		</tr>
		<tr>
			<td>Windows 10</td><td>OS X 10.11 El Capitan</td><td>Ubuntu 15.04 "Vivid Vervet"</td>
		</tr>
		<tr>
			<td>Windows 8（及8.1）</td><td>OS X 10.10 Yosemite</td><td>Ubuntu 14.10 "Utopic Unicorn"</td>
		</tr>
		<tr>
			<td>Windows 2012 （及R2）</td><td>OS X 10.9 Mavericks</td><td>Ubuntu 14.04 LTS "Trusty Tahr"</td>
		</tr>
		<tr>
			<td>Windows 7</td><td>-</td><td>-</td>
		</tr>
		<tr>
			<td>Windows Vista</td><td></td><td></td>
		</tr>
		<tr>
			<td>Windows 2008（及R2)</td><td></td><td></td>
		</tr>
		<tr>
			<td>Windows XP SP3</td><td></td><td></td>
		</tr>
		<tr>
			<td>Windows 2003 SP2 （及R2）</td><td></td><td></td>
		</tr>
	</tbody>
</table>
* 注1：Mac OSX 10.8 Lion及以下版本未经测试，其支持情况亦未知。
* 注2：Windows 10请使用Win10Pcap代替WinPcap。
* 注3：Linux版仅提供64位(x86_64/amd64)版，目前仅作者保证对Ubuntu LTS版(14.04以上)的支持。

## Change Log
* July 15, 2015 - Windows、OSX版更新至1.6，Linux版更新至1.2，增加分别保存学生区和办公区网卡、用户名、密码的功能。
* July 13, 2015 - Windows、OSX版更新至1.5，Linux版更新至1.1，增强连接稳定性，降低“发现心跳包失败”可能性。
* July 4, 2015 - 发布EasyDrcom GUI For Linux v1.0.
* June 24, 2015 - Windows版更新至1.4，修复办公区有时掉线的问题；OSX版更新至1.4，增强程序稳定性。
* June 9, 2015 - Windows、OSX版更新至1.3，增强办公区连接稳定性。
* May 18, 2015 - Windows版更新至1.2，修复XP上启动时提示“无法定位程序输入点GetTickCount64于动态链接库KERNEL32.DLL上”的问题；若有实例正在运行，当再次运行EasyDrcomGUI时，把上一实例的窗口置顶。
* May 17, 2015 - OSX版更新至1.2，修复初始化访问无效内存的BUG，感谢@liuchang988。
* May 17, 2015 - Windows、OSX版更新至1.1，增加“Authentication Fail ErrCode=05”对应的中文提示。
* May 17, 2015 - 发布EasyDrcom GUI For Windows v1.0、EasyDrcom GUI For Mac OSX v1.0.

## License
EasyDrcomGUI License:

	Copyright (C) 2014, 2015 Shindo 
	
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at
	
		http://www.apache.org/licenses/LICENSE-2.0
	
	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissionss and
	limitations under the License.

glibc License:

	GNU C Library is licensed under GNU Lesser General Public License.
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
	USA.
	
Qt License:

	This program uses Qt version 5.2.1.

	Qt is a C++ toolkit for cross-platform application development.
	Qt provides single-source portability across all major desktop operating systems. It is also available for embedded Linux and other embedded and mobile operating systems.
	Qt is available under three different licensing options designed to accommodate the needs of our various users.
	Qt licensed under our commercial license agreement is appropriate for development of proprietary/commercial software where you do not want to share any source code with third parties or otherwise cannot comply with the terms of the GNU LGPL version 2.1 or GNU GPL version 3.0.
	Qt licensed under the GNU LGPL version 2.1 is appropriate for the development of Qt applications provided you can comply with the terms and conditions of the GNU LGPL version 2.1.
	Qt licensed under the GNU General Public License version 3.0 is appropriate for the development of Qt applications where you wish to use such applications in combination with software subject to the terms of the GNU GPL version 3.0 or where you are otherwise willing to comply with the terms of the GNU GPL version 3.0.
	Please see qt.digia.com/Product/Licensing for an overview of Qt licensing.
	Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies) and other contributors.
	Qt and the Qt logo are trademarks of Digia Plc and/or its subsidiary(-ies).
	Qt is developed as an open source project on qt-project.org.
	Qt is a Digia product. See qt.digia.com for more information.

libpcap License:

	License: BSD
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:
	
	  1. Redistributions of source code must retain the above copyright
	     notice, this list of conditions and the following disclaimer.
	  2. Redistributions in binary form must reproduce the above copyright
	     notice, this list of conditions and the following disclaimer in
	     the documentation and/or other materials provided with the
	     distribution.
	  3. The names of the authors may not be used to endorse or promote
	     products derived from this software without specific prior
	     written permission.
	
	THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

STPrivilegedTask License:

	STPrivilegedTask - NSTask-like wrapper around AuthorizationExecuteWithPrivileges
	Copyright (C) 2009-2015 Sveinbjorn Thordarson <sveinbjornt@gmail.com>
	 
	BSD License
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	     * Redistributions of source code must retain the above copyright
	       notice, this list of conditions and the following disclaimer.
	     * Redistributions in binary form must reproduce the above copyright
	       notice, this list of conditions and the following disclaimer in the
	       documentation and/or other materials provided with the distribution.
	     * Neither the name of Sveinbjorn Thordarson nor that of any other
	       contributors may be used to endorse or promote products
	       derived from this software without specific prior written permission.
	  
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL  BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

WinPcap License:

	Copyright (c) 1999 - 2005 NetGroup, Politecnico di Torino (Italy).
	Copyright (c) 2005 - 2010 CACE Technologies, Davis (California).
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
	
	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. 
	3. Neither the name of the Politecnico di Torino, CACE Technologies nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

WTL License:

	Copyright © 2014 Microsoft Corporation, WTL Team. All rights reserved.
	 
	This file is a part of the Windows Template Library.
	The use and distribution terms for this software are covered by the
	Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php).
	By using this software in any fashion, you are agreeing to be bound by
	the terms of this license. You must not remove this notice, or
	any other, from this software.