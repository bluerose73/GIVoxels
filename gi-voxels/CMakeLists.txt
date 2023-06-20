add_executable(app app.cpp
                   renderer.cpp
                   g_buffer.cpp
                   shadow_map.cpp
                   voxels.cpp)
target_include_directories(app PRIVATE ..)
target_link_libraries(app PRIVATE glad glfw glm learnopengl json icecream)