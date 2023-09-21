add_library(conquest STATIC
        src/conquest.cpp
        src/game.cpp
        src/artisan.cpp
        src/orchestrator.cpp
        src/neural_net.cpp
        src/template_definitions.cpp
)

target_include_directories(conquest PUBLIC include)
target_link_libraries(conquest PUBLIC core common)
