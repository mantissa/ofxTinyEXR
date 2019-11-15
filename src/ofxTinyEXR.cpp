
#include "ofxTinyEXR.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// loads OF_IMAGE_GRAYSCALE, OF_IMAGE_COLOR, OF_IMAGE_COLOR_ALPHA

bool ofxTinyEXR::loadImage(ofFloatImage & image, const string filepath){
    
    string filepath_full =ofToDataPath(filepath);
    const char * input = filepath_full.c_str();
    
    // 1. Read EXR version.
    EXRVersion exr_version;
    
    int ret = ParseEXRVersionFromFile(&exr_version, input);
    if (ret != 0) {
        fprintf(stderr, "Invalid EXR file: %s\n", input);
        return false;
    }
    
    if (exr_version.multipart) {
        // must be multipart flag is false.
        fprintf(stderr, "Must be a singlepart image: %s\n", input);
        return false;
    }
    
    EXRHeader exr_header;
    InitEXRHeader(&exr_header);
    
    const char* err = NULL; // or `nullptr` in C++11 or later.
    ret = ParseEXRHeaderFromFile(&exr_header, &exr_version, input, &err);
    if (ret != 0) {
        fprintf(stderr, "Parse EXR err: %s\n", err);
        FreeEXRErrorMessage(err); // free's buffer for an error message
        return false;
    }
    
    cout << "file " << filepath << endl;
    cout << "channels " << exr_header.num_channels << endl;
    
    // Read HALF channel as FLOAT.
    for (int i = 0; i < exr_header.num_channels; i++) {
        if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
            exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
        }
    }
    
    EXRImage exr_image;
    InitEXRImage(&exr_image);
    
    ret = LoadEXRImageFromFile(&exr_image, &exr_header, input, &err);
    if (ret != 0) {
        fprintf(stderr, "Load EXR err: %s\n", err);
        FreeEXRHeader(&exr_header);
        FreeEXRErrorMessage(err); // free's buffer for an error message
        return false;
    }
    
    // 3. Copy image data
    
    ofLogNotice("ofxTinyEXR") << "dims: " << exr_image.width << ", " << exr_image.height << ", channels: " << exr_image.num_channels;
    
    
    // RGBA
    int idxR = -1;
    int idxG = -1;
    int idxB = -1;
    int idxA = -1;
    for (int c = 0; c < exr_header.num_channels; c++) {
        if (strcmp(exr_header.channels[c].name, "R") == 0) {
            idxR = c;
        } else if (strcmp(exr_header.channels[c].name, "G") == 0) {
            idxG = c;
        } else if (strcmp(exr_header.channels[c].name, "B") == 0) {
            idxB = c;
        } else if (strcmp(exr_header.channels[c].name, "A") == 0) {
            idxA = c;
        }
    }
    
    float * pixels;
    
    if (exr_header.num_channels == 1) {
        // Grayscale channel only.
        
        pixels = reinterpret_cast<float*>(  malloc(sizeof(float) * static_cast<size_t>(exr_image.width) * static_cast<size_t>(exr_image.height)));
        
        if (exr_header.tiled) {
            for (int it = 0; it < exr_image.num_tiles; it++) {
                for (int j = 0; j < exr_header.tile_size_y; j++) {
                    for (int i = 0; i < exr_header.tile_size_x; i++) {
                        const int ii = exr_image.tiles[it].offset_x * exr_header.tile_size_x + i;
                        const int jj = exr_image.tiles[it].offset_y * exr_header.tile_size_y + j;
                        const int idx = ii + jj * exr_image.width;
                        
                        // out of region check.
                        if (ii >= exr_image.width) {
                            continue;
                        }
                        if (jj >= exr_image.height) {
                            continue;
                        }
                        const int srcIdx = i + j * exr_header.tile_size_x;
                        unsigned char **src = exr_image.tiles[it].images;
                        (pixels)[idx + 0] = reinterpret_cast<float **>(src)[0][srcIdx];
                    }
                }
            }
        } else {
            for (int i = 0; i < exr_image.width * exr_image.height; i++) {
                const float val = reinterpret_cast<float **>(exr_image.images)[0][i];
                pixels[i] = val;
            }
        }
        
        // check for RGB
        
    } else if (exr_header.num_channels == 3) {
        
        if (idxR == -1) {
            tinyexr::SetErrorMessage("R channel not found", &err);
            
            // @todo { free exr_image }
            FreeEXRHeader(&exr_header);
            return TINYEXR_ERROR_INVALID_DATA;
        }
        
        if (idxG == -1) {
            tinyexr::SetErrorMessage("G channel not found", &err);
            // @todo { free exr_image }
            FreeEXRHeader(&exr_header);
            return TINYEXR_ERROR_INVALID_DATA;
        }
        
        if (idxB == -1) {
            tinyexr::SetErrorMessage("B channel not found", &err);
            // @todo { free exr_image }
            FreeEXRHeader(&exr_header);
            return TINYEXR_ERROR_INVALID_DATA;
        }
        
        pixels = reinterpret_cast<float*>( malloc(3 * sizeof(float) * static_cast<size_t>(exr_image.width) * static_cast<size_t>(exr_image.height)));
        
        if (exr_header.tiled) {
            for (int it = 0; it < exr_image.num_tiles; it++) {
                for (int j = 0; j < exr_header.tile_size_y; j++) {
                    for (int i = 0; i < exr_header.tile_size_x; i++) {
                        const int ii =
                        exr_image.tiles[it].offset_x * exr_header.tile_size_x + i;
                        const int jj =
                        exr_image.tiles[it].offset_y * exr_header.tile_size_y + j;
                        const int idx = ii + jj * exr_image.width;
                        
                        // out of region check.
                        if (ii >= exr_image.width) {
                            continue;
                        }
                        if (jj >= exr_image.height) {
                            continue;
                        }
                        const int srcIdx = i + j * exr_header.tile_size_x;
                        unsigned char **src = exr_image.tiles[it].images;
                        (pixels)[3 * idx + 0] = reinterpret_cast<float **>(src)[idxR][srcIdx];
                        (pixels)[3 * idx + 1] = reinterpret_cast<float **>(src)[idxG][srcIdx];
                        (pixels)[3 * idx + 2] = reinterpret_cast<float **>(src)[idxB][srcIdx];
                        
                    }
                }
            }
        } else {
            for (int i = 0; i < exr_image.width * exr_image.height; i++) {
                pixels[3 * i + 0] = reinterpret_cast<float **>(exr_image.images)[idxR][i];
                pixels[3 * i + 1] = reinterpret_cast<float **>(exr_image.images)[idxG][i];
                pixels[3 * i + 2] = reinterpret_cast<float **>(exr_image.images)[idxB][i];
            }
        }
        
    } else {
        
        // Assume RGB(A)
        
        if (idxR == -1) {
            tinyexr::SetErrorMessage("R channel not found", &err);
            
            // @todo { free exr_image }
            FreeEXRHeader(&exr_header);
            return TINYEXR_ERROR_INVALID_DATA;
        }
        
        if (idxG == -1) {
            tinyexr::SetErrorMessage("G channel not found", &err);
            // @todo { free exr_image }
            FreeEXRHeader(&exr_header);
            return TINYEXR_ERROR_INVALID_DATA;
        }
        
        if (idxB == -1) {
            tinyexr::SetErrorMessage("B channel not found", &err);
            // @todo { free exr_image }
            FreeEXRHeader(&exr_header);
            return TINYEXR_ERROR_INVALID_DATA;
        }
        
        pixels = reinterpret_cast<float*>( malloc(4 * sizeof(float) * static_cast<size_t>(exr_image.width) * static_cast<size_t>(exr_image.height)));
        
        if (exr_header.tiled) {
            for (int it = 0; it < exr_image.num_tiles; it++) {
                for (int j = 0; j < exr_header.tile_size_y; j++) {
                    for (int i = 0; i < exr_header.tile_size_x; i++) {
                        const int ii =
                        exr_image.tiles[it].offset_x * exr_header.tile_size_x + i;
                        const int jj =
                        exr_image.tiles[it].offset_y * exr_header.tile_size_y + j;
                        const int idx = ii + jj * exr_image.width;
                        
                        // out of region check.
                        if (ii >= exr_image.width) {
                            continue;
                        }
                        if (jj >= exr_image.height) {
                            continue;
                        }
                        const int srcIdx = i + j * exr_header.tile_size_x;
                        unsigned char **src = exr_image.tiles[it].images;
                        (pixels)[4 * idx + 0] =
                        reinterpret_cast<float **>(src)[idxR][srcIdx];
                        (pixels)[4 * idx + 1] =
                        reinterpret_cast<float **>(src)[idxG][srcIdx];
                        (pixels)[4 * idx + 2] =
                        reinterpret_cast<float **>(src)[idxB][srcIdx];
                        if (idxA != -1) {
                            (pixels)[4 * idx + 3] =
                            reinterpret_cast<float **>(src)[idxA][srcIdx];
                        } else {
                            (pixels)[4 * idx + 3] = 1.0;
                        }
                    }
                }
            }
        } else {
            for (int i = 0; i < exr_image.width * exr_image.height; i++) {
                pixels[4 * i + 0] =
                reinterpret_cast<float **>(exr_image.images)[idxR][i];
                pixels[4 * i + 1] =
                reinterpret_cast<float **>(exr_image.images)[idxG][i];
                pixels[4 * i + 2] =
                reinterpret_cast<float **>(exr_image.images)[idxB][i];
                if (idxA != -1) {
                    pixels[4 * i + 3] =
                    reinterpret_cast<float **>(exr_image.images)[idxA][i];
                } else {
                    pixels[4 * i + 3] = 1.0;
                }
            }
        }
    }
    
    /*
    ofImageType imgType;
    
    switch (exr_header.num_channels) {
        case 1:
            imgType = OF_IMAGE_GRAYSCALE;
            break;
            
        case 3:
            imgType = OF_IMAGE_COLOR;
            break;
            
        case 4:
            imgType = OF_IMAGE_COLOR_ALPHA;
            break;
            
        default:
            break;
    }
    
    //image.allocate( exr_image.width, exr_image.height, imgType);
    */
     
    image.getPixels().setFromPixels(pixels, exr_image.width, exr_image.height, exr_header.num_channels);
    image.update();
    
    // 4. Free image data
    FreeEXRImage(&exr_image);
    FreeEXRHeader(&exr_header);
    
    return true;
}

/*
 
 // converts all images to OF_IMAGE_COLOR_ALPHA
 
 bool ofxTinyEXR::loadImage(ofFloatImage & image, const string filepath){
 
     string filepath_full =ofToDataPath(filepath);
     const char * input = filepath_full.c_str();
     
     float * pixels;
     int width;
     int height;
     const char* err = NULL;
     
     // this function just does RGBA
     int ret = LoadEXR(&pixels, &width, &height, input, &err);
     
     if (ret != TINYEXR_SUCCESS) {
 
         if (err) {
         
            fprintf(stderr, "ERR : %s\n", err);
            FreeEXRErrorMessage(err); // release memory of error message.
         }
     
        return false;
     
     } else {
     
        ofLogNotice("ofxTinyEXR") << "dims: " << width << ", " << height; // << ", channels: " << exr_header.num_channel;
     
        image.allocate( width, height, OF_IMAGE_COLOR_ALPHA);
        image.getPixels().setFromPixels(pixels, width, height, OF_IMAGE_COLOR_ALPHA);
        image.update();
     
        free(pixels); // relase memory of image data
    }
 
    return true;
 }
 */

bool ofxTinyEXR::saveImage( const ofFloatImage & img, string filepath){
    
    string filepath_full =ofToDataPath(filepath);
    const char * filename = filepath_full.c_str();
    
    const float * data = img.getPixels().getData();
    int width = img.getPixels().getWidth();
    int height = img.getPixels().getHeight();
    int components = img.getPixels().getNumChannels();
    int save_as_fp16 = 0; // save as float, not half
    
    const char* err = NULL; // or nullptr in C++11

    int ret = SaveEXR(data, width, height, components, save_as_fp16, filename, &err);
    
    if (ret != TINYEXR_SUCCESS) {
        if (err) {
            fprintf(stderr, "ERR : %s\n", err);
            FreeEXRErrorMessage(err); // release memory of error message.
            return false;
        }
    }
    
    return true;
}

bool ofxTinyEXR::saveHDRImage( const ofFloatImage & img, string filepath){
    
    string filepath_full =ofToDataPath(filepath);
    const char * filename = filepath_full.c_str();
    
    const float * data = img.getPixels().getData();
    int width = img.getPixels().getWidth();
    int height = img.getPixels().getHeight();
    int components = img.getPixels().getNumChannels();
    
    ofLogNotice() << components << " components";
    
    int ret = stbi_write_hdr(filename, width, height, components, data);
    
    //int ret = SaveEXR(data, width, height, components, save_as_fp16, filename, &err);
    
    if (ret == 0) {
        
        ofLogError() << "Error saving HDR file\n";
        //FreeEXRErrorMessage(err); // release memory of error message.
        return false;
    
    }
    
    return true;
}