set(BUILD_SHARED_LIBS FALSE)

add_subdirectory(SFML)

add_library(seriallib seriallib/lib/serialib.cpp)
target_include_directories(seriallib PUBLIC seriallib/lib)
# set_target_properties(seriallib PROPERTIES CXX_STANDARD 17)

set(LIBTORCH_ZIPS
	"libtorch-cxx11-abi-shared-with-deps-1.11.0+cpu.zip"
	"libtorch-cxx11-abi-shared-with-deps-1.11.0+cu113.zip"
	"libtorch-win-shared-with-deps-1.11.0+cpu.zip"
	"libtorch-win-shared-with-deps-1.11.0+cu113.zip"
	"libtorch-win-shared-with-deps-debug-1.11.0+cpu.zip"
	"libtorch-win-shared-with-deps-debug-1.11.0+cu113.zip"
)

set(cu113 FALSE)


foreach(zip ${LIBTORCH_ZIPS})
	if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/libtorch/${zip})
		if(cu113)
			string(REPLACE "+" "%2B" URL "https://download.pytorch.org/libtorch/cu113/${zip}")
			message(STATUS "Downloading ${URL} to ${CMAKE_SOURCE_DIR}/external/libtorch/${zip}. This may take a while.")
			file(DOWNLOAD ${URL} ${PROJECT_SOURCE_DIR}/external/libtorch/${zip})
			set(cu113 FALSE)
		else()
			string(REPLACE "+" "%2B" URL "https://download.pytorch.org/libtorch/cpu/${zip}")
			message(STATUS "Downloading ${URL} to ${CMAKE_SOURCE_DIR}/external/libtorch/${zip}. This may take a while.")
			file(DOWNLOAD ${URL} ${PROJECT_SOURCE_DIR}/external/libtorch/${zip})
			set(cu113 TRUE)
		endif()
	endif()
endforeach()

set(LIBTORCH_DIRS
	"linux_cxx11_cpu"
	"linux_cxx11_cuda11.3"
	"win_cpu"
	"win_cuda11.3"
	"win_cpu_dbg"
	"win_cuda11.3_dbg"
)

list(LENGTH LIBTORCH_ZIPS len1)
math(EXPR len2 "${len1} - 1")

foreach(idx RANGE ${len2})
  list(GET LIBTORCH_ZIPS ${idx} zip)
  list(GET LIBTORCH_DIRS ${idx} opdir)

	if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/libtorch/${opdir}")
		message(STATUS "Extracting ${zip}. This may take a while.")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_SOURCE_DIR}/external/libtorch/${zip}"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/external/libtorch"
		)
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/external/libtorch/libtorch" "${PROJECT_SOURCE_DIR}/external/libtorch/${opdir}"
		)
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E remove_directory "${PROJECT_SOURCE_DIR}/external/libtorch/libtorch"
		)
	endif()

endforeach()
