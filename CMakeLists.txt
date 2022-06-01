cmake_minimum_required(VERSION 3.16)

project(musicmole CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external)
add_subdirectory(test)

add_executable(SFGUITest main.cpp)
target_link_libraries(SFGUITest SFGUI sfml-window sfml-graphics sfml-system)
target_include_directories(SFGUITest PRIVATE external/SFML/include)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-window> $<TARGET_FILE_DIR:SFMLTest>)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-graphics> $<TARGET_FILE_DIR:SFMLTest>)
# add_custom_command(TARGET SFMLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-system> $<TARGET_FILE_DIR:SFMLTest>)