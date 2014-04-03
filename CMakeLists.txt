cmake_minimum_required(VERSION 2.8)

# Workaround target_compile_options for cmake < 2.8.12
if(${CMAKE_VERSION} VERSION_LESS "2.8.12")
	function(target_compile_options target scope)
		set(args ${ARGV})
		list(REMOVE_AT args 0 1)
		string(REPLACE ";" " " CFLAGS_STRING "${args}")
		set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${CFLAGS_STRING}")
	endfunction()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT OpenVRML)
project(${PROJECT})

set(PACKAGE_VERSION "0.18.99")
if(MSVC)
	set(OPENVRML_API)
	set(OPENVRML_LOCAL)
else()
	set(OPENVRML_API "__attribute__((visibility(\"default\")))")
	set(OPENVRML_LOCAL "__attribute__((visibility(\"hidden\")))")
endif()
set(OPENVRML_PKGDATADIR_ "share/${PROJECT}")
set(OPENVRML_PKGLIBDIR_ "lib/${PROJECT}")
set(OPENVRML_ENABLE_THROWING_EXCEPTION_SPECS 0)
set(OPENVRML_ENABLE_NOTHROW_EXCEPTION_SPECS 0)

option(BUILD_DOCUMENTATION "Build documentation" ON)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_MODELS "Build models" OFF)
option(BUILD_TESTS "Build tests" OFF)

option(BUILD_GL "Build openvrml-gl" ON)
option(BUILD_MOZILLA "Build mozilla-plugin" ON)
option(BUILD_PLAYER "Build player" ON)
option(BUILD_XEMBED "Build xembed" ON)
option(BUILD_VRML "Build vrml97" ON)

option(BUILD_WITH_PNG "Enable png texture support" ON)
option(BUILD_WITH_JPEG "Enable jpeg texture support" ON)
option(BUILD_WITH_FREETYPE "Enable text nodes support" ON)

# TODO: Pass NDEBUG to msvc build when doing release build
# TODO: and pass _DEBUG when doing debug build

find_package(Boost REQUIRED filesystem system thread)

if(NOT MSVC)
	find_package(LibXml2 REQUIRED)
endif()

if(BUILD_WITH_PNG)
	find_package(PNG REQUIRED)
endif()

if(BUILD_WITH_JPEG)
	find_package(JPEG REQUIRED)
endif()

if(BUILD_WITH_FREETYPE)
	find_package(Freetype REQUIRED)
endif()

add_subdirectory(data)

if(BUILD_DOCUMENTATION)
   add_subdirectory(doc)
endif()

if(BUILD_EXAMPLES)
   add_subdirectory(examples)
endif()

if(BUILD_MODELS)
   add_subdirectory(models)
endif()

add_subdirectory(src)

if(BUILD_TESTS)
   add_subdirectory(tests)
endif()
