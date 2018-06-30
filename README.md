# rtaudio_speex

#### 项目介绍
用rtaudio来采集、播放，并用speexdsp来做回声消除。

multi_thread 分支在windows 下测试效果挺好的。

remote.pcm 假装是远端发来的声音。
cap.pcm 就是mic直接抓到的声音。
cancel.pcm 就是经过speex回音消除处理之后的输出声音。