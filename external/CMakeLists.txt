set(BUILD_SHARED_LIBS FALSE)

add_subdirectory(SFML)
set_target_properties(sfml-audio PROPERTIES CXX_STANDARD 98 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

set(SFGUI_BUILD_SHARED_LIBS FALSE)

set(SFML_STATIC_LIBRARIES ON)

add_subdirectory(sfgui)

add_library(seriallib seriallib/lib/serialib.cpp)
target_include_directories(seriallib PUBLIC seriallib/lib)
# set_target_properties(seriallib PROPERTIES CXX_STANDARD 17)

if (UNIX)
	message(FATAL_ERROR "Sorry, beat detection has not been developed for UNIX systems yet.")
endif()

if (WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		message(STATUS "FFTW 64 bit")
		file(DOWNLOAD https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip ${PROJECT_SOURCE_DIR}/external/fftw-3.3.5-dll64.zip)
		file(REMOVE_RECURSE "${PROJECT_SOURCE_DIR}/external/fftw")
		file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/external/fftw")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf "${PROJECT_SOURCE_DIR}/external/fftw-3.3.5-dll64.zip"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/external/fftw"
		)
		execute_process(COMMAND lib.exe /machine:X64 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3-3.lib)
		execute_process(COMMAND lib.exe /machine:X64 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3f-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3f-3.lib)
		execute_process(COMMAND lib.exe /machine:X64 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3l-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3l-3.lib)
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		message(STATUS "FFTW 32 bit")
		file(DOWNLOAD https://fftw.org/pub/fftw/fftw-3.3.5-dll32.zip ${PROJECT_SOURCE_DIR}/external/fftw-3.3.5-dll32.zip)
		file(REMOVE_RECURSE "${PROJECT_SOURCE_DIR}/external/fftw")
		file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/external/fftw")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf "${PROJECT_SOURCE_DIR}/external/fftw-3.3.5-dll32.zip"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/external/fftw"
		)
		execute_process(COMMAND lib.exe /machine:X86 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3-3.lib)
		execute_process(COMMAND lib.exe /machine:X86 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3f-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3f-3.lib)
		execute_process(COMMAND lib.exe /machine:X86 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3l-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3l-3.lib)
	endif()
	set(fftwdir ${PROJECT_SOURCE_DIR}/external/fftw PARENT_SCOPE)
endif()

add_subdirectory(dlib)
