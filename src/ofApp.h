#pragma once

#include "ofMain.h"
#include <array>
#include <atomic>
#include <string>
#include <vector>

class Star {
public:
    void setup(float width, float height, float depth);

    void update(float speed, float dt,
        float width, float height, float depth);

    void draw(float trails, float alpha);

private:
    ofVec3f pos;

    float previousZ = 800.0f;
    float maxDepth = 800.0f;
    float brightness = 255.0f;

    void reset(float width, float height,
        float depth, bool randomDepth);
};

class Particle {
public:
    void setup();
    void update(float dt);
    void draw(float opacity);

private:
    ofVec2f pos;
    ofVec2f velocity;
    float radius = 1.0f;
};

// Initialized before audio starts and unchanged afterwards.
struct Ship {
    float travel = 4.0f;
    float pause = 2.0f;
    float offset = 0.0f;

    float height = 0.5f;
    float slope = 0.0f;
    float trail = 40.0f;

    float pitch = 1200.0f;
    float soundDuration = 0.25f;

    bool movesRight = true;
};

// Only the audio callback modifies active voices.
struct LaserVoice {
    bool active = false;
    double phase = 0.0;
    double age = 0.0;
};

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;

    void audioOut(ofSoundBuffer& buffer) override;

private:
    static constexpr int shipCount = 12;

    // Timeline in seconds.
    const float musicStart = 12.0f;
    const float planetStart = 25.0f;
    const float saberStart = 50.0f;
    const float creditsStart = 65.0f;
    const float titleStart = 80.0f;
    const float duration = 95.0f;

    const float starDepth = 800.0f;

    // Audio balance.
    const float musicVolume = 0.40f;
    const double laserVolume = 0.12;
    const float saberVolume = 0.85f;

    float startTime = 0.0f;
    float previousTime = 0.0f;
    float currentTime = 0.0f;

    bool musicLoaded = false;
    bool musicStarted = false;
    bool finished = false;
    bool synthReady = false;

    bool saberAudioLoaded = false;
    bool saberAudioStarted = false;
    bool saberAudioStopped = false;

    std::vector<Star> stars;
    std::vector<Particle> particles;

    std::array<Ship, shipCount> ships;
    std::array<LaserVoice, shipCount> laserVoices;
    std::array<int, shipCount> lastSoundCycle;

    // Thread-safe communication for spaceship laser sounds.
    std::array<std::atomic<bool>, shipCount> pendingLasers;
    std::atomic<float> synthTargetGain{ 0.0f };

    // Only the audio callback changes this value.
    float synthGain = 0.0f;

    ofTrueTypeFont titleFont;
    ofTrueTypeFont textFont;

    ofSoundPlayer soundtrack;
    ofSoundPlayer saberAudio;
    ofSoundStream synthStream;

    void setupShips();
    void updateShipSounds();
    void updateSaberAudio();

    void drawScene1(float time);
    void drawScene2(float time);
    void drawScene3(float time);
    void drawSaberScene(float time);
    void drawScene4(float time);
    void drawScene5(float time);

    void drawLightsaber(float x, float y,
        float angle, float bladeLength,
        const ofColor& color, float opacity);

    void drawStars(float trails, float alpha);
    void drawParticles(float opacity);

    float fade(float time, float begin, float fadeInEnd,
        float fadeOutBegin, float end);

    void drawCenteredText(const std::string& text,
        float x, float y,
        ofTrueTypeFont& font,
        float maxWidth);
};