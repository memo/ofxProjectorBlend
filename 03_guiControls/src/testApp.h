#pragma once

#include "ofMain.h"
#include "ofxProjectorBlend.h"
#include "ofxSimpleGuiToo.h"

class testApp : public ofBaseApp{

	public:
        int numProjectors = 3;
        int projectorWidth = 320;
        int projectorHeight = 240;
        int pixelOverlap = 40;

		void setup();
        void setupGui();
		void update();
		void draw();

		void keyPressed  (int key);
    
        ofxProjectorBlend blender;
        ofVideoGrabber grabber;
        ofxSimpleGuiPage* blenderGuiPage = NULL;
};
