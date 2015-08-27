# EasyDrcomGUI
GUI Wrapper for **[EasyDrcom](https://github.com/coverxit/EasyDrcom)**, which is a 3rd Party Dr.COM Client for HITwh.

## Description
**本分支（Win32）用于存放 Windows 版 EasyDrcomGUI 的源代码。若您需要获取发行版，请移步至 [master 分支](https://github.com/coverxit/EasyDrcomGUI)。**

**若您需要获取其他平台源代码，请根据所需平台移步至对应分支：**

* Windows：[Win32分支](https://github.com/coverxit/EasyDrcomGUI/tree/Win32)
* Mac OS：[MacOS分支](https://github.com/coverxit/EasyDrcomGUI/tree/MacOS)
* Linux：[Linux分支](https://github.com/coverxit/EasyDrcomGUI/tree/Linux)

## Introduction
Windows 版 EasyDrcomGUI（以下简称**本项目**）使用 Visual Studio 2013 开发，语言为 C++（包括C++ 11的部分特性）。

本项目依赖于 **WinPcap** 和 **WTL**，上述依赖库相关头文件、库文件已包含在源代码中，依赖库的版本号如下：

* WinPcap (4.1.2 Developer's Pack)
* WTL (9.0.4140 Final)

## Change Log
**请移步至 [master 分支](https://github.com/coverxit/EasyDrcomGUI) 查看修改日志。**

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