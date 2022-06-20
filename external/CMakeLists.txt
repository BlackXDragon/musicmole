set(BUILD_SHARED_LIBS FALSE)

add_subdirectory(SFML)

add_library(seriallib seriallib/lib/serialib.cpp)
target_include_directories(seriallib PUBLIC seriallib/lib)
# set_target_properties(seriallib PROPERTIES CXX_STANDARD 17)

add_subdirectory(dlib)