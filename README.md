# XPBD Softbody Simulator


**XPBD Softbody Simulator** is a real-time 3D physics simulation engine written in C++ for experimenting with and visualizing deformable object dynamics. It implements *Extended Position Based Dynamics (XPBD)*, a constraint-based formulation that enables stable and efficient softbody simulation with physically plausible material deformation. The engine features an interactive interface for live parameter tuning, scene switching, and detailed object state logging, making it well-suited for learning, prototyping, and research-oriented exploration of real-time physics.

The project uses OpenGL for rendering and ImGui for real-time debugging and parameter modification. Meshes are loaded from `.obj` files using ASSIMP, and scenes are defined through YAML configuration files to separate data from code and support flexible scene composition. Development was informed by the XPBD paper by [Macklin et al., (2019)](https://matthias-research.github.io/pages/publications/smallsteps.pdf), as well as OpenGL learning resources such as [LearnOpenGL](https://learnopengl.com/
), which guided the rendering architecture and graphics pipeline design.

## Table of Content

- [Preview](#preview)
- [Technical Overview](#technical-overview)
  - [Constraint Formulation](#constraint-formulation)
  - [Constraint Types](#constraint-types)
  - [Simulation Loop](#simulation-loop)
- [Key Features](#key-features)
- [Build](#build)
- [Usage](#usage)
  - [Camera Controls](#camera-controls)
  - [Object Interaction](#object-interaction)
  - [Simulation Controls (ImGui Debug Window)](#simulation-controls-imgui-debug-window)
- [Future Work](#future-work)

## Preview


<p align="center">
  <img src="assets/sphere.gif" alt="XPBD Softbody Simulation - Bouncy Sphere" width="800"/>
</p>
<p align="center"><b>Gif 1:</b> Bouncy sphere with distance and volume constraints. Polygon mode is set to wireframe with vertex (green) and face (red) normals enabled.  </p>


<p align="center">
  <img src="assets/cloth.gif" alt="XPBD Softbody Simulation - Bouncy Sphere" width="800"/>
</p>
<p align="center"><b>Gif 1:</b> Cloth in zero-gravity with only distance constraints.</p>

**Note:** GIF playback does not reflect real-time performance.

## Technical Overview

This project implements **Extended Position Based Dynamics (XPBD)**, a constraint-based framework for robust and efficient real-time simulation of deformable objects. Unlike force-based methods, XPBD defines object behavior through *geometric constraints* that enforce desired relationships between particles.

### Constraint Formulation

In XPBD, constraints are scalar functions that describe relationships between particle positions:

$$C(\mathbf{x}_0, \mathbf{x}_1, ..., \mathbf{x}_n)$$

where $\mathbf{x}_i \in \mathbb{R}^3$ represents particle positions. A constraint is satisfied when above expression equals zero. The framework iteratively corrects particle positions and updates Lagrange multipliers $\lambda$ to enforce constraints, enabling physically plausible deformation while maintaining stability under large time steps.

### Constraint Types

This simulator supports two primary constraint types:

- **Distance Constraints** for maintaining fixed edge lengths between particles which is defined as:

$$C_j(\mathbf{x}_k, \mathbf{x}_l) = |\mathbf{x}_l - \mathbf{x}_k| - d_0,$$

where $d_0$ is the rest distance.


- A **Volume Constraint** for preserving the objects volume which is defined as:

$$C(\mathbf{x}_0, ..., \mathbf{x}_{n-1}) = \left( \sum^{n_\text{T} - 1}_{i = 1} \frac{1}{6} (\mathbf{x}_{t_{i,0}} \times \mathbf{x}_{t_{i,1}}) \cdot \mathbf{x}_{t_{i,2}} \right) - pV_0,$$

where $n_\text{T}$ is the triangle count, $t_{i,0}, t_{i,1},$ and $t_{i,0}$ are the three vertex indices belonging to triangle $i$, $p$ is the overpressure factor, and $V_0$ is the rest volume.

### Simulation Loop

1. **Time Step Subdivision:**

To enable stable simulation under large time steps, the frame time $\Delta t$ is subdivided into $n_\text{steps}$ smaller substeps

$$\Delta t_s = \frac{\Delta t}{n_\text{steps}},$$

which accelerates constraint convergence and eliminates the need to explicitly track Lagrange multipliers $\lambda_j$ across time steps.

2. **Position Prediction:**

At each substep, particle positions are predicted by integrating external forces (e.g., gravity):

$$\tilde{\mathbf{x}} = \mathbf{x}^n + \Delta t_s \mathbf{v}^n + \Delta t_s^2 \mathbf{M}^{-1} \mathbf{f}_\text{ext}$$

3. **Constraint Solving:**

For each substep, all constraints are iteratively enforced by computing the position and Lagrange multiplier corrections:

$$\Delta \mathbf{x} = \mathbf{M}^{-1} \nabla C_j (\mathbf{x})^T \Delta \lambda_j,$$

$$\Delta \mathbf{\lambda}_j = \frac{-C_j(\mathbf{x}) - \gamma_j \nabla C_j (\mathbf{x} - \mathbf{x}^n)}{(1 + \gamma_j) \nabla C_j \mathbf{M}^{-1} \nabla C_j^T + \tilde{\alpha}_j},$$

where

- $\mathbf{M}$ is the mass matrix
- $\tilde{\alpha}_j$ is the inverse stiffness or *compliance*
- $\gamma_j = \frac{\tilde{\alpha}_j \tilde{\beta}_j}{\Delta t_s}$ is the time step scaled
product of compliance and damping parameter $\tilde{\beta}_j$.

Positions are then updated: $\mathbf{x}^{n+1} \leftarrow \tilde{\mathbf{x}} + \Delta \mathbf{x}$

4. **Velocity Update:**

After all constraints are satisfied, velocities are updated for the next substep:

$$\mathbf{v}^{n+1} = \frac{\mathbf{x}^{n+1} - \mathbf{x}^n}{\Delta t_s}.$$

This process repeats for each substep until the full frame time $\Delta t$ is consumed.


## Key Features

- **Orbital Camera Controls:** Intuitive camera navigation with right-click orbit, scroll zoom, and camera reset functionality.
- **Real-Time Parameter Control:** Adjust simulation parameters (gravity, compliance, damping, solver substeps) live through the ImGui debug window.
- **Object Grabbing:** Interactive object manipulation using the *Möller–Trumbore ray-triangle intersection* algorithm for precise picking.
- **Collision & Containment:** Basic ground collision detection with invisible barriers to prevent objects from escaping the simulation space.
- **Scene Management:** Switch between predefined scenes loaded from YAML configuration files for flexible experimentation.
- **Lighting & Shading:** Phong lighting model with support for normal visualization and polygon mode toggling (wireframe/filled).
- **Constraint-Based Dynamics:** Supported constraint types include distance constraints (which maintain edge lengths) and volume constraints (which preserve object volume), enabling physically plausible softbody deformation.
- **Multithreaded Physics:** Object updates parallelized across all available CPU cores using a custom thread pool implementation for improved performance on multi-core systems.
- **Performance Monitoring:** Real-time FPS counter and frame duration visualization for optimization feedback.

## Build

The program requires the following dependencies:

- **C++20** compatible compiler (GCC ≥ 10 or Clang ≥ 10)
- **CMake** ≥ 3.28.3
- **OpenGL** development libraries
- **GLFW** development libraries
- **GLM**
- **ASSIMP**
- **yaml-cpp**

These can be installed as follows:

```sh
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libglew-dev libglm-dev libassimp-dev libyaml-cpp-dev
```

For building the project, do the following:

1. Clone the repository:
    ```sh
    git clone git@github.com:frederic-hallein/xpbd-softbody-simulator.git
    cd xpbd-softbody-simulator
    ```

2. Create a build directory:
    ```sh
    mkdir build && cd build
    ```

3. Configure the project:
    ```sh
    cmake -DCMAKE_BUILD_TYPE=Release ..
    ```

    This enables `-O3 -march=native` optimizations for maximum performance on your system.


4. Build:
    ```sh
    cmake --build .
    ```

## Usage

After building, launch the executable:

```sh
./xpbd-softbody-simulator
```

### Camera Controls

- **Right Mouse Button + Drag:** Orbit the camera around the origin.
- **Mouse Scroll Wheel:** Move camera radially from the origin.

### Object Interaction

- **Left Mouse Button + Drag:** Grab and move objects in the scene.

### Simulation Controls (ImGui Debug Window)

- **Scene Selection:** Switch between available scenes using a dropdown menu.
- **Performance Monitor:** View real-time frame duration and FPS, with a live FPS plot.
- **Camera Controls:** Reset camera position (button or press `C`) and view camera coordinates.
- **External Forces:** Adjust gravity using a slider or reset to default.
- **XPBD Parameters:**
  - Change solver substeps (slider or +/- buttons).
  - Toggle distance and volume constraints.
  - Adjust compliance and damping parameters.
- **Scene Reset:** Reset all objects in the scene (button or press `R`).
- **Object Panels:**
  - View mesh topology and constraint energies.
  - Inspect vertex positions, velocities, and accelerations.
  - Switch between wireframe and filled polygon modes.
  - Toggle vertex and face normal shaders.
- **`ESC`-Key:** quit the program.

## Discussion

### Performance Characteristics

This XPBD implementation prioritizes stability and ease of understanding over raw performance optimization. The simulator has been tested on an Intel i5-1035G1 with integrated Iris Plus Graphics, achieving stable **60 FPS** at low substep counts (less than 10 substeps) with scenes containing 10 deformable objects.

TODO: drop to steady 30 fps for 10 spheres (v: 647, e: 1920, tri: 1280) with 6 substeps and cloth scene (v: 1334, e: 3855, tri: 2522) with 7 substeps

### Performance Bottlenecks

The primary computational bottleneck is constraint solving, which scales with the number of constraints and solver iterations. The iterative nature of XPBD means that:

- **Substep count** has the most significant impact on frame time. Doubling substeps approximately doubles constraint solving cost.
- **Object complexity** (vertex and triangle count) affects both constraint generation and Lagrange multiplier updates.
- **Constraint density** (distance and volume constraints per object) directly influences iteration count.

### Multithreading Impact (TODO: benchmark speedup)

The custom thread pool implementation parallelizes object physics updates across all available CPU cores. On a dual-core system (as in the i5-1035G1), this provides approximately **1.5-1.8x speedup** compared to sequential updates, with gains diminishing due to synchronization overhead and shared memory contention. Integrated GPUs also introduce additional bottlenecks through OpenGL state management and draw call overhead, which are not parallelized.


## Conclusion

TODO

## Future Work

- Implement bending constraints for more realistic softbody deformation
- Add proper environment and self collision
- Introduce static and kinetic friction for more accurate physical interactions
