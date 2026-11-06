#include "ofApp.h"
#include <algorithm>
#include <cmath>

// ============================================================
// STARS
// ============================================================

void Star::reset(float width, float height,
    float depth, bool randomDepth) {
    maxDepth = depth;
    pos.z = randomDepth ? ofRandom(40.0f, depth) : depth;

    const float focalLength = width * 0.5f;
    const float sx = ofRandom(-width * 0.5f, width * 0.5f);
    const float sy = ofRandom(-height * 0.5f, height * 0.5f);

    pos.x = sx * pos.z / focalLength;
    pos.y = sy * pos.z / focalLength;

    previousZ = pos.z;
    brightness = ofRandom(150.0f, 255.0f);
}

void Star::setup(float width, float height, float depth) {
    reset(width, height, depth, true);
}

void Star::update(float speed, float dt,
    float width, float height, float depth) {
    pos.z -= speed * dt;

    if (pos.z <= 20.0f) {
        reset(width, height, depth, false);
        return;
    }

    const float focalLength = width * 0.5f;
    const float sx = pos.x * focalLength / pos.z;
    const float sy = pos.y * focalLength / pos.z;

    if (std::abs(sx) > width * 0.5f + 30.0f ||
        std::abs(sy) > height * 0.5f + 30.0f) {
        reset(width, height, depth, false);
        return;
    }

    // Fixed exposure time keeps trails independent of frame rate.
    previousZ = pos.z + speed * 0.025f;
}

void Star::draw(float trails, float alpha) {
    const float focalLength = ofGetWidth() * 0.5f;

    const float sx = pos.x * focalLength / pos.z;
    const float sy = pos.y * focalLength / pos.z;
    const float px = pos.x * focalLength / previousZ;
    const float py = pos.y * focalLength / previousZ;

    const float radius = ofMap(
        pos.z, 20.0f, maxDepth, 3.2f, 0.7f, true
    );

    const float opacity = alpha * brightness / 255.0f;

    ofFill();

    if (trails > 0.0f) {
        ofSetColor(70, 150, 255, opacity * trails * 0.18f);
        ofSetLineWidth(radius * 3.0f);
        ofDrawLine(px, py, sx, sy);

        ofSetColor(190, 225, 255, opacity * trails);
        ofSetLineWidth(radius);
        ofDrawLine(px, py, sx, sy);
    }

    ofSetColor(255, 255, 255, opacity);
    ofDrawCircle(sx, sy, radius);
    ofSetLineWidth(1.0f);
}

// ============================================================
// PARTICLES
// ============================================================

void Particle::setup() {
    pos.set(ofRandomWidth(), ofRandomHeight());
    velocity.set(ofRandom(-30.0f, 30.0f),
        ofRandom(-30.0f, 30.0f));
    radius = ofRandom(1.0f, 3.0f);
}

void Particle::update(float dt) {
    const float width = static_cast<float>(ofGetWidth());
    const float height = static_cast<float>(ofGetHeight());

    if (width <= 0.0f || height <= 0.0f) return;

    pos += velocity * dt;
    pos.x = std::fmod(pos.x, width);
    pos.y = std::fmod(pos.y, height);

    if (pos.x < 0.0f) pos.x += width;
    if (pos.y < 0.0f) pos.y += height;
}

void Particle::draw(float opacity) {
    ofFill();
    ofSetColor(0, 220, 255,
        100.0f * ofClamp(opacity, 0.0f, 1.0f));
    ofDrawCircle(pos.x, pos.y, radius);
}

// ============================================================
// SETUP
// ============================================================

void ofApp::setup() {
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(0);
    ofSetCircleResolution(64);
    ofEnableAlphaBlending();

    currentTime = 0.0f;
    previousTime = 0.0f;
    musicStarted = false;
    finished = false;
    lastSwing = -1;
    lastClash = 0;

    stars.clear();
    stars.reserve(600);

    for (int i = 0; i < 600; ++i) {
        Star star;
        star.setup(ofGetWidth(), ofGetHeight(), starDepth);
        stars.push_back(star);
    }

    particles.clear();
    particles.reserve(150);

    for (int i = 0; i < 150; ++i) {
        Particle particle;
        particle.setup();
        particles.push_back(particle);
    }

    const bool titleLoaded =
        titleFont.load("arial.ttf", 60, true, true);

    const bool textLoaded =
        textFont.load("arial.ttf", 24, true, true);

    if (!titleLoaded || !textLoaded) {
        ofLogWarning("Fonts")
            << "arial.ttf not found. Using bitmap fallback.";
    }

    musicLoaded = soundtrack.load("musica.mp3");

    if (musicLoaded) {
        soundtrack.setLoop(false);
        soundtrack.setMultiPlay(false);
        soundtrack.setVolume(musicVolume);
    }
    else {
        ofLogWarning("Audio")
            << "Could not load bin/data/musica.mp3";
    }

    // Initialize all audio state before starting callbacks.
    setupShips();

    swingVoice = SoundVoice();
    impactVoice = SoundVoice();

    pendingSwing.store(false);
    pendingImpact.store(false);
    synthTargetGain.store(0.0f);

    synthGain = 0.0f;
    filteredNoise = 0.0;
    noiseState = 123456789u;

    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.sampleRate = 44100;
    settings.bufferSize = 256;
    settings.numBuffers = 4;
    settings.numOutputChannels = 2;
    settings.numInputChannels = 0;

    synthReady = synthStream.setup(settings);

    if (!synthReady) {
        ofLogWarning("Audio")
            << "Could not open the output device for sound effects.";
    }

    startTime = ofGetElapsedTimef();
}

void ofApp::setupShips() {
    for (int i = 0; i < shipCount; ++i) {
        Ship& ship = ships[i];

        ship.travel = ofRandom(2.5f, 5.5f);
        ship.pause = ofRandom(1.0f, 3.0f);
        ship.offset = ship.travel + ofRandom(0.0f, ship.pause);

        ship.height = ofRandom(0.12f, 0.88f);
        ship.slope = ofRandom(-0.10f, 0.10f);
        ship.trail = ofRandom(18.0f, 65.0f);
        ship.movesRight = (i % 2 == 0);

        ship.pitch = ofRandom(900.0f, 1900.0f);
        ship.soundDuration = ofRandom(0.18f, 0.34f);

        lastSoundCycle[i] = -1;
        pendingLasers[i].store(false);
        laserVoices[i] = SoundVoice();
    }
}

// ============================================================
// UPDATE
// ============================================================

void ofApp::update() {
    ofSoundUpdate();

    if (finished) return;

    currentTime = ofGetElapsedTimef() - startTime;

    const float dt = currentTime - previousTime;
    previousTime = currentTime;

    if (currentTime >= duration) {
        currentTime = duration;
        synthTargetGain.store(0.0f);

        if (musicLoaded) soundtrack.stop();

        finished = true;
        return;
    }

    if (musicLoaded &&
        !musicStarted &&
        currentTime >= musicStart) {
        soundtrack.play();
        musicStarted = true;
    }

    if (musicLoaded && musicStarted) {
        soundtrack.setVolume(
            ofMap(currentTime,
                duration - 5.0f, duration,
                musicVolume, 0.0f, true)
        );
    }

    updateProceduralSounds();

    if (currentTime < planetStart &&
        ofGetWidth() > 0 && ofGetHeight() > 0) {
        float speed = 30.0f;

        if (currentTime >= musicStart) {
            speed = ofMap(
                currentTime, musicStart, planetStart - 2.0f,
                30.0f, 3000.0f, true
            );
        }

        for (auto& star : stars) {
            star.update(speed, dt,
                ofGetWidth(), ofGetHeight(), starDepth);
        }
    }

    for (auto& particle : particles) {
        particle.update(dt);
    }
}

void ofApp::updateProceduralSounds() {
    if (!synthReady) return;

    if (currentTime >= planetStart && currentTime < saberStart) {
        synthTargetGain.store(
            fade(currentTime,
                planetStart, planetStart + 2.0f,
                saberStart - 2.0f, saberStart)
        );

        const float width = static_cast<float>(ofGetWidth());

        if (width <= 0.0f || ofGetHeight() <= 0) return;

        const float localTime = currentTime - planetStart;
        const float entry = 120.0f / (width + 240.0f);
        const float leaving = 1.0f - entry;

        for (int i = 0; i < shipCount; ++i) {
            const Ship& ship = ships[i];

            const float cycleDuration = ship.travel + ship.pause;
            const float elapsed = localTime + ship.offset;

            const int cycle = static_cast<int>(
                std::floor(elapsed / cycleDuration)
                );

            const float progress =
                std::fmod(elapsed, cycleDuration) / ship.travel;

            if (progress >= entry &&
                progress < leaving &&
                cycle != lastSoundCycle[i]) {
                lastSoundCycle[i] = cycle;
                pendingLasers[i].store(true);
            }
        }
    }
    else if (currentTime >= saberStart &&
        currentTime < creditsStart) {
        const float localTime = currentTime - saberStart;

        synthTargetGain.store(
            fade(currentTime,
                saberStart, saberStart + 1.0f,
                creditsStart - 1.0f, creditsStart)
        );

        // Trigger a swing during each fast blade movement.
        if (localTime >= 0.5f) {
            const int swing = static_cast<int>(
                std::floor(localTime - 0.5f)
                );

            if (swing > lastSwing) {
                lastSwing = swing;
                pendingSwing.store(true);
            }
        }

        // Trigger impacts at local seconds 1, 3, 5, 7, 9, 11 and 13.
        const int clash = static_cast<int>(
            std::floor((localTime + 1.0f) / 2.0f)
            );

        if (clash > lastClash) {
            lastClash = clash;
            pendingImpact.store(true);
        }
    }
    else {
        synthTargetGain.store(0.0f);
    }
}

// ============================================================
// PROCEDURAL AUDIO
// ============================================================

double ofApp::nextNoise() {
    // Xorshift noise generator, used only by the audio thread.
    noiseState ^= noiseState << 13;
    noiseState ^= noiseState >> 17;
    noiseState ^= noiseState << 5;

    return (static_cast<double>(noiseState) / 4294967295.0)
        * 2.0 - 1.0;
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
    // Clear the underlying sample vector.
    auto& samples = buffer.getBuffer();
    std::fill(samples.begin(), samples.end(), 0.0f);

    const std::size_t channels = buffer.getNumChannels();
    if (channels == 0) return;

    const double sampleRate =
        buffer.getSampleRate() > 0
        ? static_cast<double>(buffer.getSampleRate())
        : 44100.0;

    const double dt = 1.0 / sampleRate;
    const double pi = 3.141592653589793;
    const double twoPi = 2.0 * pi;
    const double halfPi = pi * 0.5;

    const float target = synthTargetGain.load();

    // Receive events without changing audio state from update().
    for (int i = 0; i < shipCount; ++i) {
        if (pendingLasers[i].exchange(false) && target > 0.0f) {
            laserVoices[i].active = true;
            laserVoices[i].phase = 0.0;
            laserVoices[i].age = 0.0;
        }
    }

    if (pendingSwing.exchange(false) && target > 0.0f) {
        swingVoice.active = true;
        swingVoice.phase = 0.0;
        swingVoice.age = 0.0;
    }

    if (pendingImpact.exchange(false) && target > 0.0f) {
        impactVoice.active = true;
        impactVoice.phase = 0.0;
        impactVoice.age = 0.0;
    }

    const float smoothing = static_cast<float>(
        1.0 - std::exp(-1.0 / (0.005 * sampleRate))
        );

    const double noiseSmoothing =
        1.0 - std::exp(-twoPi * 900.0 / sampleRate);

    for (std::size_t frame = 0;
        frame < buffer.getNumFrames(); ++frame) {
        synthGain += (target - synthGain) * smoothing;

        double left = 0.0;
        double right = 0.0;

        // Spaceship laser sounds.
        for (int i = 0; i < shipCount; ++i) {
            SoundVoice& voice = laserVoices[i];
            const Ship& ship = ships[i];

            if (!voice.active) continue;

            if (voice.age >= ship.soundDuration) {
                voice.active = false;
                continue;
            }

            const double progress = voice.age / ship.soundDuration;
            const double frequency =
                140.0 + (ship.pitch - 140.0) *
                std::exp(-4.5 * progress);

            const double attack = std::min(1.0, voice.age / 0.006);
            const double release = 1.0 - progress;
            const double envelope = attack * release * release;

            const double wave =
                std::sin(voice.phase) +
                0.18 * std::sin(voice.phase * 2.0);

            const double sample = wave * envelope * laserVolume;
            const double pan = ship.movesRight ? 0.2 : 0.8;

            left += sample * std::cos(pan * halfPi);
            right += sample * std::sin(pan * halfPi);

            voice.phase += twoPi * frequency * dt;
            if (voice.phase >= twoPi) voice.phase -= twoPi;
            voice.age += dt;
        }

        // Generate noise for the saber effects.
        const double noise = nextNoise();
        filteredNoise += noiseSmoothing * (noise - filteredNoise);

        // Saber movement: low buzzing tone and filtered air noise.
        if (swingVoice.active) {
            const double length = 0.55;

            if (swingVoice.age >= length) {
                swingVoice.active = false;
            }
            else {
                const double progress = swingVoice.age / length;
                const double curve = std::sin(pi * progress);
                const double envelope = curve * curve;

                const double frequency = 85.0 + 150.0 * curve;

                const double buzz =
                    0.65 * std::sin(swingVoice.phase) +
                    0.25 * std::sin(swingVoice.phase * 2.0) +
                    0.10 * std::sin(swingVoice.phase * 3.0);

                const double sound = swingVolume * envelope *
                    (buzz + 0.8 * filteredNoise);

                left += sound * 0.707;
                right += sound * 0.707;

                swingVoice.phase += twoPi * frequency * dt;
                if (swingVoice.phase >= twoPi) {
                    swingVoice.phase -= twoPi;
                }

                swingVoice.age += dt;
            }
        }

        // Saber impact: a sharp noise burst and metallic tones.
        if (impactVoice.active) {
            const double length = 0.40;

            if (impactVoice.age >= length) {
                impactVoice.active = false;
            }
            else {
                const double age = impactVoice.age;
                const double progress = age / length;

                const double attack = std::min(1.0, age / 0.002);
                const double release = 1.0 - progress;

                const double envelope =
                    attack * release * release *
                    std::exp(-7.0 * age);

                const double metal =
                    0.45 * std::sin(twoPi * 430.0 * age) +
                    0.30 * std::sin(twoPi * 713.0 * age) +
                    0.15 * std::sin(twoPi * 1189.0 * age);

                const double sound = impactVolume * envelope *
                    (0.80 * noise + metal);

                left += sound * 0.707;
                right += sound * 0.707;

                impactVoice.age += dt;
            }
        }

        // Soft limiting controls overlapping sound effects.
        const float outLeft =
            static_cast<float>(std::tanh(left * synthGain));

        const float outRight =
            static_cast<float>(std::tanh(right * synthGain));

        if (channels == 1) {
            buffer[frame] = (outLeft + outRight) * 0.5f;
        }
        else {
            buffer[frame * channels] = outLeft;
            buffer[frame * channels + 1] = outRight;
        }
    }
}

// ============================================================
// DRAW AND CLEANUP
// ============================================================

void ofApp::draw() {
    ofBackground(0);
    if (finished || currentTime >= duration) return;

    ofPushStyle();
    ofEnableAlphaBlending();
    ofFill();
    ofSetLineWidth(1.0f);

    if (currentTime < musicStart) {
        drawScene1(currentTime);
    }
    else if (currentTime < planetStart) {
        drawScene2(currentTime);
    }
    else if (currentTime < saberStart) {
        drawScene3(currentTime);
    }
    else if (currentTime < creditsStart) {
        drawSaberScene(currentTime);
    }
    else if (currentTime < titleStart) {
        drawScene4(currentTime);
    }
    else {
        drawScene5(currentTime);
    }

    ofPopStyle();
}

void ofApp::exit() {
    synthTargetGain.store(0.0f);

    // Stop callbacks before application data is destroyed.
    synthStream.close();

    if (musicLoaded) soundtrack.stop();
}

// ============================================================
// HELPERS
// ============================================================

float ofApp::fade(float time, float begin, float fadeInEnd,
    float fadeOutBegin, float end) {
    return ofMap(time, begin, fadeInEnd, 0.0f, 1.0f, true) *
        ofMap(time, fadeOutBegin, end, 1.0f, 0.0f, true);
}

void ofApp::drawStars(float trails, float alpha) {
    ofPushStyle();
    ofPushMatrix();

    ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.5f);

    for (auto& star : stars) star.draw(trails, alpha);

    ofPopMatrix();
    ofPopStyle();
}

void ofApp::drawParticles(float opacity) {
    ofPushStyle();

    for (auto& particle : particles) particle.draw(opacity);

    ofPopStyle();
}

void ofApp::drawCenteredText(const std::string& text,
    float x, float y,
    ofTrueTypeFont& font,
    float maxWidth) {
    ofPushMatrix();
    ofTranslate(x, y);

    if (font.isLoaded()) {
        const ofRectangle bounds =
            font.getStringBoundingBox(text, 0, 0);

        float fit = 1.0f;

        if (bounds.width > maxWidth && maxWidth > 0.0f) {
            fit = maxWidth / bounds.width;
        }

        ofScale(fit, fit);

        font.drawString(
            text,
            -bounds.x - bounds.width * 0.5f,
            -bounds.y - bounds.height * 0.5f
        );
    }
    else {
        // Simplify characters for the bitmap fallback.
        std::string fallback = text;

        const std::string accentedO = "\xC3\x93";
        const std::string middleDot = "\xC2\xB7";

        std::size_t position = fallback.find(accentedO);

        while (position != std::string::npos) {
            fallback.replace(position, accentedO.size(), "O");
            position = fallback.find(accentedO);
        }

        position = fallback.find(middleDot);

        while (position != std::string::npos) {
            fallback.replace(position, middleDot.size(), ".");
            position = fallback.find(middleDot);
        }

        const float width =
            static_cast<float>(fallback.size()) * 8.0f;

        float fit = 2.0f;

        if (width * fit > maxWidth && maxWidth > 0.0f) {
            fit = maxWidth / width;
        }

        ofScale(fit, fit);
        ofDrawBitmapString(fallback, -width * 0.5f, 4.0f);
    }

    ofPopMatrix();
}

// ============================================================
// SILENT INTRODUCTION — 0 TO 12 SECONDS
// ============================================================

void ofApp::drawScene1(float time) {
    drawStars(
        0.0f,
        ofMap(time, 0.0f, 2.0f, 0.0f, 255.0f, true)
    );

    const float alpha =
        255.0f * fade(time, 3.0f, 4.0f, 10.0f, musicStart);

    if (alpha <= 0.0f) return;

    ofSetColor(85, 190, 235, alpha);

    const float cx = ofGetWidth() * 0.5f;
    const float cy = ofGetHeight() * 0.5f;
    const float maxWidth = ofGetWidth() * 0.90f;

    drawCenteredText(
        "A long time ago in a galaxy far,",
        cx, cy - 24.0f, textFont, maxWidth
    );

    drawCenteredText(
        "far away....",
        cx, cy + 24.0f, textFont, maxWidth
    );
}

// ============================================================
// HYPERSPACE — 12 TO 25 SECONDS
// ============================================================

void ofApp::drawScene2(float time) {
    drawStars(
        ofMap(time, musicStart, musicStart + 3.0f,
            0.0f, 1.0f, true),
        255.0f
    );

    const float flash = ofMap(
        time, planetStart - 2.0f, planetStart,
        0.0f, 255.0f, true
    );

    ofFill();
    ofSetColor(255, 255, 255, flash);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
}

// ============================================================
// PLANET AND SPACESHIPS — 25 TO 50 SECONDS
// ============================================================

void ofApp::drawScene3(float time) {
    const float localTime = time - planetStart;

    const float opacity = fade(
        time, planetStart, planetStart + 2.0f,
        saberStart - 2.0f, saberStart
    );

    const float width = static_cast<float>(ofGetWidth());
    const float height = static_cast<float>(ofGetHeight());

    drawParticles(opacity);

    ofPushStyle();
    ofFill();

    for (int i = 0; i < shipCount; ++i) {
        const Ship& ship = ships[i];

        const float cycleTime = std::fmod(
            localTime + ship.offset, ship.travel + ship.pause
        );

        if (cycleTime >= ship.travel) continue;

        const float progress = cycleTime / ship.travel;
        const float startX = ship.movesRight ? -120.0f : width + 120.0f;
        const float endX = ship.movesRight ? width + 120.0f : -120.0f;
        const float startY = ship.height * height;
        const float endY = startY + ship.slope * height;

        const float x = startX + (endX - startX) * progress;
        const float y = startY + (endY - startY) * progress;

        const float dx = endX - startX;
        const float dy = endY - startY;
        const float distance = std::sqrt(dx * dx + dy * dy);

        const float tailX = x - dx / distance * ship.trail;
        const float tailY = y - dy / distance * ship.trail;

        const float alpha = opacity *
            fade(progress, 0.0f, 0.12f, 0.85f, 1.0f);

        ofSetColor(255, 190, 0, alpha * 45.0f);
        ofSetLineWidth(6.0f);
        ofDrawLine(tailX, tailY, x, y);

        ofSetColor(255, 220, 30, alpha * 230.0f);
        ofSetLineWidth(2.0f);
        ofDrawLine(tailX, tailY, x, y);

        ofSetColor(255, 245, 170, alpha * 255.0f);
        ofDrawCircle(x, y, 2.0f);
    }

    ofPopStyle();
    ofPushStyle();
    ofPushMatrix();

    ofTranslate(width * 0.5f, height * 0.5f);

    const float fit = height / 720.0f;
    ofScale(fit, fit, fit);

    ofRotateYDeg(localTime * 15.0f);
    ofRotateXDeg(15.0f + ofNoise(localTime * 0.1f) * 30.0f);

    const float pulse =
        1.0f + 0.02f * std::sin(localTime * 2.0f);

    ofNoFill();
    ofSetLineWidth(1.2f);

    ofSetColor(0, 200, 255, opacity * 220.0f);
    ofDrawSphere(0, 0, 0, 180.0f * pulse);

    ofSetColor(0, 150, 255, opacity * 40.0f);
    ofDrawSphere(0, 0, 0, 184.0f * pulse);

    ofPopMatrix();
    ofPopStyle();

    const float flash = ofMap(
        time, planetStart, planetStart + 2.0f,
        255.0f, 0.0f, true
    );

    ofFill();
    ofSetColor(255, 255, 255, flash);
    ofDrawRectangle(0, 0, width, height);
}

// ============================================================
// LIGHTSABER DRAWING
// ============================================================

void ofApp::drawLightsaber(float x, float y,
    float angle, float bladeLength,
    const ofColor& color, float opacity) {
    ofPushStyle();
    ofPushMatrix();

    ofTranslate(x, y);
    ofRotateDeg(angle);
    ofFill();

    // Layer transparent shapes to create the glow.
    for (int layer = 7; layer >= 1; --layer) {
        const float radius = 3.0f + layer * 3.0f;

        ofSetColor(color.r, color.g, color.b, opacity * 12.0f);
        ofDrawRectangle(-radius, -bladeLength,
            radius * 2.0f, bladeLength);
        ofDrawCircle(0, -bladeLength, radius);
        ofDrawCircle(0, 0, radius);
    }

    ofSetColor(color.r, color.g, color.b, opacity * 240.0f);
    ofDrawRectangle(-5.0f, -bladeLength, 10.0f, bladeLength);
    ofDrawCircle(0, -bladeLength, 5.0f);

    ofSetColor(255, 255, 255, opacity * 255.0f);
    ofDrawRectangle(-2.0f, -bladeLength, 4.0f, bladeLength);
    ofDrawCircle(0, -bladeLength, 2.0f);

    // Metallic handle.
    ofSetColor(65, 70, 80, opacity * 255.0f);
    ofDrawRectangle(-10.0f, 0, 20.0f, 65.0f);

    ofSetColor(165, 175, 190, opacity * 255.0f);
    ofDrawRectangle(-7.0f, 3.0f, 4.0f, 57.0f);

    ofSetColor(25, 28, 35, opacity * 255.0f);

    for (int i = 0; i < 5; ++i) {
        ofDrawRectangle(-10.0f, 14.0f + i * 9.0f, 20.0f, 3.0f);
    }

    ofSetColor(195, 205, 220, opacity * 255.0f);
    ofDrawRectangle(-12.0f, -4.0f, 24.0f, 9.0f);

    ofPopMatrix();
    ofPopStyle();
}

// ============================================================
// LIGHTSABER COMBAT — 50 TO 65 SECONDS
// ============================================================

void ofApp::drawSaberScene(float time) {
    const float localTime = time - saberStart;

    const float opacity = fade(
        time, saberStart, saberStart + 1.0f,
        creditsStart - 1.0f, creditsStart
    );

    ofPushStyle();
    ofPushMatrix();

    const float fit = std::min(
        ofGetWidth() / 1280.0f,
        ofGetHeight() / 720.0f
    );

    ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.5f);
    ofScale(fit, fit);

    ofTranslate(
        45.0f * std::sin(localTime * 0.7f),
        18.0f * std::sin(localTime * 1.1f)
    );

    const float separation =
        0.5f * (1.0f - std::cos(PI * (localTime - 1.0f)));

    const float leftSwing =
        32.0f + 8.0f * std::sin(localTime * 0.9f);

    const float rightSwing =
        35.0f + 7.0f * std::cos(localTime * 0.8f);

    const float pivotX = 200.0f;
    const float pivotY = 100.0f;

    const float contactAngle =
        std::atan2(pivotX, pivotY) * 180.0f / PI;

    drawLightsaber(
        -pivotX, pivotY,
        contactAngle - leftSwing * separation,
        285.0f, ofColor(255, 25, 35), opacity
    );

    drawLightsaber(
        pivotX, pivotY,
        -contactAngle + rightSwing * separation,
        285.0f, ofColor(30, 110, 255), opacity
    );

    const int clash = static_cast<int>(
        std::floor((localTime + 1.0f) / 2.0f)
        );

    if (clash >= 1) {
        const float sinceHit =
            localTime - (2.0f * clash - 1.0f);

        if (sinceHit >= 0.0f && sinceHit < 0.30f) {
            const float flash = 1.0f - sinceHit / 0.30f;

            ofFill();

            for (int layer = 5; layer >= 1; --layer) {
                ofSetColor(255, 210, 130,
                    opacity * flash * 22.0f);
                ofDrawCircle(0, 0,
                    layer * 12.0f + sinceHit * 60.0f);
            }

            ofSetColor(255, 250, 225,
                opacity * flash * 255.0f);
            ofDrawCircle(0, 0, 11.0f * flash + 2.0f);

            ofSetLineWidth(2.0f);

            for (int i = 0; i < 16; ++i) {
                const float angle =
                    TWO_PI * i / 16.0f + clash * 0.37f;

                const float radius =
                    sinceHit * (170.0f + (i % 5) * 45.0f);

                const float trail = 6.0f + flash * 10.0f;
                const float x = std::cos(angle) * radius;
                const float y = std::sin(angle) * radius +
                    90.0f * sinceHit * sinceHit;

                ofSetColor(255, 210, 90,
                    opacity * flash * 255.0f);

                ofDrawLine(
                    x - std::cos(angle) * trail,
                    y - std::sin(angle) * trail,
                    x, y
                );
            }
        }
    }

    ofPopMatrix();
    ofPopStyle();
}

// ============================================================
// CREDITS — 65 TO 80 SECONDS
// ============================================================

void ofApp::drawScene4(float time) {
    drawParticles(
        fade(time, creditsStart, creditsStart + 1.0f,
            titleStart - 1.0f, titleStart)
    );

    const float localTime = time - creditsStart;

    std::string line1;
    std::string line2;
    float blockStart = creditsStart;

    if (localTime < 5.0f) {
        line1 = "UN PROJECTE DE PROGRAMACIÓ CREATIVA";
        line2 = "PRESENTAT PEL GRUP M";
    }
    else if (localTime < 10.0f) {
        blockStart += 5.0f;
        line1 = "MEMBRE 1";
        line2 = "MEMBRE 2";
    }
    else {
        blockStart += 10.0f;
        line1 = "MEMBRE 3";
        line2 = "MEMBRE 4";
    }

    const float opacity = fade(
        time, blockStart, blockStart + 1.0f,
        blockStart + 4.0f, blockStart + 5.0f
    );

    const float scale = ofMap(
        time, blockStart, blockStart + 5.0f,
        0.95f, 1.05f, true
    );

    const float offsetY = ofMap(
        time, blockStart, blockStart + 5.0f,
        10.0f, -10.0f, true
    );

    ofSetColor(255, 204, 0, opacity * 255.0f);

    ofPushMatrix();
    ofTranslate(ofGetWidth() * 0.5f,
        ofGetHeight() * 0.5f + offsetY);
    ofScale(scale, scale);

    const float maxWidth = ofGetWidth() * 0.88f / scale;

    drawCenteredText(line1, 0, -35, textFont, maxWidth);
    drawCenteredText(line2, 0, 35, textFont, maxWidth);

    ofPopMatrix();
}

// ============================================================
// FINAL TITLE — 80 TO 95 SECONDS
// ============================================================

void ofApp::drawScene5(float time) {
    const float opacity = fade(
        time, titleStart, titleStart + 3.0f,
        duration - 5.0f, duration
    );

    drawParticles(opacity);

    const float localTime = time - titleStart;
    const float pulse =
        1.0f + std::sin(localTime * 2.0f) * 0.02f;

    const float scale = ofMap(
        time, titleStart, titleStart + 6.0f,
        1.15f, 1.0f, true
    ) * pulse;

    ofPushMatrix();
    ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.5f);
    ofScale(scale, scale);

    ofSetColor(255, 204, 0, opacity * 255.0f);

    drawCenteredText(
        "EL DESPERTAR ESTEL·LAR",
        0, 0, titleFont, ofGetWidth() * 0.78f
    );

    ofPopMatrix();
}