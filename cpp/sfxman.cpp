#include "sfxman.hpp"
#include <AudioToolbox/AudioToolbox.h>
#include <dispatch/dispatch.h>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <chrono>

#define SAMPLES_PER_SEC  8000
#define BUF_SAMPLES_MAX  (SAMPLES_PER_SEC * 5)
#define DEFAULT_VOLUME   1.0f

// ---- State (all AudioQueue handles touched only on _audioQ serial thread) ----

static SfxMan*         _instance  = nullptr;
static short           _scratch[BUF_SAMPLES_MAX]; // synthesis buffer, game thread only
static std::atomic<bool> _busy{false};
static long long       _toneEndMs = 0;
static AudioQueueRef   _queue     = nullptr;       // owned by _audioQ thread

// Serial GCD queue — every AudioQueue call goes here, never on the render thread.
// Lazily created on first PlayTone call so there is no static-init ordering issue.
static dispatch_queue_t _audioQ = nullptr;
static dispatch_once_t  _audioQOnce;

static dispatch_queue_t getAudioQ() {
    dispatch_once(&_audioQOnce, ^{
        _audioQ = dispatch_queue_create("com.tunnelfever.sfx", DISPATCH_QUEUE_SERIAL);
    });
    return _audioQ;
}

// ---- Helpers ----

static long long _nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

// Fires on AudioQueue's internal thread after the buffer has been consumed.
static void _queueCallback(void*, AudioQueueRef, AudioQueueBufferRef) {
    _busy.store(false, std::memory_order_release);
}

static void _fillFmt(AudioStreamBasicDescription* f) {
    f->mSampleRate       = SAMPLES_PER_SEC;
    f->mFormatID         = kAudioFormatLinearPCM;
    f->mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    f->mBitsPerChannel   = 16;
    f->mChannelsPerFrame = 1;
    f->mBytesPerFrame    = 2;
    f->mFramesPerPacket  = 1;
    f->mBytesPerPacket   = 2;
}

// ---- SfxMan ----

SfxMan* SfxMan::GetInstance() {
    if (!_instance) _instance = new SfxMan();
    return _instance;
}

SfxMan::SfxMan() : mInitOk(true) {}

bool SfxMan::IsIdle() {
    // Time-based safety valve — if callback never fires, unlock after deadline.
    if (_busy.load(std::memory_order_acquire) && _nowMs() > _toneEndMs)
        _busy.store(false, std::memory_order_release);
    return !_busy.load(std::memory_order_acquire);
}

// ---- PCM synthesis (game thread, pure math, < 1 ms) ----

static const char* _parseInt(const char* s, int* result) {
    *result = 0;
    while (*s >= '0' && *s <= '9') { *result = *result * 10 + (*s++ - '0'); }
    return s;
}

static int _synth(int freq, int /*dur*/, float amp, short* out, int samples) {
    for (int i = 0; i < samples; i++) {
        float tv = i / (float)SAMPLES_PER_SEC;
        float v;
        if (freq > 0) {
            v = amp * sinf(freq * tv * 2.f * M_PI) +
                (amp * 0.1f) * sinf(freq * 2 * tv * 2.f * M_PI);
        } else {
            int r = rand(); r = r > 0 ? r : -r;
            v = amp * (-0.5f + (r % 1024) / 512.f);
        }
        int val = (int)(v * 32768.f);
        out[i] = (short)(val < -32767 ? -32767 : val > 32767 ? 32767 : val);
        if (i > 0 && freq > 0 && out[i-1] < 0 && out[i] >= 0) {
            int period = (int)(SAMPLES_PER_SEC / (float)freq);
            if (i + period >= samples) { return i; }
        }
    }
    return samples;
}

static void _taper(short* buf, int samples) {
    int t = (int)(0.1f * samples);
    for (int i = 0; i < t && i < samples; i++)
        buf[i] = (short)(buf[i] * (i / (float)t));
    for (int i = samples - t; i < samples; i++)
        if (i >= 0) buf[i] = (short)(buf[i] * ((samples - i) / (float)t));
}

// ---- Background audio task ----

struct AudioCtx { short* pcm; int samples; };

static void _playOnAudioThread(void* vctx) {
    AudioCtx* ctx = static_cast<AudioCtx*>(vctx);

    // Tear down previous queue synchronously (we own it here, no races).
    if (_queue) { AudioQueueDispose(_queue, true); _queue = nullptr; }

    AudioStreamBasicDescription fmt = {};
    _fillFmt(&fmt);
    AudioQueueRef q;
    if (AudioQueueNewOutput(&fmt, _queueCallback, nullptr, nullptr, nullptr, 0, &q) != noErr) {
        _busy.store(false, std::memory_order_release);
        delete[] ctx->pcm; delete ctx; return;
    }

    int bytes = ctx->samples * (int)sizeof(short);
    AudioQueueBufferRef buf;
    if (AudioQueueAllocateBuffer(q, bytes, &buf) != noErr) {
        AudioQueueDispose(q, true);
        _busy.store(false, std::memory_order_release);
        delete[] ctx->pcm; delete ctx; return;
    }

    memcpy(buf->mAudioData, ctx->pcm, bytes);
    buf->mAudioDataByteSize = (UInt32)bytes;
    _queue = q;

    if (AudioQueueEnqueueBuffer(q, buf, 0, nullptr) != noErr) {
        AudioQueueDispose(q, true); _queue = nullptr;
        _busy.store(false, std::memory_order_release);
        delete[] ctx->pcm; delete ctx; return;
    }
    AudioQueueStart(q, nullptr);

    delete[] ctx->pcm;
    delete ctx;
}

// ---- PlayTone (called from game/render thread) ----

void SfxMan::PlayTone(const char* tone) {
    if (!IsIdle()) return;

    // Synthesise entirely on the calling thread — pure arithmetic, well under 1 ms.
    int total = 0;
    int freq = 100, dur = 50, vol_int;
    float amp = DEFAULT_VOLUME;

    while (*tone) {
        switch (*tone) {
            case 'f': tone = _parseInt(tone + 1, &freq);    break;
            case 'd': tone = _parseInt(tone + 1, &dur);     break;
            case 'a':
                tone = _parseInt(tone + 1, &vol_int);
                amp = vol_int / 100.f;
                amp = amp < 0.f ? 0.f : amp > 1.f ? 1.f : amp;
                break;
            case '.': {
                int n = dur * SAMPLES_PER_SEC / 1000;
                if (n > BUF_SAMPLES_MAX - total - 1) n = BUF_SAMPLES_MAX - total - 1;
                n = _synth(freq, dur, amp, _scratch + total, n);
                total += n;
                tone++;
                break;
            }
            default: tone++; break;
        }
    }

    if (total <= 0) return;
    _taper(_scratch, total);

    // Lock the channel immediately on the game thread so back-to-back calls
    // don't race with the async setup below.
    _toneEndMs = _nowMs() + (long long)(total * 1000LL / SAMPLES_PER_SEC) + 300LL;
    _busy.store(true, std::memory_order_release);

    // Copy PCM to a heap buffer owned by the async task, then hand it off.
    // AudioQueueNewOutput / Dispose / Start all run on the serial audio thread —
    // the render loop never blocks on audio system calls.
    AudioCtx* ctx = new AudioCtx();
    ctx->samples  = total;
    ctx->pcm      = new short[total];
    memcpy(ctx->pcm, _scratch, total * sizeof(short));

    dispatch_async_f(getAudioQ(), ctx, _playOnAudioThread);
}
