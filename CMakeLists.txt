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

add_executable(SFGUITest main.cpp)
target_link_libraries(SFGUITest SFGUI sfml-window sfml-graphics sfml-system)
target_include_directories(SFGUITest PRIVATE external/SFML/include)
# target_include_directories(SFMLTest PRIVATE external/SFML/include)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-window> $<TARGET_FILE_DIR:SFMLTest>)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-graphics> $<TARGET_FILE_DIR:SFMLTest>)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-system> $<TARGET_FILE_DIR:SFMLTest>)