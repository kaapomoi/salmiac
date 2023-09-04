add_library(components STATIC src/transform.cpp)

find_package(glm CONFIG REQUIRED)

target_link_libraries(components PUBLIC glm::glm)
target_include_directories(components PUBLIC include)
