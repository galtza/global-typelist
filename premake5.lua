--[[
    MIT License

    Copyright (c) 2019-2020 Raúl Ramos

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
--]]

workspace "global-typelist"
    location ".build"

    configurations { "Release", "Debug" }
    platforms { "x64" }

	language "C++"
	cppdialect "C++11"

    flags { "FatalCompileWarnings", "FatalLinkWarnings" }

    filter { "configurations:Debug"   } defines { "DEBUG" }
    filter { "configurations:Release" } defines { "NDEBUG" }
    filter { "platforms:*64"          } architecture "x86_64"

    filter { "system:macosx", "action:gmake"}
        toolset "clang"

    filter { "system:windows", "action:vs*"}
        buildoptions { "/W3" }

    filter {"toolset:clang or toolset:gcc"}
        buildoptions { "-Wall", "-Wextra", "-pedantic" }

    filter {}

project "global-typelist"
    kind "ConsoleApp"
    targetdir ".out/%{cfg.system}/%{prj.name}/%{cfg.platform}/%{cfg.buildcfg}"
    objdir ".tmp/%{cfg.system}/%{prj.name}"
    files { "*.cpp", "*.h" }

