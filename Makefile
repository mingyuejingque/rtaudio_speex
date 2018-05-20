test: *.cpp
	g++ -o test -static -Wall \
	*.cpp \
	-std=c++11 \
	-lole32 -lwinmm -ldsound \
	-I/G/open_source_lib/boost_1_64_0 \
	-I./include	\
	-D__WINDOWS_DS__  \
