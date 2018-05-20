#include <speex/speex_echo.h>
#include <speex/speex_preprocess.h>
#include "speex_func.h"

int speex_func_init(int frame_size, int filter_length);
int speex_func_destroy();
int speex_func_echo_cancel(short* mic, short* play, short* out);

static SpeexPreprocessState* g_preprocess_state = nullptr;
static SpeexEchoState *g_echo_state = nullptr;
static int g_frame_size = 0;
static int g_filter_length = 0;

int speex_func_init(int frame_size, int filter_length) {
	g_frame_size = frame_size;
	g_filter_length = filter_length;

	g_echo_state = speex_echo_state_init(
		g_frame_size,
		g_filter_length
		);
	g_preprocess_state = speex_preprocess_state_init(
		g_frame_size,
		g_filter_length); 

	return (g_echo_state ? 1 : 0) && (g_preprocess_state? 1: 0);
}


int speex_func_destroy() {
	if (g_preprocess_state) {
		speex_preprocess_state_destroy(g_preprocess_state); 
		g_preprocess_state = nullptr;
	}
	if (g_echo_state) {
		speex_echo_state_destroy(g_echo_state);
		g_echo_state = nullptr;
	}
	return 0;
}

int speex_func_echo_cancel(short* mic, short* play, short* out) {
	if (!g_echo_state || !g_preprocess_state) {
		return 0;
	}
	speex_echo_cancellation(g_echo_state, 
		(const spx_int16_t*)mic,
		(const spx_int16_t*)play,
		(spx_int16_t*)out
		);
	return 0;
}