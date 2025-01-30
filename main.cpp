#include <iostream>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include "AudioTapper.h"

OSStatus GetAudioDeviceName(AudioDeviceID deviceID, std::string& deviceName) {
    // Get the size of the device name
    unsigned int dataSize = 0;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioDevicePropertyDeviceNameCFString,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyDataSize(deviceID, &propertyAddress, 0, nullptr, &dataSize);
    if (status != noErr) {
        std::cerr << "Error getting property data size for device name: " << status << std::endl;
        return status;
    }

    // Allocate memory for the device name
    CFStringRef deviceNameCFString = nullptr;
    status = AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, nullptr, &dataSize, &deviceNameCFString);
    if (status != noErr) {
        std::cerr << "Error getting device name: " << status << std::endl;
        return status;
    }

    // Convert CFString to std::string
    const char* nameCStr = CFStringGetCStringPtr(deviceNameCFString, kCFStringEncodingUTF8);
    if (nameCStr != nullptr) {
        deviceName = std::string(nameCStr);
    } else {
        CFIndex length = CFStringGetLength(deviceNameCFString);
        CFRange range = CFRangeMake(0, length);
        char* buffer = new char[length + 1];
        if (CFStringGetCString(deviceNameCFString, buffer, length + 1, kCFStringEncodingUTF8)) {
            deviceName = std::string(buffer);
        }
        delete[] buffer;
    }

    // Release the CFString
    CFRelease(deviceNameCFString);

    return noErr;
}

int main() {
    AudioDeviceID outputDeviceID = kAudioObjectUnknown;
    unsigned int propertySize = sizeof(outputDeviceID);

    // Get the current output device
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &propertySize, &outputDeviceID);
    if (status != noErr) {
        std::cerr << "Error getting the current output device: " << status << std::endl;
        return 1;
    }

    // Get the name of the output device
    std::string deviceName;
    status = GetAudioDeviceName(outputDeviceID, deviceName);
    
    if (status != noErr) {
        std::cerr << "Error getting device name" << std::endl;
        return 1;
    }

    std::cout << "Output Device ID: " << outputDeviceID << std::endl;
    std::cout << "Device Name: " << deviceName << std::endl;

    SetupAudioTap(outputDeviceID);

    return 0;
}