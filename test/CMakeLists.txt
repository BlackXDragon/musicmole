add_executable(GameScreenTest gameScreenTest.cpp)
target_link_libraries(GameScreenTest sfml-window sfml-graphics sfml-system)
target_include_directories(GameScreenTest PRIVATE external/SFML/include)