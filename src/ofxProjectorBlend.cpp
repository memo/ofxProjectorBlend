#include "ofxProjectorBlend.h"
#include "ofxProjectorBlendShader.h"


// --------------------------------------------------
ofxProjectorBlend::ofxProjectorBlend()
{
    gamma.resize(2, 0.5);
    blendPower.resize(2, 1);
    luminance.resize(2, 0);
}


// --------------------------------------------------
void ofxProjectorBlend::setup(int resolutionWidth, 
                              int resolutionHeight,
                              int _numProjectors,
                              int _pixelOverlap,
                              ofxProjectorBlendLayout _layout,
                              ofxProjectorBlendRotation _rotation)
{

    string l = "horizontal";
    if(layout==ofxProjectorBlend_Vertical) l = "vertical";

    string r = "normal";
    if(rotation==ofxProjectorBlend_RotatedLeft) r = "rotated left";
    else if(rotation==ofxProjectorBlend_RotatedRight) r = "rotated right";

    ofLog(OF_LOG_NOTICE, "ofxProjectorBlend: res: %d x %d * %d, overlap: %d pixels, layout: %s, rotation: %s\n", resolutionWidth, resolutionHeight, _numProjectors, _pixelOverlap, l.c_str(), r.c_str());
    numProjectors = _numProjectors;
    layout = _layout;
    rotation = _rotation;

    if(numProjectors <= 0){
        ofLog(OF_LOG_ERROR, "Cannot initialize with " + ofToString(this->numProjectors) + " projectors.");
        return;
    }

    //allow editing projector heights
    for(int i = 0; i < numProjectors; i++){
        projectorHeightOffset.push_back( 0 );
    }

    pixelOverlap = _pixelOverlap;
    projectorWidth = resolutionWidth;
    projectorHeight = resolutionHeight;

    if(rotation == ofxProjectorBlend_NoRotation) {
        singleChannelWidth = resolutionWidth;
        singleChannelHeight = resolutionHeight;
    }
    else {
        singleChannelWidth = resolutionHeight;
        singleChannelHeight = resolutionWidth;
    }

    if(layout == ofxProjectorBlend_Vertical) {
        fullTextureWidth = singleChannelWidth;
        fullTextureHeight = singleChannelHeight*numProjectors - pixelOverlap*(numProjectors-1);
    }
    else if(layout == ofxProjectorBlend_Horizontal) {
        fullTextureWidth = singleChannelWidth*numProjectors - pixelOverlap*(numProjectors-1);
        fullTextureHeight = singleChannelHeight;
    } else {
        ofLog(OF_LOG_ERROR, "ofxProjectorBlend: You have used an invalid ofxProjectorBlendLayout in ofxProjectorBlend::setup()");
        return;
    }


    displayWidth = resolutionWidth*numProjectors;
    displayHeight = resolutionHeight;

    fullTexture.allocate(fullTextureWidth, fullTextureHeight, GL_RGB, 4);
    
    blendShader.unload();
    blendShader.setupShaderFromSource(GL_FRAGMENT_SHADER, ofxProjectorBlendFragShader(numProjectors-1));
    blendShader.setupShaderFromSource(GL_VERTEX_SHADER, ofxProjectorBlendVertShader());
    blendShader.bindDefaults();
    blendShader.linkProgram();
    
    gamma.resize(numProjectors-1, 0.5);
    blendPower.resize(numProjectors-1, 1);
    luminance.resize(numProjectors-1, 0);
}


// --------------------------------------------------
void ofxProjectorBlend::begin() {
    if(!isReady()) return;

    fullTexture.begin();

    ofPushStyle();
    ofClear(0,0,0,0);
}


// --------------------------------------------------
void ofxProjectorBlend::moveDisplayVertical(unsigned int targetDisplay, int yOffset)
{
    if(targetDisplay >= numProjectors){
        ofLog(OF_LOG_ERROR, "targetDisplay (" + ofToString(targetDisplay) + ") is invalid.");
        return;
    }

    projectorHeightOffset[targetDisplay] += yOffset;
}


// --------------------------------------------------
void ofxProjectorBlend::end() {
    if(!isReady()) return;

    fullTexture.end();
    ofPopStyle();
}


// --------------------------------------------------
void ofxProjectorBlend::updateShaderUniforms()
{

    blendShader.setUniform1f("OverlapTop", 0.0f);
    blendShader.setUniform1f("OverlapLeft", 0.0f);
    blendShader.setUniform1f("OverlapBottom", 0.0f);
    blendShader.setUniform1f("OverlapRight", 0.0f);

    blendShader.setUniform1fv("BlendPower", &blendPower[0], blendPower.size());
    blendShader.setUniform1fv("SomeLuminanceControl", &luminance[0], luminance.size());
    blendShader.setUniform1fv("GammaCorrection", &gamma[0], gamma.size());
    
    blendShader.setUniform1f("projectors", this->numProjectors);
    blendShader.setUniform1f("threshold", threshold);
}


// --------------------------------------------------
void ofxProjectorBlend::draw(float x, float y) {
    if(!isReady()) return;

    ofSetHexColor(0xFFFFFF);
    ofPushMatrix();
    ofTranslate(x, y, 0);
    if(showBlend) {
        blendShader.begin();
        blendShader.setUniform1f("width", singleChannelWidth);
        blendShader.setUniform1f("height", singleChannelHeight);

        updateShaderUniforms();

        if(layout == ofxProjectorBlend_Horizontal) {
            blendShader.setUniform1f("OverlapRight", pixelOverlap);
        }
        else {
            blendShader.setUniform1f("OverlapTop", pixelOverlap);
        }

        blendShader.setUniformTexture("Tex0", fullTexture.getTextureReference(), 0);


        ofVec2f offset(0,0);
        ofPushMatrix();

        // loop through each projector and ofTranslate() to its position and draw.
        for(int i = 0; i < numProjectors; i++) {
            blendShader.setUniform2f("texCoordOffset", offset.x, offset.y);

            if(i==1) {
                // set the first edge
                if(layout == ofxProjectorBlend_Horizontal) {
                    blendShader.setUniform1f("OverlapLeft", pixelOverlap);
                }
                else {
                    blendShader.setUniform1f("OverlapBottom", pixelOverlap);
                }

            }
            // if we're at the end of the list of projectors, turn off the second edge's blend

            if(i+1 == numProjectors) {
                if(layout == ofxProjectorBlend_Horizontal) {
                    blendShader.setUniform1f("OverlapRight", 0);
                }
                else {
                    blendShader.setUniform1f("OverlapTop", 0);
                }
            }

            ofPushMatrix(); {
                if(rotation == ofxProjectorBlend_RotatedRight) {
                    ofRotate(90, 0, 0, 1);
                    ofTranslate(0, -singleChannelHeight, 0);
                }
                else if(rotation == ofxProjectorBlend_RotatedLeft) {
                    ofRotate(-90, 0, 0, 1);
                    ofTranslate(-singleChannelWidth, 0, 0);
                }

                ofTranslate(0, (float)projectorHeightOffset[i], 0);

                float w=singleChannelWidth, h=singleChannelHeight;
                quad.set(w, -h, 2, 2); // vertical flip
                quad.setPosition(w/2, h/2, 0);
                quad.mapTexCoords(1, 1, w, h);
                quad.draw();
            }
            ofPopMatrix();

            // move the texture offset and where we're drawing to.
            if(layout == ofxProjectorBlend_Horizontal) {
                offset.x += singleChannelWidth - pixelOverlap;
            }
            else {
                offset.y += singleChannelHeight - pixelOverlap;

            }

            if(rotation == ofxProjectorBlend_RotatedLeft || rotation == ofxProjectorBlend_RotatedRight) {
                ofTranslate(singleChannelHeight, 0, 0);
            }
            else {
                ofTranslate(singleChannelWidth, 0, 0);
            }

        }
        ofPopMatrix();

        blendShader.end();
    } else {
        fullTexture.draw(x, y);
    }
    ofPopMatrix();
}


