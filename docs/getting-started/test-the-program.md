# Test the program
To test the program open the terminal in the project root folder.

## Valgrind
Valgrind is a good tool to spot memory leaks and other memory-related errors.

⚠️ We suggest you to compile the program with the `make DEBUG=1 build` command because it adds useful details to the
built files. For example, it enables the use of the debugger, and it enriches the executable with information from the
source code that helps to track down bugs.

### Run locally
💡 Remember to compile first. See [here](./compile-code.md).

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./<build_folder>/BGP_simulation [path_to_config_file]

# example: valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./cmake-debug/BGP_simulation ./examples/config.yaml
```

### Run through Docker
💡 Remember to compile first. See [here](./compile-code.md).

```bash
docker-compose run --rm compiler valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./<build_folder>/BGP_simulation [path_to_config_file]

# example: docker-compose run --rm compiler valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./cmake-debug/BGP_simulation ./examples/config.yaml
```
