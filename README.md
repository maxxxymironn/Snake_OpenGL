# Snake (OpenGL)
### - [How to build project?](#how-to-build-project)

![Demo](assets/snake_preview.gif)<br>
![Demo](assets/image.png)<br>

## How to build project
To build this project you should have [CMake](https://cmake.org/download/) on your computer.
Clone repo and use these commands in console in the local repo:
```shell
cmake -B build -S .
cmake --build build
``` 
`cmake -B build -S .` to generate build files,  
`cmake --build build` to build project.

Also you can generate build files and build project with CMake GUI.

Repo contains `CMakePresets.json` and, if you have generators and compilers listed in these presets, you can use these commands instead of ones listed above:
```shell
cmake --preset <preset_name>
cmake --build --preset <preset_name>
```
