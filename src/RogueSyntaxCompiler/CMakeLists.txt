



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
    ${PARENT_DIR}/include/RogueSyntax/Opcode.h
    ${PARENT_DIR}/include/RogueSyntax/Compiler.h
)

set( compilerHeaders 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxCompiler.h
 )

set( compilerSource 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RogueSyntaxCompiler.cpp
)
if(CMAKE_TIDY_ENABLED)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,modernize-*,bugprone-*,cppcoreguidelines-*,performance-*)
endif()

add_executable(RogueSyntaxCompiler ${testSource} ${externHeaders} ${compilerHeaders})
target_link_libraries(RogueSyntaxCompiler PUBLIC RogueSyntax)

add_dependencies(RogueSyntaxCompiler RogueSyntax)

target_include_directories( RogueSyntaxCompiler	PUBLIC ${PARENT_DIR}/RogueSyntax/include/RogueSyntax )

target_sources( RogueSyntaxCompiler PRIVATE 
    ${compilerSource}
 )

 install(TARGETS RogueSyntaxCompiler
        RUNTIME           
          COMPONENT Runtime
          DESTINATION test
)
install(FILES $<TARGET_RUNTIME_DLLS:RogueSyntaxCompiler> DESTINATION test)