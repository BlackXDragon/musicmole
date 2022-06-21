add_executable(serialtest serialtest.cpp)
target_link_libraries(serialtest seriallib)

add_executable(availableSerialTest availableSerialTest.cpp)
target_include_directories(availableSerialTest PUBLIC ${PROJECT_SOURCE_DIR}/src/serial)

makeDlibExec(dlibTest dlibtest.cpp)

makeDlibExec(gestureModelTest gestureModeltest.cpp)
add_custom_command(TARGET gestureModelTest COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/test/gestureData $<TARGET_FILE_DIR:gestureModelTest>/gestureData)

makeDlibExec(liveGestureTest liveGestureTest.cpp)
target_link_libraries(liveGestureTest seriallib)