/**
 * @file audio_source_littlefs_mounted.h
 * @brief Custom AudioSource for already-mounted LittleFS filesystem
 * 
 * This header provides AudioSourceLittleFSMounted, a custom AudioSource implementation
 * that works with LittleFS filesystems that have already been mounted by the system.
 * 
 * Problem solved:
 * - AudioSourceLittleFS tries to mount LittleFS with default "spiffs" partition
 * - Our system mounts LittleFS with "littlefs" partition name
 * - This causes "partition spiffs could not be found" errors
 * 
 * Solution:
 * - AudioSourceLittleFSMounted uses already-mounted LittleFS directly
 * - No mounting conflicts or partition name issues
 * - Compatible with AudioPlayer and AudioTools library
 * 
 * @author Custom implementation for Dictionary_v2 project
 */

#pragma once

#include "AudioTools.h"
#include "AudioTools/Disk/AudioSource.h"
#include "AudioTools/Disk/SDDirect.h"
#include "LittleFS.h"

class AudioSourceLittleFSMounted : public audio_tools::AudioSource {
public:
    AudioSourceLittleFSMounted() = default;
    
    virtual void begin() override {
        // LittleFS is already mounted, just initialize the index
        idx.begin("/", ".mp3", "*");
        idx_pos = 0;
    }
    
    virtual Stream *nextStream(int offset = 1) override {
        return selectStream(idx_pos + offset);
    }
    
    virtual Stream *selectStream(int index) override {
        idx_pos = index;
        file_name = idx[index];
        if (file_name == nullptr) return nullptr;
        
        file = LittleFS.open(file_name, "r");
        return file ? &file : nullptr;
    }
    
    virtual Stream *selectStream(const char *path) override {
        file.close();
        file = LittleFS.open(path, "r");
        file_name = file.name();
        return file ? &file : nullptr;
    }
    
    virtual bool isAutoNext() override { return true; }
    void setPath(const char *p) { /* not used */ }
    long size() { return idx.size(); }
    
private:
    audio_tools::SDDirect<fs::LittleFSFS, fs::File> idx{LittleFS};
    fs::File file;
    size_t idx_pos = 0;
    const char *file_name = nullptr;
};
