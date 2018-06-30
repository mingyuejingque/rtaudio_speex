all: from_mic  from_file play


from_mic: *.cpp
	g++ -o from_mic -static -Wall \
	from_mic.cpp RtAudio.cpp speex_func.cpp   \
	-std=c++11 \
	-lole32 -lwinmm -ldsound \
	-lspeexdsp 				\
	-I./include	\
	-I/G/install/include   \
	-L./lib 		\
	-D__WINDOWS_DS__  		\



from_file: *.cpp
	g++ -o from_file -static -Wall \
	from_file.cpp RtAudio.cpp speex_func.cpp   \
	-std=c++11 \
	-lole32 -lwinmm -ldsound \
	-lspeexdsp 				\
	-I./include	\
	-I/G/install/include   \
	-L./lib 		\
	-D__WINDOWS_DS__  		\

play: play.cpp	
	g++ -o play -static -Wall \
	play.cpp RtAudio.cpp    \
	-std=c++11 \
	-lole32 -lwinmm -ldsound \
	-lspeexdsp 				\
	-I./include	\
	-I/G/install/include   \
	-L./lib 		\
	-D__WINDOWS_DS__  		\

clean: 
	rm -rf from_file  from_mic  play