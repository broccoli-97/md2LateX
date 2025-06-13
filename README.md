# md2LateX

md2LateX is a C++ project that converts Markdown to LaTeX. The project uses external libraries such as CURL and nlohmann_json, managed via vcpkg, and follows modern C++ best practices.

**Note:** The code for this project was essentially generated by AI.

## Project Structure

```
md2LateX/
├── CMakeLists.txt         # Top-level CMake configuration
├── inc/                   # Header files
└── src/
    ├── app/               # Application entry point (main.cpp)
    └── lib/               # Library code (e.g., md_converter.cpp)
```

- **inc/**: Contains all the header files used throughout the project.
- **src/lib/**: Contains the core library code with the implementation logic.
- **src/app/**: Contains the executable that uses the library.

## Building the Project

This project uses CMake as its build system.

1. **Configure the project:**

   Open the integrated terminal in VS Code and run:

   ```shell
   cmake -S . -B build
   ```

2. **Build the project:**

   ```shell
   cmake --build build
   ```

3. **Run the executable:**
   The executable is typically located in the `build` directory after a successful build:
   ```shell
   ./build/src/app/your_executable_name
   ```

## Dependencies

- [CURL](https://curl.se/libcurl/)
- [nlohmann_json](https://github.com/nlohmann/json)
- Managed using [vcpkg](https://github.com/microsoft/vcpkg)

Ensure that you have installed these dependencies via vcpkg and set the CMake toolchain file in your VS Code settings, e.g., in `.vscode/settings.json`:

```json
{
  "cmake.toolchainFile": "path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
}
```

## Code Quality

The project utilizes clang-tidy for static code analysis. Adjust the configuration in the `.clang-tidy` file as needed.
