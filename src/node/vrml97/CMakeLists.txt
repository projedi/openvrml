set(vrml97_HEADERS
)
set(vrml97_PRIVATE_HEADERS
	abstract_indexed_set.h
	abstract_light.h
	abstract_texture.h
	anchor.h
	appearance.h
	audio_clip.h
	background.h
	billboard.h
	box.h
	cad_assembly.h
	cad_layer.h
	collision.h
	color.h
	color_interpolator.h
	cone.h
	coordinate.h
	coordinate_interpolator.h
	cylinder.h
	cylinder_sensor.h
	directional_light.h
	elevation_grid.h
	extrusion.h
	fog.h
	font_style.h
	group.h
	grouping_node_base.h
	image_stream_listener.h
	image_texture.h
	indexed_face_set.h
	indexed_line_set.h
	inline.h
	lod.h
	material.h
	movie_texture.h
	navigation_info.h
	normal.h
	normal_interpolator.h
	orientation_interpolator.h
	pixel_texture.h
	plane_sensor.h
	point_light.h
	point_set.h
	position_interpolator.h
	proximity_sensor.h
	scalar_interpolator.h
	shape.h
	sound.h
	sphere.h
	sphere_sensor.h
	spot_light.h
	switch.h
	text.h
	texture_coordinate.h
	texture_transform.h
	time_sensor.h
	touch_sensor.h
	transform.h
	viewpoint.h
	visibility_sensor.h
	world_info.h
)
set(vrml97_SOURCES
	anchor.cpp
	appearance.cpp
	audio_clip.cpp
	background.cpp
	billboard.cpp
	box.cpp
	cad_assembly.cpp
	cad_layer.cpp
	collision.cpp
	color.cpp
	color_interpolator.cpp
	cone.cpp
	coordinate.cpp
	coordinate_interpolator.cpp
	cylinder.cpp
	cylinder_sensor.cpp
	directional_light.cpp
	elevation_grid.cpp
	extrusion.cpp
	fog.cpp
	font_style.cpp
	group.cpp
	image_stream_listener.cpp
	image_texture.cpp
	indexed_face_set.cpp
	indexed_line_set.cpp
	inline.cpp
	lod.cpp
	material.cpp
	movie_texture.cpp
	navigation_info.cpp
	normal.cpp
	normal_interpolator.cpp
	orientation_interpolator.cpp
	pixel_texture.cpp
	plane_sensor.cpp
	point_light.cpp
	point_set.cpp
	position_interpolator.cpp
	proximity_sensor.cpp
	register_node_metatypes.cpp
	scalar_interpolator.cpp
	shape.cpp
	sound.cpp
	sphere.cpp
	sphere_sensor.cpp
	spot_light.cpp
	switch.cpp
	text.cpp
	texture_coordinate.cpp
	texture_transform.cpp
	time_sensor.cpp
	touch_sensor.cpp
	transform.cpp
	viewpoint.cpp
	visibility_sensor.cpp
	world_info.cpp
)

source_group("Source Files" FILES ${vrml97_SOURCES})
source_group("Header Files" FILES ${vrml97_HEADERS} ${vrml97_PRIVATE_HEADERS})

set(vrml97_LIBRARIES
	${ZLIB_LIBRARIES}
)
set(vrml97_DEFINES
	-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
	-DBOOST_MPL_LIMIT_VECTOR_SIZE=30
)
set(vrml97_INCLUDE_DIRS
	../../libopenvrml
	${CMAKE_CURRENT_BINARY_DIR}/../../libopenvrml
	${BOOST_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
)

if(BUILD_WITH_PNG)
	list(APPEND vrml97_LIBRARIES ${PNG_LIBRARIES})
	list(APPEND vrml97_DEFINES ${PNG_DEFINITIONS} OPENVRML_ENABLE_PNG_TEXTURES)
	list(APPEND vrml97_INCLUDE_DIRS ${PNG_INCLUDE_DIRS})
endif()

if(BUILD_WITH_JPEG)
	list(APPEND vrml97_LIBRARIES ${JPEG_LIBRARIES})
	list(APPEND vrml97_DEFINES OPENVRML_ENABLE_JPEG_TEXTURES)
	list(APPEND vrml97_INCLUDE_DIRS ${JPEG_INCLUDE_DIRS})
endif()

if(BUILD_WITH_FREETYPE)
	list(APPEND vrml97_LIBRARIES ${FREETYPE_LIBRARIES})
	list(APPEND vrml97_DEFINES OPENVRML_ENABLE_RENDER_TEXT_NODES)
	list(APPEND vrml97_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
endif()

if(MSVC)
	list(APPEND openvrml_DEFINES
		-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_SCL_DECURE_NO_WARNINGS
		-DOPENVRML_USE_DLL
	)
endif()

add_library(vrml97 MODULE ${vrml97_SOURCES} ${vrml97_HEADERS} ${vrml97_PRIVATE_HEADERS})

target_link_libraries(vrml97 ${vrml97_LIBRARIES})
target_compile_definitions(vrml97 PUBLIC ${vrml97_DEFINES})
target_include_directories(vrml97 PUBLIC ${vrml97_INCLUDE_DIRS})

install(TARGETS vrml97
	RUNTIME DESTINATION "bin/openvrml/node/vrml97"
	LIBRARY DESTINATION "lib/openvrml/node/vrml97"
)
