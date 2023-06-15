add_library(learnopengl INTERFACE)
target_link_libraries(learnopengl INTERFACE stb assimp)
target_include_directories(learnopengl INTERFACE . "${CMAKE_CURRENT_BINARY_DIR}")
configure_file(learnopengl/root_directory.h.in learnopengl/root_directory.h)