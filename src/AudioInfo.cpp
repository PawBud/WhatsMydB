#include <iostream>
#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>

#include <iostream>
#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>


// useless code, to remove
void getDeviceID(AudioDeviceID &device_id) {
    unsigned int device_id_size = sizeof(device_id);
    // Get the current output device
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &device_id_size,
                                                 &device_id);
    if (status != noErr) {
        std::cerr << "Error getting the id of the audio device: " << status << std::endl;
        return;
    }
}

void getSampleRate(AudioDeviceID const device_id, double &sample_rate) {
    UInt32 sample_rate_size = sizeof(double);

    // The mSelector kAudioStreamPropertyPhysicalFormat or kAudioDevicePropertyNominalSampleRate doesn't work
    AudioObjectPropertyAddress propertyAddress = {
        kAudioDevicePropertyNominalSampleRate,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &sample_rate_size, &sample_rate);
    if (status != noErr) {
        std::cerr << "Error retrieving sample rate: " << status << std::endl;
    } else {
        std::cout << "Sample Rate: " << sample_rate << " Hz" << std::endl;
    }
}

void getFormatID(AudioDeviceID const device_id, AudioFormatID& audio_format) {
    UInt32 audio_format_size = sizeof(AudioStreamBasicDescription);
    AudioStreamBasicDescription stream_format;

    AudioObjectPropertyAddress propertyAddress = {
        kAudioDevicePropertyStreamFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    // Get the stream format
    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &audio_format_size, &stream_format);
    if (status != noErr) {
        std::cerr << "Error retrieving stream format: " << status << std::endl;
        return;
    }

    audio_format = stream_format.mFormatID;  // Assign correct format ID

    // Convert to human-readable format ID
    char formatID[5] = {0};
    UInt32 swappedFormat = CFSwapInt32HostToBig(audio_format);
    memcpy(formatID, &swappedFormat, sizeof(UInt32));

    std::cout << "Format ID: '" << formatID << "'" << std::endl;

    // Return the format marco
    // Check CoreAudioBaseTypes.h (https://developer.apple.com/documentation/coreaudiotypes) for all the types
    switch (audio_format) {
        case kAudioFormatLinearPCM:
            audio_format = kAudioFormatLinearPCM;
            std::cout << "Format ID: Linear PCM (kAudioFormatLinearPCM)" << std::endl;
            break;
        case kAudioFormatAC3:
            audio_format = kAudioFormatAC3;
            std::cout << "Format ID: AC3 (kAudioFormatAC3)" << std::endl;
            break;
        case kAudioFormat60958AC3:
            audio_format = kAudioFormat60958AC3;
            std::cout << "Format ID: AC3 over S/PDIF (kAudioFormat60958AC3)" << std::endl;
            break;
        case kAudioFormatAppleIMA4:
            audio_format = kAudioFormatAppleIMA4;
            std::cout << "Format ID: IMA4 (kAudioFormatAppleIMA4)" << std::endl;
            break;
        case kAudioFormatMPEG4AAC:
            audio_format = kAudioFormatMPEG4AAC;
            std::cout << "Format ID: AAC (kAudioFormatMPEG4AAC)" << std::endl;
            break;
        case kAudioFormatMPEG4CELP:
            audio_format = kAudioFormatMPEG4CELP;
            std::cout << "Format ID: MPEG-4 CELP (kAudioFormatMPEG4CELP)" << std::endl;
            break;
        case kAudioFormatMPEG4HVXC:
            audio_format = kAudioFormatMPEG4HVXC;
            std::cout << "Format ID: MPEG-4 HVXC (kAudioFormatMPEG4HVXC)" << std::endl;
            break;
        case kAudioFormatMPEG4TwinVQ:
            audio_format = kAudioFormatMPEG4TwinVQ;
            std::cout << "Format ID: MPEG-4 TwinVQ (kAudioFormatMPEG4TwinVQ)" << std::endl;
            break;
        case kAudioFormatMACE3:
            audio_format = kAudioFormatMACE3;
            std::cout << "Format ID: MACE 3:1 (kAudioFormatMACE3)" << std::endl;
            break;
        case kAudioFormatMACE6:
            audio_format = kAudioFormatMACE6;
            std::cout << "Format ID: MACE 6:1 (kAudioFormatMACE6)" << std::endl;
            break;
        case kAudioFormatULaw:
            audio_format = kAudioFormatULaw;
            std::cout << "Format ID: μ-Law (kAudioFormatULaw)" << std::endl;
            break;
        case kAudioFormatALaw:
            audio_format = kAudioFormatALaw;
            std::cout << "Format ID: A-Law (kAudioFormatALaw)" << std::endl;
            break;
        case kAudioFormatQDesign:
            audio_format = kAudioFormatQDesign;
            std::cout << "Format ID: QDesign (kAudioFormatQDesign)" << std::endl;
            break;
        case kAudioFormatQDesign2:
            audio_format = kAudioFormatQDesign2;
            std::cout << "Format ID: QDesign 2 (kAudioFormatQDesign2)" << std::endl;
            break;
        case kAudioFormatQUALCOMM:
            audio_format = kAudioFormatQUALCOMM;
            std::cout << "Format ID: Qualcomm PureVoice (kAudioFormatQUALCOMM)" << std::endl;
            break;
        case kAudioFormatMPEGLayer1:
            audio_format = kAudioFormatMPEGLayer1;
            std::cout << "Format ID: MPEG-1 Layer 1 (kAudioFormatMPEGLayer1)" << std::endl;
            break;
        case kAudioFormatMPEGLayer2:
            audio_format = kAudioFormatMPEGLayer2;
            std::cout << "Format ID: MPEG-1 Layer 2 (kAudioFormatMPEGLayer2)" << std::endl;
            break;
        case kAudioFormatMPEGLayer3:
            audio_format = kAudioFormatMPEGLayer3;
            std::cout << "Format ID: MP3 (kAudioFormatMPEGLayer3)" << std::endl;
            break;
        case kAudioFormatTimeCode:
            audio_format = kAudioFormatTimeCode;
            std::cout << "Format ID: TimeCode (kAudioFormatTimeCode)" << std::endl;
            break;
        case kAudioFormatMIDIStream:
            audio_format = kAudioFormatMIDIStream;
            std::cout << "Format ID: MIDI Stream (kAudioFormatMIDIStream)" << std::endl;
            break;
        case kAudioFormatParameterValueStream:
            audio_format = kAudioFormatParameterValueStream;
            std::cout << "Format ID: Parameter Value Stream (kAudioFormatParameterValueStream)" << std::endl;
            break;
        case kAudioFormatAppleLossless:
            audio_format = kAudioFormatAppleLossless;
            std::cout << "Format ID: Apple Lossless (kAudioFormatAppleLossless)" << std::endl;
            break;
        case kAudioFormatMPEG4AAC_HE:
            audio_format = kAudioFormatMPEG4AAC_HE;
            std::cout << "Format ID: AAC High Efficiency (kAudioFormatMPEG4AAC_HE)" << std::endl;
            break;
        case kAudioFormatMPEG4AAC_LD:
            audio_format = kAudioFormatMPEG4AAC_LD;
            std::cout << "Format ID: AAC Low Delay (kAudioFormatMPEG4AAC_LD)" << std::endl;
            break;
        case kAudioFormatMPEG4AAC_ELD:
            audio_format = kAudioFormatMPEG4AAC_ELD;
            std::cout << "Format ID: AAC Enhanced Low Delay (kAudioFormatMPEG4AAC_ELD)" << std::endl;
            break;
        case kAudioFormatMPEG4AAC_ELD_SBR:
            audio_format = kAudioFormatMPEG4AAC_ELD_SBR;
            std::cout << "Format ID: AAC ELD+SBR (kAudioFormatMPEG4AAC_ELD_SBR)" << std::endl;
            break;
        case kAudioFormatMPEG4AAC_ELD_V2:
            audio_format = kAudioFormatMPEG4AAC_ELD_V2;
            std::cout << "Format ID: AAC ELD v2 (kAudioFormatMPEG4AAC_ELD_V2)" << std::endl;
            break;
        case kAudioFormatMPEG4AAC_HE_V2:
            audio_format = kAudioFormatMPEG4AAC_HE_V2;
            std::cout << "Format ID: AAC HE v2 (kAudioFormatMPEG4AAC_HE_V2)" << std::endl;
            break;
        case kAudioFormatMPEG4AAC_Spatial:
            audio_format = kAudioFormatMPEG4AAC_Spatial;
            std::cout << "Format ID: MPEG-4 AAC Spatial (kAudioFormatMPEG4AAC_Spatial)" << std::endl;
            break;
        case kAudioFormatAMR:
            audio_format = kAudioFormatAMR;
            std::cout << "Format ID: AMR Narrowband (kAudioFormatAMR)" << std::endl;
            break;
        case kAudioFormatAMR_WB:
            audio_format = kAudioFormatAMR_WB;
            std::cout << "Format ID: AMR Wideband (kAudioFormatAMR_WB)" << std::endl;
            break;
        case kAudioFormatAudible:
            audio_format = kAudioFormatAudible;
            std::cout << "Format ID: Audible format (kAudioFormatAudible)" << std::endl;
            break;
        case kAudioFormatiLBC:
            audio_format = kAudioFormatiLBC;
            std::cout << "Format ID: iLBC (kAudioFormatiLBC)" << std::endl;
            break;
        case kAudioFormatDVIIntelIMA:
            audio_format = kAudioFormatDVIIntelIMA;
            std::cout << "Format ID: DVI/Intel IMA ADPCM (kAudioFormatDVIIntelIMA)" << std::endl;
            break;
        case kAudioFormatMicrosoftGSM:
            audio_format = kAudioFormatMicrosoftGSM;
            std::cout << "Format ID: Microsoft GSM (kAudioFormatMicrosoftGSM)" << std::endl;
            break;
        case kAudioFormatAES3:
            audio_format = kAudioFormatAES3;
            std::cout << "Format ID: AES3 (kAudioFormatAES3)" << std::endl;
            break;
        default:
            std::cout << "Format ID: Unknown (" << audio_format << ")" << std::endl;
            break;
    }
}

void getFormatFlags(AudioDeviceID const device_id, AudioFormatFlags &audio_format_flags) {
    AudioStreamBasicDescription stream_format;
    UInt32 stream_format_size = sizeof(AudioStreamBasicDescription);

    AudioObjectPropertyAddress propertyAddress = {
        kAudioDevicePropertyStreamFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    // Retrieve the full stream format
    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &stream_format_size, &stream_format);
    if (status != noErr) {
        std::cerr << "Error retrieving format flags: " << status << std::endl;
    } else {
        audio_format_flags = stream_format.mFormatFlags;
        std::cout << "Format Flags: " << std::hex << audio_format_flags << std::endl;
    }
}

//void getBitsPerChannel(AudioDeviceID const device_id, unsigned int &bits_per_channel) {
//    AudioStreamBasicDescription stream_format;
//    UInt32 stream_format_size = sizeof(AudioStreamBasicDescription);
//
//    AudioObjectPropertyAddress propertyAddress = {
//        kAudioStreamPropertyPhysicalFormat,
//        kAudioObjectPropertyScopeOutput,
//        kAudioObjectPropertyElementMain
//    };
//
//    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &stream_format_size, &stream_format);
//    if (status != noErr) {
//        std::cerr << "Error retrieving stream format: " << status << std::endl;
//    } else {
//        bits_per_channel = stream_format.mBitsPerChannel;
//        std::cout << "THIS IS Bits Per Channel: " << bits_per_channel << std::endl;
//    }
//}

void getBitsPerChannel(AudioDeviceID const device_id, unsigned int &bits_per_channel) {
    AudioStreamBasicDescription stream_format;
    UInt32 stream_format_size = sizeof(AudioStreamBasicDescription);

    // Prepare address for stream format
    AudioObjectPropertyAddress propertyAddress = {
        kAudioStreamPropertyPhysicalFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    // Get stream format
    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &stream_format_size, &stream_format);
    if (status != noErr) {
        std::cerr << "Error retrieving stream format: " << status << std::endl;
        return;
    }

//    // Print key stream format info
//    std::cout << "Sample Rate: " << stream_format.mSampleRate << " Hz" << std::endl;
//    std::cout << "Format ID: '"
//              << static_cast<char>((stream_format.mFormatID >> 24) & 0xFF)
//              << static_cast<char>((stream_format.mFormatID >> 16) & 0xFF)
//              << static_cast<char>((stream_format.mFormatID >> 8) & 0xFF)
//              << static_cast<char>((stream_format.mFormatID) & 0xFF)
//              << "' (0x" << std::hex << stream_format.mFormatID << std::dec << ")" << std::endl;

    // Check format flags
    std::cout << "Format Flags: " << std::hex << stream_format.mFormatFlags << std::dec << std::endl;
    if (stream_format.mFormatFlags & kAudioFormatFlagIsFloat) {
        std::cout << "Format is Float" << std::endl;
    }
    if (stream_format.mFormatFlags & kAudioFormatFlagIsPacked) {
        std::cout << "Format is Packed" << std::endl;
    }
    if (stream_format.mFormatFlags & kAudioFormatFlagIsSignedInteger) {
        std::cout << "Format is Signed Integer" << std::endl;
    }

    // Finally: bits per channel
    bits_per_channel = stream_format.mBitsPerChannel;
    std::cout << "THIS IS NOW THE Bits Per Channel: " << bits_per_channel << std::endl;
}


void getBytesPerFrame(AudioDeviceID const device_id, unsigned int &bytes_per_frame) {
    AudioStreamBasicDescription stream_format;
    UInt32 stream_format_size = sizeof(AudioStreamBasicDescription);

    AudioObjectPropertyAddress propertyAddress = {
        kAudioStreamPropertyPhysicalFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &stream_format_size, &stream_format);
    if (status != noErr) {
        std::cerr << "Error retrieving bytes per frame: " << status << std::endl;
    } else {
        bytes_per_frame = stream_format.mBytesPerFrame;
        std::cout << "Bytes Per Frame: " << bytes_per_frame << std::endl;
    }
}

void getChannelsPerFrame(AudioDeviceID const device_id, unsigned int &channels_per_frame) {
    AudioStreamBasicDescription stream_format;
    UInt32 stream_format_size = sizeof(AudioStreamBasicDescription);

    AudioObjectPropertyAddress propertyAddress = {
        kAudioStreamPropertyPhysicalFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &stream_format_size, &stream_format);
    if (status != noErr) {
        std::cerr << "Error retrieving stream format: " << status << std::endl;
    } else {
        channels_per_frame = stream_format.mChannelsPerFrame;  // Corrected field
        std::cout << "Channels Per Frame: " << channels_per_frame << std::endl;
    }
}

// could be calculated mathematically
void getBytesPerPacket(AudioDeviceID const device_id, unsigned int &bytes_per_packet) {
    AudioStreamBasicDescription stream_format;
    UInt32 stream_format_size = sizeof(AudioStreamBasicDescription);

    AudioObjectPropertyAddress propertyAddress = {
        kAudioStreamPropertyPhysicalFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &stream_format_size, &stream_format);
    if (status != noErr) {
        std::cerr << "Error retrieving stream format: " << status << std::endl;
    } else {
        bytes_per_packet = stream_format.mBytesPerPacket;
        std::cout << "Bytes Per Packet: " << bytes_per_packet << std::endl;
    }
}

// could be calculated mathematically
void getFramesPerPacket(AudioDeviceID const device_id, unsigned int &frames_per_packet) {
    AudioStreamBasicDescription stream_format;
    UInt32 stream_format_size = sizeof(AudioStreamBasicDescription);

    AudioObjectPropertyAddress propertyAddress = {
        kAudioStreamPropertyPhysicalFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(device_id, &propertyAddress, 0, nullptr, &stream_format_size, &stream_format);
    if (status != noErr) {
        std::cerr << "Error retrieving stream format: " << status << std::endl;
    } else {
        frames_per_packet = stream_format.mFramesPerPacket;
        std::cout << "Frames Per Packet: " << frames_per_packet << std::endl;
    }
}

unsigned int getBufferNumberOfFrames(AudioUnit const audioUnit) {
    unsigned int bufferNumberOfFrames = 0;
    unsigned int propertySize = sizeof(unsigned int);

    OSStatus status = AudioUnitGetProperty(audioUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &bufferNumberOfFrames, &propertySize);
    if (status != noErr) {
        std::cerr << "Couldn't get buffer frame size from input unit " << status << std::endl;
    }

    UInt32 bufferSizeBytes = bufferNumberOfFrames * sizeof(Float32);

    return bufferSizeBytes;
}

void printStreamFormatDetails(AudioDeviceID const device_id) {
}