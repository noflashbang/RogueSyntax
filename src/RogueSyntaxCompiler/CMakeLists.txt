
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
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/default.mk"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/bubble.mk"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/factorial.mk"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/example/fibonacci.mk"
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
    ${PARENT_DIR}/include/RogueSyntax/OpCode.h
)

set( compilerHeaders 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxCompiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clayex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Structs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Cursor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_LineNumber.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Menu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Splitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_ScrollBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Textbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Listbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_TextArea.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Button.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Form.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_OpenForm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_SaveForm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputCmd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InfoForm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OutputForm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorForm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UIConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clay_renderer_raylib.h
 )

set( compilerSource 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clayex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Cursor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_LineNumber.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Splitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_ScrollBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Textbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Listbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_TextArea.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_Form.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_OpenForm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI_Layout_SaveForm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InfoForm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OutputForm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorForm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxCompiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clay_renderer_raylib.cpp
)
if(CMAKE_TIDY_ENABLED)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,modernize-*,bugprone-*,cppcoreguidelines-*,performance-*)
endif()

add_executable(RogueSyntaxCompiler ${testSource} ${externHeaders} ${compilerHeaders})
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