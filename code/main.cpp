/* 
Name: RAMYA NAYAK	
Date: October 2024
*/

// starter code provided by Kevin Smith
// modified by Ramya Nayak

#include "ofMain.h"
#include "ofApp.h"
//========================================================================
int main() {
	
	// Use ofGLFWWindowSettings for more options like multi-monitor fullscreen
	ofGLWindowSettings settings;
	settings.setSize(1024, 768);
	settings.windowMode = OF_WINDOW; // can also be OF_FULLSCREEN

	auto window = ofCreateWindow(settings);

	ofRunApp(window, make_shared<ofApp>());
	ofRunMainLoop();
}
