find_package(SFML 2.2 COMPONENTS window graphics system REQUIRED)
add_executable(Particle Particle.cpp Particle.hpp)
target_link_libraries(Particle entity ${SFML_LIBRARIES})
target_include_directories(Particle PRIVATE ${PROJECT_SOURCE_DIR} ${SFML_INCLUDE_DIR})
