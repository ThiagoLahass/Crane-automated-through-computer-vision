#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;


//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 255;
int S_MIN = 0;
int S_MAX = 255;
int V_MIN = 0;
int V_MAX = 255;


//============= BASED ON TESTS ==========

//initial min and max HSV filter to GREEN
int H_MIN_GREEN = 78;
int H_MAX_GREEN = 98;
int S_MIN_GREEN = 129;
int S_MAX_GREEN = 256;
int V_MIN_GREEN = 122;
int V_MAX_GREEN = 211;

//initial min and max HSV filter to BLUE
int H_MIN_BLUE = 92;
int H_MAX_BLUE = 122;
int S_MIN_BLUE = 136;
int S_MAX_BLUE = 256;
int V_MIN_BLUE = 209;
int V_MAX_BLUE = 256;

//initial min and max HSV filter to RED
int H_MIN_RED = 63;
int H_MAX_RED = 104;
int S_MIN_RED = 98;
int S_MAX_RED = 256;
int V_MIN_RED = 94;
int V_MAX_RED = 211;

//initial min and max HSV filter to YELLOW
int H_MIN_YELLOW = 63;
int H_MAX_YELLOW = 104;
int S_MIN_YELLOW = 98;
int S_MAX_YELLOW = 256;
int V_MIN_YELLOW = 94;
int V_MAX_YELLOW = 211;

//=========================================


//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;

//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT * FRAME_WIDTH / 1.5;

const string trackbarWindowName = "Trackbars";

void createTrackbars();
void cleanNoiseMorphOps(Mat img);
void trackFilteredObject(int& x, int& y, Mat imgMask, Mat imgOriginal);
void drawObject(int x, int y, Mat img);

int main() {

	//some boolean variables for different functionality within this program
	bool trackObjects = true;
	bool useMorphOps = true;

	//x and y values for the location of the object
	int x = 0, y = 0;

	VideoCapture capture(1);
	Mat imgOriginal, imgHSV, imgMask;

	/* string path = "Resources/containers1.jpg";
	imgOriginal = imread(path);
	resize(imgOriginal, imgOriginal, Size(FRAME_WIDTH, FRAME_HEIGHT));*/

	createTrackbars();

	//set height and width of capture frame
	// capture.set(CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	// capture.set(CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	while (1) {

		/*Mat imgOriginal, imgHSV, imgMask;

		string path = "Resources/containers1.jpg";
		imgOriginal = imread(path);
		resize(imgOriginal, imgOriginal, Size(FRAME_WIDTH, FRAME_HEIGHT));*/


		//store video image into 'imgOriginal' matrix
		capture.read(imgOriginal);

		//convert frame from BGR to HSV colorspace
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

		Scalar lower(H_MIN, S_MIN, V_MIN);
		Scalar upper(H_MAX, S_MAX, V_MAX);
		//filter HSV image between values and store filtered image to 'imgThreshold' matrix
		inRange(imgHSV, lower, upper, imgMask);

		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if (useMorphOps) cleanNoiseMorphOps(imgMask);

		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		if (trackObjects) trackFilteredObject(x, y, imgMask, imgOriginal);

		imshow("Image Original", imgOriginal);
		imshow("Image HSV", imgHSV);
		imshow("Image Threshold", imgMask);
		waitKey(1);
	}

	return 0;
}

void createTrackbars() {
	//create window for trackbars
	namedWindow(trackbarWindowName, (640, 200));

	//create trackbars and insert them into window
	//2 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH)
	createTrackbar("Hue Min", trackbarWindowName, &H_MIN, H_MAX);
	createTrackbar("Hue Max", trackbarWindowName, &H_MAX, H_MAX);
	createTrackbar("Sat Min", trackbarWindowName, &S_MIN, S_MAX);
	createTrackbar("Sat Max", trackbarWindowName, &S_MAX, S_MAX);
	createTrackbar("Val Min", trackbarWindowName, &V_MIN, V_MAX);
	createTrackbar("Val Max", trackbarWindowName, &V_MAX, V_MAX);
}

void cleanNoiseMorphOps(Mat img) {
	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(img, img, erodeElement);
	// erode(img, img, erodeElement);

	dilate(img, img, dilateElement);
	// dilate(img, img, dilateElement);
}

void trackFilteredObject(int& x, int& y, Mat imgMask, Mat imgOriginal) {
	//temporary auxiliar image
	Mat imgTemp;
	imgMask.copyTo(imgTemp);

	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;

	//find contours of filtered image using openCV findContours function
	findContours(imgTemp, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > MIN_OBJECT_AREA && area < MAX_OBJECT_AREA && area > refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;
			}

			//let user know you found an object
			if (objectFound == true) {
				putText(imgOriginal, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				drawObject(x, y, imgOriginal);
			}

		}
		else putText(imgOriginal, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}

void drawObject(int x, int y, Mat img) {
	//use some of the openCV drawing functions to draw crosshairs on tracked image
	circle(img, Point(x, y), 20, Scalar(0, 255, 0), 2);

	if (y - 25 > 0) line(img, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(img, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);

	if (y + 25 < FRAME_HEIGHT) line(img, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(img, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	
	if (x - 25 > 0) line(img, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(img, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	
	if (x + 25 < FRAME_WIDTH) line(img, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(img, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(img, to_string(x) + "," + to_string(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);
}