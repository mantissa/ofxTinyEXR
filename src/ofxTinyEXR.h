

#pragma once

#include "ofMain.h"

#include <iostream>

class ofxTinyEXR {
    
public:
    
    bool loadImage(ofFloatImage & img, string filepath);
    //bool loadImageExp(ofFloatImage & img, string filepath);
    
    bool saveImage( const ofFloatImage & img, string filepath);
};


