#include <android/bitmap.h>
#include <common/bitmap2mat2bitmap.h>
#include <jni.h>
#include <glog/logging.h>
#include "head_pose_estimation.cpp"
#include "LinearMath/Matrix3x3.h"

using namespace std;
using namespace cv;

namespace {
  std::shared_ptr<HeadPoseEstimation> gHeadPoseEstimationPtr;
}

#ifdef __cplusplus
extern "C" {
#endif

static jclass HeadPoseGaze;
  static jmethodID HeadPoseGazeConstructor;

static jclass ArrayList;
  static jmethodID ArrayListAdd;

// ========================================================
// JNI Mapping Methods
// ========================================================
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  JNIEnv* env = NULL;

  if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
    LOG(FATAL) << "JNI_OnLoad ERROR";
    return JNI_ERR;
  }
  
  LOG(INFO) << "JNI_OnLoad OK";

  return JNI_VERSION_1_6;
}

// Macro to define correctly native method names
#define DLIB_JNI_METHOD(METHOD_NAME) \
  Java_edu_unipd_dei_dlib_HeadPoseDetector_##METHOD_NAME

jint JNIEXPORT JNICALL DLIB_JNI_METHOD(jniBitmapExtractFaceGazes)(JNIEnv* env, jobject thiz,
            jobject bitmap,
					  jobject gazesList) {

  if (gHeadPoseEstimationPtr) {
    cv::Mat rgbaMat;
    cv::Mat bgrMat;
    jnicommon::ConvertBitmapToRGBAMat(env, bitmap, rgbaMat, true, false, false);
    cv::cvtColor(rgbaMat, bgrMat, cv::COLOR_RGBA2BGR);
    
    jint size = gHeadPoseEstimationPtr->detect(bgrMat);
    LOG(INFO) << "Number of faces detected: " << size;

    auto poses = gHeadPoseEstimationPtr->poses();

    int i = 0;
    jobject gaze_found = NULL;
    LOG(INFO) << "{";
    for(auto pose : poses) {
        pose = pose.inv();

        double raw_yaw, raw_pitch, raw_roll;
        tf::Matrix3x3 mrot(
                pose(0,0), pose(0,1), pose(0,2),
                pose(1,0), pose(1,1), pose(1,2),
                pose(2,0), pose(2,1), pose(2,2));
        mrot.getRPY(raw_roll, raw_pitch, raw_yaw);

        raw_roll = raw_roll - M_PI/2;
        raw_yaw = raw_yaw + M_PI/2;

        double yaw, pitch, roll;

        roll = raw_pitch;
        yaw = raw_yaw;
        pitch = -raw_roll;

        LOG(INFO) << "\"face_" << i << "\":";

        LOG(INFO) << setprecision(1) << fixed << "{\"yaw\":" << 
          gHeadPoseEstimationPtr->todeg(yaw) << ", \"pitch\":" << 
          gHeadPoseEstimationPtr->todeg(pitch) << ", \"roll\":" << 
          gHeadPoseEstimationPtr->todeg(roll) << ",";

        LOG(INFO) << setprecision(4) << fixed << 
          "\"x\":" << pose(0,3) << ", \"y\":" << pose(1,3) << 
          ", \"z\":" << pose(2,3) << "},";

        i++;
        // Call add method on an object created from another method call
        gaze_found = env->NewObject(HeadPoseGaze, HeadPoseGazeConstructor, 
          gHeadPoseEstimationPtr->todeg(yaw), 
          gHeadPoseEstimationPtr->todeg(pitch), 
          gHeadPoseEstimationPtr->todeg(roll));
        env->CallBooleanMethod(gazesList, ArrayListAdd, gaze_found);
    }
    LOG(INFO) << "}" << flush;

    // Produce the bitmap to display
    cv::Mat rgbaResultMat;
    cv::cvtColor(gHeadPoseEstimationPtr -> resultMat, rgbaResultMat, cv::COLOR_BGR2RGBA);
    jnicommon::ConvertRGBAMatToBitmap(env, bitmap, rgbaResultMat, true);

    return JNI_OK;
  } else return JNI_ERR;
}

jint JNIEXPORT JNICALL DLIB_JNI_METHOD(jniInit)(JNIEnv* env, jobject thiz,
            jstring landmarkPath,
            jint mode,
            jfloat fx,
            jfloat fy,
            jfloat cx,
            jfloat cy,
            jfloat k1,
            jfloat k2,
            jfloat p1,
            jfloat p2,
            jfloat k3) {
  // Initialize a new estimator if it's not already there
  if (!gHeadPoseEstimationPtr) {
    const char* landmarkmodel_path = env->GetStringUTFChars(landmarkPath, 0);
    LOG(INFO) << "Initializing new HeadPoseEstimation, landmarkPath " << landmarkmodel_path << " and mode "<< mode << "and some params...";
    gHeadPoseEstimationPtr = std::make_shared<HeadPoseEstimation>(landmarkmodel_path, mode, fx, fy, cx, cy, k1, k2, p1, p2, k3);
    env->ReleaseStringUTFChars(landmarkPath, landmarkmodel_path);
  }

  // Initialize references to classes and methods
  jclass HeadPoseGaze_local = env->FindClass("edu/unipd/dei/dlib/HeadPoseGaze");
  if(HeadPoseGaze_local == NULL) {
      LOG(FATAL) << "Can't Find Class \"HeadPoseGaze\".\n";
      return JNI_ERR;
  }
  
  jclass ArrayList_local = env->FindClass("java/util/ArrayList");
  if(ArrayList_local == NULL) {
      LOG(FATAL) << "Can't Find Class \"ArrayList\".\n";
      return JNI_ERR;
  }

  // Obtain global refs...
  HeadPoseGaze = reinterpret_cast<jclass>(env->NewGlobalRef(HeadPoseGaze_local));
  HeadPoseGazeConstructor = env->GetMethodID(HeadPoseGaze, "<init>", "(DDD)V");
  if(HeadPoseGazeConstructor == NULL) {
      LOG(FATAL) << "Can't Find Method(s) in HeadPoseGaze.\n";
      return JNI_ERR;
  }

  ArrayList = reinterpret_cast<jclass>(env->NewGlobalRef(ArrayList_local));
  ArrayListAdd = env->GetMethodID(ArrayList, "add", "(Ljava/lang/Object;)Z");
  if(ArrayListAdd == NULL) {
      LOG(FATAL) << "Can't Find Method(s) in ArrayList.\n";
      return JNI_ERR;
  }

  // ...delete local refs
  env->DeleteLocalRef(HeadPoseGaze_local);
  env->DeleteLocalRef(ArrayList_local);

  LOG(INFO) << "Classes and method references init OK";

  return JNI_OK;
}

jint JNIEXPORT JNICALL DLIB_JNI_METHOD(jniDeInit)(JNIEnv* env, jobject thiz) {
  gHeadPoseEstimationPtr.reset();
  env->DeleteGlobalRef(HeadPoseGaze);
  env->DeleteGlobalRef(ArrayList);
  
  return JNI_OK;
}

#ifdef __cplusplus
}
#endif