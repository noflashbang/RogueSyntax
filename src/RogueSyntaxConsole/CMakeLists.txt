



cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

set( externHeaders 
    ${PARENT_DIR}/include/RogueSyntax/RogueSyntaxCore.h
    ${PARENT_DIR}/include/RogueSyntax/Lexer.h
    ${PARENT_DIR}/include/RogueSyntax/Token.h
    ${PARENT_DIR}/include/RogueSyntax/StandardLib.h
    ${PARENT_DIR}/include/RogueSyntax/IObject.h
    ${PARENT_DIR}/include/RogueSyntax/TypeCoercer.h
    ${PARENT_DIR}/include/RogueSyntax/Evaluator.h
    ${PARENT_DIR}/include/RogueSyntax/Environment.h
    ${PARENT_DIR}/include/RogueSyntax/Builtin.h
)

set( consoleHeaders 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxREPL.h
 )

set( consoleSource 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxREPL.cpp
)
if(CMAKE_TIDY_ENABLED)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,modernize-*,bugprone-*,cppcoreguidelines-*,performance-*)
endif()

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
          DESTINATION ${CMAKE_INSTALL_PREFIX}/RogueSyntaxCon
)
install(FILES $<TARGET_RUNTIME_DLLS:RogueSyntaxConsole> DESTINATION ${CMAKE_INSTALL_PREFIX}/RogueSyntaxCon)