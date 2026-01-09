# XPBD Softbody Implementation

## Description [TODO: update]

**XPBD Softbody Implementation** is a physics algorithm implementation written in C++ using OpenGL for rendering. The project focuses on simulating softbody dynamics using **Extended Position Based Dynamics (XPBD)** ([Macklin et al., 2019](https://matthias-research.github.io/pages/publications/smallsteps.pdf)), enabling realistic and efficient softbody physics.

## Visuals

![Physics Engine Screenshot](res/screenshots/readme-screenshot.png)

---

## Installation

### Dependencies

- **C++20** compiler (GCC 10+/Clang 10+)
- **CMake** 3.28.3 or newer
- **OpenGL** development libraries
- **GLFW3** development libraries
- **Assimp** development libraries
- **yaml-cpp** development libraries
- **ImGui** (included in the repository)

### Ubuntu/Debian

```sh
sudo apt-get update
sudo apt-get install build-essential cmake libassimp-dev libglm-dev libglfw3-dev libglew-dev libyaml-cpp-dev
```

1. **Clone the repository:**
    ```sh
    git clone https://github.com/frederic-hallein/xpbd-softbody-implementation.git
    ```

2. **Enter the project directory:**
    ```sh
    cd xpbd-softbody-implementation
    ```

3. **Create and enter the build directory:**
    ```sh
    mkdir build
    cd build
    ```

4. **Configure the project with CMake:**
    ```sh
    cmake -DCMAKE_BUILD_TYPE=Release ..
    ```

5. **Build the project:**
    ```sh
    make
    ```

6. **Run the executable:**
    ```sh
    ./xpbd-softbody
    ```

---

## Usage

### Controls

- **Left Mouse Button + Drag:** Orbit the camera around the origin.
- **Mouse Scroll Wheel:** Zoom in/out.

#### ImGui Debug Window

- **Reset Camera Button:** Resets camera (shortcut: C).
- **Reset Scene Button:** Resets all objects (shortcut: R).
- **Sliders:** Adjust gravity, alpha, beta, and solver substeps.
- **Toggles:** Enable/disable distance, volume, and collision constraints.

---

## Roadmap

- Improved collision detection and response
- Support for more constraint types (e.g. friction constraint)
- Enhanced rendering (e.g. shadows, materials)
- More object primitives and mesh import formats
- Performance optimizations and parallelization

---

## Project Status

**Not currently in active development.**
Core XPBD simulation and rendering features are implemented.
Additional features and optimizations are planned but not actively worked on.