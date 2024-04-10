#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;

int main()
{
	std::string image_path = "c:/dev/wizard.jpg";
	std::cout << "Image Path: " << image_path << std::endl;
	cv::Mat img = cv::imread(image_path, IMREAD_COLOR);

	if (img.empty())
	{
		std::cout << "Could not read image" << std::endl;
		return 1;
	}

	cv::namedWindow("Test Window", cv::WINDOW_AUTOSIZE);
	cv::imshow("Test Window", img);
	cv::moveWindow("Test Window", 0, 45);
	int k = cv::waitKey(0);
	return 0;
}