#include <iostream>
#include <cstdint>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include "AudioTapper.h"
#include "VirtualAudioDriver.h"

OSStatus GetAudioDeviceName(AudioDeviceID deviceID, std::string& deviceName)
{
    // Get the size of the device name
    unsigned int dataSize = 0;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioDevicePropertyDeviceNameCFString,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyDataSize(deviceID, &propertyAddress, 0, nullptr, &dataSize);
    if (status != noErr)
    {
        std::cerr << "Error getting property data size for device name: " << status << std::endl;
        return status;
    }

    // Allocate memory for the device name
    CFStringRef deviceNameCFString = nullptr;
    status = AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, nullptr, &dataSize, &deviceNameCFString);
    if (status != noErr)
    {
        std::cerr << "Error getting device name: " << status << std::endl;
        return status;
    }

    // Convert CFString to std::string
    const char* nameCStr = CFStringGetCStringPtr(deviceNameCFString, kCFStringEncodingUTF8);
    if (nameCStr != nullptr)
    {
        deviceName = std::string(nameCStr);
    }
    else
    {
        CFIndex length = CFStringGetLength(deviceNameCFString);
        CFRange range = CFRangeMake(0, length);
        char* buffer = new char[length + 1];
        if (CFStringGetCString(deviceNameCFString, buffer, length + 1, kCFStringEncodingUTF8))
        {
            deviceName = std::string(buffer);
        }
        delete[] buffer;
    }

    // Release the CFString
    CFRelease(deviceNameCFString);

    return noErr;
}

void getDeviceID(AudioDeviceID& device_id)
{
    unsigned int device_id_size = sizeof(device_id);
    // Get the current output device
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nullptr,
                                                 &device_id_size,
                                                 &device_id);
    if (status != noErr)
    {
        std::cerr << "Error getting the id of the audio device: " << status << std::endl;
        return;
    }
}

int main()
{
    AudioDeviceID outputDeviceID = kAudioObjectUnknown;
    unsigned int propertySize = sizeof(outputDeviceID);

    // Get the ID of the output device
    getDeviceID(outputDeviceID);


    // Get the name of the output device
    std::string deviceName;
    OSStatus status = GetAudioDeviceName(outputDeviceID, deviceName);

    std::cout << std::fixed;
    std::cout << "----- Audio Stream Format Info -----" << std::endl;
    std::cout << "Current Device Output Name: " << deviceName << std::endl;
    if (status != noErr)
    {
        std::cerr << "Error getting device name" << std::endl;
        return 1;
    }

    SetupBlackHoleTap();

    return 0;
}
