cmake_minimum_required (VERSION 3.15)
project (cpp_reflection)

if(MSVC)
  add_definitions(/MP)
  #add_definitions(/incremental)
  #add_definitions(/Debug:fastlink)
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(GLOB_RECURSE SRCFILES 
	*.cpp;
	*.c;
	*.cxx;
	*.h;
	*.hpp
)

add_executable(cpp_reflection ${SRCFILES} )

set_target_properties(
	cpp_reflection PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/../build/release"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/../build/release"
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/../build/release"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/../build/debug"
)

target_include_directories(cpp_reflection PRIVATE 
	
)
target_link_directories(cpp_reflection PRIVATE 
	
)
target_link_libraries(cpp_reflection 
	
)

target_compile_definitions(cpp_reflection PRIVATE 
	_CRT_SECURE_NO_WARNINGS
	NOMINMAX
)

