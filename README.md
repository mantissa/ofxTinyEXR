# ofxTinyEXR
EXR reader and writer for openFrameworks

Based on TinyEXR library by syoyo https://github.com/syoyo/tinyexr

EXR is a file format for saving HDR/floating point images.

## Loading EXR files

```c++
ofxTinyEXR exrIO;
ofFloatImage floatImg;
bool loaded = exrIO.loadImage(floatImg, “MyHDRImage.exr");
```

## Saving EXR files

```c++
int w = 512;
int h = 512;
int nPix = w * h * 3; 
ofImageType imgType = OF_IMAGE_COLOR; 
    
float * pix = new float[nPix];
    
for(int i=0; i<nPix; i++){
    pix[i] = ofRandomuf();
}

ofFloatImage floatImg;
floatImg.allocate(w, h, imgType);
floatImg.getPixels().setFromPixels(pix, w, h, imgType);
floatImg.update();
    
ofxTinyEXR exrIO;
bool saved = exrIO.saveImage(floatImg, "Export.exr");
if( !saved ) ofLogWarning() << "Failed to save EXR image";
```

## Writing Float FBO to files

```c++
// allocate
ofFbo floatFbo;
floatFbo.allocate(512, 512, GL_RGB16, 0);

// draw into FBO
floatFbo.begin();    
ofClear(0);
ofSetColor(255.0);
ofDrawEllipse(512/2, 512/2, 512, 512);
floatFbo.end();

// read to pixels 
ofFloatPixels pix;
pix.allocate( 512, 512, OF_IMAGE_COLOR );        
floatFbo.readToPixels(pix);
        
// copy to float image
floatImg.getPixels().setFromPixels(pix.getData(), 512, 512, OF_IMAGE_COLOR);
floatImg.update();
        
// save file
ofxTinyEXR exrIO;
bool saved = exrIO.saveImage(floatImg, "Frame.exr");
if( !saved ) ofLogWarning() << "Failed to save EXR image";
```

## Viewing EXR files in Photoshop

When importing EXR files into Adobe Photoshop, you will see a large discrepancy in the contrast & saturation of your image. 

This discrepancy be resolved by changing the “Proof Condition”:

View -> Proof setup -> Custom

Select “HDTV (Rec. 709)” and enable “Preserve RGB Numbers”.

Note: I recommend using [EXR-IO](https://www.exr-io.com/) for importing EXR files.
