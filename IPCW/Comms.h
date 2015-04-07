#pragma once
#include <string>
#include <vector>
class Comms
{
public:
	Comms();
	~Comms();
	bool Setup();
	void Send(char*);
	void SendTowerData(std::vector<char>& message);
	void Receive();
	void Close();
};

