add_library(util STATIC src/file_reader.cpp src/log.cpp)

find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(effolkronium_random CONFIG REQUIRED)

target_link_libraries(util PUBLIC fmt::fmt-header-only spdlog::spdlog_header_only effolkronium_random)
target_include_directories(util PUBLIC include)
