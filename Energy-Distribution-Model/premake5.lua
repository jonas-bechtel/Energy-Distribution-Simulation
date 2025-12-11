project "Energy-Distribution-Model"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"

	files	
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Energy-Distribution-Model/src",
		"%{wks.location}/Energy-Distribution-Model/src/Cooling-Force",
		"%{wks.location}/Energy-Distribution-Model/src/Cross-Section-Deconvolution",
		"%{wks.location}/Energy-Distribution-Model/src/Energy-Distribution-Generation",
		"%{wks.location}/Energy-Distribution-Model/src/general",
		"%{wks.location}/Energy-Distribution-Model/src/utility",
		"%{wks.location}/Energy-Distribution-Model/vendor/imgui",
		"%{wks.location}/Energy-Distribution-Model/vendor/imgui/backends",
		"%{wks.location}/Energy-Distribution-Model/vendor/implot",
		"%{wks.location}/Energy-Distribution-Model/vendor/eigen",
		"%{wks.location}/Energy-Distribution-Model/vendor/tinyfiledialogs",
		"%{wks.location}/Energy-Distribution-Model/vendor/JSPEC/include",
		"%{wks.location}/Energy-Distribution-Model/vendor/Betacool/src",
		"%{wks.location}/Energy-Distribution-Model/vendor/fmt-12.1.0/include",
		"%{wks.location}/vendor/root_v6.32.04/include"
	}

	libdirs 
	{
		"%{wks.location}/vendor/root_v6.32.04/lib"
	}

	links
	{
		"imgui",
		"implot",
		"tinyfiledialogs",
		"JSPEC",
		"Betacool",

		"gsl"
        --"gslcblas"
        --"muparser"
	}

	filter "system:windows"
		systemversion "latest"

		libdirs
        {
            "%{wks.location}/Energy-Distribution-Model/vendor/JSPEC/lib"
        }

		links
		{
			"libCore",
			"libRIO",
			"libHist",
			"libGpad",
			"libGraf",
			"libGraf3d",
			"libMatrix",
			"libMathCore",
			"libPhysics",
			"libGui",

			"d3d12",
			"d3dcompiler",
			"dxgi"
		}
		
		postbuildcommands
		{
			'cmd /c if exist vendor\\JSPEC\\lib\\*.dll xcopy /Q /Y /I vendor\\JSPEC\\lib\\*.dll "%{cfg.targetdir}" > nul',
			'cmd /c if exist "..\\..\\Energy Distribution Model Release" copy "%{cfg.buildtarget.relpath}" "..\\..\\Energy Distribution Model Release"',
			'cmd /c if exist "..\\..\\Energy Distribution Model Release" copy "imgui.ini" "..\\..\\Energy Distribution Model Release"'
		}

		debugenvs
		{ 
			"ROOTSYS=%{wks.location}\\vendor\\root_v6.32.04",
			"PATH=%{wks.location}\\vendor\\root_v6.32.04\\bin;%PATH%"
		}

	filter "system:linux"
		systemversion "latest"
		pic "On"

		includedirs
		{
			"/usr/include/SDL2"
		}

		libdirs
        {
            "/usr/lib/x86_64-linux-gnu"
        }

		links 
		{
			"Core",
			"RIO",
			"Hist",
			"Gpad",
			"Graf",
			"Graf3d",
			"Matrix",
			"MathCore",
			"Physics",
			"Gui",

			"SDL2",
			"SDL2_image"
			"GL",
			"tbb"
    	}
    

	filter "configurations:Debug"
		defines "_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "NDEBUG"
		runtime "Release"
		optimize "on"
		symbols "on"

	