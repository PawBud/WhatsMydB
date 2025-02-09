#include <iostream>
#include <cstdint>
#include "GetAudioDevice.h"
#include "AudioInfo.h"
#include <arpa/inet.h>
#include <AudioUnit/AudioUnit.h>


struct AudioCallbackData {
    AudioUnit  audioUnit;
    AudioStreamBasicDescription stream_format;
    AudioBufferList* inputBuffer;
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
    AudioBufferList*& inputBuffer = callbackData->inputBuffer;

    unsigned int bufferSizeBytes = getBufferFrameSize(audioUnit);

    if (streamFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved) {

        std::cout << "format is non-interleaved\n" << std::endl;
        // allocate an AudioBufferList plus enough space for array of AudioBuffers
        unsigned int propsize = offsetof(AudioBufferList, mBuffers) + (sizeof(AudioBuffer) * streamFormat.mChannelsPerFrame);

        //malloc buffer lists
        inputBuffer = (AudioBufferList *)malloc(propsize);
        inputBuffer->mNumberBuffers = streamFormat.mChannelsPerFrame;

        //pre-malloc buffers for AudioBufferLists
        for(unsigned int i = 0; i< inputBuffer->mNumberBuffers ; i++) {
            inputBuffer->mBuffers[i].mNumberChannels = 1;
            inputBuffer->mBuffers[i].mDataByteSize = bufferSizeBytes;
            inputBuffer->mBuffers[i].mData = malloc(bufferSizeBytes);
        }
    } else {
        // printf ("format is interleaved\n");

        // allocate an AudioBufferList plus enough space for array of AudioBuffers
        UInt32 propsize = offsetof(AudioBufferList, mBuffers) + (sizeof(AudioBuffer) * 1);

        //malloc buffer lists
        inputBuffer = (AudioBufferList *)malloc(propsize);
        inputBuffer->mNumberBuffers = 1;

        //pre-malloc buffers for AudioBufferLists
        inputBuffer->mBuffers[0].mNumberChannels = streamFormat.mChannelsPerFrame;
        inputBuffer->mBuffers[0].mDataByteSize = bufferSizeBytes;
        inputBuffer->mBuffers[0].mData = malloc(bufferSizeBytes);
    }

    // Fetch the audio data from the system
    OSStatus status = AudioUnitRender(audioUnit, ioActionFlags, inTimeStamp,
                                      inBusNumber, inNumberFrames, inputBuffer);
    if (status != noErr) {
        std::cerr << "Error rendering audio: " << status << std::endl;
        free(inputBuffer->mBuffers[0].mData);
        return status;
    }

    // Process audio data: Convert to dB
    int32_t* audioData = static_cast<int32_t*>(inputBuffer->mBuffers[0].mData);
    int numSamples = inputBuffer->mBuffers[0].mDataByteSize / sizeof(int32_t);

    std::cout << "Buffer size: " << inputBuffer->mBuffers[0].mDataByteSize
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

    // generate description that will match audio HAL
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

    // Create an Audio Unit
    AudioUnit audioUnit;
    OSStatus status = AudioComponentInstanceNew(comp, &audioUnit);
    if (status != noErr) {
        std::cerr << "Error creating audio component instance: " << status << std::endl;
        return;
    }

    // Enable input
    int enableIO = 1;
    int disable_flag = 0;
    AudioUnitScope outputBus = 0;
    AudioUnitScope inputBus = 1;
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
