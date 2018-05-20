#ifndef __SPEEX_FUNC_H__
#define __SPEEX_FUNC_H__

#pragma once

int speex_func_init(int frame_size, int filter_length);
int speex_func_destroy();
int speex_func_echo_cancel(short* mic, short* play, short* out);


#endif
