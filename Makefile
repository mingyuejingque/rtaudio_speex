echo_cancel_test: *.cpp
	g++ -o echo_cancel_test -static -Wall \
	*.cpp \
	-std=c++11 \
	-lole32 -lwinmm -ldsound \
	-lspeexdsp 				\
	-I./include	\
	-I/G/install/include   \
	-L/G/install/lib 		\
	-D__WINDOWS_DS__  		\
