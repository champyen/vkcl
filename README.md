# vkcl
OpenCL Kernel with Vulkan Host Example

Derived from https://gist.github.com/sheredom/523f02bbad2ae397d7ed255f3f3b5a7f

clspv is required to compile CL kernel

https://github.com/google/clspv

Vulkan Toolchian is required to compile GLSL compute shader

https://vulkan.lunarg.com/sdk/home

vkcl.comp is compiled by:
```
$ glslangValidator --target-env vulkan1.0 -e vkcl --source-entrypoint main vkcl.comp -o vkcl.spv
```

Execution Result:
```
$ ./vkcl vkcl.spv
queueFamilyIndex 0
spir-v size 812
DONE
```
