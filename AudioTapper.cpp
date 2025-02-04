#include <iostream>
#include <iomanip>
#include "GetAudioDevice.h"
#include "AudioInfo.h"
#include <arpa/inet.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

std::ostream& operator<<(std::ostream& out, const AudioStreamBasicDescription& format)
{
    unsigned char formatID [5];
    *(UInt32 *)formatID = OSSwapHostToBigInt32(format.mFormatID);
    formatID[4] = '\0';

    // General description
    std::cout << format.mBytesPerFrame << ", " << format.mChannelsPerFrame << " ch, " << format.mSampleRate << " Hz, '" << formatID << "' (0x" << std::hex << std::setw(8) << std::setfill('0') << format.mFormatFlags << std::dec << ") ";

    if(kAudioFormatLinearPCM == format.mFormatID) {
        // Bit depth
        UInt32 fractionalBits = ((0x3f << 7)/*kLinearPCMFormatFlagsSampleFractionMask*/ & format.mFormatFlags) >> 7/*kLinearPCMFormatFlagsSampleFractionShift*/;
        if(0 < fractionalBits)
            std::cout << (format.mBitsPerChannel - fractionalBits) << "." << fractionalBits;
        else
            std::cout << format.mBitsPerChannel;

        std::cout << "-bit";

        // Endianness
        bool isInterleaved = !(kAudioFormatFlagIsNonInterleaved & format.mFormatFlags);
        UInt32 interleavedChannelCount = (isInterleaved ? format.mChannelsPerFrame : 1);
        UInt32 sampleSize = (0 < format.mBytesPerFrame && 0 < interleavedChannelCount ? format.mBytesPerFrame / interleavedChannelCount : 0);
        if(1 < sampleSize)
            std::cout << ((kLinearPCMFormatFlagIsBigEndian & format.mFormatFlags) ? " big-endian" : " little-endian");

        // Sign
        bool isInteger = !(kLinearPCMFormatFlagIsFloat & format.mFormatFlags);
        if(isInteger)
            std::cout << ((kLinearPCMFormatFlagIsSignedInteger & format.mFormatFlags) ? " signed" : " unsigned");

        // Integer or floating
        std::cout << (isInteger ? " integer" : " float");

        // Packedness
        if(0 < sampleSize && ((sampleSize << 3) != format.mBitsPerChannel))
            std::cout << ((kLinearPCMFormatFlagIsPacked & format.mFormatFlags) ? ", packed in " : ", unpacked in ") << sampleSize << " bytes";

        // Alignment
        if((0 < sampleSize && ((sampleSize << 3) != format.mBitsPerChannel)) || (0 != (format.mBitsPerChannel & 7)))
            std::cout << ((kLinearPCMFormatFlagIsAlignedHigh & format.mFormatFlags) ? " high-aligned" : " low-aligned");

        if(!isInterleaved)
            std::cout << ", deinterleaved";
    }
    else if(kAudioFormatAppleLossless == format.mFormatID) {
        UInt32 sourceBitDepth = 0;
        switch(format.mFormatFlags) {
            case kAppleLosslessFormatFlag_16BitSourceData:      sourceBitDepth = 16;    break;
            case kAppleLosslessFormatFlag_20BitSourceData:      sourceBitDepth = 20;    break;
            case kAppleLosslessFormatFlag_24BitSourceData:      sourceBitDepth = 24;    break;
            case kAppleLosslessFormatFlag_32BitSourceData:      sourceBitDepth = 32;    break;
        }

        if(0 != sourceBitDepth)
            std::cout << "from " << sourceBitDepth << "-bit source, ";
        else
            std::cout << "from UNKNOWN source bit depth, ";

        std::cout << format.mFramesPerPacket << " frames/packet";
    }
    else
        std::cout << format.mBitsPerChannel << " bits/channel, " << format.mBytesPerPacket << " bytes/packet, " << format.mFramesPerPacket << " frames/packet, " << format.mBytesPerFrame << " bytes/frame";

    return out;
}


OSStatus RenderAudio(AudioUnit audioUnit) {
    OSStatus status = noErr;
    AudioBufferList bufferList = {0};

    // Set number of buffers (1 buffer for stereo)
    bufferList.mNumberBuffers = 1;

    // Number of channels (2 channels for stereo)
    bufferList.mBuffers[0].mNumberChannels = 2;  // Stereo

    // Set the buffer size (Assuming 512 frames, 6 bytes per frame for 24-bit stereo)
    size_t bufferSize = 512 * 6;
    bufferList.mBuffers[0].mData = malloc(bufferSize);
    bufferList.mBuffers[0].mDataByteSize = bufferSize;

    if (bufferList.mBuffers[0].mData == nullptr) {
        std::cerr << "Error allocating memory for buffer!" << std::endl;
        return -1;
    }

    // Create a valid timestamp
    AudioTimeStamp inTimeStamp = {0};  // Initialize it to 0
    inTimeStamp.mSampleTime = 0;  // You can adjust this time depending on your needs

    // Set flags to 0 (no action flags)
    AudioUnitRenderActionFlags ioActionFlags = 0;

    // Use 0 for busNumber if it's the default audio output
    UInt32 inBusNumber = 0;

    // Render audio
    status = AudioUnitRender(audioUnit, &ioActionFlags, &inTimeStamp, inBusNumber, 512, &bufferList);
    if (status != noErr) {
        std::cerr << "AudioUnitRender failed with error: " << status << std::endl;
        free(bufferList.mBuffers[0].mData);  // Free allocated memory
        return status;
    }

    // Use bufferList.mBuffers[0].mData for the audio data here
    // Make sure to process the audio data after rendering

    free(bufferList.mBuffers[0].mData);  // Free allocated memory
    return status;
}

OSStatus AudioTapCallback(void* inRefCon,
                          AudioUnitRenderActionFlags* ioActionFlags,
                          const AudioTimeStamp* inTimeStamp,
                          unsigned int inBusNumber,
                          unsigned int inNumberFrames,
                          AudioBufferList* ioData) {
    const float referenceAmplitude = 8388608.0f; // Max amplitude for 24-bit signed integer (2^23)
    float sumSquares = 0.0f;  // For RMS calculation
    int totalSamples = 0;

    for (unsigned int bufferIndex = 0; bufferIndex < ioData->mNumberBuffers; ++bufferIndex) {
        AudioBuffer buffer = ioData->mBuffers[bufferIndex];
        int32_t* audioData = static_cast<int32_t*>(buffer.mData); // Assuming 24-bit data is packed in 32-bit
        int dataSize = buffer.mDataByteSize / sizeof(int32_t); // Number of samples in this buffer

        std::cout << "Buffer size: " << buffer.mDataByteSize << " bytes, Number of samples: " << dataSize << std::endl;

        for (int i = 0; i < dataSize; ++i) {
            // Check if the sample data is in the expected range
            int32_t rawSample = audioData[i];
            std::cout << "Sample " << i << ": " << rawSample << std::endl;

            // Normalize to [-1, 1] by dividing by the reference amplitude
            float normalizedSample = static_cast<float>(rawSample) / referenceAmplitude;
            sumSquares += normalizedSample * normalizedSample;
            totalSamples++;
        }
    }

    if (totalSamples > 0) {
        float rms = sqrt(sumSquares / totalSamples);  // RMS value
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

    // if (inputASBL.mFormatFlags != kAudioFormatFlagsNativeFloatPacked) {
    //     std::cerr << "Unexpected format flags!" << std::endl;
    //     return;
    // }


    std::cout << "Audio tap is running. Press any key to stop." << std::endl;
    getchar();

    AudioOutputUnitStop(audioUnit);
    AudioComponentInstanceDispose(audioUnit);
}
