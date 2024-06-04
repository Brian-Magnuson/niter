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
- llvm
- clang
- lldb
- lldb-mi
- gcc
- gdb
- make

### Linux

Compiling Niter has been tested on Ubuntu 22.04. Other distributions may require different packages to be installed and different package managers to be used.

On Debian-based systems, the following command can be used to install the required dependencies:
```sh
sudo apt-get install clang lldb lldb-mi gcc gdb make
```

LLVM is an exception; the required version of LLVM may not be available in the default package repositories. Version 17 is recommended. If your package manager provides this version, you can install it as normal.
You can check the your installed version of LLVM by running:
```sh
llvm-config --version
```
Visit the [LLVM website](https://llvm.org/) for more information on how to install LLVM.

To build `bin/niterc`, the Niter compiler, navigate to the project root and run:
```sh
make bin/niterc
```

This project uses Catch2 for unit testing. To build and run the tests, run:
```sh
make test
```
This command may take some time on the first run as Catch2 will need to be compiled.

To try out the Niter compiler, edit the `sandbox/src/main.nit` file and run:
```sh
cd sandbox && make
```

### Windows

Installing dependencies via MSYS2 is highly recommended. MSYS2 can be installed from [here](https://www.msys2.org/).

MSYS2 offers different environments for building software. The most notable for this project ones are the "MSYS" and "UCRT64" environments. 
You may want to consider adding their respective `bin` directories to your PATH. 
Usually, these directories are located at `C:\msys64\usr\bin` and `C:\msys64\ucrt64\bin`. 
Other environments may work.

If you have other means of installing the above dependencies, feel free to try them. Otherwise, the following instructions will guide you through installing the dependencies via MSYS2.

It is recommended to update the package database before installing any packages:
```sh
pacman -Syu
```

To install the required dependencies, open the UCRT64 environment and run the following command:
```sh
pacman -S mingw-w64-ucrt-x86_64-{clang,llvm,lldb,lldb-mi,gcc,gdb} make
```

Note: `make` is the only dependency that should explicitly be installed in the MSYS environment. The rest should be installed in the UCRT64 environment.

To build `bin/niterc`, the Niter compiler, navigate to the project root and run:
```sh
make bin/niterc
```

This project uses Catch2 for unit testing. To build and run the tests, run:
```sh
make test
```
This command may take some time on the first run as Catch2 will need to be compiled.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Copyright (c) 2024 Brian Magnuson
