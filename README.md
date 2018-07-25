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

Android NDK:
```
$ ndk-build NDK_PROJECT_PATH=. NDK_APPLICATION_MK=Application.mk APP_BUILD_SCRIPT=Android.mk
$ adb push libs/arm64-v8a/vkcl /data/local/tmp ; adb shell "chmod 755 /data/local/tmp/vkcl"
$ adb push vkcl.spv /data/local/tmp
$ adb shell "/data/local/tmp/vkcl /data/local/tmp/vkcl.spv"
```
