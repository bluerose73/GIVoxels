# GIVoxels

An OpenGL implementation of Voxel Cone Tracing.

## Build

This is a CMake project, and this repo has included all dependencies in ``thirdparty/`` .
To build the project, run

```shell
mkdir build
cd build
cmake ..
cmake --build .
```

The first build run may last several minutes, because it has to build all those dependencies.

## Run

Find ``app`` or ``app.exe`` in ``build/`` directory. Run it.

## Configuration

You can config run-time behaviour of the shading demo by modifying ``config.json``.

The most important option is ``"mode"``. It can be one of the following values:

- gi: global illuminaiton
- voxels: visualize voxels
- voxels_w_direct_light: visualize voxels with direct light
- direct_light: direct lighting only

## Credits

Our code is built on top of [LearnOpenGL](learnopengl.com) (See ``thirdparty/learnopengl``).

Other Implementations on GitHub:

- [VXGI](https://github.com/mcela/vxgi)
- [VcTest](https://github.com/AdamYuan/VcTest)