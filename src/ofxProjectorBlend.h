// See README.md for credits and examples

#pragma once
#include "ofMain.h"

enum ofxProjectorBlendLayout {
    ofxProjectorBlend_Horizontal,
    ofxProjectorBlend_Vertical,
};

enum ofxProjectorBlendRotation {
    ofxProjectorBlend_NoRotation,
    ofxProjectorBlend_RotatedLeft,
    ofxProjectorBlend_RotatedRight
};

class ofxProjectorBlend
{
public:
    ofxProjectorBlend();

    /**
     * Set resolutionWidth and height to the resolutions of each projector output - if you're portrait,
     * still give it a landscape resolution.
     *
     * layout   - this is how the projectors are stacked, so if they are next to eachother, it's horizontal
     *            and if they're on-top of eachother, it's vertical. (this is regardless of rotation)
     *
     * rotation - this is the rotation of the projector. If the projectors are on their sides then it's
     *            rotatedRight, if it's the right way round, it's noRotation.
     *
     */
    void setup(int resolutionWidth, int resolutionHeight, int numProjectors, int pixelOverlap,
               ofxProjectorBlendLayout layout = ofxProjectorBlend_Horizontal,
               ofxProjectorBlendRotation rotation = ofxProjectorBlend_NoRotation);

    //call begin and end like you would if if you were using a normal fbo
    void begin();
    void end();

    void draw(float x = 0, float y = 0);

    bool isReady() const { return enabled && fullTexture.isAllocated(); }

    // this is how big the area to draw to is.
    float getCanvasWidth() const { return isReady() ? fullTextureWidth : ofGetWidth(); }
    float getCanvasHeight() const { return isReady() ? fullTextureHeight : ofGetHeight(); }

    ofFbo & getFullTexture() { return fullTexture; }
    const ofFbo & getFullTexture() const { return fullTexture; }

    /**
     * This is how big all the projector resolutions would be
     * if laid out horizontally, next to eachother. This is
     * essentially the output resolution (regardless of rotation
     * and orientation.) - the output to set your project to.
     *
     * On a mac, if you want multiple screens doing fullscreen,
     * you need to put your screens next to eachother with the
     * main one (the one with the menu) on the left hand side.
     * If you don't, you'll only get the first screen.
     */
    float getDisplayWidth() const { return isReady() ? displayWidth : ofGetWidth(); }
    float getDisplayHeight() const { return isReady() ? displayHeight : ofGetHeight(); }

    float getProjectorWidth() const { return projectorWidth; }
    float getProjectorHeight() const { return projectorHeight; }

    float getPixelOverlap() const { return pixelOverlap; }
    int getNumProjectors() const { return numProjectors; }

    void moveDisplayVertical(unsigned int targetDisplay, int yOffset);


    /** This changes your app's window size to the correct output size */
    void setWindowToDisplaySize() const { if(isReady()) ofSetWindowShape(getDisplayWidth(), getDisplayHeight()); }

    bool showBlend = true;
    bool enabled = true; // if false, bypasses everything (draw commands inside begin()/end() pass straight through to openframeworks)
	bool doBlend = true;

    // variables that control the blend
    // more info here on what these variables do
    // http://local.wasp.uwa.edu.au/~pbourke/texture_colour/edgeblend/
    vector<float> blendPower;
    vector<float> gamma;
    vector<float> luminance;
    //float blendPower2;
    //float gamma2;
    //float luminance2;
    float threshold = 0;

protected:

    vector<int> projectorHeightOffset;

    // resolution of single projector
    float projectorWidth = 0;
    float projectorHeight = 0;

    // resolution of single projector considering projector rotation
    float singleChannelWidth = 0;
    float singleChannelHeight = 0;

    // canvas size
    float fullTextureWidth = 0;
    float fullTextureHeight = 0;

    float displayWidth = 0;
    float displayHeight = 0;

    float pixelOverlap = 0;
    int numProjectors = 0;
    ofxProjectorBlendLayout layout;
    ofxProjectorBlendRotation rotation;
    ofShader blendShader;
    ofFbo fullTexture;
    ofPlanePrimitive quad;
    void updateShaderUniforms();

};
