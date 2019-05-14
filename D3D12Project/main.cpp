/// Renderer
#include "D3D12/D3D12Renderer.h"

/// MULTI-THREADING
#include <thread>
#include <atomic>
#include <stdint.h>

/// TIME
#include "Tools/Benchmark.h"

/// Locator
#include "Tools/Locator.h"

/// MEMORY LEAKS
#include <crtdbg.h>

using namespace std;
D3D12Renderer* renderer;

void GPUSignal(ID3D12Fence1* fence, UINT64 fenceVal) {
	//Signal and increment the fence value.
	const UINT64 tempFence = fenceVal;
	Locator::getCommandQueue()->Signal(fence, tempFence);
}

void GPUWait(ID3D12Fence1* fence, UINT64* fenceVal) {
	Locator::getCommandQueue()->Wait(fence, *fenceVal);
	(*fenceVal)++;
}

void CPUWait(ID3D12Fence1* fence, UINT64 fenceVal) {
	const UINT64 tempFenceVal = fenceVal;

	HANDLE eventHandle = nullptr;

	eventHandle = CreateEvent(0, false, false, 0);

	//Wait until command queue is done.
	if (fence->GetCompletedValue() < tempFenceVal)
	{
		fence->SetEventOnCompletion(tempFenceVal, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
	}
}

void run() {
	MSG msg = {};

	// Thread Variables
	std::thread* t_updateCPU;
	std::thread* t_recordLists;

	// Time-since-last-update variable
	double lastDelta = 0.0;

	// FPS variables
	int framesExecuted = 0;
	double fpsUpdateCounter = 0.0;
	double fpsUpdateInterval = 0.5; // Milliseconds
	double fps = 0.0;

	//Benchmarking
	int benchmarkID = Locator::getBenchmark()->getClockID("FrameDeltaTime");
	Locator::getBenchmark()->startTest(benchmarkID);
	int frameCounter = 0;
	int benchmarkThisManyFrames = TESTDATA::OPTIONS::FRAMECOUNT;
	int benchmarkCounter = 0;
	int doThisManyBenchmarkRecordings = TESTDATA::OPTIONS::ITERATIONS;
	bool recording = false;

	//Fences
	UINT64 fenceVal = 0;
	ID3D12Fence1 *fence = nullptr;

	Locator::getDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	fenceVal = 1;

	// RENDER LOOP: Entry Point
	while (WM_QUIT != msg.message && !(msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Record benchmarks
		if (!recording && benchmarkCounter < doThisManyBenchmarkRecordings) {
			Locator::getBenchmark()->startRecording();
			frameCounter = 0;
			recording = true;
			benchmarkCounter++;
		}

		if (recording) {
			if (frameCounter >= benchmarkThisManyFrames) {
				Locator::getBenchmark()->stopRecording();
				recording = false;
			}

			frameCounter++;
		}

		//Get time since last update
		Locator::getBenchmark()->stopTest(benchmarkID);
		lastDelta = Locator::getBenchmark()->getTestResult(benchmarkID) * 0.001;

		//Restart timer
		Locator::getBenchmark()->startTest(benchmarkID);

		/// C R E A T E     T1_
		// FIRST thread; will update the scene (mesh positions, camera, etc.)
		t_updateCPU = new std::thread(&D3D12Renderer::updateScene, renderer, lastDelta);

		//Wait for allocator to be done
		if (fenceVal > BACKBUFFERCOUNT - 1) {
			CPUWait(fence, fenceVal - (BACKBUFFERCOUNT - 1));
		}

		/// C R E A T E     T2_
		// SECOND thread; will record all commands for commandlist(s)
		t_recordLists = new std::thread(&D3D12Renderer::recordList, renderer);

		/// J O I N     T1_
		t_updateCPU->join();
		delete t_updateCPU;
		t_updateCPU = nullptr;

		//The CPUWait makes sure that this does not write to constant buffers that are currently being used.
		// Prepare for next frame; update GPU with new data
		renderer->updateMemoryGPU();

		/// J O I N     T2_
		t_recordLists->join();
		delete t_recordLists;
		t_recordLists = nullptr;

		GPUSignal(fence, fenceVal);
		GPUWait(fence, &fenceVal); //Increases the fence val
		// Execute the frame by swapping backbuffer and executing the commandlist(s)
		renderer->executeFrame();

		// Save GPU-Benchmark Data
		renderer->appendGPUDataToFile("Test.txt");

		// Present the new frame
		renderer->present();

		//----FPS update----
		framesExecuted++; //Keep track of executed frames
		fpsUpdateCounter += lastDelta;
		if (fpsUpdateCounter >= fpsUpdateInterval) {
			fpsUpdateCounter /= framesExecuted; //Devide time elapsed by frames executed to get average time for one frame.

			fps = 1.0 / fpsUpdateCounter; //Calculate fps
			//Reset counters
			fpsUpdateCounter = 0.0;
			framesExecuted = 0;

			//Set window title
			char title[256];
			sprintf_s(title, "D3D12 - %3.0lf (FPS)", fps);
			renderer->setWinTitle(title);
		}
		//------------------
	}

	if (fence != NULL)
	{
		fence->Release();
	}
}

int main(int argc, char *argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//Delta time recording
	Benchmark* benchmark = new Benchmark;
	Locator::provide(&benchmark);

	// ------  MODIFIED  ------ 
	renderer = D3D12Renderer::makeRenderer();
	renderer->initialize(800, 600);
	renderer->setWinTitle("Direct3D 12");
	renderer->setClearColor(0.0f, 0.2f, 0.4f, 1.0f);

	std::cout << "testbench" << std::endl;
	renderer->initialiseTestbench();
	run();
	renderer->shutdown();
	delete benchmark;
	delete renderer;

	return 0;
};