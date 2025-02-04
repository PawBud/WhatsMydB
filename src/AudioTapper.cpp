#include <iostream>
#include <cstdint>
#include "GetAudioDevice.h"
#include "AudioInfo.h"
#include <arpa/inet.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>


struct AudioCallbackData {
    AudioUnit  audioUnit;
    AudioStreamBasicDescription stream_format;
};

OSStatus AudioTapCallback(void* inRefCon,
                          AudioUnitRenderActionFlags* ioActionFlags,
                          const AudioTimeStamp* inTimeStamp,
                          UInt32 inBusNumber,
                          UInt32 inNumberFrames,
                          AudioBufferList* ioData) {

    const float referenceAmplitude = 8388608.0f;
    float sumSquares = 0.0f;
    int totalSamples = 0;

    // Retrieve callback data
    AudioCallbackData* callbackData = static_cast<AudioCallbackData*>(inRefCon);
    if (!callbackData) {
        std::cerr << "Error: Callback data is NULL!" << std::endl;
        return -1;
    }

    AudioUnit audioUnit = callbackData->audioUnit;
    AudioStreamBasicDescription streamFormat = callbackData->stream_format;

    AudioBufferList* bufferList = (AudioBufferList*)malloc(sizeof(AudioBufferList) + sizeof(AudioBuffer));
    if (!bufferList) {
        std::cerr << "Error: Failed to allocate buffer list!" << std::endl;
        return -1;
    }

    std::cout << "mChannelsPerFrame: " << streamFormat.mChannelsPerFrame << std::endl;
    std::cout << "mBytesPerFrame: " << streamFormat.mBytesPerFrame << std::endl;
    std::cout << "inNumberFrames: " << inNumberFrames << std::endl;
    std::cout << "inNumberFrames * streamFormat.mBytesPerFrame: " << inNumberFrames * streamFormat.mBytesPerFrame << std::endl;



    bufferList->mNumberBuffers = 1;
    bufferList->mBuffers[0].mNumberChannels = streamFormat.mChannelsPerFrame;
    bufferList->mBuffers[0].mDataByteSize = 1600;
    bufferList->mBuffers[0].mData = malloc(bufferList->mBuffers[0].mDataByteSize);

    if (!bufferList->mBuffers[0].mData) {
        std::cerr << "Error: Failed to allocate audio buffer!" << std::endl;
        free(bufferList);
        return -1;
    }

    // Fetch the audio data from the system
    OSStatus status = AudioUnitRender(audioUnit, ioActionFlags, inTimeStamp,
                                      inBusNumber, 200, bufferList);
    if (status != noErr) {
        std::cerr << "Error rendering audio: " << status << std::endl;
        free(bufferList->mBuffers[0].mData);
        return status;
    }

    // Process audio data: Convert to dB
    int32_t* audioData = static_cast<int32_t*>(bufferList->mBuffers[0].mData);
    int numSamples = bufferList->mBuffers[0].mDataByteSize / sizeof(int32_t);

    std::cout << "Buffer size: " << bufferList->mBuffers[0].mDataByteSize
              << " bytes, Number of samples: " << numSamples << std::endl;

    for (int i = 0; i < numSamples; ++i) {
        int32_t rawSample = audioData[i];

        // Normalize sample to [-1, 1]
        float normalizedSample = static_cast<float>(rawSample) / referenceAmplitude;
        sumSquares += normalizedSample * normalizedSample;
        totalSamples++;
    }

    // Compute RMS and convert to dB
    if (totalSamples > 0) {
        float rms = sqrt(sumSquares / totalSamples);
        float decibels = (rms > 0.0001f) ? 20.0f * log10(rms) : -96.0f; // Avoid log(0)

        std::cout << "RMS Level: " << decibels << " dB" << std::endl;
    } else {
        std::cerr << "No valid audio data received!" << std::endl;
    }

    // Clean up
    free(bufferList->mBuffers[0].mData);
    free(bufferList);
    return noErr;
}

void getDeviceStreamFormats(AudioDeviceID &device_id, AudioStreamBasicDescription &stream_format){
    // now we have to get each of the fields of the AudioStreamBasicDescription struct
    // by getting each of the fields one-by-one using the deviceID
    // Following are the fields
    // mSampleRate
    double current_sample_rate;
    getSampleRate(device_id, current_sample_rate);
    // mFormatID
    AudioFormatID current_audio_format_id;
    getFormatID(device_id, current_audio_format_id);
    // mFormatFlags
    AudioFormatFlags current_audio_format_flags;
    getFormatFlags(device_id, current_audio_format_flags);
    // mBitsPerChannel
    unsigned int current_bits_per_chanel;
    getBitsPerChannel(device_id, current_bits_per_chanel);
    // mChannelsPerFrame
    unsigned int current_channels_per_frame;
    getChannelsPerFrame(device_id, current_channels_per_frame);
    // mBytesPerFrame
    unsigned int current_bytes_per_frame;
    getBytesPerFrame(device_id, current_bytes_per_frame);
    // mBytesPerPacket
    unsigned int current_bytes_per_packet;
    getBytesPerPacket(device_id, current_bytes_per_packet);
    // mFramesPerPacket
    unsigned int current_frames_per_packet;
    getFramesPerPacket(device_id, current_frames_per_packet);


    stream_format.mSampleRate = current_sample_rate;
    stream_format.mFormatID = current_audio_format_id;
    stream_format.mFormatFlags = current_audio_format_flags;
    stream_format.mBitsPerChannel = current_bits_per_chanel;
    stream_format.mChannelsPerFrame = current_channels_per_frame;
    stream_format.mBytesPerFrame = current_bytes_per_frame;
    stream_format.mBytesPerPacket = current_bytes_per_packet;
    stream_format.mFramesPerPacket = current_frames_per_packet;
}

void SetupAudioTap(AudioDeviceID deviceID) {
    // Get the stream format
    AudioStreamBasicDescription stream_format;
    getDeviceStreamFormats(deviceID, stream_format);

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

    // Create an Audio Unit
    AudioUnit audioUnit;
    OSStatus status = AudioComponentInstanceNew(comp, &audioUnit);
    if (status != noErr) {
        std::cerr << "Error creating audio component instance: " << status << std::endl;
        return;
    }

    // Enable input
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

    // Set the stream format
    status = AudioUnitSetProperty(audioUnit, kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Output, 1, &stream_format, sizeof(stream_format));
    if (status != noErr) {
        std::cerr << "Error setting stream format: " << status << std::endl;
        return;
    }

    // Passing the data to the callback struct
    AudioCallbackData* audio_callback_data = new AudioCallbackData();
    audio_callback_data->audioUnit = audioUnit;
    audio_callback_data->stream_format = stream_format;

    // Set the input callback
    AURenderCallbackStruct callback;
    callback.inputProc = AudioTapCallback;
    callback.inputProcRefCon = audio_callback_data;

    status = AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Input, 0, &callback, sizeof(callback));
    if (status != noErr) {
        std::cerr << "Error setting input callback: " << status << std::endl;
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
    delete audio_callback_data;
}
