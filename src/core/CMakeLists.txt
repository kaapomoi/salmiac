find_package(EnTT CONFIG REQUIRED)

add_library(core STATIC
        src/application.cpp
)

target_link_libraries(core PUBLIC EnTT::EnTT gfx util input components)
target_include_directories(core PUBLIC include)
