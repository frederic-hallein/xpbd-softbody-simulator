#pragma once

#include <array>
#include <tuple>
#include <string_view>

// Scene configuration
static constexpr std::array<std::pair<std::string_view, std::string_view>, 2> SCENE_LIST = {{
    {"Test Scene 1", "test_scene_1.yaml"},
    {"Test Scene 2", "test_scene_2.yaml"}
}};

// Shader configuration
static constexpr std::array<std::tuple<std::string_view, std::string_view, std::string_view>, 5> SHADER_DATA = {{
    {"vertexNormal", "vertexNormal.vsh", "vertexNormal.fsh"},
    {"faceNormal", "faceNormal.vsh", "faceNormal.fsh"},
    {"platform", "platform.vsh", "platform.fsh"},
    {"dirtblock", "dirtblock.vsh", "dirtblock.fsh"},
    {"sphere", "sphere.vsh", "sphere.fsh"},
}};

// Mesh configuration
static constexpr std::array<std::pair<std::string_view, std::string_view>, 2> MESH_DATA = {{
    {"cube", "cube.obj"},
    {"sphere", "sphere.obj"},
}};

// Texture configuration
static constexpr std::array<std::pair<std::string_view, std::string_view>, 1> TEXTURE_DATA = {{
    {"dirtblock", "dirtblock.jpg"}
}};