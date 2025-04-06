#include <iostream>
#include <cstdint>
#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include "VirtualAudioDriver.h"

#include <cmath> // Needed for log10, sqrt

struct AudioCallbackData {
    AudioUnit audioUnit;
};

// Callback function to process audio
OSStatus RenderCallback(void* inRefCon,
                        AudioUnitRenderActionFlags* ioActionFlags,
                        const AudioTimeStamp* inTimeStamp,
                        UInt32 inBusNumber,
                        UInt32 inNumberFrames,
                        AudioBufferList* ioData) {
    AudioCallbackData* callbackData = static_cast<AudioCallbackData*>(inRefCon);
    AudioUnit audioUnit = callbackData->audioUnit;

    // Prepare buffer to receive input audio
    AudioBufferList bufferList;
    bufferList.mNumberBuffers = 1;
    bufferList.mBuffers[0].mNumberChannels = 2;
    bufferList.mBuffers[0].mDataByteSize = inNumberFrames * sizeof(Float32) * 2;
    Float32* buffer = (Float32*)calloc(inNumberFrames * 2, sizeof(Float32)); // 2 channels
    if (!buffer) return -1;

    bufferList.mBuffers[0].mData = buffer;

    // Render the audio from the input bus (BlackHole)
    OSStatus status = AudioUnitRender(audioUnit, ioActionFlags, inTimeStamp, 1, inNumberFrames, &bufferList);
    if (status != noErr) {
        std::cerr << "Error rendering audio: " << status << std::endl;
        free(buffer);
        return status;
    }

    // Compute RMS to calculate the dB level
    Float32* samples = (Float32*)bufferList.mBuffers[0].mData;
    double sumSquares = 0.0;
    int totalSamples = inNumberFrames * bufferList.mBuffers[0].mNumberChannels;

    for (int i = 0; i < totalSamples; ++i) {
        sumSquares += samples[i] * samples[i];
    }

    float rms = sqrt(sumSquares / totalSamples);
    float db = (rms > 0.0001f) ? 20.0f * log10(rms) : -96.0f;
    std::cout << "Input dB Level: " << db << " dB" << std::endl;

    // Pass the audio to the output bus (Scarlett)
    for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
        AudioBuffer& outBuffer = ioData->mBuffers[i];
        Float32* outputBuffer = (Float32*)outBuffer.mData;
        // Copy input buffer to output buffer
        memcpy(outputBuffer, samples, inNumberFrames * sizeof(Float32) * 2);
    }

    // Clean up
    free(buffer);
    return noErr;
}
// OSStatus RenderCallback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp,
//                         UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) {
//
//     AudioCallbackData* callbackData = (AudioCallbackData*) inRefCon;
//     AudioUnit audioUnit = callbackData->audioUnit;
//
//     // Debug: Print information about the incoming buffer
//     if (inBusNumber == 1) { // Input bus
//         std::cout << "Receiving audio on input bus 1" << std::endl;
//     }
//
//     // Check for valid buffer data
//     if (ioData == nullptr || ioData->mNumberBuffers == 0) {
//         std::cerr << "Error: No buffer data received!" << std::endl;
//         return noErr;
//     }
//
//     // For each buffer, check its size and content
//     for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
//         AudioBuffer& buffer = ioData->mBuffers[i];
//         std::cout << "Buffer " << i << ": " << buffer.mDataByteSize << " bytes of data." << std::endl;
//
//         // If the buffer is empty or has very small data, it might be a problem.
//         if (buffer.mDataByteSize == 0) {
//             std::cerr << "Warning: Empty buffer received!" << std::endl;
//         }
//     }
//
//     // Render the audio to the output (Scarlett device)
//     OSStatus status = AudioUnitRender(audioUnit, ioActionFlags, inTimeStamp, 0, inNumberFrames, ioData);
//     if (status != noErr) {
//         std::cerr << "Error rendering audio: " << status << std::endl;
//         return status;
//     }
//
//     return noErr;
// }

AudioDeviceID getDeviceIDByName(const std::string& targetName) {
    UInt32 size = 0;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &size);
    if (status != noErr) return kAudioObjectUnknown;

    int numDevices = size / sizeof(AudioDeviceID);
    AudioDeviceID* devices = new AudioDeviceID[numDevices];

    status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &size, devices);
    if (status != noErr) {
        delete[] devices;
        return kAudioObjectUnknown;
    }

    for (int i = 0; i < numDevices; ++i) {
        CFStringRef deviceName = nullptr;
        UInt32 nameSize = sizeof(deviceName);

        AudioObjectPropertyAddress nameAddress = {
            kAudioDevicePropertyDeviceNameCFString,
            kAudioObjectPropertyScopeGlobal,
            kAudioObjectPropertyElementMain
        };

        if (AudioObjectGetPropertyData(devices[i], &nameAddress, 0, nullptr, &nameSize, &deviceName) == noErr) {
            char name[256];
            if (CFStringGetCString(deviceName, name, sizeof(name), kCFStringEncodingUTF8)) {
                if (targetName == name) {
                    AudioDeviceID foundID = devices[i];
                    delete[] devices;
                    return foundID;
                }
            }
        }
    }

    delete[] devices;
    return kAudioObjectUnknown;
}

void printAudioDeviceList() {
    UInt32 size = 0;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &size);
    if (status != noErr) {
        std::cerr << "Error getting device list size!" << std::endl;
        return;
    }

    int numDevices = size / sizeof(AudioDeviceID);
    AudioDeviceID* devices = new AudioDeviceID[numDevices];

    status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &size, devices);
    if (status != noErr) {
        std::cerr << "Error getting device list!" << std::endl;
        delete[] devices;
        return;
    }

    for (int i = 0; i < numDevices; ++i) {
        AudioDeviceID deviceID = devices[i];
        AudioObjectPropertyAddress nameAddress = {
            kAudioDevicePropertyDeviceNameCFString,
            kAudioObjectPropertyScopeGlobal,
            kAudioObjectPropertyElementMain
        };

        CFStringRef deviceName;
        size = sizeof(deviceName);
        status = AudioObjectGetPropertyData(deviceID, &nameAddress, 0, nullptr, &size, &deviceName);

        if (status == noErr) {
            char name[256];
            if (CFStringGetCString(deviceName, name, sizeof(name), kCFStringEncodingUTF8)) {
                std::cout << "Device " << i << ": " << name << std::endl;
            }
        } else {
            std::cerr << "Error retrieving device name!" << std::endl;
        }
    }

    delete[] devices;
}

void SetupBlackHoleTap() {
    // NOTE :-
    // Tried adding a multi output device, and using the multi output device as the input and the output,
    // Error starting AudioUnit: -10867
    // Get the AudioDeviceID for the Aggregate Device
    AudioDeviceID aggregateDeviceID = getDeviceIDByName("Black-Scar");

    if (aggregateDeviceID == kAudioObjectUnknown) {
        std::cerr << "Error: Aggregate Device not found!" << std::endl;
        return;
    }

    std::cout << "Found Aggregate Device ID: " << aggregateDeviceID << std::endl;

    // Set up HAL Output AudioUnit
    AudioComponentDescription hal_desc = {
        kAudioUnitType_Output,
        kAudioUnitSubType_HALOutput,
        kAudioUnitManufacturer_Apple
    };

    AudioComponent comp = AudioComponentFindNext(nullptr, &hal_desc);
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

    // Enable input and output
    int enableInput = 1;
    status = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &enableInput, sizeof(enableInput));
    if (status != noErr) {
        std::cerr << "Error enabling input: " << status << std::endl;
        return;
    }

    int enableOutput = 1;
    status = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enableOutput, sizeof(enableOutput));
    if (status != noErr) {
        std::cerr << "Error enabling output: " << status << std::endl;
        return;
    }

    // Set Aggregate Device as input and output device
    status = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 1, &aggregateDeviceID, sizeof(aggregateDeviceID));
    if (status != noErr) {
        std::cerr << "Error setting input device for Aggregate: " << status << std::endl;
        return;
    }

    status = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &aggregateDeviceID, sizeof(aggregateDeviceID));
    if (status != noErr) {
        std::cerr << "Error setting output device for Aggregate: " << status << std::endl;
        return;
    }

    // Set up callback and stream format as necessary...
    // Continue with your callback setup as you had before.
    status = AudioOutputUnitStart(audioUnit);
    if (status != noErr) {
        std::cerr << "Error starting AudioUnit: " << status << std::endl;
        return;
    }

    std::cout << "Audio tap is running on Aggregate Device. Press Enter to stop." << std::endl;
    getchar();

    AudioOutputUnitStop(audioUnit);
    AudioComponentInstanceDispose(audioUnit);
}
