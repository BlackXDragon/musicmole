cmake_minimum_required(VERSION 3.16)

project(SFMLTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external)

if(WIN32)
	option(USEGPU "Use GPU" ON)
elseif(UNIX)
	option(USEGPU "Use GPU" OFF)
endif()

message(STATUS "USEGPU: ${USEGPU}")

if(WIN32)
	if(USEGPU)
		if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
			message(FATAL "Torch debug build is currently not supported on Windows")
			set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/external/libtorch/win_cuda11.3_dbg")
		else()
			set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/external/libtorch/win_cuda11.3")
		endif()
	else()
		if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
			message(FATAL "Torch debug build is currently not supported on Windows")
			set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/external/libtorch/win_cpu_dbg")
		else()
			set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/external/libtorch/win_cpu")
		endif()
	endif()
elseif(UNIX)
	if(USEGPU)
		message(FATAL "Sorry, CUDA support is not yet available on UNIX")
		set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/external/libtorch/linux_cxx11_cuda11.3")
	else()
		set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/external/libtorch/linux_cxx11_cpu")
	endif()
endif()

message(STATUS "Using Torch prefix: ${CMAKE_PREFIX_PATH}")

macro(makeTorchExecutable target executables)
	find_package(Torch REQUIRED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

	add_executable(${target} ${executables})
	target_link_libraries(${target} "${TORCH_LIBRARIES}")
	set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)

	# The following code block is suggested to be used on Windows.
	# According to https://github.com/pytorch/pytorch/issues/25457,
	# the DLLs need to be copied to avoid memory errors.
	if (MSVC)
	file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
	add_custom_command(TARGET ${target}
						POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy_if_different
						${TORCH_DLLS}
						$<TARGET_FILE_DIR:${target}>)
	endif (MSVC)
endmacro()

add_subdirectory(test)

add_executable(SFMLTest main.cpp)
target_link_libraries(SFMLTest sfml-window sfml-graphics sfml-system)
target_include_directories(SFMLTest PRIVATE external/SFML/include)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-window> $<TARGET_FILE_DIR:SFMLTest>)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-graphics> $<TARGET_FILE_DIR:SFMLTest>)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-system> $<TARGET_FILE_DIR:SFMLTest>)