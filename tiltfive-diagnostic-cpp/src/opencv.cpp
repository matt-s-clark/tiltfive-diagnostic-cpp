#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/aruco_dictionary.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <iostream>


using namespace cv;

static std::string roundNum(float num)
{
	double value = std::round(num * 1000.0) / 1000.0;
	std::string num_text = std::to_string(value);

	return num_text.substr(0, num_text.find(".") + 4);
}

static int displayCapturedTiltFiveImage()
{
	std::string image_path = "C:/dev/code/visual-studio/tiltfive-diagnostic-cpp/tiltfive-diagnostic-cpp/saved-frame.png";
	std::cout << "Image Path: " << image_path << std::endl;
	cv::Mat img = cv::imread(image_path, IMREAD_COLOR);

	if (img.empty())
	{
		std::cout << "Could not read image" << std::endl;
		return 1;
	}

	cv::namedWindow("Test Window", cv::WINDOW_AUTOSIZE);
	cv::imshow("Test Window", img);
	return cv::waitKey(0);
}

bool generateMarker(int markerId)
{
	cv::Mat markerImage;
	cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
	cv::aruco::generateImageMarker(dictionary, markerId, 200, markerImage, 1);

	cv::imshow("Marker", markerImage);
	int keyId = cv::waitKey(0);

	const int P_KEY = 112;
	const int Q_KEY = 113;

	if (keyId == P_KEY)
	{
		const std::string fileName = "marker" + std::to_string(markerId) + ".png";
		cv::imwrite(fileName, markerImage);
	}
	else if (keyId == Q_KEY) {
		return true;
	}

	return false;
}

static int showArucoMarkers()
{
	cv::namedWindow("Marker", cv::WINDOW_AUTOSIZE);

	for (int i = 0; i < 250; i++)
	{
		std::cout << "\rImage id: " << i;
		bool quitPressed = generateMarker(i);

		if (quitPressed)
		{

			std::cout << "\n\nQuit pressed\n";
			return 0;
		}
	}

	std::cout << "\n\Cycled through all images. Quitting.\n";

	return 0;
}

static int showArucoGrid()
{
	const int MARKER_SIDE = 220;
	const int MARGIN = 20;
	const int ROWS = 6;
	const int COLS = 5;
	const int CANVAS_WIDTH = MARKER_SIDE * COLS + 2 * MARGIN;
	const int CANVAS_HEIGHT = MARKER_SIDE * ROWS + 2 * MARGIN;

	cv::Mat canvas(cv::Size(CANVAS_WIDTH, CANVAS_HEIGHT), CV_64FC1, Scalar(255));
	cv::Mat markerImage;


	int markerList[] = { 19, 29, 31, 43, 62, 65, 67, 68, 82, 93, 96, 98, 100, 126, 127, 129, 130, 155, 205, 206, 220, 227, 228, 231, 247, 248,
		0, 1, 2, 3, 4, 5, 6, 7, 8};
	int markerListLength = sizeof(markerList) / sizeof(markerList[0]);
	int markerIterator = 0;

	for (int rows = 0; rows < ROWS; rows++) {
		for (int cols = 0; cols < COLS; cols++) {
			if (markerIterator < markerListLength) {
				cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
				cv::aruco::generateImageMarker(dictionary, markerList[markerIterator], 200, markerImage, 1);
				markerImage.copyTo(canvas(cv::Rect(
					cols * MARKER_SIDE + MARGIN,
					rows * MARKER_SIDE + MARGIN,
					markerImage.cols,
					markerImage.rows
				)));

				markerIterator++;
			}
		}
	}

	cv::imshow("Grid", canvas);
	int keyId = cv::waitKey(0);

	const std::string fileName = "markerPage.png";
	cv::imwrite(fileName, canvas);
}

int main()
{
	//displayCapturedTiltFiveImage();

	//showArucoMarkers();

	showArucoGrid();

	return 0;

}