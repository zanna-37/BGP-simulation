# Run the program
To run the program open the terminal in the project root folder. The path to the executable depends on the command use for the build.

```bash
./<build_folder>/BGP_simulation [config_file]
```

- `build_folder`
    - The build folder can be `cmake-debug` or `cmake-release`.
- `config_file`
    - The path to the configuration file to use.

## Example
```bash
./cmake-release/BGP_simulation ./examples/config.yaml
```
