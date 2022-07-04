add_executable(PeriodicTickerTest PeriodicTickerTest.cpp)

add_executable(ConsoleGameTest ConsoleGameTest.cpp)

add_executable(NumericalControllerTest NumericalControllerTest.cpp)
target_link_libraries(NumericalControllerTest sfml-window sfml-graphics sfml-system)
target_include_directories(NumericalControllerTest PRIVATE external/SFML/include)
add_custom_command(TARGET NumericalControllerTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/Roboto-Italic.ttf $<TARGET_FILE_DIR:NumericalControllerTest>)

add_executable(NumericalControllerGameTest NumericalControllerGameTest.cpp)
target_link_libraries(NumericalControllerGameTest sfml-window sfml-graphics sfml-system)
target_include_directories(NumericalControllerGameTest PRIVATE external/SFML/include)
add_custom_command(TARGET NumericalControllerGameTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/Roboto-Italic.ttf $<TARGET_FILE_DIR:NumericalControllerGameTest>)