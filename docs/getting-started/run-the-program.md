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
docker run --rm -v "</absolute_path_to_project>:/project" --name build_env igang/build-cpp-env ./<build_folder>/BGP_simulation [path_to_config_file]

# example: docker run --rm -v "/Users/nickname/protocol_processing_security/simulation:/project" --name build_env igang/build-cpp-env ./cmake-release/BGP_simulation ./examples/config.yaml
```
