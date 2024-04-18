#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/aruco_dictionary.hpp>

using namespace cv;

int detectArucoMarker19() {
	std::string image_path = "C:/dev/code/visual-studio/tiltfive-diagnostic-cpp/tiltfive-diagnostic-cpp/aruco-capture.png";
	std::cout << "Image Path: " << image_path << std::endl;
	cv::Mat img = cv::imread(image_path, IMREAD_COLOR);

	if (img.empty()) {
		std::cout << "Could not read image" << std::endl;
		return 1;
	}

	std::vector<int> markerIds;
	std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
	cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
	cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
	cv::aruco::ArucoDetector detector(dictionary, detectorParams);
	detector.detectMarkers(img, markerCorners, markerIds, rejectedCandidates);

	cv::Mat outputImage = img.clone();
	cv::aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);

	cv::namedWindow("Test Window", cv::WINDOW_AUTOSIZE);
	cv::imshow("Test Window", outputImage);
	return cv::waitKey(0);
}

int main() {
	detectArucoMarker19();

	return 0;
}