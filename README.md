# EasyDrcomGUI
GUI Wrapper for **[EasyDrcom](https://github.com/coverxit/EasyDrcom)**, which is a 3rd Party Dr.COM Client for HITwh.

## Description
**本分支（MacOS）用于存放 Mac OS 版 EasyDrcomGUI 的源代码。若您需要获取发行版，请移步至 [master 分支](https://github.com/coverxit/EasyDrcomGUI)。**

**若您需要获取其他平台源代码，请根据所需平台移步至对应分支：**

* Windows：[Win32分支](https://github.com/coverxit/EasyDrcomGUI/tree/Win32)
* Mac OS：[MacOS分支](https://github.com/coverxit/EasyDrcomGUI/tree/MacOS)
* Linux：[Linux分支](https://github.com/coverxit/EasyDrcomGUI/tree/Linux)

## Introduction
Linux 版 EasyDrcomGUI（以下简称**本项目**）使用 Qt Creator 3.0.1 (based on Qt 5.2.1) 开发，语言为 C++（包括C++ 11的部分特性）。

本项目依赖于 **libc**、**libqt5gui**、**libpcap**，依赖库的版本号如下：

* libc6
* libqt5gui5
* libpcap0.8（静态链接）

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