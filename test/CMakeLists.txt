add_executable(NumericalControllerTest NumericalControllerTest.cpp)
target_link_libraries(NumericalControllerTest sfml-window sfml-graphics sfml-system)
target_include_directories(NumericalControllerTest PRIVATE external/SFML/include)
add_custom_command(TARGET NumericalControllerTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/Roboto-Italic.ttf $<TARGET_FILE_DIR:NumericalControllerTest>)