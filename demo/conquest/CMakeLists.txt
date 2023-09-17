add_library(conquest STATIC
        src/conquest.cpp
        src/game.cpp
)

target_include_directories(conquest PUBLIC include)
target_link_libraries(conquest PUBLIC core common)
