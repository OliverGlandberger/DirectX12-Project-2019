#include "Benchmark.h"

//////////////////////////

/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

void Benchmark::saveRecordingsToFile() 
{
	for (auto r : m_recordings) {
		std::ofstream file;
		std::string fileName = "TimeRecords/" + clocks[r.first].name + "_B";
		if (USEBUNDLES) {
			fileName += "Y";
		}
		else {
			fileName += "N";
		}
		fileName += "_N" + std::to_string(TRIANGLECOUNT);
		fileName += ".txt";
		file.open(fileName, std::ofstream::app);
		if (file.is_open() == true) {
			m_tempAvg = 0.0;

			for (unsigned int i = 0; i < r.second.size(); i++) {
				m_tempAvg += r.second[i];
			}

			m_tempAvg /= r.second.size();
			file << m_tempAvg << "\n";

			file.close();
		}
		
	}

	m_recordings.clear();
}

void Benchmark::incrementClockSlot()
{
	this->currentClock++;
	this->currentClock %= CLOCK_COUNT;
}

/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

Benchmark::Benchmark()
{

}

Benchmark::~Benchmark()
{

}

int Benchmark::getClockID(std::string clockName)
{
	int returnValue = this->currentClock;
	clocks[returnValue].name = clockName;
	this->incrementClockSlot();

	return returnValue;
}

void Benchmark::startTest(int clockID)
{
	this->clocks[clockID].timeStart = std::chrono::high_resolution_clock::now();
}

void Benchmark::stopTest(int clockID)
{
	this->clocks[clockID].timeFinish = std::chrono::high_resolution_clock::now();

	if (recording) {
		m_recordings[clockID].push_back(getTestResult(clockID)); //Save result in recordings
	}
}

double Benchmark::getTestResult(int clockID)
{
	this->clocks[clockID].calculateTimeElapsed();
	return this->clocks[clockID].timeElapsed.count();
}

int Benchmark::startRecording() 
{
	if (!recording) {
		recording = true;
		return 0;
	}
	else {
		return -1;
	}
}

int Benchmark::stopRecording() 
{
	if (recording) {
		recording = false;
		saveRecordingsToFile();
		return 0;
	}
	else {
		return -1;
	}
}