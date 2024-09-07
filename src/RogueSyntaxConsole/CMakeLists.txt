



cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

set( externHeaders 
    ${PARENT_DIR}/include/RogueSyntax/RogueSyntaxCore.h
    ${PARENT_DIR}/include/RogueSyntax/Lexer.h
    ${PARENT_DIR}/include/RogueSyntax/Token.h
    ${PARENT_DIR}/include/RogueSyntax/StandardLib.h
    ${PARENT_DIR}/include/RogueSyntax/IObject.h
    ${PARENT_DIR}/include/RogueSyntax/Evaluator.h
    ${PARENT_DIR}/include/RogueSyntax/Environment.h
)

set( consoleHeaders 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxREPL.h
 )

set( consoleSource 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxREPL.cpp
)

add_executable(RogueSyntaxConsole ${testSource} ${externHeaders} ${consoleHeaders})
target_link_libraries(RogueSyntaxConsole PUBLIC RogueSyntax)

add_dependencies(RogueSyntaxConsole RogueSyntax)

target_include_directories( RogueSyntaxConsole	PUBLIC ${PARENT_DIR}/RogueSyntax/include/RogueSyntax )

target_sources( RogueSyntaxConsole PRIVATE 
    ${consoleSource}
 )

 install(TARGETS RogueSyntaxConsole
        RUNTIME           
          COMPONENT Runtime
          DESTINATION test
)
install(FILES $<TARGET_RUNTIME_DLLS:RogueSyntaxConsole> DESTINATION test)