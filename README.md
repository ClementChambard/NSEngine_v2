# NSEngine version 2

## NSEngine

NSEngine is a game engine written in C++ with Vulkan support.
Code greatly inspired by **Kohi game engine** by [TravisVroman](https://www.youtube.com/@TravisVroman) (currently following the series)
This is WIP and has not replace the v1 yet ([NSEngine v1](https://github.com/ClementChambard/NSEngine))

## nsmk

use tools/nsmk to build and run

- nsmk configure : configure the project (no options for now)
- nsmk : build the project
- nsmk run : build and run the testbed
- nsmk test : build and run the tests
- nsmk clean : clean the object files
- nsmk cleanall : remove all the build files
- nsmk log : show the logs of the previous session

shortcut for nvim users:

- nsmk vim : open the project in nvim
- nsmk vimlog : open the logs in nvim
- nsmk edit : open tools/nsmk in nvim
