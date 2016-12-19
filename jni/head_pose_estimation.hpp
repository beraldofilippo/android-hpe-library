#ifndef __HEAD_POSE_ESTIMATION
#define __HEAD_POSE_ESTIMATION

#include <opencv2/core/core.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>

#include <vector>
#include <array>
#include <string>

const static cv::Point3f P3D_SELLION(0., 0.,0.);
const static cv::Point3f P3D_RIGHT_EYE(-20., -65.5,-5.);
const static cv::Point3f P3D_LEFT_EYE(-20., 65.5,-5.);
const static cv::Point3f P3D_RIGHT_EAR(-100., -77.5,-6.);
const static cv::Point3f P3D_LEFT_EAR(-100., 77.5,-6.);
const static cv::Point3f P3D_NOSE(21.0, 0., -48.0);
const static cv::Point3f P3D_STOMMION(10.0, 0., -75.0);
const static cv::Point3f P3D_MENTON(0., 0.,-133.0);

static const int MAX_FEATURES_TO_TRACK=100;

// Interesting facial features with their landmark index
enum FACIAL_FEATURE {
    NOSE=30,
    RIGHT_EYE=36,
    LEFT_EYE=45,
    RIGHT_SIDE=0,
    LEFT_SIDE=16,
    EYEBROW_RIGHT=21,
    EYEBROW_LEFT=22,
    MOUTH_UP=51,
    MOUTH_DOWN=57,
    MOUTH_RIGHT=48,
    MOUTH_LEFT=54,
    SELLION=27,
    MOUTH_CENTER_TOP=62,
    MOUTH_CENTER_BOTTOM=66,
    MENTON=8
};

const static int MODE_ITERATIVE = 0;
const static int MODE_P3P = 1;
const static int MODE_EPNP = 2;

typedef cv::Matx44d head_pose;

class HeadPoseEstimation {

public:
    HeadPoseEstimation(const std::string& face_detection_model = "shape_predictor_68_face_landmarks.dat", 
        int mode=MODE_ITERATIVE, 
        float fx = 0, 
        float fy = 0, 
        float cx = 0, 
        float cy = 0,
        float k1 = 0, 
        float k2 = 0, 
        float p1 = 0, 
        float p2 = 0,
        float k3 = 0);

    int detect(cv::Mat& image);

    head_pose pose(size_t face_idx) const;

    std::vector<head_pose> poses() const;

    virtual inline double todeg(double rad) {  return rad * 180 / M_PI; }

    cv::Matx33f cameraMatrix;
    /*
        cameraMatrix – Input camera matrix  A =   [{fx},  {0},    {cx},
                                                {0},    {fy},   {cy},
                                                {0},    {0},    {1}]
    */
    cv::Mat1f distCoeffs;
    /*
        distCoeffs – Input vector of distortion coefficients (k_1, k_2, p_1, p_2[, k_3[, k_4, k_5, k_6],[s_1, s_2, s_3, s_4]])
    */

    mutable cv::Mat resultMat;

    int mode;

private:
    dlib::cv_image<dlib::bgr_pixel> current_image;

    dlib::frontal_face_detector detector;
    dlib::shape_predictor pose_model;

    std::vector<dlib::rectangle> faces;

    std::vector<dlib::full_object_detection> shapes;

    /** Return the point corresponding to the dictionary marker.
    */
    cv::Point2f coordsOf(size_t face_idx, FACIAL_FEATURE feature) const;

    /** Returns true if the lines intersect (and set r to the intersection
     *  coordinates), false otherwise.
     */
    bool intersection(cv::Point2f o1, cv::Point2f p1,
                      cv::Point2f o2, cv::Point2f p2,
                      cv::Point2f &r) const;
};

#endif // __HEAD_POSE_ESTIMATION
