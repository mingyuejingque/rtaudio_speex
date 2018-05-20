test: *.cpp
	g++ -o test -static -Wall \
	*.cpp \
	-std=c++11 \
	-lole32 -lwinmm -ldsound \
	-lspeexdsp 				\
	-I/G/open_source_lib/boost_1_64_0 \
	-I./include	\
	-I/G/install/include   \
	-L/G/install/lib 		\
	-D__WINDOWS_DS__  		\
