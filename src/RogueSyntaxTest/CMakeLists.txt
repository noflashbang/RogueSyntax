

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2 # or a later release
)

FetchContent_MakeAvailable(Catch2)



cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

set( externHeaders 
	${PARENT_DIR}/include/RogueSyntax/RogueSyntaxApi.h
    ${PARENT_DIR}/include/RogueSyntax/RogueSyntaxCore.h
)

set( testSource 
   ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(RogueSyntaxTest ${testSource} ${externHeaders})
target_link_libraries(RogueSyntaxTest PRIVATE Catch2::Catch2WithMain)
target_link_libraries(RogueSyntaxTest PUBLIC RogueSyntax)

add_dependencies(RogueSyntaxTest RogueSyntax)

target_include_directories( RogueSyntaxTest	PUBLIC ${PARENT_DIR}/RogueSyntax/include/RogueSyntax )

target_sources( RogueSyntaxTest PRIVATE 
    ${testSource}
 )

 install(TARGETS RogueSyntaxTest
        RUNTIME           
          COMPONENT Runtime
          DESTINATION test
)
install(FILES $<TARGET_RUNTIME_DLLS:RogueSyntaxTest> DESTINATION test)