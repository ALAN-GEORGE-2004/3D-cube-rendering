# 🎨 3D Cube Rendering

### 🧊 An Interactive 3D Cube Viewer Built with C++, Qt & OpenGL

3D Cube Rendering is a lightweight 3D graphics application developed using **C++17, Qt 6, and OpenGL 3.3**. It renders a colorful 3D cube with real-time lighting, interactive rotation, and mouse-controlled zoom.

This project demonstrates fundamental computer graphics concepts, including 3D transformations, perspective projection, shader programming, and lighting calculations.


## ✨ Features

- 🧊 **3D Cube Rendering** — Renders a colorful cube using OpenGL.
- 🎨 **Multicolored Faces** — Each face has its own color.
- 🖱️ **Interactive Rotation** — Rotate the cube using mouse dragging.
- 🔍 **Zoom Control** — Zoom in and out using the mouse wheel.
- 💡 **Real-Time Lighting** — Ambient, diffuse, and specular lighting effects.
- 🌀 **3D Transformations** — Uses model, view, and projection matrices.
- 🎥 **Perspective Projection** — Creates a realistic 3D viewing effect.
- ⚡ **GPU-Accelerated Rendering** — Uses OpenGL vertex and fragment shaders.
- 🛡️ **Depth Testing** — Correctly renders overlapping 3D surfaces.
- 🖼️ **Multisampling** — Requests 4× multisampling to improve edge smoothness.

## 🛠️ Tech Stack

| Technology | Purpose |
|---|---|
| 💻 C++17 | Core application logic |
| 🖼️ Qt 6 Widgets | Application framework |
| 🎮 OpenGL 3.3 Core | 3D rendering |
| 🎨 GLSL | Vertex and fragment shaders |
| 📐 `QMatrix4x4` | 3D transformations |
| 🧮 `QVector3D` | 3D vectors and lighting calculations |
| 🧊 `QOpenGLWidget` | OpenGL rendering surface |
| 🚀 qmake | Project configuration and build system |

## 📂 Project Structure

```text
PrismView/
├── main.cpp
├── openglwidget.h
├── openglwidget.cpp
├── Module_4.pro
├── README.md
├── .gitignore
```

> 📌 The structure above shows the main files. Generated build files, executables, and IDE caches should not be committed.

## 🚀 Getting Started

### 📋 Prerequisites

Before building the project, ensure you have:

- ✅ Qt 6 with OpenGL widget support
- ✅ Qt Creator
- ✅ A compatible C++17 compiler
- ✅ A compatible OpenGL 3.3 implementation and graphics driver
- ✅ The build tools required by your selected Qt kit


## 🎮 Controls

| Input | Action |
|---|---|
| 🖱️ Left mouse button + drag | Rotate the cube |
| 🖱️ Mouse wheel up | Zoom in |
| 🖱️ Mouse wheel down | Zoom out |

## 💡 Graphics Concepts Demonstrated

### 📐 3D Transformations

The application uses model, view, and projection matrices to transform 3D vertices into screen coordinates.

### 🎨 Shader Programming

GLSL shaders handle vertex transformations and calculate lighting for each rendered fragment.

### 💡 Lighting Model

The fragment shader combines three lighting components:

- **Ambient lighting** — Provides general illumination.
- **Diffuse lighting** — Simulates light scattered by a surface.
- **Specular lighting** — Adds highlights that create a sense of shininess.

### 🧊 Vertex Buffers

Vertex buffer objects store cube vertex attributes, including positions, normals, and colors, for GPU rendering.

### 🎥 Perspective Projection

Perspective projection makes objects appear smaller as their distance from the camera increases.


## 🔮 Future Improvements

- 🔄 Continuous cube rotation
- 🎛️ Adjustable lighting controls
- 🎨 Customizable face colors
- 📷 Reset-camera functionality
- 🧊 Support for additional 3D models
- 🖱️ Improved camera controls
- 🎚️ Adjustable rotation speed
- 🧪 Automated tests for application logic

## 🤝 Contributing

Contributions, suggestions, and bug reports are welcome!

1. 🍴 Fork the repository.
2. 🌿 Create a new feature branch.
3. 💻 Implement your changes.
4. ✅ Test your changes.
5. 📤 Submit a pull request.

## 👨‍💻 Author

**Alan George**