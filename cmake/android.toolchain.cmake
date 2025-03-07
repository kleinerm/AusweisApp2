cmake_minimum_required(VERSION 3.16.0)
if(CMAKE_VERSION VERSION_LESS "3.16.4")
	message(WARNING "Check your cmake: https://gitlab.kitware.com/cmake/cmake/commit/e3d3b7ddeb5922a4d17b962984e698e6387f7544")
endif()

function(READ_REVISION _var _regex _files)
	foreach(file ${_files})
		if(EXISTS "${file}")
			file(READ "${file}" content)
			string(REGEX MATCH "${_regex}" _unused "${content}")
			set(${_var} ${CMAKE_MATCH_1} PARENT_SCOPE)
			return()
		endif()
	endforeach()
endfunction()

IF(NOT CMAKE_ANDROID_NDK)
	set(_ndk_dest $ENV{ANDROID_NDK_ROOT} $ENV{ANDROID_NDK_HOME} $ENV{ANDROID_NDK})
	foreach(_ndk ${_ndk_dest})
		if(EXISTS "${_ndk}")
			set(CMAKE_ANDROID_NDK ${_ndk})
		endif()
	endforeach()

	if(NOT CMAKE_ANDROID_NDK)
		message(FATAL_ERROR "Cannot find ANDROID_NDK_ROOT")
	endif()
endif()

set(ANDROID_SDK $ENV{ANDROID_SDK_ROOT})
set(ANDROID_BUILD_TOOLS_REVISION $ENV{ANDROID_BUILD_TOOLS_REVISION})

if(NOT EXISTS "${ANDROID_SDK}")
	message(FATAL_ERROR "Environment variable ANDROID_SDK_ROOT is undefined")
endif()

if(NOT ANDROID_BUILD_TOOLS_REVISION)
	set(_android_build_tools_dir "${ANDROID_SDK}/build-tools")

	file(GLOB build_tools_list ${_android_build_tools_dir}/*)
	list(LENGTH build_tools_list build_tools_len)
	if(build_tools_len GREATER 0)
		list(SORT build_tools_list)
		list(GET build_tools_list -1 build_tools)
	endif()

	if(NOT build_tools)
		message(FATAL_ERROR "Cannot determine ANDROID_BUILD_TOOLS_REVISION: ${_android_build_tools_dir}")
	endif()

	get_filename_component(ANDROID_BUILD_TOOLS_REVISION "${build_tools}" NAME)
endif()

READ_REVISION(ANDROID_NDK_REVISION ".*Revision = ([0-9|\\.]+)" "${CMAKE_ANDROID_NDK}/source.properties")
READ_REVISION(ANDROID_SDK_REVISION ".*Revision=([0-9|\\.]+)" "${ANDROID_SDK}/cmdline-tools/latest/source.properties;${ANDROID_SDK}/tools/source.properties")

set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 24)
set(ANDROID_TARGET_SDK_VERSION 31)
set(CMAKE_ANDROID_STL_TYPE c++_shared)

if(NOT CMAKE_ANDROID_ARCH_ABI)
	if(INTEGRATED_SDK)
		set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
	else()
		set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
	endif()
endif()

if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
	set(CMAKE_ANDROID_ARM_MODE ON)
	set(CMAKE_ANDROID_ARM_NEON ON)
endif()

# Emulate NDK CMake-Variable as Qt 5.14 needs this (Multi-ABI)
set(ANDROID_ABI ${CMAKE_ANDROID_ARCH_ABI})


set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH} CACHE STRING "android find search path root")
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
