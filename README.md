# OpenGL - Build High Performance Graphics [![Build Status](https://travis-ci.org/wow2006/OpenGLBuildHighPerformanceGraphics.svg?branch=master)](https://travis-ci.org/wow2006/OpenGLBuildHighPerformanceGraphics)
Port OpenGL Build High Performance Graphics to CMake. ([Book link](https://www.amazon.com/OpenGL-Build-high-performance-graphics-ebook/dp/B07124RCBT/ref=sr_1_1?ie=UTF8&qid=1533807476&sr=8-1&keywords=OpenGL+Build+High+Performance+Graphics), [Original Source code](https://github.com/PacktPublishing/OpenGL-Build-High-Performance-Graphics))

This book split to three modules
- [Module 1](Module1/) : OpenGL Development Cookbook `WIP`
- [Module 2](Module2/) : OpenGL 4.0 Shading Language Cookbook `TODO`
- [Module 3](Module3/) : OpenGL Data Visualization Cookbook `TODO`

### Dependies:
- OpenGL
- SDL2
- GL3W
- GLM
- GSL 

Installation:
-------------

#### Ubuntu 16.04
```sh
$ sudo apt install libsdl2-dev libglew-dev libglm-dev
```
#### Arch
```sh
$ sudo pacman -S glew glm sdl2
```

#### Windows
```bash
> # Install vcpkg
> git clone https://github.com/Microsoft/vcpkg.git
> cd vcpkg

> .\bootstrap-vcpkg.bat
> .\vcpkg integrate install

> .\vcpkg.exe install glew:x64-windows
> .\vcpkg.exe install sdl2:x64-windows
> .\vcpkg.exe install glm:x64-windows
```

#### Clone
```
> git clone --recursive https://github.com/wow2006/OpenGLBuildHighPerformanceGraphics.git
```

### License
MIT
