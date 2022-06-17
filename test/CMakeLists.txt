add_executable(serialtest serialtest.cpp)
target_link_libraries(serialtest seriallib)

add_executable(availableSerialTest availableSerialTest.cpp)
target_include_directories(availableSerialTest PUBLIC ${PROJECT_SOURCE_DIR}/src/serial)

makeTorchExecutable(libTorchTest libtorchtest.cpp)

add_custom_command(TARGET libTorchTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/test/MNISTdata $<TARGET_FILE_DIR:libTorchTest>/MNISTdata)

makeTorchExecutable(gestureModelTest gestureModeltest.cpp)

add_custom_command(TARGET gestureModelTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/test/gestureData $<TARGET_FILE_DIR:gestureModelTest>/gestureData)
