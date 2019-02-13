//
//  ofxProjectorBlendShader.h
//  twoScreens
//
//  Created by Jeffrey Crouse on 11/23/12.
//
//  ofxProjectorBlendVertShader and ofxProjectorBlendFragShader


#pragma once

string ofxProjectorBlendVertShader(){
    string shaderProgram0 = R"(
                            void main()
                            {
                            gl_Position = ftransform();
                            gl_TexCoord[0] = gl_MultiTexCoord0;
                            }
                            )";

    string shaderProgram3 = R"(
                            #version 150

                            uniform mat4 modelViewProjectionMatrix;
                            in vec4 position;
                            in vec2 texcoord;
                            out vec2 varyingtexcoord;

                            void main(){
                               varyingtexcoord = texcoord;
                               gl_Position = modelViewProjectionMatrix * position;
                            }
                            )";
    return ofIsGLProgrammableRenderer() ? shaderProgram3 : shaderProgram0;
}


string prepShader(string header1, string header3, string body) {
    string str;
    if (ofIsGLProgrammableRenderer()) {
        str = header3 + body;
        ofStringReplace(str, "gl_FragColor", "outColor");
        ofStringReplace(str, "gl_TexCoord[0]", "varyingtexcoord");    }
    else {
        str = header1 + body;
    }
    return str;
}



string ofxProjectorBlendFragShader(int blends) {
        string shaderHeader1 = R"(
                               varying vec2 varyingtexcoord;
                               )";
        string shaderHeader3 = R"(
                               #version 150
                               in vec2 varyingtexcoord;
                               out vec4 outColor;
                               )";
//        string shaderBody = R"(
    string shader = "uniform float BlendPower["+ofToString(blends)+"]; \
    uniform float SomeLuminanceControl["+ofToString(blends)+"]; \
    uniform float GammaCorrection["+ofToString(blends)+"];";
    shader += R"(
    uniform sampler2DRect Tex0;
    uniform float width;
    uniform float height;
    uniform float OverlapLeft;
    uniform float OverlapRight; 
    uniform float OverlapTop;
    uniform float OverlapBottom;
    uniform float BlackOutLeft;
    uniform float BlackOutRight; 
    uniform float BlackOutTop;
    uniform float BlackOutBottom;
    uniform float SolidEdgeEnable;
    uniform vec4  SolidEdgeColor;
    uniform vec2  texCoordOffset;
    uniform float projectors;
    uniform float threshold;

    vec4 drawSmoothEdge(float loc, float extend, float blePow, float lumCon, vec3 gamma)
    {
        vec4 edge;
        float curve;

        curve = loc / extend;
        curve = (curve < 0.5) ? 
            lumCon * pow(2.0 * curve, blePow) 
            : 
            1.0 - (1.0 - lumCon) * pow(2.0 * (1.0 - curve), blePow);
        
        edge = vec4(pow(curve, 1.0 / gamma.r), 
                    pow(curve, 1.0 / gamma.g),
                    pow(curve, 1.0 / gamma.b),
                    1.0);
        
        return edge;
    }

    vec4 drawSmoothEdges(in vec4 overlap, in vec4 blankout, in vec4 color)
    {
        vec2 xy = gl_TexCoord[0].xy;
        vec2 xy_t = xy + texCoordOffset.xy;
        vec4 blank = vec4(0.0,0.0,0.0,0.0);	
        
        /*
        float proj2 = 1.0; //projectors ... this basically defines a limit;
        float thresh = proj2 * width + overlap.x; //(projectors - overlap.x) * width;
        float blnPwr = (xy_t.x > thresh) ?  BlendPower[1] : BlendPower[0];
        float slc    = (xy_t.x > thresh) ?  SomeLuminanceControl[1] : SomeLuminanceControl[0];
        float _g   = (xy_t.x > thresh) ?  GammaCorrection[1] : GammaCorrection[0];
        vec3 gamma = vec3(_g, _g, _g);
         */
        
        int b = int( xy_t.x / width );
        float blnPwr = BlendPower[b];
        float slc = SomeLuminanceControl[b] + 0.5;
        vec3 gamma = vec3(GammaCorrection[b], GammaCorrection[b], GammaCorrection[b]);
        //slc += 0.5;
        
        vec4 sebImage = 
            (blankout.x + overlap.x > xy.x) ? 
                (blankout.x > xy.x) ? 	
                    blank 							//leftBlankout
                :	(blankout.z + overlap.z > xy.y) ?
                        (blankout.z > xy.y) ? 
                            blank					//leftBottomBlankOut
                        : 	texture2DRect(Tex0, xy_t) 	*		//leftBottomBlend
                            drawSmoothEdge(xy.x - blankout.x, overlap.x, blnPwr, slc, gamma) * 
                            drawSmoothEdge(xy.y - blankout.z, overlap.z, blnPwr, slc, gamma)
                    : 	(height - blankout.w - overlap.w < xy.y) ?  
                            (height - blankout.w < xy.y) ? 
                                blank				//leftTopBlankout
                            : 	texture2DRect(Tex0, xy_t) 	*	//LeftTopBlend
                                drawSmoothEdge(xy.x - blankout.x, overlap.x, blnPwr, slc, gamma) * 
                                drawSmoothEdge(height - blankout.w - xy.y, overlap.w, blnPwr, slc, gamma)					
                                :	texture2DRect(Tex0, xy_t) *		//leftBlankout
                            drawSmoothEdge(xy.x - blankout.x, overlap.x, blnPwr, slc, gamma)// + vec4(0.5, 0, 0.5, 0)	// debug: magenta
            :	(width - blankout.y - overlap.y < xy.x) ? 
                    (width - blankout.y < xy.x) ? 
                        blank						//rightBlankout
                    :	(blankout.z + overlap.z > xy.y) ?
                            (blankout.z > xy.y) ? 
                                blank 				//rightBottomBlankout
                            : 	texture2DRect(Tex0, xy_t) 	*	//rightBottomBlend
                                drawSmoothEdge(width - blankout.y - xy.x, overlap.y, blnPwr, slc, gamma) *
                                drawSmoothEdge(xy.y - blankout.z, overlap.z, blnPwr, slc, gamma)
                        :	(height - blankout.w - overlap.w < xy.y) ?
                                (height - blankout.w < xy.y) ? 
                                    blank 			//rightTopBlankout
                                :	texture2DRect(Tex0, xy_t) * //rightTopBlend
                                    drawSmoothEdge(width - blankout.y - xy.x, overlap.y, blnPwr, slc, gamma) * 
                                    drawSmoothEdge(height - blankout.w - xy.y, overlap.w, blnPwr, slc, gamma)
                            :	texture2DRect(Tex0, xy_t) * 	//rightBlend
                                drawSmoothEdge(width - blankout.y - xy.x, overlap.y, blnPwr, slc, gamma)// + vec4(0, 0, 0.5, 0)	// debug: blue
                :	(blankout.z + overlap.z > xy.y) ?
                        (blankout.z > xy.y) ? 
                            blank 					//BottomBlankout
                        : 	texture2DRect(Tex0, xy_t) * 		//BottomBlend
                            drawSmoothEdge(xy.y - blankout.z, overlap.z, blnPwr, slc, gamma)
                    :	(height - blankout.w - overlap.w < xy.y) ?
                            (height - blankout.w < xy.y) ? 
                                blank 				//TopBlankout
                            :	texture2DRect(Tex0, xy_t) * 	//TopBlend
                                drawSmoothEdge(height - blankout.w - xy.y, overlap.w, blnPwr, slc, gamma)		
                                :	texture2DRect(Tex0, xy_t);// + vec4(0, 0.5, 0.5, 0);	// debug: teal
        
        //	if(xy_t.x < thresh) sebImage.rgb += vec3(0, 1.0, 0);	// debug
        
        return sebImage;
    }

    vec4 drawSolidEdges(vec4 overlap, vec4 blankout, vec4 color)
    {	
        vec2 xy = gl_TexCoord[0].xy;
        vec2 xy_t = xy + texCoordOffset.xy;
        vec4 blank = vec4(0.0,0.0,0.0,0.0);		
        vec4 edgeImage =
                (blankout.x + overlap.x > xy.x) ?
                    (blankout.x > xy.x) ?
                        blank				//leftBlankout
                    :	(blankout.z + overlap.z > xy.y) ?
                            (blankout.z > xy.y) ?
                                blank			//leftBottomBlankout
                            : 	color			//leftBottomColor
                        : 	(height - blankout.w - overlap.w < xy.y) ?
                                (height - blankout.w < xy.y) ?
                                    blank 		//leftTop
                                : 	color 		//leftTopColor
                            :	color			//leftColor
                :	(width - blankout.y - overlap.y < xy.x) ?
                        (width - blankout.y < xy.x) ?
                            blank				//rightBlankout
                        :	(blankout.z + overlap.z > xy.y) ?
                                (blankout.z > xy.y) ?
                                    blank 		//rightBottomBlankout
                                : 	color		//rightBottomColor
                            :	(height - blankout.w - overlap.w < xy.y) ?
                                    (height - blankout.w < xy.y) ? 
                                        blank 	//rightTopBlankout
                                    :	color 	//rightToColor
                                :	color 		//rightColor
                    :	(blankout.z + overlap.z > xy.y) ?
                            (blankout.z > xy.y) ? 
                                blank 			//BottomBlankout
                            : 	color			//BottomColor
                        :	(height - blankout.w - overlap.w < xy.y) ?
                                (height - blankout.w < xy.y) ? 
                                    blank 		//TopBlankout
                                :	color		//TopColor
                            :	texture2DRect(Tex0, xy_t);		
        return edgeImage;
    }

    void main (void)
    {
        vec4 overlap = vec4 (OverlapLeft, OverlapRight, OverlapBottom, OverlapTop);
        vec4 blankout = vec4 (BlackOutLeft, BlackOutRight, BlackOutBottom, BlackOutTop);
        gl_FragColor = (SolidEdgeEnable == 1.0) ?
                drawSolidEdges(overlap, blankout, SolidEdgeColor)
            :	drawSmoothEdges(overlap, blankout, SolidEdgeColor);
    }
            )";
    return prepShader(shaderHeader1, shaderHeader3, shader);;
}


