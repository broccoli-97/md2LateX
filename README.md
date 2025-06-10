# md2LateX

md2LateX is a C++ project that converts Markdown to LaTeX. The project uses external libraries such as CURL and nlohmann_json, managed via vcpkg, and follows modern C++ best practices.

**Note:** A significant portion of this project was generated with the assistance of AI coding tools, helping streamline development and code quality.

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

## MIT License

```
MIT License

Copyright (c) [Year] [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

Replace `[2025]` and `[Lach]` with the appropriate details.
