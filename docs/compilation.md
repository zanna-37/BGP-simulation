# Compilation
There are multiple possible ways to compile the code and the documentation.

## Compile code
In this section we assume that the terminal is in the project root folder.

### Build locally
- `make build`
    - Compiles all the files, put them in the `build` directory and produce the executable file.
- `make run`
    - Compiles all the files and run the program.
- `make clean`
    - Deletes all the files inside the `build` folder.

### Build through Docker
#### Build the Docker image
First, we need to build the docker image, this step is only required once.

```bash
docker build -t igang/build-cpp-env -f Dockerfile .
```

#### Run the container
Then we run the container that will compile all the sources.
We should specify the `[make command]` to execute. See the [Build locally](#build-locally) section.

```bash
docker run --rm -v ".:/project" --name build_env igang/build-cpp-env [make command]
```

💡 **Troubleshooting**

Windows requires full path when using docker run.

```bash
docker run --rm -v "absolute\path\to\project:/project" --name build_env igang/build-cpp-env [make command]
```

### Build through Clion
Clion can compile code using a remote host. In this case the remote host is the docker machine.

#### Build the Docker image
First, we need to build the docker image, this step is only required once.

```bash
docker build -t igang/build-clion-cpp-env -f Dockerfile-build-clion .
```

#### Run the container
```bash
docker run --rm -d --cap-add sys_ptrace -p 2222:22 --name build_clion_remote_env igang/build-clion-cpp-env
```

#### Setup Clion
In order to connect with the remote host we need to open Clion and go to `File > Settings > Build, Execution, Deployment > Toolchains`. Create a new configuration using `Remote host`. The ssh credentials to use are `user@password`, remember to choose "authentication with password". Use the host `localhost` and the port `2222`.

If this is the first connection and the port `2222` has already been used with other hosts, a warning might be issued. To clear the local ssh host key use the following command.

```bash
ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[localhost]:2222"
```

🔗 See also the [official documentation](https://www.jetbrains.com/help/clion/clion-toolchains-in-docker.html).

#### Stop the container
When finiseh remember to stop the cointainer.

```bash
docker stop build_clion_remote_env
```

## Compile documentation
In this section we assume that the terminal is in the `docs` directory which can be found in the project root folder.

### Build docs locally
- `make docs`
    - Generate the documentation and put it in the `_build` folder.
    - See the [quickstart guide](https://docs.readthedocs.io/en/stable/intro/getting-started-with-sphinx.html).

- `make clean`
    - Deletes all the files inside the `_build` folder.

### Build through Docker
#### Build the Docker image
First, we need to build the docker image, this step is only required once.

```bash
docker build -t igang/docs-builder -f Dockerfile-docs .
```

#### Run the container
Then we run the container that will compile all the sources.
We should specify the `[make command]` to execute. See the [Build docs locally](#build-docs-locally) section.

```bash
docker run --rm -v ./docs:/docs igang/docs-builder [make command]
```

💡 **Troubleshooting**

Windows requires full path when using docker run.

```bash
docker run --rm -v "absolute\path\to\project\docs:/docs" igang/docs-builder [make command]
```

