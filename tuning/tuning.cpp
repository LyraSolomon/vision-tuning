#include <opencv2/opencv.hpp>
#include <utility>
#include <vector>
#include <iostream>

cv::Scalar ToScalar(int* data) {
  return cv::Scalar(data[0], data[1], data[2]);
}

std::vector<std::pair<int, char*>> colorspaces = {
  std::make_pair<int, char*>(CV_BGR2RGB, "RGB"),
  std::make_pair<int, char*>(CV_BGR2Lab, "Lab"),
  std::make_pair<int, char*>(CV_BGR2YUV, "YUV"),
  std::make_pair<int, char*>(CV_BGR2Luv, "Luv"),
  std::make_pair<int, char*>(CV_BGR2YCrCb, "YCrCb"),
  std::make_pair<int, char*>(CV_BGR2HSV, "HSV"),
  std::make_pair<int, char*>(CV_BGR2HLS, "HLS")
};

int lower_bound[] = {0, 0, 0}, upper_bound[] = {255, 255, 255};
int speed = 100; // Playback speed
int position = 0;  // Position in video, in frames
int colorspace = 0; // Index in colorspaces
cv::VideoCapture cap;
cv::Mat raw;

// Opencv doesn't like lambdas
void OnPositionTrackbar(int, void*){
  cap.set(CV_CAP_PROP_POS_FRAMES, position);
  cap >> raw;
}
  
int main(int n, char** args) {
  if(n == 2) cap.open(args[1]);
  else return 0;

  cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
  cv::namedWindow("tuned", cv::WINDOW_AUTOSIZE);

  cv::createTrackbar("low 0", "tuned", &lower_bound[0], 255, 0);
  cv::createTrackbar("low 1", "tuned", &lower_bound[1], 255, 0);
  cv::createTrackbar("low 2", "tuned", &lower_bound[2], 255, 0);
  cv::createTrackbar("high 0", "tuned", &upper_bound[0], 255, 0);
  cv::createTrackbar("high 1", "tuned", &upper_bound[1], 255, 0);
  cv::createTrackbar("high 2", "tuned", &upper_bound[2], 255, 0);
  cv::createTrackbar("speed", "image", &speed, 200, [](int, void*){});
  cv::createTrackbar("position", "image", &position, cap.get(CV_CAP_PROP_FRAME_COUNT), OnPositionTrackbar);
  cv::createTrackbar("colorspace", "image", &colorspace, colorspaces.size() - 1, 0);

  cv::Mat tuned, recolored;
  int counter = 0; // Percentage of way to next frame
  while(cap.isOpened()) {

    counter += speed;
    while(counter >= 200) {
      position++;
      counter -= 100;
    }
    if(counter >= 100) {
      // Calling this calls OnPositionTrackbar
      cv::setTrackbarPos("position", "image", position);
      counter -= 100;
    }

    if(raw.empty()) { // End of data, restart
      position = 1;
      cv::setTrackbarPos("position", "image", position);
    }

    position = cap.get(CV_CAP_PROP_POS_FRAMES);

    // Non-rgb colorspaces are difficult to visualize, so it helps to
    // show the converted version.
    cv::cvtColor(raw, recolored, colorspaces[colorspace].first);
    cv::cvtColor(raw, tuned, colorspaces[colorspace].first);
    cv::inRange(tuned, ToScalar(lower_bound), ToScalar(upper_bound), tuned);
    cv::cvtColor(tuned, tuned, CV_GRAY2BGR);

    cv::putText(tuned, colorspaces[colorspace].second, cv::Point(5, 30), 0, 1, cv::Scalar(0, 255, 0), 2, 8);
    cv::imshow("tuned", tuned);
    cv::imshow("image", recolored);
    cv::waitKey(1);
  }
}
