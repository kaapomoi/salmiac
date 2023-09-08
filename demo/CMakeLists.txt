add_library(demo STATIC 
    src/n_body_sim.cpp
    src/oct.cpp
    src/node.cpp
    )

target_include_directories(demo PUBLIC include)
target_link_libraries(demo PUBLIC core)
