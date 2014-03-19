add_subdirectory(libopenvrml)

if(BUILD_GL)
	add_subdirectory(libopenvrml-gl)
endif()

# TODO: Build local/libopenvrml-control when requested
add_subdirectory(local/libopenvrml-dl)

if(BUILD_MOZILLA)
	add_subdirectory(mozilla-plugin)
endif()

add_subdirectory(node)

if(BUILD_PLAYER)
	add_subdirectory(openvrml-player)
endif()

if(BUILD_XEMBED)
	add_subdirectory(openvrml-xembed)
endif()

add_subdirectory(script)
