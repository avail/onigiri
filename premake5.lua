local submodule = require "tools/submodule"

systemversion "10.0.25272.0"
toolset "v143"
platforms { "x64" }

submodule.define({
	name = "minhook",
	language = "c",
	libmode = "staticlib",
	includes = {
		"vendor/minhook/include"
	},
	files = {
		"vendor/minhook/src/buffer.c",
		"vendor/minhook/src/hde/hde64.c",
		"vendor/minhook/src/hook.c",
		"vendor/minhook/src/trampoline.c"
	}
})

workspace "REKT.GTA5"
	configurations { "Release" }

	location "build"

	startproject "onigiri"

	symbols "On"
	staticruntime "On"
	floatingpoint "Fast"
	vectorextensions "AVX2"
	cppdialect "c++latest"

	defines {
		"NOMINMAX",
		"WIN32_LEAN_AND_MEAN",
		"_CRT_SECURE_NO_WARNINGS",
		"_USE_MATH_DEFINES"
	}

	filter "configurations:Release"
		flags { "LinkTimeOptimization", "NoManifest", "MultiProcessorCompile" }
		targetdir "bin"
		defines { "NDEBUG" }
		optimize "speed"

	filter {}

	flags { "NoIncrementalLink" }

	editandcontinue "Off"

project "shared"
	kind "StaticLib"
	language "C++"
	staticruntime "On"

	includedirs {
		"src/shared"
	}

	files {
		"src/shared/**.*"
	}

project "clientdll"
	targetname "onigiri"
	kind "SharedLib"
	language "C++"
	staticruntime "On"

	pchheader "stdinc.hpp"
	pchsource "src/clientdll/stdinc.cpp"

	includedirs {
		"src/clientdll",
		"src/shared"
	}

	files {
		"src/clientdll/**.cpp",
		"src/clientdll/**.hpp",
		"src/clientdll/**.h"
	}

	links {
		"shared",
		"winmm"
	}

	submodule.include({ "minhook" })

project "onigiri"
	kind "ConsoleApp"
	language "c++"
	staticruntime "On"

	pchheader "stdinc.hpp"
	pchsource "src/loader/stdinc.cpp"

	includedirs {
		"src/loader"
	}

	files {
		"src/loader/**.cpp",
		"src/loader/**.hpp",
	}

group "vendor"
	submodule.register_all()