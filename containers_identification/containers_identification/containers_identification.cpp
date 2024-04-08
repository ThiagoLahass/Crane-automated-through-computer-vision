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

//initial min and max HSV filter to RED
int H_MIN_RED = 145;
int H_MAX_RED = 182;
int S_MIN_RED = 69;
int S_MAX_RED = 151;
int V_MIN_RED = 202;
int V_MAX_RED = 255;

//initial min and max HSV filter to GREEN
int H_MIN_GREEN = 92;
int H_MAX_GREEN = 105;
int S_MIN_GREEN = 94;
int S_MAX_GREEN = 214;
int V_MIN_GREEN = 98;
int V_MAX_GREEN = 173;

//initial min and max HSV filter to BLUE
int H_MIN_BLUE = 92;
int H_MAX_BLUE = 125;
int S_MIN_BLUE = 121;
int S_MAX_BLUE = 250;
int V_MIN_BLUE = 206;
int V_MAX_BLUE = 255;

//initial min and max HSV filter to YELLOW
int H_MIN_YELLOW = 0;
int H_MAX_YELLOW = 49;
int S_MIN_YELLOW = 14;
int S_MAX_YELLOW = 69;
int V_MIN_YELLOW = 157;
int V_MAX_YELLOW = 255;

#define RED		0
#define GREEN	1
#define BLUE	2
#define YELLOW	3

int COLOR = 0;

vector<vector<int>> myColorsThresholds{
	{H_MIN_RED,		S_MIN_RED,		V_MIN_RED,		H_MAX_RED,		S_MAX_RED,		V_MAX_RED},		// RED
	{H_MIN_GREEN,	S_MIN_GREEN,	V_MIN_GREEN,	H_MAX_GREEN,	S_MAX_GREEN,	V_MAX_GREEN},	// GREEN
	{H_MIN_BLUE,	S_MIN_BLUE,		V_MIN_BLUE,		H_MAX_BLUE,		S_MAX_BLUE,		V_MAX_BLUE},	// BLUE
	{H_MIN_YELLOW,	S_MIN_YELLOW,	V_MIN_YELLOW,	H_MAX_YELLOW,	S_MAX_YELLOW,	V_MAX_YELLOW}	// YELLOW
};

vector<Scalar> myColorsValues{
//   B  G  R
	{0, 0, 255},  // RED
	{0, 255, 0},  // GREEN
	{255, 0, 0},  // BLUE
	{0, 255, 255} // YELLOW
};

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
void trackFilteredObject(int& x, int& y, int color, Mat imgMask, Mat imgOriginal);
void drawCrosshairs(int x, int y, int color, Mat img);
Point getCenterOfNearestContainer(Mat imgMask, Mat imgOriginal, int color);

int main() {

	//some boolean variables for different functionality within this program
	bool trackObjects = true;
	bool useMorphOps = true;

	//x and y values for the location of the object
	int x = 0, y = 0;

	VideoCapture capture(0);
	Mat imgOriginal, imgHSV, imgMask;
	Mat imgMaskRed, imgMaskGreen, imgMaskBlue, imgMaskYellow;
	Mat imgMasks[4] = { imgMaskRed, imgMaskGreen, imgMaskBlue, imgMaskYellow };

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


		// findColors()

		vector<vector<int>> newPoints;

		for (int i = 0; i < myColorsThresholds.size(); i++) {
			Scalar lower(myColorsThresholds[i][0], myColorsThresholds[i][1], myColorsThresholds[i][2]);
			Scalar upper(myColorsThresholds[i][3], myColorsThresholds[i][4], myColorsThresholds[i][5]);

			inRange(imgHSV, lower, upper, imgMasks[i]);

			//perform morphological operations on thresholded image to eliminate noise
			//and emphasize the filtered object(s)
			if (useMorphOps) cleanNoiseMorphOps(imgMasks[i]);

			imshow(to_string(i), imgMasks[i]);

			Point myContainerPoint(0, 0);


			//pass in thresholded frame to our object tracking function
			//this function will return the x and y coordinates of the
			//filtered object
			if (trackObjects && i == COLOR) {
				myContainerPoint = getCenterOfNearestContainer(imgMasks[i], imgOriginal, COLOR);

				if (myContainerPoint.x != 0 && myContainerPoint.y != 0) {
					drawCrosshairs(myContainerPoint.x, myContainerPoint.y, COLOR, imgOriginal);
				}
				//trackFilteredObject(x, y, i, imgMasks[i], imgOriginal);
			}
		}


		//Scalar lower(H_MIN, S_MIN, V_MIN);
		//Scalar upper(H_MAX, S_MAX, V_MAX);
		////filter HSV image between values and store filtered image to 'imgThreshold' matrix
		//inRange(imgHSV, lower, upper, imgMask);

		////perform morphological operations on thresholded image to eliminate noise
		////and emphasize the filtered object(s)
		//if (useMorphOps) cleanNoiseMorphOps(imgMask);

		////pass in thresholded frame to our object tracking function
		////this function will return the x and y coordinates of the
		////filtered object
		//if (trackObjects) trackFilteredObject(x, y, GREEN, imgMask, imgOriginal);

		imshow("Image Original", imgOriginal);
		//imshow("Image HSV", imgHSV);
		//imshow("Image Threshold", imgMask);
		waitKey(1);
	}

	return 0;
}

void onTrackbar_H_MIN(int value, void* pointer) {
	H_MIN = value;
}

void onTrackbar_H_MAX(int value, void* pointer) {
	H_MAX = value;
}

void onTrackbar_S_MIN(int value, void* pointer) {
	S_MIN = value;
}

void onTrackbar_S_MAX(int value, void* pointer) {
	S_MAX = value;
}

void onTrackbar_V_MIN(int value, void* pointer) {
	V_MIN = value;
}

void onTrackbar_V_MAX(int value, void* pointer) {
	V_MAX = value;
}

void onTrackbar_COLOR(int value, void* pointer) {
	COLOR = value;
}


void createTrackbars() {
	//create window for trackbars
	namedWindow(trackbarWindowName, (640, 200));

	//create trackbars and insert them into window
	//2 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH)
	createTrackbar("Hue Min", trackbarWindowName, NULL, H_MAX, onTrackbar_H_MIN);
	createTrackbar("Hue Max", trackbarWindowName, NULL, H_MAX, onTrackbar_H_MAX);
	setTrackbarPos("Hue Max", trackbarWindowName, H_MAX);
	createTrackbar("Sat Min", trackbarWindowName, NULL, S_MAX, onTrackbar_S_MIN);
	createTrackbar("Sat Max", trackbarWindowName, NULL, S_MAX, onTrackbar_S_MAX);
	setTrackbarPos("Sat Max", trackbarWindowName, S_MAX);
	createTrackbar("Val Min", trackbarWindowName, NULL, V_MAX, onTrackbar_V_MIN);
	createTrackbar("Val Max", trackbarWindowName, NULL, V_MAX, onTrackbar_V_MAX);
	setTrackbarPos("Val Max", trackbarWindowName, V_MAX);

	createTrackbar("Color", trackbarWindowName, &COLOR, myColorsThresholds.size() - 1, onTrackbar_COLOR);
}

void cleanNoiseMorphOps(Mat img) {
	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(img, img, erodeElement);
	//erode(img, img, erodeElement);

	dilate(img, img, dilateElement);
	//dilate(img, img, dilateElement);
}

void trackFilteredObject(int& x, int& y, int color, Mat imgMask, Mat imgOriginal) {
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
				drawCrosshairs(x, y, color, imgOriginal);
			}

		}
		else putText(imgOriginal, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}

void drawCrosshairs(int x, int y, int color, Mat img) {
	//use some of the openCV drawing functions to draw crosshairs on tracked image
	circle(img, Point(x, y), 20, myColorsValues[color], 2);

	if (y - 25 > 0) line(img, Point(x, y), Point(x, y - 25), myColorsValues[color], 2);
	else line(img, Point(x, y), Point(x, 0), myColorsValues[color], 2);

	if (y + 25 < FRAME_HEIGHT) line(img, Point(x, y), Point(x, y + 25), myColorsValues[color], 2);
	else line(img, Point(x, y), Point(x, FRAME_HEIGHT), myColorsValues[color], 2);
	
	if (x - 25 > 0) line(img, Point(x, y), Point(x - 25, y), myColorsValues[color], 2);
	else line(img, Point(x, y), Point(0, y), myColorsValues[color], 2);
	
	if (x + 25 < FRAME_WIDTH) line(img, Point(x, y), Point(x + 25, y), myColorsValues[color], 2);
	else line(img, Point(x, y), Point(FRAME_WIDTH, y), myColorsValues[color], 2);

	putText(img, to_string(x) + "," + to_string(y), Point(x, y + 30), 1, 1, Scalar(0, 0, 255), 2);
}

Point getCenterOfNearestContainer(Mat imgMask, Mat imgOriginal, int color) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(imgMask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> contoursPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	Point myPoint(0, 0);

	// filter by area before draw to reject noise
	for (int i = 0; i < contours.size(); i++) {
		int area = contourArea(contours[i]);
		// cout << area << endl;

		if (area > MIN_OBJECT_AREA) {
			float perimeter = arcLength(contours[i], true);
			approxPolyDP(contours[i], contoursPoly[i], 0.01 * perimeter, true);
			//cout << contoursPoly[i].size() << endl;
			boundRect[i] = boundingRect(contoursPoly[i]);

			drawContours(imgOriginal, contoursPoly, i, myColorsValues[color], 2);
			// rectangle(imgOriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);

			if ((boundRect[i].y + boundRect[i].height / 2)  > myPoint.y) {
				myPoint.x = boundRect[i].x + boundRect[i].width / 2;
				myPoint.y = boundRect[i].y + boundRect[i].height / 2;
			}
		}
	}

	return myPoint;
}