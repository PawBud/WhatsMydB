#ifndef AUDIOINFO_H
#define AUDIOINFO_H

#ifndef AUDIO_DEVICE_INFO_H
#define AUDIO_DEVICE_INFO_H

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>

void getDeviceID(AudioDeviceID &device_id);

void getSampleRate(AudioDeviceID device_id, double &sample_rate);

void getFormatID(AudioDeviceID device_id, AudioFormatID &audio_format);

void getFormatFlags(AudioDeviceID device_id, AudioFormatFlags &audio_format_flags);

void getBitsPerChannel(AudioDeviceID device_id, unsigned int &bits_per_channel);

void getBytesPerFrame(AudioDeviceID device_id, unsigned int &bytes_per_frame);

void getChannelsPerFrame(AudioDeviceID device_id, unsigned int &channels_per_frame);

void getBytesPerPacket(AudioDeviceID device_id, unsigned int &bytes_per_packet);

void getFramesPerPacket(AudioDeviceID device_id, unsigned int &frames_per_packet);

unsigned int getBufferFrameSize(AudioUnit const audioUnit);

#endif // AUDIO_DEVICE_INFO_H

#endif //AUDIOINFO_H
