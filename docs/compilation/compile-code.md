# Compile code
💡 In this section we assume that the terminal is in the project root folder.

## Build locally
- `make build`
  - Compiles all the files, put them into the `cmake-release` directory, and produce the executable file.
- `make run`
  - Compiles all the files and run the program.
- `make dbuild`
  - Similar to `make build` but all the files are compiled in DEBUG mode (`DEBUG_GUARD` is set). The build directory is `cmake-debug`.
- `make drun`
  - Similar to `make run` but relies on `make dbuild` for the compilation.
- `make clean`
  - Deletes the build folders: `cmake-release` and `cmake-debug`.

## Build through Docker
### Build the Docker image
First, you need to build the docker image, this step is only required once.

```bash
docker build -t igang/build-cpp-env -f Dockerfile .
```

### Run the container
Then, run the container that will compile all the sources. You should specify the `[make command]` to execute. See the [Build locally](#build-locally) section.

```bash
docker run --rm -v "absolute/path/to/project:/project" --name build_env igang/build-cpp-env [make command]
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
In order to connect with the remote host you need to open Clion and go to `File > Settings > Build, Execution, Deployment > Toolchains`. Create a new configuration using `Remote host`. The ssh credentials to use are `user@password`, remember to choose "authentication with password". Use the host `localhost` and the port `2222`.

If this is the first connection and the port `2222` has already been used with other hosts, a warning might be issued. To clear the local ssh host key use the following command.

```bash
ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[localhost]:2222"
```

🔗 See also the [official documentation](https://www.jetbrains.com/help/clion/clion-toolchains-in-docker.html).

### Stop the container
When your are finished remember to stop the container.

```bash
docker stop build_clion_remote_env
```
