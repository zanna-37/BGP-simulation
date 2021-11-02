# Compile documentation

## Build docs locally
💡 We assume you have already installed all the pre-requisites to run the command. In case you haven't, you can refer to
the [quickstart guide](https://docs.readthedocs.io/en/stable/intro/getting-started-with-sphinx.html) or check
the `{REPO_ROOT_DIR}/docker/docs/Dockerfile` file.

💡 We assume that the terminal is in the `docs` directory which can be found in the project root folder.

### `make html`
Generate the documentation and put it into the `_build` folder.

### `make clean`
Deletes the `_build` folder.

## Build through Docker
Run the container that will compile all the sources. You should specify the `[make command]` to execute. See
the [Build docs locally](#build-docs-locally) section.

```bash
docker-compose run --rm docs [make command]

# example: docker-compose run --rm docs make html
```
