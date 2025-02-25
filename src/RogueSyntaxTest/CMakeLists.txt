

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2 # or a later release
)

FetchContent_MakeAvailable(Catch2)

include(CTest)
include(Catch)
enable_testing()

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

set( externHeaders 
    ${PARENT_DIR}/include/RogueSyntax/RogueSyntax.h
    ${PARENT_DIR}/include/RogueSyntax/RogueSyntaxCore.h
    ${PARENT_DIR}/include/RogueSyntax/Lexer.h
    ${PARENT_DIR}/include/RogueSyntax/Token.h
    ${PARENT_DIR}/include/RogueSyntax/AstNode.h
    ${PARENT_DIR}/include/RogueSyntax/Parser.h
    ${PARENT_DIR}/include/RogueSyntax/StandardLib.h
    ${PARENT_DIR}/include/RogueSyntax/IObject.h
    ${PARENT_DIR}/include/RogueSyntax/TypeCoercer.h
    ${PARENT_DIR}/include/RogueSyntax/Evaluator.h
    ${PARENT_DIR}/include/RogueSyntax/Environment.h
    ${PARENT_DIR}/include/RogueSyntax/Builtin.h
    ${PARENT_DIR}/include/RogueSyntax/OpCode.h
    ${PARENT_DIR}/include/RogueSyntax/VirtualMachine.h

)

set( testHeaders
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CompilerTestHelpers.h
 )

set( testSource 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LexerTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ParserTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EvalTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CompilerTestHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CompilerTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VirtualMachineTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp

)

#add_compile_definitions(DO_BENCHMARK)
#add_compile_definitions(CATCH_CONFIG_CONSOLE_WIDTH=200)

add_executable(RogueSyntaxTest ${testSource} ${externHeaders} ${testHeaders})
target_link_libraries(RogueSyntaxTest PRIVATE Catch2::Catch2WithMain)
target_link_libraries(RogueSyntaxTest PUBLIC RogueSyntax)

add_dependencies(RogueSyntaxTest RogueSyntax)

target_include_directories( RogueSyntaxTest	PUBLIC ${PARENT_DIR}/RogueSyntax/include/RogueSyntax )

target_sources( RogueSyntaxTest PRIVATE 
    ${testSource}
 )

 catch_discover_tests(RogueSyntaxTest)

 install(TARGETS RogueSyntaxTest
        RUNTIME           
          COMPONENT Runtime
          DESTINATION RogueSyntaxTest
)

install(FILES $<TARGET_RUNTIME_DLLS:RogueSyntaxTest> DESTINATION ${CMAKE_INSTALL_PREFIX}/RogueSyntaxTest)