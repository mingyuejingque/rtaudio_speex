
#include <iostream>
#include <string>
#include <cstdlib>
#include <string.h>
#include "RtAudio.h"
#include <mutex>
#include <thread>
#include <boost/circular_buffer.hpp>

const int channels = 1;
const int fmt = RTAUDIO_SINT16;
unsigned int sampleRate = 8000;
unsigned int bufferFrames = 160;
std::mutex g_mutex;

typedef struct buffer_s {
    char buff[320];
} buffer_t;

boost::circular_buffer<buffer_t> buff(1024);

int input_cb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *userData)
{
	if (status)
		std::cout << "Stream overflow detected!" << status << std::endl;
	// Do something with the data in the "inputBuffer" buffer.

    size_t size = nBufferFrames * channels * 2;
	std::lock_guard<std::mutex> guard(g_mutex);
    buffer_t bf;
    memcpy(bf.buff, inputBuffer, size);
	buff.push_back(bf);
	std::cout << "\t\t\t\t\t" << __func__ 
		<< " tid: " << std::this_thread::get_id()
		<< " put bytes: " << size 
		<< std::endl;

#if 0
    static FILE* fp = fopen("./out.pcm", "wb");
    fwrite(inputBuffer, 1, size, fp);
#endif
	return 0;
}

int output_cb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *userData)
{
	if (status)
		std::cout << "Stream overflow detected!" << status << std::endl;
	// Do something with the data in the "inputBuffer" buffer.

	size_t want_size = nBufferFrames * channels * 2;
#if 0
	static FILE *fp = fopen("out2.pcm", "rb");
	fread(outputBuffer, 1, want_size, fp);
#endif

#if 1
	size_t buf_size = 0;
	do {
		std::lock_guard<std::mutex> guard(g_mutex);
        buf_size = buff.size();
		if (buf_size > 0) {
            memcpy(outputBuffer, buff[0].buff, want_size);
            buff.pop_front();
			break;
		}
		std::this_thread::yield();
	} while(buf_size == 0);
#endif
	std::cout << __func__ 
		<< " tid: " << std::this_thread::get_id()
		<< " writes bytes: "
		<< want_size 
		<< "buff.size: " << buff.size()
		<< std::endl;
	return 0;
}

void stream_filter() {
	RtAudio adc_input, adc_output;
	if (adc_input.getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		exit(0);
	}
	RtAudio::StreamParameters in;
	in.deviceId = adc_input.getDefaultInputDevice();
	in.nChannels = channels;
	in.firstChannel = 0;

	auto out = in;
	out.deviceId = adc_output.getDefaultOutputDevice();
    std::cout << "input device: " << in.deviceId
            << "output device: " << out.deviceId
            << std::endl;
	try {
		adc_input.openStream(nullptr, &in, fmt,
			sampleRate, &bufferFrames, &input_cb, &adc_input);
		adc_input.startStream();

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
		adc_input.stopStream();
	}
	catch (RtAudioError& e) {
		e.printMessage();
	}
	if (adc_input.isStreamOpen()) adc_input.closeStream();
	if (adc_output.isStreamOpen()) adc_output.closeStream();
}

int main()
{
	stream_filter();
	return 0;
}

