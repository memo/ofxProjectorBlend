#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    // NOTE: having to setup gui twice
    // FIRST to load the number of projectors
    setupGui();
    blenderGuiPage = &gui.currentPage(); // save reference to gui page
    gui.loadFromXML();

    // AGAIN to reconstruct the gui with correct number of projectors
    blenderGuiPage->clear(); // clear page first
    setupGui();
    gui.loadFromXML();

    gui.show();
    gui.setAutoSave(false);

    grabber.setup(320, 240);
}

//--------------------------------------------------------------
void testApp::setupGui(){
    gui.addToggle("enabled", blender.enabled);
    gui.addToggle("Show Blend", blender.showBlend);
    gui.addTitle("Init");
    gui.addSlider("numProjectors", numProjectors, 2, 4);
    gui.addSlider("projectorWidth", projectorWidth, 0, 1920);
    gui.addSlider("projectorHeight", projectorHeight, 0, 1080);
    gui.addSlider("pixelOverlap", pixelOverlap, 0, 200);

    for (int i=0; i<numProjectors-1; i++)
    {
        string si = ofToString(i+1);
        gui.addTitle("Blend " + si);
        gui.addSlider("Blend Power "+si, blender.blendPower[i], 0.0, 4.0);
        gui.addSlider("Gamma "+si, blender.gamma[i], 0.1, 4.0);
        gui.addSlider("Luminance "+si, blender.luminance[i], 0.0, 1.0);
    }
}


//--------------------------------------------------------------
void testApp::update(){
    grabber.update();

    // if any of the initialization parameters have changed, setup the blender
    if(blender.getNumProjectors() != numProjectors ||
            blender.getProjectorWidth() != projectorWidth ||
            blender.getProjectorHeight() != projectorHeight||
            blender.getPixelOverlap() != pixelOverlap) {

        // if number of projectors have changed, reset the gui
        bool bInitGui = blender.getNumProjectors() != numProjectors;

        blender.setup(projectorWidth, projectorHeight, numProjectors, pixelOverlap);
//        blender.setWindowToDisplaySize();
        ofSetWindowShape(blender.getDisplayWidth(), ofGetHeight());

        // if number of projectors have changed, reset the gui
        if(bInitGui) {
            blenderGuiPage->clear(); // clear page first
            setupGui();
        }
    }

}

//--------------------------------------------------------------
void testApp::draw(){
    blender.begin(); //call blender.begin() to draw onto the blendable canvas
    {
        ofSetColor(255);
        grabber.draw(0, 0, blender.getCanvasWidth(), blender.getCanvasHeight());

        //thick grid lines for blending
        ofSetLineWidth(3);

        //vertical line
        for(int i = 0; i <= blender.getCanvasWidth(); i+=40){
            ofLine(i, 0, i, blender.getCanvasHeight());
        }

        //horizontal lines
        for(int j = 0; j <= blender.getCanvasHeight(); j+=40){
            ofLine(0, j, blender.getCanvasWidth(), j);
        }
    }
    blender.end(); //call when you are finished drawing


    //this draws to the main window
    blender.draw();


    // We might not want the GUI to be blended...
    gui.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key=='g' || key=='G')
        gui.toggleDraw();
}
