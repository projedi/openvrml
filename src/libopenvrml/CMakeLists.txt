set(openvrml_HEADERS
	openvrml/bad_url.h
	openvrml/basetypes.h
	openvrml/vrml97_grammar.h
	openvrml/x3d_vrml_grammar.h
	openvrml/field_value.h
	openvrml/event.h
	openvrml/exposedfield.h
	openvrml/scope.h
	openvrml/node.h
	openvrml/bounding_volume.h
	openvrml/script.h
	openvrml/scene.h
	openvrml/browser.h
	openvrml/viewer.h
	openvrml/rendering_context.h
	openvrml/frustum.h
	openvrml/node_impl_util.h
	openvrml-common.h
)

set(openvrml_PRIVATE_HEADERS
	openvrml/local/conf.h
	openvrml/local/error.h
	openvrml/local/uri.h
	openvrml/local/xml_reader.h
	openvrml/local/parse_vrml.h
	openvrml/local/component.h
	openvrml/local/proto.h
	openvrml/local/externproto.h
	openvrml/local/field_value_types.h
	openvrml/local/float.h
	openvrml/local/node_metatype_registry_impl.h
	private.h
)

set(openvrml_SOURCES
	openvrml/bad_url.cpp
	openvrml/vrml97_grammar.cpp
	openvrml/x3d_vrml_grammar.cpp
	openvrml/basetypes.cpp
	openvrml/field_value.cpp
	openvrml/event.cpp
	openvrml/exposedfield.cpp
	openvrml/scope.cpp
	openvrml/node.cpp
	openvrml/script.cpp
	openvrml/bounding_volume.cpp
	openvrml/scene.cpp
	openvrml/browser.cpp
	openvrml/viewer.cpp
	openvrml/rendering_context.cpp
	openvrml/frustum.cpp
	openvrml/node_impl_util.cpp
	openvrml/local/conf.cpp
	openvrml/local/error.cpp
	openvrml/local/uri.cpp
	openvrml/local/xml_reader.cpp
	openvrml/local/parse_vrml.cpp
	openvrml/local/component.cpp
	openvrml/local/proto.cpp
	openvrml/local/externproto.cpp
	openvrml/local/node_metatype_registry_impl.cpp
)

source_group("Source Files" FILES ${openvrml_SOURCES})
source_group("Header Files" FILES ${openvrml_HEADERS} ${openvrml_PRIVATE_HEADERS})

set(openvrml_LIBRARIES
	openvrml-dl
	${LIBXML2_LIBRARIES}
)
set(openvrml_DEFINES
	-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS -DBOOST_MPL_LIMIT_VECTOR_SIZE=30
	-DBOOST_NO_RVALUE_REFERENCES -DBOOST_FILESYSTEM_VERSION=3
	-DPACKAGE_VERSION=\"${PACKAGE_VERSION}\"
	-DOPENVRML_PKGDATADIR_=\"${OPENVRML_PKGDATADIR_}\"
	-DOPENVRML_PKGLIBDIR_=\"${OPENVRML_PKGLIBDIR_}\"
	${LIBXML2_DEFINITIONS}
)
set(openvrml_INCLUDE_DIRS
	../local/libopenvrml-dl
	${LIBXML2_INCLUDE_DIR}
	${BOOST_INCLUDE_DIRS}
)

if(MSVC)
	list(APPEND openvrml_HEADERS openvrml-config-win32.h)
	list(APPEND openvrml_SOURCES openvrml.rc)
	list(APPEND openvrml_LIBRARIES shlwapi XmlLite)
	list(APPEND openvrml_DEFINES
		-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_SCL_DECURE_NO_WARNINGS
		-DOPENVRML_BUILD_DLL
	)
else()
	configure_file(openvrml-config.h.in openvrml-config.h)
endif()

add_library(openvrml SHARED ${openvrml_SOURCES} ${openvrml_HEADERS}
	${openvrml_PRIVATE_HEADERS})

target_link_libraries(openvrml ${openvrml_LIBRARIES})
target_compile_definitions(openvrml PUBLIC ${openvrml_DEFINES})
target_include_directories(openvrml PUBLIC ${openvrml_INCLUDE_DIRS})

install(TARGETS openvrml
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
)
install(FILES ${openvrml_HEADERS}
	DESTINATION "include/openvrml"
)
if(NOT MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/openvrml-config.h
		DESTINATION "include/openvrml"
	)
endif()
