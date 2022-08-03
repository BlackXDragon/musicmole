cmake_minimum_required(VERSION 3.16)

project(musicmole CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(external)

macro(addlibfftw target)
	if (WIN32)
		target_link_libraries(${target} ${fftwdir}/libfftw3-3.lib)
		add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/external/fftw/libfftw3-3.dll $<TARGET_FILE_DIR:${target}>)
		target_include_directories(${target} PUBLIC ${fftwdir})
	endif()
endmacro(addlibfftw)

macro(makeDlibExec target sources)
	add_executable(${target} ${sources})
	target_link_libraries(${target} dlib)
	if(MSVC)
		set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endmacro(makeDlibExec)

add_subdirectory(test)

makeDlibExec(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} SFGUI sfml-audio sfml-window sfml-graphics sfml-system seriallib)
target_include_directories(${PROJECT_NAME} PRIVATE external/SFML/include)
addlibfftw(${PROJECT_NAME})