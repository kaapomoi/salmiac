add_library(common STATIC
        src/camera_controller.cpp
)

target_include_directories(common PUBLIC include)
target_link_libraries(common PUBLIC core)
