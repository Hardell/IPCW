#pragma once
#include <opencv\cv.h>
#include <time.h>
class Turret
{
public:
	unsigned int ID;
	cv::Point centre;
	unsigned int angle;
	time_t time;
	bool sent;
	bool toBeRemoved;
	Turret(unsigned int, cv::Point, unsigned int, time_t);
	~Turret();
	static unsigned int getNextID();
private:
	static unsigned int nextID;
};

