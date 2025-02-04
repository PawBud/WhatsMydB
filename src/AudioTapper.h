#ifndef AUDIOTAPPER_H
#define AUDIOTAPPER_H
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

void SetupAudioTap(AudioDeviceID deviceID);

// incomplete

OSStatus AudioTapCallback(void* inRefCon,
                      AudioUnitRenderActionFlags* ioActionFlags,
                      const AudioTimeStamp* inTimeStamp,
                      unsigned int inBusNumber,
                      unsigned int inNumberFrames,
                      AudioBufferList* ioData);

#endif //AUDIOTAPPER_H
