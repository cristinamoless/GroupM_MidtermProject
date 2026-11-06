#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetFrameRate(60);
    ofBackground(0);
    ofSetCircleResolution(32);

    // Configurar i reproduir la música
    // ASSEGURA'T de tenir un arxiu anomenat "musica.mp3" a bin/data/
    soundtrack.load("musica.mp3");
    soundtrack.setLoop(false);
    soundtrack.play();

    // Inicialitzar estrelles
    for (int i = 0; i < 600; i++) {
        Star s;
        s.setup(ofGetWidth(), ofGetHeight(), 800);
        stars.push_back(s);
    }

    // Inicialitzar partícules
    for (int i = 0; i < 150; i++) {
        Particle p;
        p.setup();
        particles.push_back(p);
    }

    // Carregar tipografia
    titleFont.load("arial.ttf", 60, true, true, true);
    textFont.load("arial.ttf", 24, true, true, true);
}

//--------------------------------------------------------------
void ofApp::update() {
    currentTime = ofGetElapsedTimef();

    // Actualitzar partícules de fons
    for (auto& p : particles) {
        p.update();
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofEnableAlphaBlending();

    if (currentTime < 15.0) {
        drawScene1(currentTime);
    }
    else if (currentTime < 35.0) {
        drawScene2(currentTime);
    }
    else if (currentTime < 60.0) {
        drawScene3(currentTime);
    }
    else if (currentTime < 90.0) {
        drawScene4(currentTime);
    }
    else if (currentTime < 105.0) {
        drawScene5(currentTime);
    }
    else {
        // Després dels 105 segons: Pantalla negra
        ofBackground(0);
    }
}

//--------------------------------------------------------------
void ofApp::drawScene1(float time) {
    ofBackground(0);

    float speed = ofMap(time, 0, 15, 0.5, 5.0, true);
    float alpha = ofMap(time, 0, 2, 0, 255, true);

    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
    for (auto& s : stars) {
        s.update(speed, ofGetWidth(), ofGetHeight(), 800);
        s.draw(false, alpha);
    }
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::drawScene2(float time) {
    ofBackground(0);

    float speed = ofMap(time, 15, 33, 5.0, 150.0, true);

    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
    for (auto& s : stars) {
        s.update(speed, ofGetWidth(), ofGetHeight(), 800);
        s.draw(true, 255);
    }
    ofPopMatrix();

    if (time > 33.0) {
        float flashAlpha = ofMap(time, 33.0, 35.0, 0, 255, true);
        ofSetColor(255, 255, 255, flashAlpha);
        ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    }
}

//--------------------------------------------------------------
void ofApp::drawScene3(float time) {
    float sceneAlpha = ofMap(time, 35.0, 37.0, 0, 255, true) * ofMap(time, 58.0, 60.0, 1, 0, true);
    ofSetColor(0, 0, 0, 255);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

    for (auto& p : particles) {
        p.draw(sceneAlpha / 255.0);
    }

    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

    float rotY = time * 15.0;
    float rotX = ofNoise(time * 0.1) * 45.0;

    ofRotateYDeg(rotY);
    ofRotateXDeg(rotX);

    ofNoFill();
    ofSetColor(0, 200, 255, sceneAlpha);
    ofSetLineWidth(1.5);

    ofDrawSphere(0, 0, 0, 180);

    ofRotateXDeg(90);
    ofSetColor(0, 150, 255, sceneAlpha * 0.6);
    ofDrawCircle(0, 0, 240);
    ofDrawCircle(0, 0, 260);

    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::drawScene4(float time) {
    ofBackground(0);
    float fadeOut = ofMap(time, 88.0, 90.0, 1.0, 0.0, true);

    for (auto& p : particles) {
        p.draw(fadeOut);
    }

    ofSetColor(255, 204, 0, 255 * fadeOut);

    if (time >= 60.0 && time < 70.0) {
        float alpha = ofMap(time, 60.0, 62.0, 0, 255, true) * ofMap(time, 68.0, 70.0, 1, 0, true);
        ofSetColor(255, 204, 0, alpha);
        drawCenteredText("UN PROJECTE DE PROGRAMACIÓ CREATIVA", ofGetHeight() / 2 - 40, textFont, !textFont.isLoaded());
        drawCenteredText("PRESENTAT PEL GRUP M", ofGetHeight() / 2 + 40, textFont, !textFont.isLoaded());
    }
    else if (time >= 70.0 && time < 80.0) {
        float alpha = ofMap(time, 70.0, 72.0, 0, 255, true) * ofMap(time, 78.0, 80.0, 1, 0, true);
        ofSetColor(255, 204, 0, alpha);
        ofPushMatrix();
        ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
        float scale = ofMap(time, 70.0, 80.0, 0.9, 1.1);
        ofScale(scale, scale);
        drawCenteredText("MEMBRE 1", -30, textFont, !textFont.isLoaded());
        drawCenteredText("MEMBRE 2", 30, textFont, !textFont.isLoaded());
        ofPopMatrix();
    }
    else if (time >= 80.0 && time < 90.0) {
        float alpha = ofMap(time, 80.0, 82.0, 0, 255, true) * ofMap(time, 88.0, 90.0, 1, 0, true);
        ofSetColor(255, 204, 0, alpha);
        ofPushMatrix();
        ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
        float scale = ofMap(time, 80.0, 90.0, 0.9, 1.1);
        ofScale(scale, scale);
        drawCenteredText("MEMBRE 3", -30, textFont, !textFont.isLoaded());
        drawCenteredText("MEMBRE 4", 30, textFont, !textFont.isLoaded());
        ofPopMatrix();
    }
}

//--------------------------------------------------------------
void ofApp::drawScene5(float time) {
    ofBackground(0);

    float alpha = ofMap(time, 90.0, 93.0, 0, 255, true) * ofMap(time, 100.0, 105.0, 1, 0, true);

    for (auto& p : particles) {
        p.draw(alpha / 255.0);
    }

    ofSetColor(255, 204, 0, alpha);

    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

    float pulse = 1.0 + (sin(time * 2.0) * 0.02);
    float scale = ofMap(time, 90.0, 105.0, 1.5, 0.5);

    ofScale(scale * pulse, scale * pulse);
    drawCenteredText("EL DESPERTAR ESTEL·LAR", 0, titleFont, !titleFont.isLoaded());

    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::drawCenteredText(string text, float y, ofTrueTypeFont& font, bool useFallback) {
    if (useFallback) {
        float width = text.length() * 8.0;
        ofDrawBitmapString(text, -width / 2, y);
    }
    else {
        ofRectangle bounds = font.getStringBoundingBox(text, 0, 0);
        font.drawString(text, -bounds.width / 2.0, y + bounds.height / 2.0);
    }
}