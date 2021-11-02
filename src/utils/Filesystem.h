#ifndef BGP_SIMULATION_UTILS_FILESYSTEM_H
#define BGP_SIMULATION_UTILS_FILESYSTEM_H

#include <filesystem>

std::filesystem::path getExecutableParentPath() {
    // TODO cache the result for following invocations
    return std::filesystem::canonical("/proc/self/exe").parent_path();
}

#endif  // BGP_SIMULATION_UTILS_FILESYSTEM_H
