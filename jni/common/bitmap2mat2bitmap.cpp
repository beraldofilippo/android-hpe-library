/*
 * bitmap2mat.cpp using google-style
 *
 *     Author : TzuTaLin
 * Created on : 07/25 2016
 *
 * Copyright (c) 2016 Tzutalin. All rights reserved.
 */

#include <common/bitmap2mat2bitmap.h>
namespace jnicommon{

using namespace cv;

void ConvertBitmapToRGBAMat(JNIEnv* env, jobject& bitmap, Mat& dst,
                                   bool needUnPremultiplyAlpha,
                                   bool flipX,
                                   bool flipY) {
  AndroidBitmapInfo info;
  void* pixels = 0;

  try {
    CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
    CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
              info.format == ANDROID_BITMAP_FORMAT_RGB_565);
    CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
    CV_Assert(pixels);
    dst.create(info.height, info.width, CV_8UC4);
    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
      LOG(INFO) << "nBitmapToMat: RGBA_8888 -> CV_8UC4";
      Mat tmp(info.height, info.width, CV_8UC4, pixels);
      if (needUnPremultiplyAlpha)
        cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
      else
        tmp.copyTo(dst);
    } else {
      // info.format == ANDROID_BITMAP_FORMAT_RGB_565
      LOG(INFO) << "nBitmapToMat: RGB_565 -> CV_8UC4";
      Mat tmp(info.height, info.width, CV_8UC2, pixels);
      cvtColor(tmp, dst, COLOR_BGR5652RGBA);
    }
    // Perform flip if required
    if(flipX) {
        flip(dst, dst, 0);
    } else if(flipY) {
        flip(dst, dst, 1);
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    return;
  } catch (const cv::Exception& e) {
    AndroidBitmap_unlockPixels(env, bitmap);
    LOG(FATAL) << "nBitmapToMat catched cv::Exception:" << e.what();
    jclass je = env->FindClass("org/opencv/core/CvException");
    if (!je) je = env->FindClass("java/lang/Exception");
    env->ThrowNew(je, e.what());
    return;
  } catch (...) {
    AndroidBitmap_unlockPixels(env, bitmap);
    LOG(FATAL) << "nBitmapToMat catched unknown exception (...)";
    jclass je = env->FindClass("java/lang/Exception");
    env->ThrowNew(je, "Unknown exception in JNI code {nBitmapToMat}");
    return;
  }
}

void ConvertRGBAMatToBitmap(JNIEnv * env, jobject& bitmap, cv::Mat& src, bool needPremultiplyAlpha)
{
    AndroidBitmapInfo  info;
    void*              pixels = 0;

    try {
        CV_Assert( AndroidBitmap_getInfo(env, bitmap, &info) >= 0 );
        CV_Assert( info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                   info.format == ANDROID_BITMAP_FORMAT_RGB_565 );
        CV_Assert( src.dims == 2 && info.height == (uint32_t)src.rows && info.width == (uint32_t)src.cols );
        CV_Assert( src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4 );
        CV_Assert( AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0 );
        CV_Assert( pixels );
        if( info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 )
        {
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if(src.type() == CV_8UC1)
            {
                LOG(INFO) << "nMatToBitmap: CV_8UC1 -> RGBA_8888";
                cvtColor(src, tmp, COLOR_GRAY2RGBA);
            } else if(src.type() == CV_8UC3){
                LOG(INFO) << "nMatToBitmap: CV_8UC3 -> RGBA_8888";
                cvtColor(src, tmp, COLOR_RGB2RGBA);
            } else if(src.type() == CV_8UC4){
                LOG(INFO) << "nMatToBitmap: CV_8UC4 -> RGBA_8888";
                if(needPremultiplyAlpha) cvtColor(src, tmp, COLOR_RGBA2mRGBA);
                else src.copyTo(tmp);
            }
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            if(src.type() == CV_8UC1)
            {
                LOG(INFO) << "nMatToBitmap: CV_8UC1 -> RGB_565";
                cvtColor(src, tmp, COLOR_GRAY2BGR565);
            } else if(src.type() == CV_8UC3){
                LOG(INFO) << "nMatToBitmap: CV_8UC3 -> RGB_565";
                cvtColor(src, tmp, COLOR_RGB2BGR565);
            } else if(src.type() == CV_8UC4){
                LOG(INFO) << "nMatToBitmap: CV_8UC4 -> RGB_565";
                cvtColor(src, tmp, COLOR_RGBA2BGR565);
            }
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch(const cv::Exception& e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOG(FATAL) << "nMatToBitmap catched cv::Exception: %s", e.what();
        jclass je = env->FindClass("org/opencv/core/CvException");
        if(!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOG(FATAL) << "nMatToBitmap catched unknown exception (...)";
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nMatToBitmap}");
        return;
    }
}

}  // end jnicommon
