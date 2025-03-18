# OpenGL Learning

## Getting Started

Clone this repository:

```sh
git clone https://github.com/wtchrs/opengl_tutorial
cd opengl_tutorial
```

Clone and bootstrap `vcpkg` (skip if you are using the global installation):

```sh
git clone --depth=1 https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh
```

Add the `CMakeUserPresets.json` file to the project root directory:

```json
{
  "version": 2,
  "configurePresets": [
    {
      "name": "default",
      "inherits": "vcpkg",
      "environment": {
        "VCPKG_ROOT": "${sourceDir}/vcpkg"
      }
    }
  ]
}
```

Adjust the `environment` setting to suit your system.

Run the following commands to build and launch this project:

```sh
cmake --preset default
cmake --build build
./build/ibl_test # or any other examples
```

For more details on configuring `vcpkg` and `cmake`, visit https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash

