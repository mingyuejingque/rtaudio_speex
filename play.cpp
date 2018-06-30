
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include "RtAudio.h"

const int delay_count = 1;
const int channels = 1;
const int fmt = RTAUDIO_SINT16;
static unsigned int sampleRate = 8000;
static unsigned int bufferFrames = 160;
static unsigned int tail = 4096; //估算的1/3

std::default_random_engine engine(time(nullptr));
std::uniform_int_distribution<> dis(1, 30);

int output_cb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *userData)
{
	if (status)
		std::cout << "Stream overflow detected!" << status << std::endl;
	// Do something with the data in the "inputBuffer" buffer.

	static size_t size = nBufferFrames * channels * 2;
	static FILE* fp = fopen("remote.pcm", "rb");
	fread(outputBuffer, 1, size, fp);
	int delay = dis(engine);
	std::cout << "delay: " << delay << " ms" << std::endl;
	std::this_thread::sleep_for(
		std::chrono::milliseconds(delay)
		);

	return 0;
}



void stream_filter() {
	RtAudio adc_input, adc_output;
	if (adc_input.getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		exit(0);
	}

	RtAudio::StreamParameters out;
	out.nChannels = channels;
	out.deviceId = adc_output.getDefaultOutputDevice();
    std::cout << " output device: " << out.deviceId
            << std::endl;
	try {
		adc_output.openStream(&out, nullptr, fmt,
			sampleRate, &bufferFrames, &output_cb, &adc_output);
		adc_output.startStream();		
	}
	catch (RtAudioError& e) {
		e.printMessage();
		exit(0);
	}

	char input;
	std::cout << "\nRecording ... press <enter> to quit.\n";
	std::cin.get(input);
	try {
		// Stop the stream
		adc_output.stopStream();

	}
	catch (RtAudioError& e) {
		e.printMessage();
	}
	if (adc_output.isStreamOpen()) adc_output.closeStream();
}

int main()
{
	stream_filter();	
	return 0;
}
