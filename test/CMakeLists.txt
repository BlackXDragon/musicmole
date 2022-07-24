add_executable(PeriodicTickerTest PeriodicTickerTest.cpp)

add_executable(ConsoleGameTest ConsoleGameTest.cpp)

add_executable(serialtest serialtest.cpp)
target_link_libraries(serialtest seriallib)

add_executable(availableSerialTest availableSerialTest.cpp)
target_include_directories(availableSerialTest PUBLIC ${PROJECT_SOURCE_DIR}/src/serial)

add_executable(NumericalControllerTest NumericalControllerTest.cpp)
target_link_libraries(NumericalControllerTest sfml-window sfml-graphics sfml-system)
target_include_directories(NumericalControllerTest PRIVATE external/SFML/include)
add_custom_command(TARGET NumericalControllerTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/Roboto-Italic.ttf $<TARGET_FILE_DIR:NumericalControllerTest>)

add_executable(NumericalControllerGameTest NumericalControllerGameTest.cpp)
target_link_libraries(NumericalControllerGameTest sfml-window sfml-graphics sfml-system)
target_include_directories(NumericalControllerGameTest PRIVATE external/SFML/include)
add_custom_command(TARGET NumericalControllerGameTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/Roboto-Italic.ttf $<TARGET_FILE_DIR:NumericalControllerGameTest>)

makeDlibExec(MenuTest menuTest.cpp)
target_link_libraries(MenuTest SFGUI sfml-audio sfml-window sfml-graphics sfml-system seriallib)
target_include_directories(MenuTest PRIVATE external/SFML/include)
addlibfftw(MenuTest)

add_executable(libfftwTest libfftwTest.cpp)
addlibfftw(libfftwTest)

set(MUSIC_FILE "The Chainsmokers & Coldplay - Something Just Like This (Lyric).mp3")

add_executable(beatDetectionTest beatDetectionTest.cpp)
target_compile_definitions(beatDetectionTest PUBLIC MUSIC_FILE="${MUSIC_FILE}")
addlibfftw(beatDetectionTest)
target_link_libraries(beatDetectionTest sfml-audio sfml-window sfml-graphics sfml-system)
target_include_directories(beatDetectionTest PRIVATE external/SFML/include)
add_custom_command(TARGET beatDetectionTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/${MUSIC_FILE} $<TARGET_FILE_DIR:beatDetectionTest>)
if (WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		add_custom_command(TARGET beatDetectionTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/external/SFML/extlibs/bin/x64/openal32.dll $<TARGET_FILE_DIR:beatDetectionTest>)
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		add_custom_command(TARGET beatDetectionTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/external/SFML/extlibs/bin/x86/openal32.dll $<TARGET_FILE_DIR:beatDetectionTest>)
	endif()
endif()

add_executable(musicalTickerTest musicalTickerTest.cpp)
target_compile_definitions(musicalTickerTest PUBLIC MUSIC_FILE="${MUSIC_FILE}")
addlibfftw(musicalTickerTest)
target_link_libraries(musicalTickerTest sfml-audio sfml-window sfml-graphics sfml-system)
target_include_directories(musicalTickerTest PRIVATE external/SFML/include)
add_custom_command(TARGET musicalTickerTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/${MUSIC_FILE} $<TARGET_FILE_DIR:beatDetectionTest>)
if (WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		add_custom_command(TARGET musicalTickerTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/external/SFML/extlibs/bin/x64/openal32.dll $<TARGET_FILE_DIR:beatDetectionTest>)
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		add_custom_command(TARGET musicalTickerTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/external/SFML/extlibs/bin/x86/openal32.dll $<TARGET_FILE_DIR:beatDetectionTest>)
	endif()
endif()

makeDlibExec(dlibTest dlibtest.cpp)

makeDlibExec(gestureModelTest gestureModeltest.cpp)
add_custom_command(TARGET gestureModelTest COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/test/gestureData $<TARGET_FILE_DIR:gestureModelTest>/gestureData)

makeDlibExec(liveGestureTest liveGestureTest.cpp)
target_link_libraries(liveGestureTest seriallib)

makeDlibExec(GestureControllerTest GestureControllerTest.cpp)
target_link_libraries(GestureControllerTest sfml-window sfml-graphics sfml-system seriallib)
target_include_directories(GestureControllerTest PRIVATE external/SFML/include)
add_custom_command(TARGET GestureControllerTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/Roboto-Italic.ttf $<TARGET_FILE_DIR:GestureControllerTest>)

makeDlibExec(GameWithMenuTest GameWithMenuTest.cpp)
target_link_libraries(GameWithMenuTest SFGUI sfml-audio sfml-window sfml-graphics sfml-system seriallib)
target_include_directories(GameWithMenuTest PRIVATE external/SFML/include)
addlibfftw(GameWithMenuTest)
add_custom_command(TARGET GameWithMenuTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/test/Roboto-Italic.ttf $<TARGET_FILE_DIR:GameWithMenuTest>)

add_executable(GameScreenTest gameScreenTest.cpp)
target_link_libraries(GameScreenTest sfml-window sfml-graphics sfml-system)
target_include_directories(GameScreenTest PRIVATE external/SFML/include)
