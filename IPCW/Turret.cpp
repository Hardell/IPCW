#include "Turret.h"

using namespace cv;
unsigned int Turret::nextID;
Turret::Turret(unsigned int ID, Point centre, unsigned int angle, time_t time)
{

	this->ID = ID;
	this->centre = centre;
	this->angle = angle;
	this->time = time;
	this->toBeRemoved = false;
	this->sent = false;
}


Turret::~Turret()
{
}

unsigned int Turret::getNextID()
{
	nextID++;
	return nextID;
}