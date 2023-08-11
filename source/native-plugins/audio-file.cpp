/*
 * Carla Native Plugins
 * Copyright (C) 2013-2023 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

#include "CarlaNativePrograms.hpp"
#include "CarlaString.hpp"

#include "audio-base.hpp"

// --------------------------------------------------------------------------------------------------------------------

#ifndef __MOD_DEVICES__
class AudioFilePlugin : public NativePluginWithMidiPrograms<FileAudio>
#else
class AudioFilePlugin : public NativePluginClass
#endif
{
public:
   #ifndef __MOD_DEVICES__
    static constexpr const char* const audiofilesWildcard =
      #ifdef HAVE_SNDFILE
        "*.aif;*.aifc;*.aiff;*.au;*.bwf;*.flac;*.htk;*.iff;*.mat4;*.mat5;*.oga;*.ogg;*.opus;"
        "*.paf;*.pvf;*.pvf5;*.sd2;*.sf;*.snd;*.svx;*.vcc;*.w64;*.wav;*.xi;"
      #endif
      #ifdef HAVE_FFMPEG
        "*.3g2;*.3gp;*.aac;*.ac3;*.amr;*.ape;*.mp2;*.mp3;*.mpc;*.wma;"
       #ifndef HAVE_SNDFILE
        "*.flac;*.oga;*.ogg;*.w64;*.wav;"
       #endif
      #else
        "*.mp3;"
      #endif
    ;

    enum PendingInlineDisplay : uint8_t {
        InlineDisplayNotPending,
        InlineDisplayNeedRequest,
        InlineDisplayRequesting
    };
   #endif

    enum Parameters {
        kParameterLooping,
        kParameterHostSync,
        kParameterVolume,
        kParameterEnabled,
        kParameterInfoChannels,
        kParameterInfoBitRate,
        kParameterInfoBitDepth,
        kParameterInfoSampleRate,
        kParameterInfoLength,
        kParameterInfoPosition,
        kParameterInfoPoolFill,
        kParameterCount
    };

    AudioFilePlugin(const NativeHostDescriptor* const host)
       #ifndef __MOD_DEVICES__
        : NativePluginWithMidiPrograms<FileAudio>(host, fPrograms, 3),
          fPrograms(hostGetFilePath("audio"), audiofilesWildcard)
       #else
        : NativePluginClass(host)
       #endif
//           fWasPlayingBefore(false),
//           fNeedsFileRead(false),
//           fEntireFileLoaded(false),
//           fMaxFrame(0),
//           fLastPoolFill(0.0f),
//           fPool(),
//           fReader(),
//           fFilename(),
//           fPreviewData()
//        #ifndef __MOD_DEVICES__
//         , fInlineDisplay()
//        #endif
    {
    }

    ~AudioFilePlugin() override
    {
//         fReader.destroy();
//         fPool.destroy();
    }

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // Plugin parameter calls

    uint32_t getParameterCount() const override
    {
        return kParameterCount;
    }

    const NativeParameter* getParameterInfo(const uint32_t index) const override
    {
        static NativeParameter param;

        param.scalePointCount  = 0;
        param.scalePoints      = nullptr;
        param.unit             = nullptr;
        param.ranges.step      = 1.0f;
        param.ranges.stepSmall = 1.0f;
        param.ranges.stepLarge = 1.0f;
        param.designation      = NATIVE_PARAMETER_DESIGNATION_NONE;

        switch (index)
        {
        case kParameterLooping:
            param.name  = "Loop Mode";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_BOOLEAN);
            param.ranges.def = 1.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = 1.0f;
            break;
        case kParameterHostSync:
            param.name  = "Host Sync";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_BOOLEAN);
           #ifdef __MOD_DEVICES__
            param.ranges.def = 0.0f;
           #else
            param.ranges.def = 1.0f;
           #endif
            param.ranges.min = 0.0f;
            param.ranges.max = 1.0f;
            break;
        case kParameterVolume:
            param.name  = "Volume";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED);
            param.ranges.def = 100.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = 127.0f;
            param.ranges.stepSmall = 0.5f;
            param.ranges.stepLarge = 10.0f;
            param.unit = "%";
            break;
        case kParameterEnabled:
            param.name  = "Enabled";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_BOOLEAN|
                                                            NATIVE_PARAMETER_USES_DESIGNATION);
            param.ranges.def = 1.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = 1.0f;
            param.designation = NATIVE_PARAMETER_DESIGNATION_ENABLED;
            break;
        case kParameterInfoChannels:
            param.name  = "Num Channels";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_INTEGER|
                                                            NATIVE_PARAMETER_IS_OUTPUT);
            param.ranges.def = 0.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = 2.0f;
            break;
        case kParameterInfoBitRate:
            param.name  = "Bit Rate";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_INTEGER|
                                                            NATIVE_PARAMETER_IS_OUTPUT);
            param.ranges.def = 0.0f;
            param.ranges.min = -1.0f;
            param.ranges.max = 384000.0f * 64.0f * 2.0f;
            break;
        case kParameterInfoBitDepth:
            param.name  = "Bit Depth";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_INTEGER|
                                                            NATIVE_PARAMETER_IS_OUTPUT);
            param.ranges.def = 0.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = 64.0f;
            break;
        case kParameterInfoSampleRate:
            param.name  = "Sample Rate";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_INTEGER|
                                                            NATIVE_PARAMETER_IS_OUTPUT);
            param.ranges.def = 0.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = 384000.0f;
            break;
        case kParameterInfoLength:
            param.name  = "Length";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_OUTPUT);
            param.ranges.def = 0.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = (float)INT64_MAX;
            param.unit = "s";
            break;
        case kParameterInfoPosition:
            param.name  = "Position";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_OUTPUT);
            param.ranges.def = 0.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = 100.0f;
            param.unit = "%";
            break;
        case kParameterInfoPoolFill:
            param.name  = "Pool Fill";
            param.hints = static_cast<NativeParameterHints>(NATIVE_PARAMETER_IS_AUTOMATABLE|
                                                            NATIVE_PARAMETER_IS_ENABLED|
                                                            NATIVE_PARAMETER_IS_OUTPUT);
            param.ranges.def = 0.0f;
            param.ranges.min = 0.0f;
            param.ranges.max = 100.0f;
            param.unit = "%";
            break;
        default:
            return nullptr;
        }

        return &param;
    }

    float getParameterValue(const uint32_t index) const override
    {
        switch (index)
        {
        case kParameterLooping:
            return fLoopMode ? 1.0f : 0.0f;
        case kParameterHostSync:
            return fHostSync ? 1.f : 0.f;
        case kParameterEnabled:
            return fEnabled ? 1.f : 0.f;
        case kParameterVolume:
            return fVolume * 100.f;
        case kParameterInfoPosition:
            return fLastPosition;
//         case kParameterInfoPoolFill:
//             return fLastPoolFill;
        case kParameterInfoBitRate:
            return static_cast<float>(fReader.getCurrentBitRate());
        }

        const ADInfo nfo = fReader.getFileInfo();

        switch (index)
        {
        case kParameterInfoChannels:
            return static_cast<float>(nfo.channels);
        case kParameterInfoBitDepth:
            return static_cast<float>(nfo.bit_depth);
        case kParameterInfoSampleRate:
            return static_cast<float>(nfo.sample_rate);
        case kParameterInfoLength:
            return static_cast<float>(nfo.length)/1000.f;
        }

        return 0.f;
    }

    void setParameterValue(const uint32_t index, const float value) override
    {
        if (index == kParameterVolume)
        {
            fVolume = value / 100.f;
            return;
        }

        const bool b = value > 0.5f;

        switch (index)
        {
        case kParameterLooping:
            if (fLoopMode != b)
            {
                fLoopMode = b;
                // fReader.setLoopingMode(b);
            }
            break;
        case kParameterHostSync:
            if (fHostSync != b)
            {
                fInternalTransportFrame = 0;
                fHostSync = b;
            }
            break;
        case kParameterEnabled:
            if (fEnabled != b)
            {
                fInternalTransportFrame = 0;
                fEnabled = b;
            }
            break;
        default:
            break;
        }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Plugin state calls

    void setCustomData(const char* const key, const char* const value) override
    {
        if (std::strcmp(key, "file") != 0)
            return;

       #ifndef __MOD_DEVICES__
        invalidateNextFilename();
       #endif
        loadFilename(value);
    }

   #ifndef __MOD_DEVICES__
    void setStateFromFile(const char* const filename) override
    {
        loadFilename(filename);
    }
   #endif

    // ----------------------------------------------------------------------------------------------------------------
    // Plugin process calls

   #ifndef __MOD_DEVICES__
    void process2(const float* const*, float** const outBuffer, const uint32_t frames,
                  const NativeMidiEvent*, uint32_t) override
   #else
    void process(const float* const*, float** const outBuffer, const uint32_t frames,
                 const NativeMidiEvent*, uint32_t) override
   #endif
    {
        float* const out1 = outBuffer[0];
        float* const out2 = outBuffer[1];
        float* const playCV = outBuffer[2];

//         const water::GenericScopedLock<water::SpinLock> gsl(fPool.mutex);

        if (! fDoProcess)
        {
            // carla_stderr("P: no process");
            carla_zeroFloats(out1, frames);
            carla_zeroFloats(out2, frames);
            carla_zeroFloats(playCV, frames);
            fLastPosition = 0.0f;
            return;
        }

//         const bool loopMode = fLoopMode;
        bool needsIdleRequest = false;
        bool playing;
        uint64_t framePos;

        if (fHostSync)
        {
            const NativeTimeInfo* const timePos = getTimeInfo();
            playing = fEnabled && timePos->playing;
            framePos = timePos->frame;
        }
        else
        {
            playing = fEnabled;
            framePos = fInternalTransportFrame;

            if (playing)
                fInternalTransportFrame += frames;
        }

        // not playing
        if (! playing)
        {
//             // carla_stderr("P: not playing");
//             if (framePos == 0 && fWasPlayingBefore)
//                 fReader.setNeedsRead(framePos);

            carla_zeroFloats(out1, frames);
            carla_zeroFloats(out2, frames);
            carla_zeroFloats(playCV, frames);
//             fWasPlayingBefore = false;
            return;
        }
//         else
//         {
//             fWasPlayingBefore = true;
//         }

        fLastPosition = fReader.tickFrames(outBuffer, 0, frames, framePos, fLoopMode, isOffline()) * 100.f;

//         // out of reach
//         if ((frame < fPool.startFrame || frame >= fMaxFrame) && !loopMode)
//         {
//             if (frame < fPool.startFrame)
//             {
//                 needsIdleRequest = true;
//                 fNeedsFileRead = true;
//                 fReader.setNeedsRead(frame);
//             }
//
//             carla_zeroFloats(out1, frames);
//             carla_zeroFloats(out2, frames);
//             carla_zeroFloats(playCV, frames);
//
//            #ifndef __MOD_DEVICES__
//             if (fInlineDisplay.writtenValues < 32)
//             {
//                 fInlineDisplay.lastValuesL[fInlineDisplay.writtenValues] = 0.0f;
//                 fInlineDisplay.lastValuesR[fInlineDisplay.writtenValues] = 0.0f;
//                 ++fInlineDisplay.writtenValues;
//             }
//             if (fInlineDisplay.pending == InlineDisplayNotPending)
//             {
//                 needsIdleRequest = true;
//                 fInlineDisplay.pending = InlineDisplayNeedRequest;
//             }
//            #endif

//             if (needsIdleRequest)
//                 hostRequestIdle();
//
//             if (frame == 0)
//                 fLastPosition = 0.0f;
//             else if (frame >= fMaxFrame)
//                 fLastPosition = 100.0f;
//             else
//                 fLastPosition = static_cast<float>(frame) / static_cast<float>(fMaxFrame) * 100.0f;
//             return;
//         }

//         if (fEntireFileLoaded)
//         {
//             // NOTE: frame is always < fMaxFrame (or looping)
//             uint32_t targetStartFrame = static_cast<uint32_t>(loopMode ? frame % fMaxFrame : frame);
//
//             for (uint32_t framesDone=0, framesToDo=frames, remainingFrames; framesDone < frames;)
//             {
//                 if (targetStartFrame + framesToDo <= fMaxFrame)
//                 {
//                     // everything fits together
//                     carla_copyFloats(out1+framesDone, fPool.buffer[0]+targetStartFrame, framesToDo);
//                     carla_copyFloats(out2+framesDone, fPool.buffer[1]+targetStartFrame, framesToDo);
//                     carla_fillFloatsWithSingleValue(playCV+framesDone, 10.f, framesToDo);
//                     break;
//                 }
//
//                 remainingFrames = std::min(fMaxFrame - targetStartFrame, framesToDo);
//                 carla_copyFloats(out1+framesDone, fPool.buffer[0]+targetStartFrame, remainingFrames);
//                 carla_copyFloats(out2+framesDone, fPool.buffer[1]+targetStartFrame, remainingFrames);
//                 carla_fillFloatsWithSingleValue(playCV+framesDone, 10.f, remainingFrames);
//                 framesDone += remainingFrames;
//                 framesToDo -= remainingFrames;
//
//                 if (! loopMode)
//                 {
//                     // not looping, stop here
//                     if (framesToDo != 0)
//                     {
//                         carla_zeroFloats(out1+framesDone, framesToDo);
//                         carla_zeroFloats(out2+framesDone, framesToDo);
//                         carla_zeroFloats(playCV+framesDone, framesToDo);
//                     }
//                     break;
//                 }
//
//                 // reset for next loop
//                 targetStartFrame = 0;
//             }

//             fLastPosition = static_cast<float>(targetStartFrame) / static_cast<float>(fMaxFrame) * 100.0f;
//         }
//         else
//         {
//             const bool offline = isOffline();

//             if (fReader.tryPutData(fPool, out1, out2, frame, frames, loopMode, offline, needsIdleRequest))
//             {
//                 carla_fillFloatsWithSingleValue(playCV, 10.f, frames);
//             }
//             else
//             {
//                 carla_zeroFloats(out1, frames);
//                 carla_zeroFloats(out2, frames);
//                 carla_zeroFloats(playCV, frames);
//             }
//
//             if (needsIdleRequest)
//             {
//                 fNeedsFileRead = true;
//
//                 if (isOffline())
//                 {
//                     needsIdleRequest = false;
//                     fReader.readPoll();
//
//                     if (fReader.tryPutData(fPool, out1, out2, frame, frames, loopMode, offline, needsIdleRequest))
//                     {
//                         carla_fillFloatsWithSingleValue(playCV, 10.f, frames);
//                     }
//                     else
//                     {
//                         carla_zeroFloats(out1, frames);
//                         carla_zeroFloats(out2, frames);
//                         carla_zeroFloats(playCV, frames);
//                     }
//
//                     if (needsIdleRequest)
//                         fNeedsFileRead = true;
//                 }
//             }
//
//             const uint32_t modframe = static_cast<uint32_t>(frame % fMaxFrame);
//             fLastPosition = static_cast<float>(modframe) / static_cast<float>(fMaxFrame) * 100.0f;
//
//             if (modframe > fPool.startFrame)
//                 fLastPoolFill = static_cast<float>(modframe - fPool.startFrame) / static_cast<float>(fPool.numFrames) * 100.0f;
//             else
//                 fLastPoolFill = 100.0f;
//         }

        const float volume = fVolume;
        if (carla_isNotEqual(volume, 1.0f))
        {
            carla_multiply(out1, volume, frames);
            carla_multiply(out2, volume, frames);
        }

       #ifndef __MOD_DEVICES__
        if (fInlineDisplay.writtenValues < 32)
        {
            fInlineDisplay.lastValuesL[fInlineDisplay.writtenValues] = carla_findMaxNormalizedFloat(out1, frames);
            fInlineDisplay.lastValuesR[fInlineDisplay.writtenValues] = carla_findMaxNormalizedFloat(out2, frames);
            ++fInlineDisplay.writtenValues;
        }
        if (fInlineDisplay.pending == InlineDisplayNotPending)
        {
            needsIdleRequest = true;
            fInlineDisplay.pending = InlineDisplayNeedRequest;
        }
       #endif

        if (needsIdleRequest)
            hostRequestIdle();
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Plugin UI calls

    void uiShow(const bool show) override
    {
        if (! show)
            return;

        if (const char* const filename = uiOpenFile(false, "Open Audio File", ""))
            uiCustomDataChanged("file", filename);

        uiClosed();
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Plugin dispatcher calls

    void idle() override
    {
       #ifndef __MOD_DEVICES__
        NativePluginWithMidiPrograms<FileAudio>::idle();

        if (fInlineDisplay.pending == InlineDisplayNeedRequest)
        {
            fInlineDisplay.pending = InlineDisplayRequesting;
            hostQueueDrawInlineDisplay();
        }
       #endif

//         if (fNeedsFileRead)
//         {
//             fReader.readPoll();
//             fNeedsFileRead = false;
//         }
    }

    void sampleRateChanged(double) override
    {
        if (char* const filename = fFilename.releaseBufferPointer())
        {
            loadFilename(filename);
            std::free(filename);
        }
    }

   #ifndef __MOD_DEVICES__
    const NativeInlineDisplayImageSurface* renderInlineDisplay(const uint32_t rwidth, const uint32_t height) override
    {
        CARLA_SAFE_ASSERT_RETURN(height > 4, nullptr);

        const uint32_t width = rwidth == height ? height * 4 : rwidth;

        /* NOTE the code is this function is not optimized, still learning my way through pixels...
         */
        const size_t stride = width * 4;
        const size_t dataSize = stride * height;
        const uint pxToMove = fDoProcess ? fInlineDisplay.writtenValues : 0;

        uchar* data = fInlineDisplay.data;

        if (fInlineDisplay.dataSize != dataSize || data == nullptr)
        {
            delete[] data;
            data = new uchar[dataSize];
            std::memset(data, 0, dataSize);
            fInlineDisplay.data = data;
            fInlineDisplay.dataSize = dataSize;
        }
        else if (pxToMove != 0)
        {
            // shift all previous values to the left
            for (uint w=0; w < width - pxToMove; ++w)
                for (uint h=0; h < height; ++h)
                    std::memmove(&data[h * stride + w * 4], &data[h * stride + (w+pxToMove) * 4], 4);
        }

        fInlineDisplay.width  = static_cast<int>(width);
        fInlineDisplay.height = static_cast<int>(height);
        fInlineDisplay.stride = static_cast<int>(stride);

        if (pxToMove != 0)
        {
            const uint h2 = height / 2;

            // clear current line
            for (uint w=width-pxToMove; w < width; ++w)
                for (uint h=0; h < height; ++h)
                    memset(&data[h * stride + w * 4], 0, 4);

            // draw upper/left
            for (uint i=0; i < pxToMove && i < 32; ++i)
            {
                const float valueL = fInlineDisplay.lastValuesL[i];
                const float valueR = fInlineDisplay.lastValuesR[i];

                const uint h2L = static_cast<uint>(valueL * (float)h2);
                const uint h2R = static_cast<uint>(valueR * (float)h2);
                const uint w   = width - pxToMove + i;

                for (uint h=0; h < h2L; ++h)
                {
                    // -30dB
                    //if (valueL < 0.032f)
                    //    continue;

                    data[(h2 - h) * stride + w * 4 + 3] = 160;

                    // -12dB
                    if (valueL < 0.25f)
                    {
                        data[(h2 - h) * stride + w * 4 + 1] = 255;
                    }
                    // -3dB
                    else if (valueL < 0.70f)
                    {
                        data[(h2 - h) * stride + w * 4 + 2] = 255;
                        data[(h2 - h) * stride + w * 4 + 1] = 255;
                    }
                    else
                    {
                        data[(h2 - h) * stride + w * 4 + 2] = 255;
                    }
                }

                for (uint h=0; h < h2R; ++h)
                {
                    // -30dB
                    //if (valueR < 0.032f)
                    //    continue;

                    data[(h2 + h) * stride + w * 4 + 3] = 160;

                    // -12dB
                    if (valueR < 0.25f)
                    {
                        data[(h2 + h) * stride + w * 4 + 1] = 255;
                    }
                    // -3dB
                    else if (valueR < 0.70f)
                    {
                        data[(h2 + h) * stride + w * 4 + 2] = 255;
                        data[(h2 + h) * stride + w * 4 + 1] = 255;
                    }
                    else
                    {
                        data[(h2 + h) * stride + w * 4 + 2] = 255;
                    }
                }
            }
        }

        fInlineDisplay.writtenValues = 0;
        fInlineDisplay.pending = InlineDisplayNotPending;
        return (NativeInlineDisplayImageSurface*)(NativeInlineDisplayImageSurfaceCompat*)&fInlineDisplay;
    }
   #endif

    // ----------------------------------------------------------------------------------------------------------------

private:
    bool fLoopMode = true;
   #ifdef __MOD_DEVICES__
    bool fHostSync = false;
   #else
    bool fHostSync = true;
   #endif
    bool fEnabled = true;
    bool fDoProcess = false;
//     bool fWasPlayingBefore;
//     volatile bool fNeedsFileRead;
//
//     bool fEntireFileLoaded;
//     uint32_t fMaxFrame;
    uint32_t fInternalTransportFrame = 0;
    float fLastPosition = 0.f;
//     float fLastPoolFill;
    float fVolume = 1.f;

//     AudioFilePool   fPool;
    AudioFileReader fReader;
    CarlaString     fFilename;

    float fPreviewData[108] = {};

   #ifndef __MOD_DEVICES__
    NativeMidiPrograms fPrograms;

    struct InlineDisplay : NativeInlineDisplayImageSurfaceCompat {
        float lastValuesL[32] = {};
        float lastValuesR[32] = {};
        volatile PendingInlineDisplay pending = InlineDisplayNotPending;
        volatile uint8_t writtenValues = 0;

        InlineDisplay()
            : NativeInlineDisplayImageSurfaceCompat() {}

        ~InlineDisplay()
        {
            if (data != nullptr)
            {
                delete[] data;
                data = nullptr;
            }
        }

        CARLA_DECLARE_NON_COPYABLE(InlineDisplay)
        CARLA_PREVENT_HEAP_ALLOCATION
    } fInlineDisplay;
   #endif

    void loadFilename(const char* const filename)
    {
        CARLA_ASSERT(filename != nullptr);
        carla_stdout("AudioFilePlugin::loadFilename(\"%s\")", filename);

        fDoProcess = false;
//         fLastPoolFill = 0.0f;
//         fPool.destroy();
        fReader.destroy();
        fFilename.clear();

        if (filename == nullptr || *filename == '\0')
        {
//             fMaxFrame = 0;
            return;
        }

        constexpr uint32_t kPreviewDataLen = sizeof(fPreviewData)/sizeof(float);

        if (fReader.loadFilename(filename, static_cast<uint32_t>(getSampleRate()), kPreviewDataLen, fPreviewData))
        {
//             fEntireFileLoaded = fReader.isEntireFileLoaded();
//             fMaxFrame = fReader.getMaxFrame();
//
//             if (fEntireFileLoaded)
//             {
//                 fReader.putAndSwapAllData(fPool);
//                 fLastPoolFill = 100.0f;
//             }
//             else
//             {
//                 fReader.createSwapablePool(fPool);
//                 fReader.readPoll();
//             }

            fInternalTransportFrame = 0;
            fDoProcess = true;
            fFilename = filename;
            hostSendPreviewBufferData('f', kPreviewDataLen, fPreviewData);
        }
        else
        {
//             fEntireFileLoaded = false;
//             fMaxFrame = 0;
        }
    }

    PluginClassEND(AudioFilePlugin)

    static const char* _get_buffer_port_name(NativePluginHandle, const uint32_t index, const bool isOutput)
    {
        if (!isOutput || index != 2)
            return nullptr;

        return "Play status";
    }

    static const NativePortRange* _get_buffer_port_range(NativePluginHandle, const uint32_t index, const bool isOutput)
    {
        if (!isOutput || index != 2)
            return nullptr;

        static NativePortRange npr = { 0.f, 10.f };
        return &npr;
    }

    CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFilePlugin)
};

// --------------------------------------------------------------------------------------------------------------------

CARLA_API_EXPORT
void carla_register_native_plugin_audiofile();

CARLA_API_EXPORT
void carla_register_native_plugin_audiofile()
{
    static const NativePluginDescriptor audiofileDesc = {
        /* category  */ NATIVE_PLUGIN_CATEGORY_UTILITY,
        /* hints     */ static_cast<NativePluginHints>(NATIVE_PLUGIN_IS_RTSAFE
                                                     #ifndef __MOD_DEVICES__
                                                      |NATIVE_PLUGIN_HAS_INLINE_DISPLAY
                                                     #endif
                                                      |NATIVE_PLUGIN_HAS_UI
                                                      |NATIVE_PLUGIN_NEEDS_UI_OPEN_SAVE
                                                      |NATIVE_PLUGIN_REQUESTS_IDLE
                                                      |NATIVE_PLUGIN_USES_CONTROL_VOLTAGE
                                                      |NATIVE_PLUGIN_USES_TIME),
        /* supports  */ NATIVE_PLUGIN_SUPPORTS_NOTHING,
        /* audioIns  */ 0,
        /* audioOuts */ 2,
        /* midiIns   */ 0,
        /* midiOuts  */ 0,
        /* paramIns  */ 1,
        /* paramOuts */ 0,
        /* name      */ "Audio File",
        /* label     */ "audiofile",
        /* maker     */ "falkTX",
        /* copyright */ "GNU GPL v2+",
        AudioFilePlugin::_instantiate,
        AudioFilePlugin::_cleanup,
        AudioFilePlugin::_get_parameter_count,
        AudioFilePlugin::_get_parameter_info,
        AudioFilePlugin::_get_parameter_value,
        AudioFilePlugin::_get_midi_program_count,
        AudioFilePlugin::_get_midi_program_info,
        AudioFilePlugin::_set_parameter_value,
        AudioFilePlugin::_set_midi_program,
        AudioFilePlugin::_set_custom_data,
        AudioFilePlugin::_ui_show,
        AudioFilePlugin::_ui_idle,
        AudioFilePlugin::_ui_set_parameter_value,
        AudioFilePlugin::_ui_set_midi_program,
        AudioFilePlugin::_ui_set_custom_data,
        AudioFilePlugin::_activate,
        AudioFilePlugin::_deactivate,
        AudioFilePlugin::_process,
        AudioFilePlugin::_get_state,
        AudioFilePlugin::_set_state,
        AudioFilePlugin::_dispatcher,
        AudioFilePlugin::_render_inline_display,
        /* cvIns  */ 0,
        /* cvOuts */ 1,
        AudioFilePlugin::_get_buffer_port_name,
        AudioFilePlugin::_get_buffer_port_range,
        /* ui_width  */ 0,
        /* ui_height */ 0
    };

    carla_register_native_plugin(&audiofileDesc);
}

// --------------------------------------------------------------------------------------------------------------------
