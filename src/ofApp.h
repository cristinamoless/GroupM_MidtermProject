#pragma once

#include "ofMain.h"
#include <array>
#include <atomic>
#include <string>
#include <vector>

// We use this class for the starfield and hyperspace effects.
class Star {
public:
    void setup(float width, float height, float depth);

    void update(float speed, float dt,
        float width, float height, float depth);

    void draw(float trails, float alpha);

private:
    // Store each star's position in 3D space.
    ofVec3f pos;

    // We use depth to calculate perspective and light trails.
    float previousZ = 800.0f;
    float maxDepth = 800.0f;
    float brightness = 255.0f;

    // We reuse stars when they leave the visible area.
    void reset(float width, float height,
        float depth, bool randomDepth);
};

// We use small floating particles to add movement to the background.
class Particle {
public:
    void setup();
    void update(float dt);
    void draw(float opacity);

private:
    // We give each particle its own position, speed and size.
    ofVec2f pos;
    ofVec2f velocity;
    float radius = 1.0f;
};

// We prepare each ship's settings before starting the audio.
struct Ship {
    // We vary travel times and pauses to spread out the ship entrances.
    float travel = 4.0f;
    float pause = 2.0f;
    float offset = 0.0f;

    // We use these values to shape the path and light trail.
    float height = 0.5f;
    float slope = 0.0f;
    float trail = 40.0f;

    // We give each ship a different laser tone and sound length.
    float pitch = 1200.0f;
    float soundDuration = 0.25f;

    bool movesRight = true;
};

// We store the progress of each laser sound separately.
// We only update these values inside audioOut() after setup.
struct LaserVoice {
    bool active = false;
    double phase = 0.0;
    double age = 0.0;
};

// We organize the animation, scenes and audio in the main app.
class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;

    // We generate the spaceship sounds here, sample by sample.
    void audioOut(ofSoundBuffer& buffer) override;

private:
    // We reuse twelve ships throughout the planet scene.
    static constexpr int shipCount = 12;

    // We keep all scene start times together, in seconds.
    const float musicStart = 12.0f;
    const float planetStart = 25.0f;
    const float saberStart = 50.0f;
    const float creditsStart = 65.0f;
    const float titleStart = 80.0f;
    const float duration = 95.0f;

    // We use this as the far end of the starfield.
    const float starDepth = 800.0f;

    // We balance the music and sound effects separately.
    const float musicVolume = 0.40f;
    const double laserVolume = 0.12;
    const float saberVolume = 0.85f;

    // We track elapsed time and the time between frames.
    float startTime = 0.0f;
    float previousTime = 0.0f;
    float currentTime = 0.0f;

    // We check whether audio is ready and avoid restarting playback.
    bool musicLoaded = false;
    bool musicStarted = false;
    bool finished = false;
    bool synthReady = false;

    // We play the fight audio once and stop it when the scene ends.
    bool saberAudioLoaded = false;
    bool saberAudioStarted = false;
    bool saberAudioStopped = false;

    // We store the stars and particles so we can update them in loops.
    std::vector<Star> stars;
    std::vector<Particle> particles;

    // We keep one sound voice for each ship.
    std::array<Ship, shipCount> ships;
    std::array<LaserVoice, shipCount> laserVoices;

    // We remember the last pass to avoid repeating the same sound trigger.
    std::array<int, shipCount> lastSoundCycle;

    // We use atomic values to safely send triggers and volume to the audio.
    std::array<std::atomic<bool>, shipCount> pendingLasers;
    std::atomic<float> synthTargetGain{ 0.0f };

    // We smooth the effects volume inside audioOut().
    float synthGain = 0.0f;

    // We use different font sizes for the final title and smaller text.
    ofTrueTypeFont titleFont;
    ofTrueTypeFont textFont;

    // We load music and fight audio from files.
    ofSoundPlayer soundtrack;
    ofSoundPlayer saberAudio;

    // We generate the spaceship sounds without an audio file.
    ofSoundStream synthStream;

    void setupShips();
    void updateShipSounds();
    void updateSaberAudio();

    // We keep each scene in a separate function.
    void drawScene1(float time);
    void drawScene2(float time);
    void drawScene3(float time);
    void drawSaberScene(float time);
    void drawScene4(float time);
    void drawScene5(float time);

    // We reuse the same drawing function for both lightsabers.
    void drawLightsaber(float x, float y,
        float angle, float bladeLength,
        const ofColor& color, float opacity);

    // We reuse these helpers to draw the background elements.
    void drawStars(float trails, float alpha);
    void drawParticles(float opacity);

    // We use one fade function for visual and audio transitions.
    float fade(float time, float begin, float fadeInEnd,
        float fadeOutBegin, float end);

    // We center the text and shrink it if it is too wide.
    void drawCenteredText(const std::string& text,
        float x, float y,
        ofTrueTypeFont& font,
        float maxWidth);
};