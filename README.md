
# ALGOL60 -> WebAssembly proof of concept

This a proof of concept for building ALGOL60 to WebAssembly

## How it works
1. Builds and installs the [GNU Marst](https://www.gnu.org/software/marst/) ALGOL60 compiler

1. Builds an LLVM bitcode version of libalgol

1. Transpile ALGOL60 source to C using Marst
1. Build WebAssembly (and helper JS) using [emscripten](https://emscripten.org/), linking in libalgol bitcode

## Testing it out
The whole development environment is contained in the `.devContainer` directory.

Use Docker and the [containers remote extension](https://code.visualstudio.com/docs/remote/containers) for VSCode to reliably reproduce this environment

## Live Demo
[moxon6.github.io/algol-60-webassembly/](https://moxon6.github.io/algol-60-webassembly/)
