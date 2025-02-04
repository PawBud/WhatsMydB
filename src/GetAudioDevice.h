#ifndef GETAUDIODEVICE_H
#define GETAUDIODEVICE_H
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

OSStatus GetAudioDeviceName(AudioDeviceID deviceID, std::string &deviceName);
#endif //GETAUDIODEVICE_H
