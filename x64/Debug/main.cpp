//objectTrackingTutorial.cpp

//Written by  Kyle Hounslow 2013

//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software")
//, to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.
#define _CRT_SECURE_NO_DEPRECATE


#include <sstream>
#include <string>
#include <iostream>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <conio.h>
#include "Comms.h"
#include "Turret.h"
#include <iostream>
#include <fstream>
#include <thread>

#include <windows.h>

using namespace cv;
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 140;
int V_MAX = 255;
int SIZE_MIN = 250;
int globalTLX = 25;
int globalTLY = 25;
int globalTRX = 25;
int globalTRY = 25;
int globalBLX = 25;
int globalBLY = 25;
int globalBRX = 25;
int globalBRY = 25;
int shittyGlobalCounterButIDontCareILoveIt = 0;
//default capture width and height
const int FRAME_WIDTH = 1280;
const int FRAME_HEIGHT = 720;
const int TRACKED_WIDTH = 860;
const int TRACKED_HEIGHT = 645;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
//PI
const double PI = 3.141592653589793238463;
int state = 0;

//yo
void receiveTask(Comms* connection)
{
	while (true)
	{
		connection->Receive2(&state);
	}
}

void onmouse(int event, int x, int y, int flags, void* userdata)
{
	if (event == 1) //left click
	{
		if (shittyGlobalCounterButIDontCareILoveIt == 0)
		{
			globalTLX = x;
			globalTLY = y;
			shittyGlobalCounterButIDontCareILoveIt++;
		}
		else if (shittyGlobalCounterButIDontCareILoveIt == 1)
		{
			globalTRX = x;
			globalTRY = y;
			shittyGlobalCounterButIDontCareILoveIt++;
		}
		else if (shittyGlobalCounterButIDontCareILoveIt == 2)
		{
			globalBLX = x;
			globalBLY = y;
			shittyGlobalCounterButIDontCareILoveIt++;
		}
		else if (shittyGlobalCounterButIDontCareILoveIt == 3)
		{
			globalBRX = x;
			globalBRY = y;
			shittyGlobalCounterButIDontCareILoveIt=0;
		}
	}
}

void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed
}

void createTrackbars(){
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);
	sprintf(TrackbarName, "SIZE_MIN", SIZE_MIN);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, 255, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, 255, on_trackbar);
	createTrackbar("SIZE_MIN", trackbarWindowName, &SIZE_MIN, 5000, on_trackbar);
}

Point2f corners_center(std::vector<Point2f> const &corners)
{
	cv::Mat mean_;
	cv::reduce(corners, mean_, CV_REDUCE_AVG, 1);
	// convert from Mat to Point - there may be even a simpler conversion, 
	// but I do not know about it.
	cv::Point2f mean(mean_.at<float>(0, 0), mean_.at<float>(0, 1));
	return mean;
}

Point corners_center(vector<Point> const &corners)
{
	int x = 0, y = 0;
	for (auto const corner : corners)
	{
		x += corner.x;
		y += corner.y;
	}
	return Point(x/corners.size(),y/corners.size());
}

void sort_rect_corners(vector<Point2f> &corners)
{
	std::vector<Point2f> top, bot;
	Point2f center = corners_center(corners);
	for (size_t i = 0; i < corners.size(); i++){
		if (corners[i].y < center.y)
			top.emplace_back(corners[i]);
		else
			bot.emplace_back(corners[i]);
	}
	Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
	Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
	Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
	Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

	corners.clear();
	corners.emplace_back(tl);
	corners.emplace_back(bl);
	corners.emplace_back(br);
	corners.emplace_back(tr);
	return;
}

void sort_rect_corners(vector<Point> &corners)
{
	std::vector<Point2f> top, bot;
	Point2f center = corners_center(corners);
	for (size_t i = 0; i < corners.size(); i++){
		if (corners[i].y < center.y)
			top.emplace_back(corners[i]);
		else
			bot.emplace_back(corners[i]);
	}
	Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
	Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
	Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
	Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

	corners.clear();
	corners.emplace_back(tl);
	corners.emplace_back(bl);
	corners.emplace_back(br);
	corners.emplace_back(tr);
	return;
}

void remove_contours(std::vector<std::vector<cv::Point> > &contours, double cmin, double cmax)
{
	auto it = std::partition(std::begin(contours), std::end(contours), [=](std::vector<cv::Point> const &data)
	{
		auto const size = cv::contourArea(data);
		return !(size < cmin || size > cmax);
	});
	contours.erase(it, std::end(contours));
}

double distance(Point first, Point second)
{
	return norm(first - second);
}


void trainPerspective(VideoCapture capture, Mat &transmtx)
{
	Mat cameraFeed;
	Mat cameraFeedOriginal;
	Mat threshold;
	Mat HSV;
	int counter = 0;
	Point tl = Point(0, 0);
	Point tr = Point(0, 0);
	Point bl = Point(0, 0);
	Point br = Point(0, 0);

	//wait for training
	while (!_kbhit())
	{
		capture.read(cameraFeed);
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);

		//imshow(windowName1, HSV);
		//imshow("thr", threshold);
		imshow("src", cameraFeed);
		waitKey(30);
	}
	_getch(); // clear the cin buffer
	////////V_MINV_MINV_MINV_MINV_MINV_MIN
	//train until keyPressed
	while (!_kbhit())
	{
		capture.read(cameraFeed);
		cameraFeedOriginal = cameraFeed.clone();
		//HSV slider values.
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);

		vector< vector <Point> > contours; // Vector for storing contour
		vector< Vec4i > hierarchy;
		int largest_contour_index = 0;
		int largest_area = 0;
		Mat dst(cameraFeed.rows, cameraFeed.cols, CV_8UC1, Scalar::all(0)); //create destination image
		findContours(threshold.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE); // Find the contours in the image
		if (!contours.size())
		{
			waitKey(30);
			continue;
		}
		for (int i = 0; i < contours.size(); i++){
			double a = contourArea(contours[i], false);  //  Find the area of contour
			if (a > largest_area){
				largest_area = a;
				largest_contour_index = i;                //Store the index of largest contour
			}
		}

		vector<Point> contours_poly;
		approxPolyDP(Mat(contours[largest_contour_index]), contours_poly, 5, true);
		
		if (contours_poly.size() == 4){
			counter++;
			tl.x += contours_poly[0].x;
			tr.x += contours_poly[1].x;
			bl.x += contours_poly[3].x;
			br.x += contours_poly[2].x;
			tl.y += contours_poly[0].y;
			tr.y += contours_poly[1].y;
			bl.y += contours_poly[3].y;
			br.y += contours_poly[2].y;
			
			//std::fstream myfile("data.txt", std::ios_base::in);
			std::vector<Point2f> quad_pts;
			std::vector<Point2f> squre_pts;

		/*	int a;
			int first = NULL;*/
			
		/*	quad_pts.push_back(Point2f(globalTLX, globalTLY));
			circle(cameraFeedOriginal, Point2f(globalTLX, globalTLY), 2, Scalar(255, 0, 0), 2);

			quad_pts.push_back(Point2f(globalTRX, globalTRY));
			circle(cameraFeedOriginal, Point2f(globalTRX, globalTRY), 2, Scalar(255, 0, 0), 2);

			quad_pts.push_back(Point2f(globalBLX, globalBLY));
			circle(cameraFeedOriginal, Point2f(globalBLX, globalBLY), 2, Scalar(255, 0, 0), 2);

			quad_pts.push_back(Point2f(globalBRX, globalBRY));
			circle(cameraFeedOriginal, Point2f(globalBRX, globalBRY), 2, Scalar(255, 0, 0), 2);

			*/
			/*while (myfile >> a)
			{
				if (first != NULL)
				{
					quad_pts.push_back(Point2f(first, a));
					circle(cameraFeedOriginal, Point2f(first, a), 2, Scalar(255, 0, 0), 2);
					first = NULL;

				}
				else
					first = a;
			}*/


			quad_pts.push_back(Point2f(tl.x / counter, tl.y / counter));
			quad_pts.push_back(Point2f(tr.x / counter, tr.y / counter));
			quad_pts.push_back(Point2f(br.x / counter, br.y / counter));
			quad_pts.push_back(Point2f(bl.x / counter, bl.y / counter));

			sort_rect_corners(quad_pts);

			squre_pts.push_back(Point2f(0, 0));
			squre_pts.push_back(Point2f(0, TRACKED_HEIGHT - 1));
			squre_pts.push_back(Point2f(TRACKED_WIDTH - 1, TRACKED_HEIGHT - 1));
			squre_pts.push_back(Point2f(TRACKED_WIDTH - 1, 0));

			transmtx = getPerspectiveTransform(quad_pts, squre_pts);
			Mat transformed = Mat::zeros(TRACKED_HEIGHT, TRACKED_WIDTH, CV_8UC3);
			warpPerspective(cameraFeed, transformed, transmtx, transformed.size());


			//show frames 
			imshow("quadrilateral", transformed);
			//imshow(windowName1, HSV);
			//imshow("thr", threshold);
			imshow("src", cameraFeedOriginal);

		}
		waitKey(30);
	}
	_getch(); // clear the cin buffer
	printf("training done\n");
	return;
}

bool IsMarkedToDelete(const Turret* o)
{
	return o->toBeRemoved;
}

int getAngle(vector<Point> &contours_poly) {
	auto centre = contours_poly[0];
		// corners_center(contours_poly);//get centre
	auto corner = contours_poly[2]; // get upper right corner.
	int deltaY = corner.y - centre.y;
	int deltaX = corner.x - centre.x;
	int angle = abs(int(atan((double)deltaY / (double)deltaX) * 180 / PI)); //calculate the angle
	return (angle-45+720)%180;
}

void detect(VideoCapture capture, Mat &transmtx, Comms* connection)
{
	//while (!_kbhit())
	//{
	//	waitKey(30);
	//}
	//_getch();
	printf("detection starting.\n");
	Mat cameraFeed; //frame
	Mat threshold;
	Mat HSV;
	Mat transformed = Mat::zeros(TRACKED_HEIGHT, TRACKED_WIDTH, CV_8UC3);
	RNG rng(0xFFFFFFFF);
	vector <Turret*> turrets;
	bool sendUpdate = false;
	vector<char> message;
	time_t lastUpdate = time(0);
	int counter = 0;
	V_MIN = 0;
	V_MAX = 90;
	//int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	//int frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	//VideoWriter video("out.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, Size(frame_width, frame_height), true);


	while (state==1)
	{
		capture.read(cameraFeed);
		//video.write(cameraFeed);
		warpPerspective(cameraFeed, transformed, transmtx, transformed.size());
		//HSV slider values.
		cvtColor(transformed, HSV, COLOR_BGR2HSV);
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		vector< vector <Point> > contours; // Vector for storing contours
		vector<Point> contours_poly;
		//vector<Point2f> corners;
		findContours(threshold.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		remove_contours(contours, SIZE_MIN, 50000);
		
		//for each contour, try to fit a rectangle
		for (auto const &data : contours)
		{
			cv::approxPolyDP(data, contours_poly,
			cv::arcLength(data, true) * 0.02, true);
			if ((contours_poly.size() == 4) && isContourConvex(contours_poly))
			{
				int blue = contours_poly[0].x * 123 % 256;
				int green = contours_poly[0].x * 13 % 256;
				int red = contours_poly[0].x * 17 % 256;
				int count = 1;
				sort_rect_corners(contours_poly);
				for (auto const &data : contours_poly)
				{
					circle(transformed, data, 3, Scalar(blue, green, red), 2);
					count++;
				}

				//create a new instance if it's a new turret, or update the old one.
				bool included = false;
				for (Turret* t : turrets)
				{
					if (t->sent)
						t->time = time(0);
					if (distance(t->centre, corners_center(data)) <= 20) //if it's close enough to be considered the same turret
					{
						int angle = getAngle(contours_poly);
						putText(transformed, std::to_string(angle), contours_poly[3], FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0, 0, 0), 1, CV_AA);
						included = true;
						if (distance(t->centre, corners_center(data)) >= 4) // if it's too far from it's original position, update it and send a message about it to the game
						{
							t->centre = corners_center(data);
							t->time = time(0);
						}
						/*else if (t->angle - angle >= 7) {
							t->angle = angle;
							t->time = time(0);
							}*/
						else if (time(0) - t->time >= 2 && t->sent != true) { //if distance and angle didn't change for 2 sec
							message.push_back(abs(255 - (int)((double)t->centre.x * 255.0 / (double)TRACKED_WIDTH)));
							message.push_back(abs(255 - (int)((double)t->centre.y * 255.0 / (double)TRACKED_HEIGHT)));
							message.push_back(t->angle);
							sendUpdate = true;
							t->sent = true;
						}
						break;
					}
				}
				if (!included) // if it's a new turret, create an instance and message the engine
				{
					int angle = getAngle(contours_poly);
					putText(transformed, std::to_string(angle), contours_poly[3], FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0, 0, 0), 1, CV_AA);
					turrets.push_back(new Turret(Turret::getNextID(),corners_center(data), angle, time(0)));
					Turret* t = turrets.back();
					std::cout << " new " << counter++ <<"\n";
				}
			}
		}
		//draw turret centres and mark old ones to be removed.
		for (Turret* t : turrets)
		{
			if (!t->sent && time(0) - t->time >= 2)
			{
				t->toBeRemoved = true;
				//if (t->sent)
				//{
					//message.push_back(abs(100 - (int)((double)t->centre.x * 100.0 / (double)tracked_width)));
					//message.push_back(abs(100 - (int)((double)t->centre.y * 100.0 / (double)tracked_height)));
					//message.push_back(99);
					//sendupdate = true;
				//}
				//sendupdate = true;
				//message += "del_" + std::to_string(t->id) + "|";
				continue;
			}
			circle(transformed, t->centre, 2, Scalar(255, 0, 0), 2);
			putText(transformed, std::to_string(t->ID), t->centre,
				FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200, 200, 250), 1, CV_AA);
		}
		//remove old turrets
		turrets.erase(
			std::remove_if(turrets.begin(), turrets.end(), IsMarkedToDelete),
			turrets.end());
		

		//show frames 
		imshow("quadrilateral", transformed);
		//imshow(windowName1, HSV);
		imshow("thr", threshold);
		//imshow("src", cameraFeed);
		if (sendUpdate)
		{
			/*message.push_back(30+counter);
			message.push_back(30 + counter);
			message.push_back(35 + counter);*/
			sendUpdate = false;
			connection->SendTowerData(message);
			message.clear();
			//counter += 3;
			//lastUpdate = time(0);
			std::cout << " sending\n";
		}
		waitKey(30);
	}
	for (Turret* t : turrets)
	{
		t->toBeRemoved = true;
	}
	//remove turrets
	turrets.erase(
		std::remove_if(turrets.begin(), turrets.end(), IsMarkedToDelete),
		turrets.end());

	//_getch(); // clear the cin buffer
	return;
}

int main(int argc, char* argv[])
{
	//Matrix to store each frame of the webcam feed
	Mat transmtx;	//trained transform matrix
	Mat cameraFeed; //frame
	//video capture object to acquire webcam feed
	VideoCapture capture(0);
	
	Comms* connection = new Comms();
	while (!connection->Setup())
	{
	}

	connection->Receive();
	char handShake[5] = { '1', '0', '0', '0', '0' };
	connection->Send(handShake); // let the server know this is the vision connection
	printf("handshake sent");
	//char command[5] = { 0, 0, 0, 0, 0 };

	/*
	while (true)
	{
		//connection->Receive();
		Sleep(3000);
		connection->Send(command);
	}*/
	createTrackbars();
	namedWindow("src");
	setMouseCallback("src", onmouse, NULL); // pass address of img here
	//open capture object at location zero (default location for webcam)
	capture.open(0);
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//warm up the camera
	do
	{
		capture.read(cameraFeed);
	} while (cameraFeed.empty());
	//trainPerspective(capture, transmtx);
	std::thread t1(receiveTask, connection);
	while (true)
	{
		while (state == 0)
		{
			Sleep(500);
			std::cout <<state << "Sleeping, waiting for instructions. \n";
		}
		if (state == 1)
			detect(capture, transmtx, connection);
		else if (state == 2)
			trainPerspective(capture, transmtx);
		/*connection->Close();
		connection = new Comms();
		while (!connection->Setup())
		{
		}
		//connection->Receive();
		char handShake[5] = { '1', '0', '0', '0', '0' };
		connection->Send(handShake); // let the server know this is the vision connection
		printf("handshake sent");*/
	}
	return 0;
}
