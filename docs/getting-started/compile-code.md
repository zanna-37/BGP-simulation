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
### Build the Docker image
First, you need to build the docker image, this step is only required once.

```bash
docker build -t igang/build-cpp-env -f Dockerfile .
```

### Run the container
Then, run the container that will compile all the sources. You should specify the `[make command]` to execute. See the [Build locally](#build-locally) section.

```bash
docker run --rm -v "</absolute_path_to_project>:/project" --name build_env igang/build-cpp-env [make command]

# example: docker run --rm -v "/Users/nickname/protocol_processing_security/simulation:/project" --name build_env igang/build-cpp-env make build
```

## Build through Clion Remote Host
Clion can compile code using a remote host. In this case the remote host is the docker machine.

### Build the Docker image
First, you need to build the docker image, this step is only required once.

```bash
docker build -t igang/build-clion-cpp-env -f Dockerfile-build-clion .
```

### Run the container

```bash
docker run --rm -d --cap-add sys_ptrace -p 2222:22 --name build_clion_remote_env igang/build-clion-cpp-env
```

### Setup Clion
In order to connect with the remote host you need to open Clion and go
to `File > Settings > Build, Execution, Deployment > Toolchains`. Create a new configuration using `Remote host`. The
ssh credentials to use are `user@password`, remember to choose "authentication with password". Use the host `localhost`
and the port `2222`.

The default compilers are `cc` and `c++`. Since we use `clang`, put `/usr/bin/clang` as `c` compiler
and `/usr/bin/clang++` as `c++` compiler.

If this is the first connection and the port `2222` has already been used with other hosts, a warning might be issued.
To clear the local ssh host key use the following command.

```bash
ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[localhost]:2222"
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
docker stop build_clion_remote_env
```
