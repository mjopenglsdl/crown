The flexible game engine. [![Build Status](https://github.com/dbartolini/crown/workflows/build_and_test/badge.svg)](https://github.com/dbartolini/crown/actions)
=====================================

Crown is a general purpose data-driven game engine, written from scratch in [orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b) with a minimalistic and data-oriented design philosophy in mind.

It is loosely inspired by Bitsquid (now Stingray) engine and its design principles; the current Lua API is similar to that of Bitsquid but this engine is *not* meant to be its clone *nor* to be API compatible with it.

## Download

  * [Crown v0.36.0 for Linux 64-bits](https://github.com/dbartolini/crown/releases/download/v0.36.0/crown-0.36.0-linux-x64.tar.gz)
  * [Changelog](https://github.com/dbartolini/crown/blob/master/docs/changelog.rst)
  
## Support & Development

  * [Manual](http://dbartolini.github.io/crown/html/latest)
  * [Lua API](http://dbartolini.github.io/crown/html/latest/lua_api.html)
  * [C++ API](http://dbartolini.github.io/crown/doxygen/modules)
  * [Discord](https://discord.gg/CeXVWCT): [![Discord Chat](https://img.shields.io/discord/572468149358690314.svg)](https://discord.gg/CeXVWCT)
  * [Trello Roadmap](https://trello.com/b/h88kbJNm/crown-game-engine)

## Sponsors
**Register sponsors**  

**L1$ sponsors**  

**L2$ supporters**  

**L3$ supporters**  

## Screenshots

### [Level Editor](https://github.com/dbartolini/crown/tree/master/tools/level_editor)

![level-editor](https://raw.githubusercontent.com/dbartolini/crown/master/docs/shots/level-editor.png)

### [00-empty](https://github.com/dbartolini/crown/tree/master/samples/00-empty)

Engine initialization and shutdown.

### [01-physics](https://github.com/dbartolini/crown/tree/master/samples/01-physics)
![01-physics](https://raw.githubusercontent.com/dbartolini/crown/master/docs/shots/01-physics.png)

### [02-animation](https://github.com/dbartolini/crown/tree/master/samples/02-animation)
![02-animation](https://raw.githubusercontent.com/dbartolini/crown/master/docs/shots/02-animation.png)

## Building

### Prerequisites

### Android

Android NDK (https://developer.android.com/tools/sdk/ndk/index.html)

	$ export ANDROID_NDK_ROOT=<path/to/android_ndk>
	$ export ANDROID_NDK_ARM=<path/to/android_ndk_arm>

### Linux (Ubuntu >= 16.04)

    $ sudo add-apt-repository ppa:vala-team
    $ sudo apt-get install libgtk-3-dev valac libgee-0.8-dev
    $ sudo apt-get install mesa-common-dev libgl1-mesa-dev libpulse-dev libxrandr-dev

### Windows

MSYS2 (http://www.msys2.org)

### Building and running Level Editor

	$ make tools-linux-release64
	$ cd build/linux64/bin
	$ ./level-editor-release ../../../samples/01-physics

Contact
-------

Daniele Bartolini ([@aa_dani_bart](https://twitter.com/aa_dani_bart))  
Project page: https://github.com/dbartolini/crown

Contributors
------------

In chronological order.

Daniele Bartolini ([@dbartolini](https://github.com/dbartolini))  
Simone Boscaratto ([@Xed89](https://github.com/Xed89))  
Michele Rossi ([@mikymod](https://github.com/mikymod))  
Michela Iacchelli - Pepper logo.  
Raphael de Vasconcelos Nascimento ([@vasconssa](https://github.com/vasconssa))

License
-------

	Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
