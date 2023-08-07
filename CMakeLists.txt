cmake_minimum_required(VERSION 3.21)

# 设置NDK路径
set(NDK_PATH C:/Users/Administrator/AppData/Local/Android/Sdk/ndk/25.1.8937393)

##################### Android设置 #####################
set(CMAKE_SYSTEM_NAME ANDROID) # 设置目标编译平台参数 Android
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_SYSTEM_VERSION 21) # 系统版本
set(ANDROID_PLATFORM 21) # 平台版本
set(ANDROID_SDK_TOOLS_VERSION 20) # 平台版本
set(ANDROID_ABI arm64-v8a) # 设置目标构架 armeabi-v7a arm64-v8a x86 x86_64
set(ANDROID_NDK ${NDK_PATH}) # 设置ndk路径
set(CMAKE_TOOLCHAIN_FILE ${NDK_PATH}/build/cmake/android.toolchain.cmake)
set(ANDROID_SDK_ROOT ${NDK_PATH})


project(NativeSurface)

set(CMAKE_CXX_STANDARD 17)

##################### 输出文件重定向 #####################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_SOURCE_DIR}/outputs/${CMAKE_ANDROID_ARCH_ABI}/
        ) # 重定向输出产物(可执行文件)
##################### 输出文件重定向 #####################

##################### CMake头文件设置 #####################
# 遍历子目录下所有符合情况的头文件
FILE(GLOB_RECURSE FILE_INCLUDES include/*.h*)
include_directories(# 设置全局头文件目录 使其他源码文件可在任意目录<头文件.h>
        include/
        include/ImGui
        include/Android_touch
        include/Android_shm
        include/native_surface
        include/Android_draw
        include/Android_read
        include/tools
        ffmpeg/include
        my_android_opencv/include
        my_android_opencv/modules/highgui/include
        my_android_opencv/modules/imgcodecs/include
        my_android_opencv/modules/imgproc/include
        my_android_opencv/modules/core/include
        my_android_opencv/modules/video/include
        my_android_opencv/modules/videoio/include
        my_android_opencv/modules/world/include
        my_android_opencv/modules/dnn/include
        my_android_opencv/modules/ml/include
        my_android_opencv/modules/features2d/include
        my_android_opencv/modules/calib3d/include
        my_android_opencv/modules/flann/include
        my_android_opencv/modules/objdetect/include
        my_android_opencv/modules/photo/include
        my_android_opencv/modules/stitching/include
        )
##################### CMake头文件设置 #####################

##################### CMake源文件设置 #####################
FILE(GLOB_RECURSE FILE_SOURCES # 遍历子目录下所有符合情况的源文件
        src/source/*.c*
        )
##################### CMake源文件设置 #####################

##################### 设置三方库文件目录 #####################
link_directories(
        ffmpeg/lib/${ANDROID_ABI})

##################### 设置三方库文件目录 #####################

##################### 添加产物 #####################
add_executable(NativeSurface # 生成可执行文件
        ${FILE_INCLUDES} # 头文件
        ${FILE_SOURCES} # 源文件
        src/surface.cpp
        )




target_link_libraries(NativeSurface PRIVATE EGL GLESv3 log
        android
#[[        GLESv2 m dl mediandk avformat
        avcodec
        avfilter
        swresample
        swscale
        avutil]]
        )