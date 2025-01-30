#include <iostream>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

OSStatus AudioTapCallback(void* inRefCon,
                      AudioUnitRenderActionFlags* ioActionFlags,
                      const AudioTimeStamp* inTimeStamp,
                      unsigned int inBusNumber,
                      unsigned int inNumberFrames,
                      AudioBufferList* ioData) {
    // Access audio data
    AudioBuffer buffer = ioData->mBuffers[0];
    float* audioData = static_cast<float*>(buffer.mData);
    int dataSize = buffer.mDataByteSize / sizeof(float);

    // Process the audio data (e.g., calculate dB levels)
    for (int i = 0; i < dataSize; ++i) {
        float sample = audioData[i];
        // Example: Print the sample values
        // std::cout << sample << std::endl;
    }

    return NULL;
}

void SetupAudioTap(AudioDeviceID deviceID) {
    AudioComponentDescription desc = {
        kAudioUnitType_Output,
        kAudioUnitSubType_HALOutput,
        kAudioUnitManufacturer_Apple
    };

    AudioComponent comp = AudioComponentFindNext(nullptr, &desc);
    if (comp == nullptr) {
        std::cerr << "Error finding audio component" << std::endl;
        return;
    }

    AudioUnit audioUnit;
    OSStatus status = AudioComponentInstanceNew(comp, &audioUnit);
    if (status != noErr) {
        std::cerr << "Error creating audio component instance: " << status << std::endl;
        return;
    }

    // Enable input on the audio unit
    int enableIO = 1;
    status = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Input, 1, &enableIO, sizeof(enableIO));
    if (status != noErr) {
        std::cerr << "Error enabling IO: " << status << std::endl;
        return;
    }

    // Set the device
    status = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_CurrentDevice,
                                  kAudioUnitScope_Global, 0, &deviceID, sizeof(deviceID));
    if (status != noErr) {
        std::cerr << "Error setting output device: " << status << std::endl;
        return;
    }

    // Set the callback
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = AudioTapCallback;
    callbackStruct.inputProcRefCon = nullptr;

    status = AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Global, 0, &callbackStruct, sizeof(callbackStruct));
    if (status != noErr) {
        std::cerr << "Error setting render callback: " << status << std::endl;
        return;
    }

    // Initialize and start the audio unit
    status = AudioUnitInitialize(audioUnit);
    if (status != noErr) {
        std::cerr << "Error initializing audio unit: " << status << std::endl;
        return;
    }

    status = AudioOutputUnitStart(audioUnit);
    if (status != noErr) {
        std::cerr << "Error starting audio unit: " << status << std::endl;
    }

    std::cout << "Audio tap is running. Press any key to stop." << std::endl;
    getchar();

    AudioOutputUnitStop(audioUnit);
    AudioComponentInstanceDispose(audioUnit);
}