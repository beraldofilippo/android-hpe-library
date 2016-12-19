NDK_TOOLCHAIN_VERSION := clang
APP_ABI := armeabi-v7a arm64-v8a
# armeabi-v7a x86 arm64-v8a x86_64
APP_CPPFLAGS := -std=c++11 -frtti -fexceptions
APP_PLATFORM := android-8
APP_STL := gnustl_static