#include "head_pose_estimation.hpp"
#include <opencv2/calib3d/calib3d.hpp>

using namespace dlib;
using namespace std;
using namespace cv;

inline Point2f toCv(const dlib::point& p) {
    return Point2f(p.x(), p.y());
}

HeadPoseEstimation::HeadPoseEstimation(const string& face_detection_model, int mod, 
    float fx, float fy, float cx, float cy, 
    float k1, float k2, float p1, float p2, float k3) {
    // Load face detection and pose estimation models.
    detector = get_frontal_face_detector();
    deserialize(face_detection_model) >> pose_model;
    mode = mod; // Set correct mode

    // Set cameraMatrix
    cv::Mat m = cv::Mat::zeros(3,3,CV_32F);
    cameraMatrix = m;
    cameraMatrix(0,0) = fx; // focalLengthX
    cameraMatrix(1,1) = fy; // focalLengthY
    cameraMatrix(0,2) = cx; // opticalCenterX
    cameraMatrix(1,2) = cy; // opticalCenterY
    cameraMatrix(2,2) = 1;
    LOG(INFO) << "Initialized Camera Matrix with:\n{fx} = " << fx << " {0}" << " {cx} = " << cx 
            << "\n{0}" << " {fy} = " << fy << " {cy} = " << cy <<
            "\n{0} {0} {1}";

    distCoeffs = (Mat1d(1, 5) << k1, k2, p1, p2, k3);
    LOG(INFO) << "Initialized Default Camera Distortion Matrix with:\n{k1} = " << k1 << " {k2} = " << k2 << " {p1} = " << p1  << " {p2} = " << p2<< " {k3} = " << k3;
}

int HeadPoseEstimation::detect(cv::Mat& image) {
    // If optical center contains default(=invalid values), use an estimate of them
    if(cameraMatrix(0,0) == 0) {
        cv::Mat m = cv::Mat::zeros(3,3,CV_32F);
        cameraMatrix = m;
        cameraMatrix(0,0) = 455.; // focalLength
        cameraMatrix(1,1) = 455.; // focalLength
        cameraMatrix(0,2) = image.cols / 2; // opticalCenterX
        cameraMatrix(1,2) = image.rows / 2; // opticalCenterY
        cameraMatrix(2,2) = 1;
        LOG(INFO) << "Initialized DEFAULT Camera Matrix with:\n{fx} = " << 455. << " {0}" << " {cx} = " << image.cols / 2 
            << "\n{0}" << " {fy} = " << 455. << " {cy} = " << image.rows / 2 
            << "\n{0} {0} {1}";

        distCoeffs = (Mat1d(1, 5) << 0, 0, 0, 0, 0);
        LOG(INFO) << "Initialized DEFAULT Camera Distortion Matrix with values to ZERO";
    }
    
    // Check that the image is valid
    if (image.empty()) return 0;

    // Set as current image
    current_image = dlib::cv_image<dlib::bgr_pixel>(image);
    // Perform detection
    faces = detector(current_image);
    // Put the results into a collection, and update how many found
    shapes.clear();
    int count = 0;
    for (auto face : faces){
        shapes.push_back(pose_model(current_image, face));
        count++;
    }

    // Get a clone to draw on
    resultMat = image.clone();

    // Draw lines for landmarks
    auto color = Scalar(0,255,0);
    for (unsigned long i = 0; i < shapes.size(); ++i)
    {
        const full_object_detection& d = shapes[i];

        for (unsigned long i = 1; i <= 16; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);

        for (unsigned long i = 28; i <= 30; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);

        for (unsigned long i = 18; i <= 21; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);
        for (unsigned long i = 23; i <= 26; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);
        for (unsigned long i = 31; i <= 35; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);
        line(resultMat, toCv(d.part(30)), toCv(d.part(35)), color, 2, CV_AA);

        for (unsigned long i = 37; i <= 41; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);
        line(resultMat, toCv(d.part(36)), toCv(d.part(41)), color, 2, CV_AA);

        for (unsigned long i = 43; i <= 47; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);
        line(resultMat, toCv(d.part(42)), toCv(d.part(47)), color, 2, CV_AA);

        for (unsigned long i = 49; i <= 59; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);
        line(resultMat, toCv(d.part(48)), toCv(d.part(59)), color, 2, CV_AA);

        for (unsigned long i = 61; i <= 67; ++i)
            line(resultMat, toCv(d.part(i)), toCv(d.part(i-1)), color, 2, CV_AA);
        line(resultMat, toCv(d.part(60)), toCv(d.part(67)), color, 2, CV_AA);
    }

    return count;
}

head_pose HeadPoseEstimation::pose(size_t face_idx) const {

    /*
        solvePnP

        Finds an object pose from 3D-2D point correspondences.

        C++: bool solvePnP(InputArray objectPoints, InputArray imagePoints, InputArray cameraMatrix, InputArray distCoeffs, OutputArray rvec, OutputArray tvec, bool useExtrinsicGuess=false, int flags=SOLVEPNP_ITERATIVE )
        
        Parameters: 
        objectPoints – Array of object points in the object coordinate space, 3xN/Nx3 1-channel or 1xN/Nx1 3-channel, where N is the number of points. vector<Point3f> can be also passed here.
        imagePoints – Array of corresponding image points, 2xN/Nx2 1-channel or 1xN/Nx1 2-channel, where N is the number of points. vector<Point2f> can be also passed here.
        cameraMatrix – Input camera matrix  A = \vecthreethree{fx}{0}{cx}{0}{fy}{cy}{0}{0}{1} .
        distCoeffs – Input vector of distortion coefficients (k_1, k_2, p_1, p_2[, k_3[, k_4, k_5, k_6],[s_1, s_2, s_3, s_4]]) of 4, 5, 8 or 12 elements. If the vector is NULL/empty, the zero distortion coefficients are assumed.
        rvec – Output rotation vector (see Rodrigues() ) that, together with tvec , brings points from the model coordinate system to the camera coordinate system.
        tvec – Output translation vector.
        useExtrinsicGuess – Parameter used for SOLVEPNP_ITERATIVE. If true (1), the function uses the provided rvec and tvec values as initial approximations of the rotation and translation vectors, respectively, and further optimizes them.
        flags –
        Method for solving a PnP problem:

        SOLVEPNP_ITERATIVE Iterative method is based on Levenberg-Marquardt optimization. In this case the function finds such a pose that minimizes projections error, that is the sum of squared distances between the observed projections imagePoints and the projected (using projectPoints() ) objectPoints .
        SOLVEPNP_P3P Method is based on the paper of X.S. Gao, X.-R. Hou, J. Tang, H.-F. Chang “Complete Solution Classification for the Perspective-Three-Point Problem”. In this case the function requires exactly four object and image points.
        SOLVEPNP_EPNP Method has been introduced by F.Moreno-Noguer, V.Lepetit and P.Fua in the paper “EPnP: Efficient Perspective-n-Point Camera Pose Estimation”.
        SOLVEPNP_DLS Method is based on the paper of Joel A. Hesch and Stergios I. Roumeliotis. “A Direct Least-Squares (DLS) Method for PnP”.
        SOLVEPNP_UPNP Method is based on the paper of A.Penate-Sanchez, J.Andrade-Cetto, F.Moreno-Noguer. “Exhaustive Linearization for Robust Camera Pose and Focal Length Estimation”. In this case the function also estimates the parameters f_x and f_y assuming that both have the same value. Then the cameraMatrix is updated with the estimated focal length.
        The function estimates the object pose given a set of object points, their corresponding image projections, as well as the camera matrix and the distortion coefficients.
    */

    std::vector<Point3f> head_points;
    std::vector<Point2f> detected_points;

    // Initializing the head pose 1m away, roughly facing the robot
    // This initialization is important as it prevents solvePnP to find the
    // mirror solution (head *behind* the camera)
    Mat tvec = (Mat_<double>(3,1) << 0., 0., 1000.);
    Mat rvec = (Mat_<double>(3,1) << 1.2, 1.2, -1.2);

    if(mode == MODE_ITERATIVE) {
        // List of 3D points
        head_points.push_back(P3D_SELLION);
        head_points.push_back(P3D_RIGHT_EYE);
        head_points.push_back(P3D_LEFT_EYE);
        head_points.push_back(P3D_RIGHT_EAR);
        head_points.push_back(P3D_LEFT_EAR);
        head_points.push_back(P3D_MENTON);
        head_points.push_back(P3D_NOSE);
        head_points.push_back(P3D_STOMMION);

        // List of 2D points
        detected_points.push_back(coordsOf(face_idx, SELLION));
        detected_points.push_back(coordsOf(face_idx, RIGHT_EYE));
        detected_points.push_back(coordsOf(face_idx, LEFT_EYE));
        detected_points.push_back(coordsOf(face_idx, RIGHT_SIDE));
        detected_points.push_back(coordsOf(face_idx, LEFT_SIDE));
        detected_points.push_back(coordsOf(face_idx, MENTON));
        detected_points.push_back(coordsOf(face_idx, NOSE));

        // Stommion is the mean point between upper and lower lip, I must calculate it since there's not such landmark
        auto stomion = (coordsOf(face_idx, MOUTH_CENTER_TOP) + coordsOf(face_idx, MOUTH_CENTER_BOTTOM)) * 0.5;
        detected_points.push_back(stomion);

        // Find the 3D pose of our head
        solvePnP(head_points, detected_points,
            cameraMatrix, distCoeffs,
            rvec, tvec, true, SOLVEPNP_ITERATIVE);
    } else if(mode == MODE_P3P) {
        // List of 3D points
        head_points.push_back(P3D_NOSE);
        head_points.push_back(P3D_RIGHT_EAR);
        head_points.push_back(P3D_LEFT_EAR);
        head_points.push_back(P3D_MENTON);

        // List of 2D points
        detected_points.push_back(coordsOf(face_idx, NOSE));
        detected_points.push_back(coordsOf(face_idx, RIGHT_SIDE));
        detected_points.push_back(coordsOf(face_idx, LEFT_SIDE));
        detected_points.push_back(coordsOf(face_idx, MENTON));

        // Find the 3D pose of our head
        solvePnP(head_points, detected_points,
            cameraMatrix, distCoeffs,
            rvec, tvec, true, SOLVEPNP_P3P);
    } else if(mode == MODE_EPNP) {
        // List of 3D points
        head_points.push_back(P3D_SELLION);
        head_points.push_back(P3D_RIGHT_EYE);
        head_points.push_back(P3D_LEFT_EYE);
        head_points.push_back(P3D_RIGHT_EAR);
        head_points.push_back(P3D_LEFT_EAR);
        head_points.push_back(P3D_MENTON);
        head_points.push_back(P3D_NOSE);
        head_points.push_back(P3D_STOMMION);

        // List of 2D points
        detected_points.push_back(coordsOf(face_idx, SELLION));
        detected_points.push_back(coordsOf(face_idx, RIGHT_EYE));
        detected_points.push_back(coordsOf(face_idx, LEFT_EYE));
        detected_points.push_back(coordsOf(face_idx, RIGHT_SIDE));
        detected_points.push_back(coordsOf(face_idx, LEFT_SIDE));
        detected_points.push_back(coordsOf(face_idx, MENTON));
        detected_points.push_back(coordsOf(face_idx, NOSE));

        // Stommion is the mean point between upper and lower lip, I must calculate it since there's not such landmark
        auto stomion = (coordsOf(face_idx, MOUTH_CENTER_TOP) + coordsOf(face_idx, MOUTH_CENTER_BOTTOM)) * 0.5;
        detected_points.push_back(stomion);

        // Find the 3D pose of our head
        solvePnP(head_points, detected_points,
            cameraMatrix, distCoeffs,
            rvec, tvec, true, SOLVEPNP_EPNP);
    }

    Matx33d rotation;
    Rodrigues(rvec, rotation);

    head_pose pose = {
        rotation(0,0),    rotation(0,1),    rotation(0,2),    tvec.at<double>(0)/1000,
        rotation(1,0),    rotation(1,1),    rotation(1,2),    tvec.at<double>(1)/1000,
        rotation(2,0),    rotation(2,1),    rotation(2,2),    tvec.at<double>(2)/1000,
                    0,                0,                0,                     1
    };

    // Istantiate head_points, reproject them with rvec and tvec, and draw them onto the resultMat
    std::vector<Point2f> reprojected_points;
    projectPoints(head_points, rvec, tvec, cameraMatrix, noArray(), reprojected_points);

    for (auto point : reprojected_points) {
        circle(resultMat, point,2, Scalar(0,255,255), 2);
    }

    // Istantiate axes, reproject them with rvec and tvec, and draw them onto the resultMat
    std::vector<Point3f> axes;
    axes.push_back(Point3f(0,0,0));
    axes.push_back(Point3f(50,0,0));
    axes.push_back(Point3f(0,50,0));
    axes.push_back(Point3f(0,0,50));

    std::vector<Point2f> projected_axes;
    projectPoints(axes, rvec, tvec, cameraMatrix, noArray(), projected_axes);

    line(resultMat, projected_axes[0], projected_axes[3], Scalar(255,0,0),2,CV_AA);
    line(resultMat, projected_axes[0], projected_axes[2], Scalar(0,255,0),2,CV_AA);
    line(resultMat, projected_axes[0], projected_axes[1], Scalar(0,0,255),2,CV_AA);

    return pose;
}

std::vector<head_pose> HeadPoseEstimation::poses() const {
    std::vector<head_pose> res;
    for (auto i = 0; i < faces.size(); i++){
        res.push_back(pose(i));
    }
    return res;
}

/** Return the point corresponding to the dictionary marker.
*/
Point2f HeadPoseEstimation::coordsOf(size_t face_idx, FACIAL_FEATURE feature) const {
    return toCv(shapes[face_idx].part(feature));
}

/** Returns true if the lines intersect (and set r to the intersection
 *  coordinates), false otherwise.
 */
// Finds the intersection of two lines, or returns false.
// The lines are defined by (o1, p1) and (o2, p2).
// taken from: http://stackoverflow.com/a/7448287/828379
bool HeadPoseEstimation::intersection(Point2f o1, Point2f p1, Point2f o2, Point2f p2, Point2f &r) const {
    Point2f x = o2 - o1;
    Point2f d1 = p1 - o1;
    Point2f d2 = p2 - o2;

    float cross = d1.x*d2.y - d1.y*d2.x;
    if (abs(cross) < /*EPS*/1e-8)
        return false;

    double t1 = (x.x * d2.y - x.y * d2.x)/cross;
    r = o1 + d1 * t1;
    return true;
}