# Run the program
To run the program open the terminal in the project root folder. The path to the executable depends on the command use for the build.

- `build_folder`
    - The build folder can be `cmake-debug` or `cmake-release`.
- `config_file`
    - The path to the configuration file to use.

## Run locally
```bash
./<build_folder>/BGP_simulation [path_to_config_file]

# example: ./cmake-release/BGP_simulation ./examples/config.yaml
```

## Run through Docker
```bash
docker-compose run --rm --service-ports compiler ./<build_folder>/BGP_simulation [path_to_config_file]

# example: docker-compose run --rm --service-ports compiler ./cmake-release/BGP_simulation ./examples/config.yaml
```
