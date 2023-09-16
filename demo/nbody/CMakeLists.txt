add_library(nbody STATIC
        src/n_body_sim.cpp
        src/oct.cpp
        src/node.cpp
)

target_include_directories(nbody PUBLIC include)
target_link_libraries(nbody PUBLIC core common)
