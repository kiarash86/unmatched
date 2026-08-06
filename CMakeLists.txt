cmake_minimum_required(VERSION 3.16)
project(unmatched LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(raylib)


FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")

add_executable(CardGameEngine
    main.cpp
    ${SOURCES}
    ${HEADERS}
)
target_include_directories(CardGameEngine PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(CardGameEngine PRIVATE raylib nlohmann_json::nlohmann_json)

add_custom_command(TARGET CardGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:CardGameEngine>/assets"
)
add_custom_command(TARGET CardGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:CardGameEngine>/data"
)

# add_executable(MenuTest
#     test_menu.cpp
#     ${SOURCES}
# )
# target_include_directories(MenuTest PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_link_libraries(MenuTest PRIVATE raylib nlohmann_json::nlohmann_json)

# add_custom_command(TARGET MenuTest POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     "${CMAKE_SOURCE_DIR}/assets"
#     "$<TARGET_FILE_DIR:MenuTest>/assets"
# )
# add_custom_command(TARGET MenuTest POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     "${CMAKE_SOURCE_DIR}/data"
#     "$<TARGET_FILE_DIR:MenuTest>/data"
# )
