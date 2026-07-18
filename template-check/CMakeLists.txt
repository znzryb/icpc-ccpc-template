cmake_minimum_required(VERSION 4.1)
project(template_check)

set(CMAKE_CXX_STANDARD 20)

add_executable(template_check main.cpp)

# 保证跨平台一致性。（实际上是为了关闭 GCC 的C++20 Modules 的实验性支持，是避免宏冲突导致报错）
set(CMAKE_CXX_EXTENSIONS OFF)

# 不能解决 debug 示数不可信的问题，应该已经设置过了
# set(CMAKE_BUILD_TYPE Debug)

# [MacOS GCC Fix] 解决系统库缺失
if (APPLE)
    # 指定 Xcode SDK 路径，解决 "library 'System' not found"
    set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk" CACHE PATH "" FORCE)
endif ()

add_compile_definitions(LOCAL)

# 这个不要随便启用，启用后可能 debug 示数不可信
# add_compile_definitions(_GLIBCXX_DEBUG)

# add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
# add_link_options(-fsanitize=address,undefined)

add_executable(XOR "src/XOR.cpp")
add_executable(P3292_SCOI2016_ "src/P3292_SCOI2016_.cpp")
add_executable(P2613 "src/P2613.cpp")
add_executable(P3372 "src/P3372.cpp")
add_executable(HDU6579 "src/HDU6579.cpp")
add_executable(rng_benchmark "src/rng_benchmark/benchmark.cpp")
add_executable(OpenJBailian1811 "src/OpenJBailian1811.cpp")