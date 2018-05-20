
#include <iostream>
#include <string>
#include <cstdlib>
#include <string.h>
#include <mutex>
#include <thread>
#include <queue>
#include "RtAudio.h"
#include "speex_func.h"

//测试目标：
/*
1 改成从文件读入数据，当作远端传来的音频 remote.pcm
2 把mic读到数据写到文件  cap.pcm
3 把speex 处理后的数据写到文件 cancel.pcm
*/

const int delay_count = 1;
const int channels = 1;
const int fmt = RTAUDIO_SINT16;
static unsigned int sampleRate = 8000;
static unsigned int bufferFrames = 160;
static unsigned int tail = bufferFrames * 8;
std::mutex g_mutex_input;
std::mutex g_mutex_output;

typedef struct buffer_s {
    unsigned short buff[320];
} buffer_t;


std::queue<buffer_t> inputq;
std::queue<buffer_t> outputq;

void thread_func(bool& exit_flags);

int input_cb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *userData)
{
	if (status)
		std::cout << "Stream overflow detected!" << status << std::endl;
	// Do something with the data in the "inputBuffer" buffer.

    static size_t size = nBufferFrames * channels * 2;

    static FILE* fi = fopen("./cap.pcm", "wb");
    fwrite(inputBuffer, 1, size, fi);

    static bool bxx = false;
    if (!bxx) {  //扔掉一帧试试
    	bxx = true;
    	return 0;
    }

    buffer_t bf;
    memcpy(bf.buff, inputBuffer, size);
    std::lock_guard<std::mutex> guard(g_mutex_input);
    inputq.push(bf);
    
	return 0;
}

int output_cb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *userData)
{
	if (status)
		std::cout << "Stream overflow detected!" << status << std::endl;
	// Do something with the data in the "inputBuffer" buffer.

	static size_t size = nBufferFrames * channels * 2;
	static FILE* fp = fopen("remote.pcm", "rb");
	fread(outputBuffer, 1, size, fp);

	buffer_t bf;
	memcpy(bf.buff, outputBuffer, size);
	std::lock_guard<std::mutex> guard(g_mutex_output);
	outputq.push(bf);

	return 0;
}

void thread_func(bool& exit_flags) {
	static size_t size = bufferFrames * channels * 2;
	while (!exit_flags) {
		std::lock_guard<std::mutex> guard(g_mutex_input);
		std::lock_guard<std::mutex> guard2(g_mutex_output);
		int len1 = inputq.size();
		int len2 = outputq.size();
		if (len1 == 0 || len2 == 0) {
			std::this_thread::yield();
			continue;
		}

		static buffer_t out;
		static FILE* fr = fopen("cancel.pcm", "wb");
		speex_func_echo_cancel((short*)inputq.front().buff, (short*)outputq.front().buff, (short*)out.buff);
		fwrite(out.buff, 1, size, fr);
		//降噪后的输出缓冲区

		inputq.pop();
		outputq.pop();
	}
	std::cout << __func__ << " exit." << std::endl;
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
            << " output device: " << out.deviceId
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
	bool exit_flags = false;
	std::thread t(thread_func, std::ref(exit_flags));
	speex_func_init(bufferFrames, tail, sampleRate);
	stream_filter();
	speex_func_destroy();
	exit_flags = true;
	t.join();
	return 0;
}

