#pragma once

#include "ofMain.h"

// Classe per al camp d'estrelles i l'hiperespai
class Star {
public:
    ofVec3f pos;
    float pz; // Z prèvia per dibuixar les esteles

    void setup(float w, float h, float d) {
        pos.x = ofRandom(-w, w);
        pos.y = ofRandom(-h, h);
        pos.z = ofRandom(10, d);
        pz = pos.z;
    }

    void update(float speed, float w, float h, float d) {
        pz = pos.z;
        pos.z -= speed;
        if (pos.z < 1) {
            pos.z = d;
            pz = d;
            pos.x = ofRandom(-w, w);
            pos.y = ofRandom(-h, h);
        }
    }

    void draw(bool useTrails, float alpha) {
        float sx = ofMap(pos.x / pos.z, 0, 1, 0, ofGetWidth());
        float sy = ofMap(pos.y / pos.z, 0, 1, 0, ofGetHeight());

        float r = ofMap(pos.z, 0, 800, 4, 0);

        if (useTrails) {
            float px = ofMap(pos.x / pz, 0, 1, 0, ofGetWidth());
            float py = ofMap(pos.y / pz, 0, 1, 0, ofGetHeight());
            ofSetColor(200, 220, 255, alpha);
            ofSetLineWidth(r * 1.5);
            ofDrawLine(px, py, sx, sy);
        }
        else {
            ofSetColor(255, 255, 255, alpha);
            ofDrawCircle(sx, sy, r);
        }
    }
};

// Classe per a les partícules flotants (Escenes 3, 4, 5)
class Particle {
public:
    ofVec2f pos;
    ofVec2f vel;
    float radius;

    void setup() {
        pos.set(ofRandomWidth(), ofRandomHeight());
        vel.set(ofRandom(-0.5, 0.5), ofRandom(-0.5, 0.5));
        radius = ofRandom(1, 3);
    }

    void update() {
        pos += vel;
        if (pos.x < 0) pos.x = ofGetWidth();
        if (pos.x > ofGetWidth()) pos.x = 0;
        if (pos.y < 0) pos.y = ofGetHeight();
        if (pos.y > ofGetHeight()) pos.y = 0;
    }

    void draw(float alpha) {
        ofSetColor(0, 255, 255, alpha * 100);
        ofDrawCircle(pos.x, pos.y, radius);
    }
};

class ofApp : public ofBaseApp {

public:
    void setup();
    void update();
    void draw();

    void drawScene1(float time);
    void drawScene2(float time);
    void drawScene3(float time);
    void drawScene4(float time);
    void drawScene5(float time);

    // Línia de temps
    float currentTime;

    // Elements generatius
    vector<Star> stars;
    vector<Particle> particles;

    // Tipografia
    ofTrueTypeFont titleFont;
    ofTrueTypeFont textFont;

    // Àudio
    ofSoundPlayer soundtrack;

    // Funcions auxiliars
    void drawCenteredText(string text, float y, ofTrueTypeFont& font, bool useFallback);
};