# SoRDIC

**Software Rendering Demo Engine In C v0.51**

https://github.com/aurb/SoRDIC

Copyright (C) 2024 https://github.com/aurb
Copyright (C) 2019 Sascha Ende

## Introduction

SoRDIC is a creative coding framework. It provides an engine for software-only real-time graphics generation. It has been implemented in pure C99 to provide cross-platform, cross-compilation capabilities. Portability was achieved by, among other things, giving up any means of hardware acceleration of graphics. Currently, to access the host platform, SoRDIC uses libsdl2, however, swapping to another platform backend should be feasible.

The target application of SoRDIC is real-time graphical presentations and animations. SoRDIC was created for use on the demoscene. Development of simple games should also be possible. The library comes with a set of examples showing how to use it.

## Quick HowTo

First follow [Build dependencies](#Build dependencies) section.

Then inside the project root directory build everything with the call:

`make all`

Built examples will be put into `examples` subdirectory. To consecutively run them all type:

`make run-all`

## Features

- Display interface
- Keyboard handling
- Music playback support
- Music annotation engine (for animation synchronisation)
- Multiple rendering buffer targets with Z buffer support
- Rendering buffers layering
    - Addition
    - Masking
    - Per-pixel alpha blending
- Color calculation/conversion functions
- Universal 1D transition curve functions (linear/square/cube/sin)
- Ability to use image files as texture maps and/or height/bump maps
- 3D graphics
    - Geometry transformations with object hierarchy
    - Camera support
    - Lighting with Gouraud shading model
        - Flat shading (per-polygon)
        - Interpolated shading (per-vertex)
        - Single ambient light
        - Single directional light
        - Multiple point lights
    - Object rendering
        - Z buffer support
        - Wireframe
        - Flat (per-polygon color)
        - Linear interpolated (per-vertex colors)
        - Affine texture mapped
            - Singletexturing
            - Singletexturing with color layers
                - Base texture
                - Multiplication color flat/interpolated (optional)
                - Addition color flat/interpolated (optional)
            - Multitexturing
                - Base texture
                - Multiplication texture (optional)
                - Addition texture (optional)
            - Bump texturing
                - Bump map
                - Reflection texture
    - Object/mesh generators
        - Regular polyhedrons
        - Parametric function mesh generator
            - Quad polygon mesh
            - Equilateral triangle mesh
            - Right triangle mesh
        - Toroids
        - Cycloids (up to 3 harmonics)
- Interface for direct line/polygon drawing
- Procedural 2D maps generators
    - Sine pattern with user-defined gradient (demoscene "plasma")

## Current status

Currently it is just my toy project. Much of the intended functionality has been brought to a fairly satisfactory state, making SoRDIC somewhat usable. However, other functionalities are still in the planning stages. The most important of these are:
- Support for objects and entire scenes/animations from the 3D modelers (Blender, etc.).
- A dedicated demo editor with a support for the sound annotations.
- New procedural map generators.
- New/fancy 3D object rendering/rasterization algorithms.
- Separation of the T&L and rasterization stages into the separate threads. Possibly with further paralellization of both.
- Improvements of map/rendering buffer layering logic.
- 2D filters (blurs, color manipulation, etc.) to apply on maps.

## Project structure

    +-- analyses     Static analysis results for make stu
    +-- engine       Library code and compilation results
    |   +-- clang    Clang-specific results (used by static analysis targets)
    |   +-- inc      Library includes
    |   +-- obj      Library compilation object output
    |   +-- src      Library source files
    +-- examples     Examples code and executables
        +-- assets   Texture maps, music and music annotations used by examples
        +-- clang    Clang-specific results (used by static analysis targets)
        +-- src      Examples source files
    Makefile         Makefile with all the build/analysis targets in it.
    README.md        This README.

## Build dependencies

To build SoRDIC you need gcc, make and libsdl2.

Install dependencies with:

`sudo apt-get install -y build-essential libsdl2-2.0-0 libsdl2-doc libsdl2-dev libsdl2-image-2.0-0 libsdl2-image-dev libsdl2-mixer-dev`

Compilation with clang is also supported. Change variable CC inside Makefile to enable it (if you find it useful).

## Diagnostics dependencies
 
Makefile provides some targets to facilitate static and dynamic analysis.

Install needed dependencies with:

`sudo apt-get install -y strace ltrace clang clang-tools bear`

## Build targets

At first it is recommended to create needed project directories with:

`make dirs`

Clean project:

`make clean`

Build library and examples with debug symbols:

`make debug`

Build library and examples with enabled optimizations and no debug symbols:

`make release`

Create project directories (if not existing already), clean project and build everything without debugging symbols:

`make all`

In order to change the examples window resolution alter the following line:

`EXAMPLES_FLAGS := -DDISPLAY_W=1200 -DDISPLAY_H=900`

If you want the example window to occupy the whole screen (without changing the screen resolution), then comment out above line, and uncomment the following one:

`EXAMPLES_FLAGS := -DFULL_DESKTOP=1`

## Run targets

Run one example. `example_name` shall be replaced with the name of the example to run.

`make run BIN=example_name`

Run all the examples (one after another):

`make run-all`

## Analysis targets

Some analyses are configured in the Makefile. For below listed calls, `example_name` shall be replaced with the name of the example to analyse.

System call trace. Results are stored in strace.example_name.out

`make strace BIN=example_name`

User library call trace. Results are stored in ltrace.example_name.out

`make ltrace BIN=example_name`

System and user library call trace. Results are stored in sltrace.example_name.out

`make sltrace BIN=example_name`

Static analysis for single translation units. Resulta are placed in $(ANALYSES) directory.

`make stu`

Static analysis for cross translation units.

`make ctu`

## License

### Source code, graphics assets, music annotations

**Software Rendered Demo Engine In C**

Copyright (C) 2024 https://github.com/aurb

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>. */

### Music assets

**Tranceverse**

Copyright (C) 2019 Sascha Ende

CC BY 4.0

https://creativecommons.org/licenses/by/4.0/legalcode

