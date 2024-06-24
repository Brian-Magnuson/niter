# Niter

A fast, statically-typed, programming language with a simple compiler built on LLVM.

```
extern variadic fun printf(char*): i32;

fun main() {
    const message = "World"
    printf("Hello, %s!\n", message)
    return 0
}
```

Niter philosophy:

- Explicit is better than implicit
- Readability counts
- Purity and immutability make for predictability
- Simplicity is better than complexity
- Versatility is better than simplicity
- Consistency is golden
- Manual memory management is always an option

## Setup

Compiling Niter generally requires the following dependencies to be installed:
- llvm (version 18)
- clang (version 18)
- lldb
- gcc (version 7 or later)
- gdb
- catch2 (version 3.40 or later; can be fetched via CMake)
- make

### Linux

Compiling Niter has been tested on Ubuntu 22.04. Other distributions may require different packages to be installed and different package managers to be used.

On Debian-based systems, the following command can be used to install the required dependencies:
```sh
sudo apt-get install gcc gdb make
```

Clang and LLVM are exceptions; the required version of LLVM may not be available in the default package repositories. Version 18 is recommended. If your package manager provides this version, you can install it as normal.
You can check the your installed version of LLVM by running:
```sh
llvm-config --version
```
Visit the [LLVM website](https://llvm.org/) for more information on how to install LLVM.

### Windows

Installing dependencies via MSYS2 is highly recommended. MSYS2 can be installed from [here](https://www.msys2.org/).

MSYS2 offers different environments for building software. The most notable for this project ones are the "MSYS" and "UCRT64" environments. 
You may want to consider adding their respective `bin` directories to your PATH. 
Usually, these directories are located at `C:\msys64\usr\bin` and `C:\msys64\ucrt64\bin`. 
Other environments may work.

If you have other means of installing the above dependencies, feel free to try them. Otherwise, the following instructions will guide you through installing the dependencies via MSYS2.

To install the required dependencies, open the UCRT64 environment and run the following command:
```sh
pacman -S mingw-w64-ucrt-x86_64-{clang,llvm,lldb,lldb-mi,gcc,gdb} make
```

To update your installed packages, run the following command:
```sh
pacman -Syu
```

Note: `make` is the only dependency that should explicitly be installed in the MSYS environment. The rest should be installed in the UCRT64 environment.

## Building

This project uses CMake to build. Version 3.19 or later is recommended. Some package managers may provide a package for CMake. It can also be installed from the [CMake website](https://cmake.org/).

CMake is designed to simplify the build process across different platforms. It generates native build files for your platform, which can then be used to build the project.
Some settings for CMake, such as the generator and compiler, vary depending on the environment. 
For Unix-like systems, we recommend using "Unix Makefiles" as the generator. 
For Windows, "MSYS Makefiles" is recommended.
Presets for Linux and Windows have been provided in the [`CMakePresets.json`](./CMakePresets.json) file.
Users may specify their own presets in a `CMakeUserPresets.json` file in the project root.

To build the project, run the following commands in the project root:
```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```
This will generate the build files in the `build` directory, then build the project.

Note: If you do not have Catch2 installed, CMake will attempt to fetch it from the GitHub repository and install it in the build directory.

If you want to clean up the build files, you can simply delete the `build` directory and then run the `cmake` commands again to regenerate the build files.

Some IDEs provide tools for working with CMake projects. For example, with Visual Studio Code, you can install the [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) to work with CMake projects.
This can be helpful for managing the build process, debugging, and running tests.

## Running

A makefile has been provided to run the Niter compiler. Navigate to the `sandbox` directory and run:
```sh
make
```
This will build the Niter compiler using CMake, then compile the `main.nit` file in the `sandbox/src` directory, then run the compiled executable in the `sandbox/bin` directory.

For debugging, the unoptimized LLVM IR will be dumped in the `sandbox/debug` directory.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Copyright (c) 2024 Brian Magnuson
