
# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib)

set(CLAY_INCLUDE_ALL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLAY_INCLUDE_DEMOS OFF CACHE BOOL "" FORCE)
set(CLAY_INCLUDE_CPP_EXAMPLE OFF CACHE BOOL "" FORCE)
set(CLAY_INCLUDE_RAYLIB_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLAY_INCLUDE_SDL2_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLAY_INCLUDE_SDL3_EXAMPLES OFF CACHE BOOL "" FORCE)

#check to see when the repo gets updated to include this pr: https://github.com/nicbarker/clay/pull/216
#FetchContent_Declare(
#  Clay
#  GIT_REPOSITORY https://github.com/nicbarker/clay.git
#  GIT_TAG        main
#)
#
#FetchContent_MakeAvailable(Clay)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

set(FONT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/anonymous_pro_bold.ttf")
set(EXAMPLE_FILE 
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/bubble.mk"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/factorial.mk"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/fibonacci.mk"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/heap.mk"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/isPrime.mk"
)

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
    ${PARENT_DIR}/include/RogueSyntax/Opcode.h
)

set( compilerHeaders 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxCompiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clayex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Splitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputCmd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputForm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UIConfig.h


    ${CMAKE_CURRENT_SOURCE_DIR}/src/clay_renderer_raylib.h
 )

set( compilerSource 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clayex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Splitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputForm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxCompiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clay_renderer_raylib.cpp
)
if(CMAKE_TIDY_ENABLED)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,modernize-*,bugprone-*,cppcoreguidelines-*,performance-*)
endif()

add_executable(RogueSyntaxCompiler ${testSource} ${externHeaders} ${compilerHeaders} "src/UI_Layout_Splitter.cpp")
target_link_libraries(RogueSyntaxCompiler PRIVATE raylib)
target_link_libraries(RogueSyntaxCompiler PUBLIC RogueSyntax)

add_dependencies(RogueSyntaxCompiler RogueSyntax)

target_include_directories( RogueSyntaxCompiler	PUBLIC ${PARENT_DIR}/RogueSyntax/include/RogueSyntax )

target_sources( RogueSyntaxCompiler PRIVATE 
    ${compilerSource}
 )

 install(TARGETS RogueSyntaxCompiler
        RUNTIME           
          COMPONENT Runtime
          DESTINATION ${CMAKE_INSTALL_PREFIX}/RogueSyntaxCompiler
)
install(FILES $<TARGET_RUNTIME_DLLS:RogueSyntaxCompiler> DESTINATION ${CMAKE_INSTALL_PREFIX}/RogueSyntaxCompiler)
install(FILES ${FONT_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/RogueSyntaxCompiler/resources)
install(FILES ${EXAMPLE_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/RogueSyntaxCompiler/example)