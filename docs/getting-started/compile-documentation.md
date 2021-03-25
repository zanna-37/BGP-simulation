# Compile documentation
💡 In this section we assume that the terminal is in the `docs` directory which can be found in the project root folder.

## Build docs locally
💡 We are assuming you have already installed all the pre-requisites to run the command. In case you haven't, you can refer to the [quickstart guide](https://docs.readthedocs.io/en/stable/intro/getting-started-with-sphinx.html) or check the `Dockerfile-docs` file.

- `make html`
  - Generate the documentation and put it into the `_build` folder.
- `make clean`
  - Deletes the `_build` folder.

## Build through Docker
#### Build the Docker image
First, we need to build the docker image, this step is only required once.

```bash
docker build -t igang/docs-builder -f Dockerfile-docs .
```

### Run the container
Then we run the container that will compile all the sources. You should specify the `[make command]` to execute. See the [Build docs locally](#build-docs-locally) section.

```bash
docker run --rm -v "absolute/path/to/project/docs:/docs" --name build_docs_env igang/docs-builder [make command]
```
