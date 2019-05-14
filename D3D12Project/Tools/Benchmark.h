#ifndef BENCHMARK_H
#define BENCHMARK_H

#define CLOCK_COUNT 100

#include <chrono>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>

#include "../GlobalDefines.h"

struct Clock
{
	std::string name;

	std::chrono::time_point<std::chrono::high_resolution_clock> timeStart;
	std::chrono::time_point<std::chrono::high_resolution_clock> timeFinish;
	std::chrono::duration<double> timeElapsed;

	void calculateTimeElapsed() {
		this->timeElapsed = (this->timeFinish - this->timeStart) * 1000; //Return milliseconds.
	}
};

class Benchmark
{
private:
	Clock clocks[CLOCK_COUNT];
	int currentClock = 0;

	//Recording
	bool recording = false;
	std::unordered_map<int, std::vector<double>> m_recordings;
	double m_tempAvg = 0.0;
	void saveRecordingsToFile();

	void incrementClockSlot();

public:
	Benchmark();
	~Benchmark();

	int getClockID(std::string clockName);

	void startTest(int clockID);
	void stopTest(int clockID);
	double getTestResult(int clockID);

	int startRecording();
	int stopRecording();
};

#endif
