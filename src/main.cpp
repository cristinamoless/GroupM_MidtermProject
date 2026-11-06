#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
    // Resolució de 1280 x 720
    ofSetupOpenGL(1280, 720, OF_WINDOW);

    // Executa l'aplicació
    ofRunApp(new ofApp());
}