# Compile code
💡 In this section we assume that the terminal is in the project root folder.

## Build locally
### `make build`
Compiles all the files and produce the executable file.  
By default, the files are generated under `cmake-release` directory.

The build can be customized with some **flags**:  
_Example: `make DEBUG=1 build`_

- `DEBUG=1`  
  All the files are compiled in debug mode (`DEBUG_GUARD` is set). The build directory is `cmake-debug`.
- `IWYU=1`  
  It enables the "Include What You Use" warnings during compilation.

### `make clean`
Deletes the build folders: `cmake-release` and `cmake-debug`.

## Build through Docker
Run the container that will compile all the sources. You should specify the `[make command]` to execute. See
the [Build locally](#build-locally) section.

```bash
docker-compose run --rm compiler [make command]

# example: docker-compose run --rm compiler make build
```

## Build through Clion Remote Host
Clion can compile code using a remote host. In this case the remote host is the docker machine.

### Run the container

```bash
docker-compose up -d compiler_clion
```

### Setup Clion
In order to connect with the remote host you need to open Clion and go
to `File > Settings > Build, Execution, Deployment > Toolchains`. Create a new configuration using `Remote host`. The
ssh credentials to use are `user@password`, remember to choose "authentication with password". Use the host `localhost`
and the port `3022`.

The default compilers are `cc` and `c++`. Since we use `clang`, put `/usr/bin/clang` as `c` compiler
and `/usr/bin/clang++` as `c++` compiler.

If this is the first connection and the port has already been used with other hosts, a warning might be issued. To clear
the local ssh host key use the following command.

```bash
ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[localhost]:3022"
```

To set the compilation flags, go to `File > Settings > Build, Execution, Deployment > CMake` and create the profile
needed. The flag `DEBUG=1` described in [Build locally](#build-locally) can be achieved by setting the `Build type`
to `Debug`, `CMake options` to `-DUSE_DEBUG=ON`, and `Build directory` to `cmake-debug`. Similarly `IWYU=1` can be
achieved by setting `CMake options` to `-DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="iwyu;-Xiwyu;--no_fwd_decls"`.  
In general, refer to the Makefile and try to replicate what is done there inside the CLion configuration.

🔗 See also the [official documentation](https://www.jetbrains.com/help/clion/clion-toolchains-in-docker.html).

### Stop the container
When your are finished remember to stop the container.

```bash
docker-compose stop compiler_clion
```

### Remove the container
Removing the container is usually discouraged because the compilation cache is removed. Remove the container when its
compilation service is no longer needed.

```bash
docker-compose rm --stop compiler_clion
```
