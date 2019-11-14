# ofxTinyEXR
EXR reader and writer for OF
Based on TinyEXR library by syoyo https://github.com/syoyo/tinyexr

## Loading EXR files

```c++
ofxTinyEXR exrIO;
bool loaded = exrIO.loadImageExp(floatImg, "Export.exr");
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
    
floatImg.allocate(w, h, imgType);
floatImg.getPixels().setFromPixels(pix, w, h, imgType);
floatImg.update();
    
bool saved = exrIO.saveImage(floatImg, "Export.exr");
    
if( !saved ) ofLogWarning() << "Failed to save EXR image";
```

