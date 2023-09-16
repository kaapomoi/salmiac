add_library(conquest STATIC
        src/conquest.cpp
)

target_include_directories(conquest PUBLIC include)
target_link_libraries(conquest PUBLIC core common)
