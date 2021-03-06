# TToY Graphical Terminal Emulator

## Disclaimer
ttoy is currently alpha software. While it may appear functional, many of
the bugs that might adversely affect programs running with ttoy have not
yet been ironed out. As such, ttoy is not suitable for production use at
this time.

## Introduction
ttoy is a graphically embellished terminal emulator for Linux systems.
ttoy is inspired by the excellent Shadertoy web application, and especially
its clone GLSL Sandbox. The defining feature of ttoy is the ability to load
graphical plugins, called *toys*, and re-compile those plugins as the
programmer edits them in real-time.  The idea is to provide a *native*
environment for computer graphics experimentation with immediate feedback.
Contrary to the existing web-based GLSL Sandbox, this environment should be
comforting to graphics programmers inclined to programming with command-line
editors such as Vim, Emacs, or Nano.

ttoy will eventually include toys compatible with GLSL shaders created with
Shadertoy.  ttoy is not, however, limited to just GLSL fragment shaders
rendered to a single quad. In theory, ttoy should be able to support any
graphical application that can link with the ttoy C API and render with
OpenGL (even the requirement for OpenGL is not set in stone; support for Vulkan
and/or Direct3D is being considered as well.)

It is hoped that ttoy will also be useful as a general purpose terminal
emulator. Many of the toys written for ttoy, especially those that are
relatively lightweight and not distracting, are suitable as everyday
backgrounds. Power management features are also planned, which will make
ttoy more useful in power-limited environments such as laptops.

## Compiling
ttoy currently uses some Linux-only system API's, including epoll(7) and
posix_openpt(3). Compilation almost certainly fails on any other platforms,
although patches are welcome.

ttoy leverages a number of open-source libraries including libtsm, SDL2,
GLEW, and FreeType. All of these libraries along with CMake are needed to
compile ttoy.

## License
ttoy is copyright 2016-2017 Jonathan Glines and is distributed under the terms
of the MIT License. See the contents of COPYING for details.
