#ifndef BITMAP2MAT2BITMAP_H
#define BITMAP2MAT2BITMAP_H

#include <android/bitmap.h>
#include <glog/logging.h>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

/*
 *     Author : TzuTaLin
 * Created on : 07/25 2016
 *
 * Copyright (c) 2016 Tzutalin. All rights reserved.
 */
namespace jnicommon {

void ConvertBitmapToRGBAMat(JNIEnv * env, jobject& bitmap, cv::Mat& dst, bool needUnPremultiplyAlpha, bool flipX, bool flipY);

void ConvertRGBAMatToBitmap(JNIEnv * env, jobject& bitmap, cv::Mat& src, bool needPremultiplyAlpha);

} //end jnicommon

#endif /* BITMAP2MAT2BITMAP_H */
