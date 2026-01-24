#pragma once

#include <array>
#include <tuple>
#include <string_view>


// Scene configuration
static constexpr std::array<std::pair<std::string_view, std::string_view>, 3> SCENE_LIST = {{
    {"Test Scene 1", "test_scene_1.yaml"},
    {"Test Scene 2", "test_scene_2.yaml"},
    {"Test Scene 3", "test_scene_3.yaml"}
}};

// Shader configuration
static constexpr std::array<std::tuple<std::string_view, std::string_view, std::string_view>, 5> SHADER_DATA = {{
    {"vertexNormal", "vertexNormal.vsh", "vertexNormal.fsh"},
    {"faceNormal", "faceNormal.vsh", "faceNormal.fsh"},
    {"ground", "ground.vsh", "ground.fsh"},
    {"light", "light.vsh", "light.fsh"},
    {"default", "default.vsh", "default.fsh"},
}};

// Mesh configuration
static constexpr std::array<std::pair<std::string_view, std::string_view>, 6> MESH_DATA = {{
    {"surface", "surface.obj"},
    {"cube", "cube.obj"},
    {"sphere", "sphere.obj"},
    // {"cone", "cone.obj"},
    // {"torus", "torus.obj"},
    // {"bunny", "bunny.obj"},
}};

// Texture configuration
static constexpr std::array<std::pair<std::string_view, std::string_view>, 4> TEXTURE_DATA = {{
    {"dirtblock", "dirtblock.jpg"},
    {"checkerboard", "checkerboard.png"},
}};