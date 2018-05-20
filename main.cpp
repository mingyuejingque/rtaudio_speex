
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

const int delay_count = 2;
const int channels = 1;
const int fmt = RTAUDIO_SINT16;
unsigned int sampleRate = 8000;
unsigned int bufferFrames = 160;
std::mutex g_mutex;

typedef struct buffer_s {
    char buff[320];
} buffer_t;

std::queue<buffer_t> capq;  //capture form mic.
std::queue<buffer_t> refq;  //ref by speexdsp.

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
	capq.push(bf);
#ifdef DEBUG	
	std::cout << "\t\t\t\t\t" << __func__ 
		<< " tid: " << std::this_thread::get_id()
		<< " put bytes: " << size 
		<< std::endl;
#endif		
	if (refq.size() > delay_count) {
		static buffer_t sbuf;
		static FILE* fp = fopen("cal.pcm", "wb");
		speex_func_echo_cancel((short*)inputBuffer, (short*)refq.front().buff, (short*)sbuf.buff);
		fwrite(sbuf.buff, 1, size, fp);
		//降噪后的输出缓冲区
	}

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

#if 1
	size_t buf_size = 0;
	do {
		std::lock_guard<std::mutex> guard(g_mutex);
        buf_size = capq.size();
		if (buf_size > 0) {
            memcpy(outputBuffer, capq.front().buff, want_size);
            capq.pop(); 

            buffer_t bf;
            memcpy(bf.buff, outputBuffer, want_size);
            refq.push(bf);
			break;
		}
		std::this_thread::yield();
	} while(buf_size == 0);
#endif

	static FILE* fp = fopen("play.pcm", "wb");
	fwrite(outputBuffer, 1, want_size, fp);
#ifdef DEBUG	
	std::cout << __func__ 
		<< " tid: " << std::this_thread::get_id()
		<< " writes bytes: "
		<< want_size 
		<< " buff.size: " << capq.size()
		<< std::endl;
#endif		
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
	speex_func_init(bufferFrames, bufferFrames * 2);
	stream_filter();
	speex_func_destroy();
	return 0;
}

