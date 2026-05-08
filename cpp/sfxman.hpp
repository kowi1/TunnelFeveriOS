#ifndef endlesstunnel_sfxman_hpp
#define endlesstunnel_sfxman_hpp

// Sound effect manager — iOS AudioQueue implementation.
// Keeps the identical PlayTone() API from the original Android/OpenSL ES version.
class SfxMan {
public:
    SfxMan();

    static SfxMan* GetInstance();

    // Play a synthesised tone from a recipe string.
    // Format: tokens separated by '.':
    //   f<hz>   — frequency (0 = white noise)
    //   d<ms>   — duration in milliseconds
    //   a<pct>  — amplitude 0-100
    //   .       — emit tone with current settings
    void PlayTone(const char* tone);

    bool IsIdle();

private:
    bool mInitOk;
};

#endif
