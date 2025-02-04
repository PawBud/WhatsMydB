#include <iostream>
#include <cstdint>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include "AudioTapper.h"
#include "AudioInfo.h"

OSStatus GetAudioDeviceName(AudioDeviceID deviceID, std::string &deviceName) {
    // Get the size of the device name
    unsigned int dataSize = 0;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioDevicePropertyDeviceNameCFString,
        kAudioObjectPropertyScopeOutput,
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
    const char *nameCStr = CFStringGetCStringPtr(deviceNameCFString, kCFStringEncodingUTF8);
    if (nameCStr != nullptr) {
        deviceName = std::string(nameCStr);
    } else {
        CFIndex length = CFStringGetLength(deviceNameCFString);
        CFRange range = CFRangeMake(0, length);
        char *buffer = new char[length + 1];
        if (CFStringGetCString(deviceNameCFString, buffer, length + 1, kCFStringEncodingUTF8)) {
            deviceName = std::string(buffer);
        }
        delete[] buffer;
    }

    // Release the CFString
    CFRelease(deviceNameCFString);

    return noErr;
}

std::string getFormatName(uint32_t formatID) {
    char formatName[5]; // FourCC is 4 chars + null terminator
    formatName[0] = static_cast<char>((formatID >> 24) & 0xFF); // Extract MSB
    formatName[1] = static_cast<char>((formatID >> 16) & 0xFF);
    formatName[2] = static_cast<char>((formatID >> 8) & 0xFF);
    formatName[3] = static_cast<char>(formatID & 0xFF); // Extract LSB
    formatName[4] = '\0'; // Null terminate

    return std::string(formatName);
}

int main() {
    AudioDeviceID outputDeviceID = kAudioObjectUnknown;
    unsigned int propertySize = sizeof(outputDeviceID);

    // Get the ID of the output device
    getDeviceID(outputDeviceID);

    // Get the name of the output device
    std::string deviceName;
    OSStatus status = GetAudioDeviceName(outputDeviceID, deviceName);
    std::cout << "current device output name: " << deviceName << std::endl;
    if (status != noErr) {
        std::cerr << "Error getting device name" << std::endl;
        return 1;
    }
    SetupAudioTap(outputDeviceID);
    return 0;
}
