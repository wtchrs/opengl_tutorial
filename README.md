# OpenGL Learning

## Get started

Ensure that `vcpkg` is installed on your computer.

Add the `CMakeUserPresets.json` file to the project root directory:

```json
{
  "version": 2,
  "configurePresets": [
    {
      "name": "default",
      "inherits": "vcpkg",
      "environment": {
        "VCPKG_ROOT": "<path to vcpkg>"
      }
    }
  ]
}
```

`configurePresets[0].environment.VCPKG_ROOT` is not necessary if you have already added this as an environment variable.

Run the following commands to build and launch this project:

```sh
cmake --preset default
cmake --build build
./build/opengl_tutorial
```

If you want to know more about these `vcpkg` and `cmake` settings, visit https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash

