#pragma once
#include "AudioTools/CoreAudio/AudioMetaData/MimeDetector.h"
#include "AudioTools/Disk/AudioSourceURL.h"

// Add this class definition
class AudioSourceDynamicURLNoAutoNext : public AudioSourceDynamicURL {
public:
  AudioSourceDynamicURLNoAutoNext(AbstractURLStream &urlStream, const char *mime, int startPos = 0)
      : AudioSourceDynamicURL(urlStream, mime, startPos) {}

  virtual bool isAutoNext() override { return false; }
};