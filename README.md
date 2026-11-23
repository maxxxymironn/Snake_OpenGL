# Snake (OpenGL)
### - [How to build project?](#how-to-build-project)  
### - [Hotkeys](#hotkeys)  
### - [Config](#config)

![Demo](assets/snake-v1-7-8.gif)<br>

## How to build project
You should have [CMake](https://cmake.org/download/) on your computer to build this project.
Clone repo and use these commands in console in the local repo:
```shell
cmake -B build -S .
cmake --build build
``` 
`cmake -B build -S .` to generate build files,  
`cmake --build build` to build project.

You may have some issues with building project because idk how to write correct CMakeLists.txt.

## Hotkeys
`wasd` or `key_up key_down key_left key_right` - move snake
`escape` in menu - exit game,  
         in game - exit to menu
`space` in menu - play game
`y`, `n` in game to choose 1st/2nd buttons
`p` - freeze/unfreeze game  
`ctrl + =` - scale up  
`ctrl + -` - scale down  
`ctrl + z` - zen mode  

## Config
You can change field size, snake color, and more in config file. Program creates directory and config file if you start the program for the first time in the path:  
- `AppData/Snake_OpenGL/config` for Windows;
- `home/.local/share/Snake_OpenGL/config` for Linux.
