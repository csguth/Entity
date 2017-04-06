find_package(SFML COMPONENTS window graphics system)
add_executable(Particle Particle.cpp Particle.hpp)
target_link_libraries(Particle Entity ${SFML_LIBRARIES})
target_include_directories(Particle PRIVATE ${PROJECT_SOURCE_DIR} ${SFML_INCLUDE_DIR})
