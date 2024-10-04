
add_compile_definitions(ROGUESYNTAX_EXPORTS)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

set( externHeaders 
    ${PARENT_DIR}/include/RogueSyntax/RogueSyntaxCore.h
    ${PARENT_DIR}/include/RogueSyntax/Identifiable.h
    ${PARENT_DIR}/include/RogueSyntax/Token.h
    ${PARENT_DIR}/include/RogueSyntax/Lexer.h
    ${PARENT_DIR}/include/RogueSyntax/AstNode.h
    ${PARENT_DIR}/include/RogueSyntax/AstNodeStore.h
    ${PARENT_DIR}/include/RogueSyntax/Parser.h
    ${PARENT_DIR}/include/RogueSyntax/StandardLib.h
    ${PARENT_DIR}/include/RogueSyntax/IObject.h
    ${PARENT_DIR}/include/RogueSyntax/ObjectStore.h
    ${PARENT_DIR}/include/RogueSyntax/TypeCoercer.h
    ${PARENT_DIR}/include/RogueSyntax/Evaluator.h
    ${PARENT_DIR}/include/RogueSyntax/Environment.h
    ${PARENT_DIR}/include/RogueSyntax/Builtin.h
    ${PARENT_DIR}/include/RogueSyntax/OpCode.h
    ${PARENT_DIR}/include/RogueSyntax/Compiler.h
    ${PARENT_DIR}/include/RogueSyntax/VirtualMachine.h


)

set( libHeaders 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StackEvaluator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RecursiveEvaluator.h
 )

set( libSource
 "src/Token.cpp"
 "src/Lexer.cpp"
 "src/AstNode.cpp"
 "src/AstNodeStore.cpp"
 "src/Parser.cpp"  
 "src/IObject.cpp"
 "src/ObjectStore.cpp"
 "src/TypeCoercer.cpp"
 "src/Evaluator.cpp"
 "src/Environment.cpp"
 "src/Builtin.cpp"
 "src/StackEvaluator.cpp"
 "src/RecursiveEvaluator.cpp"
 "src/OpCode.cpp"
 "src/Compiler.cpp"
 "src/VirtualMachine.cpp"
)

if(CMAKE_TIDY_ENABLED)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,modernize-*,bugprone-*,cppcoreguidelines-*,performance-* --extra-arg=/EHsc)
endif()

add_library (RogueSyntax STATIC ${libSource} ${libHeaders} ${externHeaders}) 
target_include_directories( RogueSyntax PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/RogueSyntax )
target_include_directories( RogueSyntax PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src )

target_precompile_headers( RogueSyntax PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h )

target_sources( RogueSyntax PRIVATE ${libSource} ${libHeaders} )

target_sources( RogueSyntax PUBLIC FILE_SET HEADERS BASE_DIRS ${PARENT_DIR}/include/RogueSyntax FILES ${externHeaders} )

install(TARGETS RogueSyntax
        RUNTIME DESTINATION bin          
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        FILE_SET HEADERS COMPONENT Development
)
