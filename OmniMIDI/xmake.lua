set_xmakever("2.9.8")
set_project("OmniMIDIv2")

set_allowedplats("windows", "linux")
set_allowedmodes("debug", "release")
set_allowedarchs("x86", "x64", "x86_64", "arm64")
	
add_rules("mode.release", "mode.debug")
set_languages("clatest", "cxxlatest")
set_runtimes("stdc++_static")

target("OmniMIDI")
	set_kind("shared")

	if is_mode("debug") then
		add_defines("DEBUG")
		set_symbols("debug")
		set_optimize("none")
	else	
		add_defines("NDEBUG")
		set_symbols("none")
		set_optimize("fastest")
	end
	
	add_defines("OMNIMIDI_EXPORTS")

	add_ldflags("-j")

	add_includedirs("inc")
	add_files("src/*.cpp")

	if is_plat("windows") then	
		set_toolchains("mingw")

		-- Remove lib prefix
		set_prefixname("")
		
		add_cxxflags("clang::-fexperimental-library", { force = true })
		add_shflags("-static-libgcc", { force = true })
		add_syslinks("winmm", "shlwapi", "ole32", "-l:libwinpthread.a")
		add_defines("_WIN32", "_WINXP", "_WIN32_WINNT=0x6000")

		remove_files("UnixEntry.cpp")
	else
		set_toolchains("gcc")

		add_cxflags("-fvisibility=hidden", "-fvisibility-inlines-hidden")

		remove_files("bassasio.cpp")
		remove_files("basswasapi.cpp")
		remove_files("WDMEntry.cpp")
		remove_files("StreamPlayer.cpp")
		remove_files("WDMDrv.cpp")
		remove_files("WDMEntry.cpp")	
	end
target_end()