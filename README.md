# Glare Engine

Glare is an experimental data-driven game engine, built in modern C++.
For a general overview of the engine's features, please [view the wiki](https://github.com/ImmutableOctet/glare/wiki).

The engine itself is custom built atop [several libraries](vcpkg.json), including [EnTT](https://github.com/skypjack/entt) as its ECS solution, [SDL2](https://github.com/libsdl-org/SDL) for windowing and input, [Bullet](https://github.com/bulletphysics/bullet3) for collision detection, and a simple deferred renderer built in OpenGL (~3.3 or later).

My intention is to eventually support Linux via Clang or GCC, but my current focus is on Windows builds via CMake + Visual Studio 2022.
> Glare currently targets /std:c++latest on MSVC for Windows (x64).

Some portions of the project, most notably, the `app` and `graphics` submodules are subject to major refactoring at a later date. With that said, **no part of this API is finalized**. This project has been made open source purely for educational purposes (and possibly collaborative purposes later down the road). Please do not expect any kind of API stability or support from this project at this time.

# Contributions

I am not currently accepting contributions at this time, but feel free to make feature suggestions, report bugs or give general feedback if you're interested in the project.

# Licensing

This project and related materials fall under the [MIT license](LICENSE.md).
