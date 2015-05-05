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
	bool Receive();
	void Receive2(int*);
	void Close();
};

