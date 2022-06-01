set(BUILD_SHARED_LIBS FALSE)

add_subdirectory(SFML)

set(SFGUI_BUILD_SHARED_LIBS FALSE)

set(SFML_STATIC_LIBRARIES ON)

add_subdirectory(sfgui)

add_library(seriallib seriallib/lib/serialib.cpp)
target_include_directories(seriallib PUBLIC seriallib/lib)
# set_target_properties(seriallib PROPERTIES CXX_STANDARD 17)
