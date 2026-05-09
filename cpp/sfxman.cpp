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

// ---- State (AudioQueue handles touched only on _audioQ serial thread) ----

static SfxMan*           _instance   = nullptr;
static short             _scratch[BUF_SAMPLES_MAX];
static std::atomic<bool> _busy{false};
static long long         _toneEndMs  = 0;
static AudioQueueRef     _queue      = nullptr;

// Monotonically-increasing generation counter.  Bumped each time a new tone
// starts.  The AudioQueue callback receives its generation as context so it
// can tell whether it belongs to the *current* tone before clearing _busy.
//
// Why this matters: AudioQueueDispose(q, inImmediate=true) returns every
// enqueued buffer to its callback synchronously.  If the safety-timeout
// fires, a new tone is started, and then _playOnAudioThread disposes the
// old queue, the old callback fires on the audioQ thread — but now _busy
// already belongs to the new tone.  Without the generation check that old
// callback would clear _busy mid-play, letting a third tone start and
// immediately clobber the second via another immediate dispose → glitch.
static std::atomic<uint32_t> _generation{0};

// Serial GCD queue — every AudioQueue call goes here, never on render thread.
static dispatch_queue_t _audioQ     = nullptr;
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

// Fires on AudioQueue's own internal thread after the buffer is consumed.
// ctx carries the generation this queue was created with.
static void _queueCallback(void* ctx, AudioQueueRef, AudioQueueBufferRef) {
    uintptr_t callbackGen = reinterpret_cast<uintptr_t>(ctx);
    // Only clear _busy if no newer tone has since taken ownership.
    if ((uint32_t)callbackGen == _generation.load(std::memory_order_acquire))
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

struct AudioCtx { short* pcm; int samples; uint32_t gen; };

static void _playOnAudioThread(void* vctx) {
    AudioCtx* ctx = static_cast<AudioCtx*>(vctx);

    // Tear down the previous queue (we own _queue exclusively on this thread).
    // inImmediate=true returns any enqueued buffers to their callback; the
    // generation check in _queueCallback prevents those stale callbacks from
    // touching _busy for the new tone.
    if (_queue) { AudioQueueDispose(_queue, true); _queue = nullptr; }

    AudioStreamBasicDescription fmt = {};
    _fillFmt(&fmt);
    AudioQueueRef q;
    void* cbCtx = reinterpret_cast<void*>((uintptr_t)ctx->gen);
    if (AudioQueueNewOutput(&fmt, _queueCallback, cbCtx, nullptr, nullptr, 0, &q) != noErr) {
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

    // Bump generation before storing _busy=true so the new generation is
    // visible to any concurrent callback check on the AudioQueue thread.
    uint32_t gen = _generation.fetch_add(1, std::memory_order_acq_rel) + 1;

    _toneEndMs = _nowMs() + (long long)(total * 1000LL / SAMPLES_PER_SEC) + 300LL;
    _busy.store(true, std::memory_order_release);

    AudioCtx* ctx = new AudioCtx();
    ctx->samples  = total;
    ctx->gen      = gen;
    ctx->pcm      = new short[total];
    memcpy(ctx->pcm, _scratch, total * sizeof(short));

    dispatch_async_f(getAudioQ(), ctx, _playOnAudioThread);
}
