#ifndef VERSION_MANAGER_H
#define VERSION_MANAGER_H

#include <Arduino.h>
#include "storage/storage.h"
#include "config.h"
// // Firmware version (stored in code)
// #define FIRMWARE_VERSION_MAJOR 1
// #define FIRMWARE_VERSION_MINOR 0
// #define FIRMWARE_VERSION_PATCH 0

// Key for storing hardware revision in STORAGE
#define HARDWARE_REVISION_KEY "hw_rev"

// Version structure
struct Version {
    int major;
    int minor;
    int patch;

    Version(int maj = 0, int min = 0, int pat = 0) : major(maj), minor(min), patch(pat) {}

    String toString() const {
        return String(major) + "." + String(minor) + "." + String(patch);
    }

    int toInt() const {
        return (major * 10000) + (minor * 100) + patch;
    }

    bool operator==(const Version& other) const {
        return major == other.major && minor == other.minor && patch == other.patch;
    }

    bool operator<(const Version& other) const {
        return toInt() < other.toInt();
    }
};

// Function to set hardware revision
inline bool setHardwareRevision( const Version& revision) {
    return STORAGE.store(HARDWARE_REVISION_KEY, revision.toInt());
}

// Function to get hardware revision
inline Version getHardwareRevision() {
    int revInt = STORAGE.getInt(HARDWARE_REVISION_KEY, 0);
    return Version(revInt / 10000, (revInt / 100) % 100, revInt % 100);
}

// Function to get firmware version
inline Version getFirmwareVersion() {
    return Version(FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH);
}

// Function to get firmware version string
inline String getFirmwareVersionString() {
    return getFirmwareVersion().toString();
}

// Function to compare versions
inline bool isHardwareVersion(int major, int minor, int patch) {
    Version v1(major, minor, patch);
    Version v2 = getHardwareRevision();
    return v1 == v2;
}

#endif // VERSION_MANAGER_H