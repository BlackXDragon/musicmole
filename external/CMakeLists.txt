set(BUILD_SHARED_LIBS FALSE)

add_subdirectory(SFML)
set_target_properties(sfml-audio PROPERTIES CXX_STANDARD 98 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

if (UNIX)
	message(FATAL_ERROR "Sorry, beat detection has not been developed for UNIX systems yet.")
endif()

if (WIN32)
	if (NOT EXISTS ${PROJECT_SOURCE_DIR}/external/fftw-3.3.5-dll32.zip)
	file(DOWNLOAD https://fftw.org/pub/fftw/fftw-3.3.5-dll32.zip ${PROJECT_SOURCE_DIR}/external/fftw-3.3.5-dll32.zip)
	file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/external/fftw")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xzf "${PROJECT_SOURCE_DIR}/external/fftw-3.3.5-dll32.zip"
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/external/fftw"
	)
	endif()
	execute_process(COMMAND lib.exe /machine:X86 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3-3.lib)
	execute_process(COMMAND lib.exe /machine:X86 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3f-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3f-3.lib)
	execute_process(COMMAND lib.exe /machine:X86 /def:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3l-3.def /out:${PROJECT_SOURCE_DIR}/external/fftw/libfftw3l-3.lib)
	set(fftwdir ${PROJECT_SOURCE_DIR}/external/fftw PARENT_SCOPE)
endif()