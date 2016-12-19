LOCAL_PATH := $(call my-dir)

# Defining paths
TOP_LEVEL_PATH := $(abspath $(LOCAL_PATH)/..)
#$(info TOP Level Path: $(TOP_LEVEL_PATH))

EXT_INSTALL_PATH = $(TOP_LEVEL_PATH)/third_party

OPENCV_ANDROID_SDK := $(EXT_INSTALL_PATH)/OpenCV-android-sdk
MINIGLOG_DIR := $(EXT_INSTALL_PATH)/miniglog
DLIB_DIR := $(TOP_LEVEL_PATH)/dlib

#========================== dlib static library ===================================
include $(CLEAR_VARS)
LOCAL_MODULE := dlib
LOCAL_C_INCLUDES := $(DLIB_DIR)
LOCAL_EXPORT_C_INCLUDES := $(DLIB_DIR)

LOCAL_SRC_FILES += \
    $(DLIB_DIR)/dlib/threads/threads_kernel_shared.cpp \
    $(DLIB_DIR)/dlib/entropy_decoder/entropy_decoder_kernel_2.cpp \
    $(DLIB_DIR)/dlib/base64/base64_kernel_1.cpp \
    $(DLIB_DIR)/dlib/threads/threads_kernel_1.cpp \
    $(DLIB_DIR)/dlib/threads/threads_kernel_2.cpp

include $(BUILD_STATIC_LIBRARY)

#========================== miniglog static library ================================
include $(CLEAR_VARS)
LOCAL_MODULE := miniglog
LOCAL_EXPORT_C_INCLUDES := $(MINIGLOG_DIR)
LOCAL_C_INCLUDES := $(MINIGLOG_DIR)
LOCAL_SRC_FILES := $(MINIGLOG_DIR)/glog/logging.cc

include $(BUILD_STATIC_LIBRARY)

#========================== android-hpe jni shared library ========================
include $(CLEAR_VARS)
OpenCV_INSTALL_MODULES := on
OPENCV_CAMERA_MODULES := off
OPENCV_LIB_TYPE := STATIC
include $(OPENCV_ANDROID_SDK)/sdk/native/jni/OpenCV.mk

LOCAL_MODULE := head_pose_det

LOCAL_C_INCLUDES +=  \
          $(OPENCV_ANDROID_SDK)/sdk/native/jni/include

LOCAL_SRC_FILES += \
    jni_head_pose_det.cpp \
    imageutils_jni.cpp \
    common/rgb2yuv.cpp \
    common/yuv2rgb.cpp \
    common/bitmap2mat2bitmap.cpp 

LOCAL_LDLIBS += -lm -llog -ldl -lz -ljnigraphics -latomic

# import static libraries
LOCAL_STATIC_LIBRARIES += dlib
LOCAL_STATIC_LIBRARIES += miniglog

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
    LOCAL_ARM_MODE := arm
	LOCAL_ARM_NEON := true
    #LOCAL_CFLAGS= -march=armv7-a -mfloat-abi=softfp -mfpu=neon
    #LOCAL_LDFLAGS= -march=armv7-a -Wl,--fix-cortex-a8
endif

include $(BUILD_SHARED_LIBRARY)
