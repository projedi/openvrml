set(openvrml-dl_PRIVATE_HEADERS openvrml/local/dl.h)
set(openvrml-dl_SOURCES openvrml/local/dl.cpp)

source_group("Source Files" FILES ${openvrml-dl_SOURCES})
source_group("Header Files" FILES ${openvrml-dl_HEADERS} ${openvrml-dl_PRIVATE_HEADERS})

set(openvrml-dl_LIBRARIES
	${Boost_FILESYSTEM_LIBRARY}
)
set(openvrml-dl_DEFINES
	BOOST_FILESYSTEM_VERSION=3 OPENVRML_BUILD_DLL BOOST_ALL_NO_LIB
)
set(openvrml-dl_INCLUDE_DIRS
	../../libopenvrml
	${CMAKE_CURRENT_BINARY_DIR}/../../libopenvrml
	${Boost_INCLUDE_DIRS}
)

if(UNIX)
   list(APPEND openvrml-dl_LIBRARIES ltdl)
endif()

add_library(openvrml-dl STATIC ${openvrml-dl_SOURCES}
	${openvrml-dl_PRIVATE_HEADERS})

target_link_libraries(openvrml-dl ${openvrml-dl_LIBRARIES})
target_compile_definitions(openvrml-dl PUBLIC ${openvrml-dl_DEFINES})
target_include_directories(openvrml-dl PUBLIC ${openvrml-dl_INCLUDE_DIRS})
set_target_properties(openvrml-dl PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
