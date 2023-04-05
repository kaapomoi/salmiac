add_library(input STATIC
        src/input_manager.cpp
        )

target_include_directories(input PUBLIC include)