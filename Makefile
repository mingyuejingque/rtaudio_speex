test: *.cpp
	g++ -o test -static -Wall \
	*.cpp \
	-std=c++11 \
	-lole32 -lwinmm -ldsound \
	-I/G/open_source_lib/boost_1_64_0 \
	-I./include	\
	-L/G/open_source_lib/boost_1_64_0/lib64-msvc-14.1
	-D__WINDOWS_DS__  \
