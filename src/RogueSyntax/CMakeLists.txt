
add_compile_definitions(ROGUESYNTAX_EXPORTS)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

set( externHeaders 
    ${PARENT_DIR}/include/RogueSyntax/RogueSyntaxApi.h
    ${PARENT_DIR}/include/RogueSyntax/RogueSyntaxCore.h
)

set( libHeaders 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StandardLib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.h
)

set( libSource
	${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxApi.cpp
)

add_library (RogueSyntax SHARED ${libSource} ${libHeaders} ${externHeaders}) 
target_include_directories( RogueSyntax PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/RogueSyntax )
target_include_directories( RogueSyntax PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src )

target_sources( RogueSyntax PRIVATE ${libSource} ${libHeaders} )

target_sources( RogueSyntax PUBLIC FILE_SET HEADERS BASE_DIRS ${PARENT_DIR}/include/RogueSyntax FILES ${externHeaders} )

install(TARGETS RogueSyntax
        RUNTIME DESTINATION bin          
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        FILE_SET HEADERS COMPONENT Development
)
