// Use board "ESP32C6 Dev Module" (last tested on v3.2.0)

#define LV_CONF_INCLUDE_SIMPLE

#include <lvgl.h>
#include "PINS_ESP32-C6-LCD-1_47.h"
#include "Arduino.h"

// Declare external image (compiled separately)
extern const lv_img_dsc_t img2;

#define SCREEN_BRIGHTNESS 255
#define SCREEN_WIDTH 172
#define SCREEN_HEIGHT 320
#define DISPLAY_TIME 5000  // 3 seconds per image
#define SPLASH_TIME 1500   // 1.5 seconds for splash screen

// LVGL Display
lv_display_t *disp;

// Image display objects
static lv_obj_t *imgDisplay;
static lv_obj_t *statusLabel;

// Single shared processing buffer - reused for all operations
static uint16_t *processingBuffer = NULL;
static uint8_t *grayscaleBuffer = NULL;

// Current state
static uint8_t currentState = 0;
static uint32_t lastSwitchTime = 0;
static lv_img_dsc_t currentDisplayImage;

// Forward declarations
void convertToGrayscale(const lv_img_dsc_t* imgDsc, uint8_t* grayData);
void histogramEqualizationRGB(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565);
void applySepiaEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565);
void applyLowPolyEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565);
void applyUnderwaterEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565);
void detectEdges(const lv_img_dsc_t* imgDsc, uint8_t* edgeData);
void applyHalftoneEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565);
void applyThermalMood(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565, uint8_t mode);
void displayImageOnScreen(const lv_img_dsc_t *imgDsc, const char *title);
void displaySplashScreen(const char *nextImage);
lv_img_dsc_t createGrayscaleImage(const uint8_t* grayData, uint16_t width, uint16_t height);
lv_img_dsc_t createRGB565Image(const uint16_t* rgbData, uint16_t width, uint16_t height);
void clearProcessingBuffers();

// LVGL tick function
uint32_t millis_cb(void) {
    return millis();
}

// LVGL display flush callback
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
    lv_disp_flush_ready(disp);
}

// LVGL print function for debugging
void my_print(lv_log_level_t level, const char *buf) {
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}

// Clear processing buffers
void clearProcessingBuffers() {
    if (processingBuffer) {
        memset(processingBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
    }
    if (grayscaleBuffer) {
        memset(grayscaleBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    }
}

// Display beautiful splash screen
void displaySplashScreen(const char *nextImage) {
    lv_obj_clean(lv_scr_act());
    
    // Create gradient background
    lv_obj_t *bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bg, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(bg, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_grad_color(bg, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_grad_dir(bg, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_border_width(bg, 0, 0);
    lv_obj_set_style_pad_all(bg, 0, 0);
    
    // Create animated loading circle
    lv_obj_t *spinner = lv_spinner_create(lv_scr_act());
    lv_obj_set_size(spinner, 60, 60);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_style_arc_width(spinner, 6, 0);
    
    // Main title
    lv_obj_t *titleLabel = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(0x00d4ff), 0);
    lv_label_set_text(titleLabel, "LOADING");
    lv_obj_align(titleLabel, LV_ALIGN_CENTER, 0, 30);
    
    // Next image label
    lv_obj_t *nextLabel = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(nextLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(nextLabel, lv_color_hex(0xffffff), 0);
    lv_label_set_text(nextLabel, nextImage);
    lv_obj_align(nextLabel, LV_ALIGN_CENTER, 0, 55);
    
    // Decorative dots
    lv_obj_t *dot1 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dot1, 8, 8);
    lv_obj_set_style_radius(dot1, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot1, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_style_border_width(dot1, 0, 0);
    lv_obj_align(dot1, LV_ALIGN_BOTTOM_MID, -20, -30);
    
    lv_obj_t *dot2 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dot2, 8, 8);
    lv_obj_set_style_radius(dot2, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot2, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_style_border_width(dot2, 0, 0);
    lv_obj_align(dot2, LV_ALIGN_BOTTOM_MID, 0, -30);
    
    lv_obj_t *dot3 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dot3, 8, 8);
    lv_obj_set_style_radius(dot3, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot3, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_style_border_width(dot3, 0, 0);
    lv_obj_align(dot3, LV_ALIGN_BOTTOM_MID, 20, -30);
    
    // Clear buffers after splash loads
    clearProcessingBuffers();
}

// Convert RGB565 image to grayscale
void convertToGrayscale(const lv_img_dsc_t* imgDsc, uint8_t* grayData) {
    Serial.println("Converting to grayscale...");
    
    uint16_t width = imgDsc->header.w;
    uint16_t height = imgDsc->header.h;
    const uint16_t* rgbData = (const uint16_t*)imgDsc->data;
    
    for (uint32_t i = 0; i < width * height; i++) {
        uint16_t pixel = rgbData[i];
        
        // Extract RGB components from RGB565
        uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
        uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
        uint8_t b = (pixel & 0x1F) * 255 / 31;
        
        // Standard grayscale conversion (ITU-R BT.601)
        grayData[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
    }
    
    Serial.println("Grayscale conversion complete");
}

// Apply sepia effect
void applySepiaEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565) {
    Serial.println("Applying sepia effect...");
    
    uint16_t width = imgDsc->header.w;
    uint16_t height = imgDsc->header.h;
    const uint16_t* rgbData = (const uint16_t*)imgDsc->data;
    
    for (uint32_t i = 0; i < width * height; i++) {
        uint16_t pixel = rgbData[i];
        
        // Extract RGB components from RGB565
        uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
        uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
        uint8_t b = (pixel & 0x1F) * 255 / 31;
        
        // Apply sepia tone transformation
        int tr = (int)(0.393f * r + 0.769f * g + 0.189f * b);
        int tg = (int)(0.349f * r + 0.686f * g + 0.168f * b);
        int tb = (int)(0.272f * r + 0.534f * g + 0.131f * b);
        
        // Clamp values to 0-255
        uint8_t sepiaR = (tr > 255) ? 255 : tr;
        uint8_t sepiaG = (tg > 255) ? 255 : tg;
        uint8_t sepiaB = (tb > 255) ? 255 : tb;
        
        // Convert back to RGB565
        uint16_t r5 = (sepiaR >> 3) & 0x1F;
        uint16_t g6 = (sepiaG >> 2) & 0x3F;
        uint16_t b5 = (sepiaB >> 3) & 0x1F;
        
        outputRGB565[i] = (r5 << 11) | (g6 << 5) | b5;
    }
    
    Serial.println("Sepia effect complete");
}

// Sobel edge detection (lite version)
void detectEdges(const lv_img_dsc_t* imgDsc, uint8_t* edgeData) {
    Serial.println("Detecting edges...");
    
    uint16_t width = imgDsc->header.w;
    uint16_t height = imgDsc->header.h;
    const uint16_t* rgbData = (const uint16_t*)imgDsc->data;
    
    // First convert to grayscale for edge detection
    for (uint32_t i = 0; i < width * height; i++) {
        uint16_t pixel = rgbData[i];
        uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
        uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
        uint8_t b = (pixel & 0x1F) * 255 / 31;
        grayscaleBuffer[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
    }
    
    // Sobel operators
    int8_t sobelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int8_t sobelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    
    // Apply Sobel filter
    for (uint16_t y = 1; y < height - 1; y++) {
        for (uint16_t x = 1; x < width - 1; x++) {
            int16_t gx = 0, gy = 0;
            
            // Convolve with Sobel kernels
            for (int8_t ky = -1; ky <= 1; ky++) {
                for (int8_t kx = -1; kx <= 1; kx++) {
                    uint32_t idx = (y + ky) * width + (x + kx);
                    uint8_t pixel = grayscaleBuffer[idx];
                    gx += pixel * sobelX[ky + 1][kx + 1];
                    gy += pixel * sobelY[ky + 1][kx + 1];
                }
            }
            
            // Calculate gradient magnitude
            int16_t magnitude = abs(gx) + abs(gy);
            edgeData[y * width + x] = (magnitude > 255) ? 255 : magnitude;
        }
    }
    
    // Set borders to zero
    for (uint16_t x = 0; x < width; x++) {
        edgeData[x] = 0;
        edgeData[(height - 1) * width + x] = 0;
    }
    for (uint16_t y = 0; y < height; y++) {
        edgeData[y * width] = 0;
        edgeData[y * width + (width - 1)] = 0;
    }
    
    Serial.println("Edge detection complete");
}

// Apply Low Poly Effect (Triangulated/Faceted Look)
void applyLowPolyEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565) {
    Serial.println("Applying low poly effect...");
    
    uint16_t width = imgDsc->header.w;
    uint16_t height = imgDsc->header.h;
    const uint16_t* rgbData = (const uint16_t*)imgDsc->data;
    
    // Triangle/polygon size - larger = fewer polygons
    const uint8_t polySize = 12;
    
    // Process image in blocks to create faceted triangular regions
    for (uint16_t y = 0; y < height; y += polySize) {
        for (uint16_t x = 0; x < width; x += polySize) {
            // Calculate average color for this block region
            uint32_t sumR = 0, sumG = 0, sumB = 0;
            uint16_t count = 0;
            
            // Sample the block
            for (uint16_t dy = 0; dy < polySize && (y + dy) < height; dy++) {
                for (uint16_t dx = 0; dx < polySize && (x + dx) < width; dx++) {
                    uint32_t idx = (y + dy) * width + (x + dx);
                    uint16_t pixel = rgbData[idx];
                    
                    sumR += ((pixel >> 11) & 0x1F);
                    sumG += ((pixel >> 5) & 0x3F);
                    sumB += (pixel & 0x1F);
                    count++;
                }
            }
            
            // Calculate average color in RGB565 components
            uint16_t avgR5 = (sumR / count) & 0x1F;
            uint16_t avgG6 = (sumG / count) & 0x3F;
            uint16_t avgB5 = (sumB / count) & 0x1F;
            
            // Create two triangular regions within the block for more geometric look
            for (uint16_t dy = 0; dy < polySize && (y + dy) < height; dy++) {
                for (uint16_t dx = 0; dx < polySize && (x + dx) < width; dx++) {
                    uint32_t idx = (y + dy) * width + (x + dx);
                    
                    // Determine which triangle this pixel belongs to
                    // Diagonal split creates two triangles
                    bool upperTriangle = (dx + dy) < polySize;
                    
                    // Add slight variation between triangles for faceted look
                    uint16_t r5 = avgR5;
                    uint16_t g6 = avgG6;
                    uint16_t b5 = avgB5;
                    
                    if (!upperTriangle) {
                        // Lower triangle - slightly darker
                        r5 = (r5 * 90 / 100);
                        g6 = (g6 * 90 / 100);
                        b5 = (b5 * 90 / 100);
                    }
                    
                    // Add edge darkening for polygon borders
                    if (dx == 0 || dy == 0 || dx == polySize-1 || dy == polySize-1 ||
                        (upperTriangle && dx + dy == polySize - 1) ||
                        (!upperTriangle && dx + dy == polySize)) {
                        r5 = r5 * 70 / 100;
                        g6 = g6 * 70 / 100;
                        b5 = b5 * 70 / 100;
                    }
                    
                    outputRGB565[idx] = (r5 << 11) | (g6 << 5) | b5;
                }
            }
        }
    }
    
    Serial.println("Low poly effect complete");
}

// Apply Underwater Effect with caustics and distortion
void applyUnderwaterEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565) {
    Serial.println("Applying underwater effect...");
    
    uint16_t width = imgDsc->header.w;
    uint16_t height = imgDsc->header.h;
    const uint16_t* rgbData = (const uint16_t*)imgDsc->data;
    
    // Time-based animation seed (use millis for dynamic effect)
    float timeOffset = (millis() % 10000) / 1000.0f;
    
    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            uint32_t idx = y * width + x;
            
            // Create wave distortion (simulates water surface refraction)
            // Multiple sine waves at different frequencies
            float wave1 = sin((x * 0.05f) + (y * 0.03f) + timeOffset) * 2.5f;
            float wave2 = sin((x * 0.08f) - (y * 0.04f) + timeOffset * 1.3f) * 1.8f;
            float totalWave = wave1 + wave2;
            
            // Apply distortion offset
            int16_t sourceX = x + (int16_t)totalWave;
            int16_t sourceY = y + (int16_t)(totalWave * 0.6f);
            
            // Clamp to image boundaries
            sourceX = (sourceX < 0) ? 0 : ((sourceX >= width) ? width - 1 : sourceX);
            sourceY = (sourceY < 0) ? 0 : ((sourceY >= height) ? height - 1 : sourceY);
            
            uint32_t sourceIdx = sourceY * width + sourceX;
            uint16_t pixel = rgbData[sourceIdx];
            
            // Extract RGB components
            uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
            uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
            uint8_t b = (pixel & 0x1F) * 255 / 31;
            
            // Apply blue-green underwater tint
            r = r * 70 / 100;  // Reduce red
            g = g * 95 / 100;  // Keep most green
            b = b * 120 / 100; // Boost blue
            b = (b > 255) ? 255 : b;
            
            // Create caustic light patterns (underwater sunlight ripples)
            float causticX = x * 0.08f + timeOffset * 0.5f;
            float causticY = y * 0.08f + timeOffset * 0.7f;
            
            float caustic1 = sin(causticX) * cos(causticY);
            float caustic2 = sin(causticX * 1.5f + 1.0f) * cos(causticY * 1.3f + 0.5f);
            float causticPattern = (caustic1 + caustic2) * 0.5f + 0.5f; // Normalize to 0-1
            
            // Create brighter caustic spots
            if (causticPattern > 0.7f) {
                float intensity = (causticPattern - 0.7f) / 0.3f; // 0-1 range for bright spots
                r += (uint8_t)(intensity * 60);
                g += (uint8_t)(intensity * 80);
                b += (uint8_t)(intensity * 40);
            }
            
            // Add shimmering highlights in upper portion (simulates surface)
            if (y < height / 3) {
                float shimmer = sin(x * 0.15f + timeOffset * 2.0f) * 
                                cos(y * 0.12f + timeOffset * 1.8f);
                shimmer = (shimmer + 1.0f) * 0.5f; // Normalize to 0-1
                
                if (shimmer > 0.8f) {
                    float sparkle = (shimmer - 0.8f) / 0.2f;
                    r += (uint8_t)(sparkle * 50);
                    g += (uint8_t)(sparkle * 70);
                    b += (uint8_t)(sparkle * 90);
                }
            }
            
            // Add depth gradient (darker towards bottom)
            float depthFactor = 1.0f - (y / (float)height) * 0.3f;
            r = (uint8_t)(r * depthFactor);
            g = (uint8_t)(g * depthFactor);
            b = (uint8_t)(b * depthFactor);
            
            // Add slight green tint to deeper areas
            if (y > height / 2) {
                g += (uint8_t)((y - height/2) * 0.1f);
            }
            
            // Clamp all values
            r = (r > 255) ? 255 : r;
            g = (g > 255) ? 255 : g;
            b = (b > 255) ? 255 : b;
            
            // Convert back to RGB565
            uint16_t r5 = (r >> 3) & 0x1F;
            uint16_t g6 = (g >> 2) & 0x3F;
            uint16_t b5 = (b >> 3) & 0x1F;
            
            outputRGB565[idx] = (r5 << 11) | (g6 << 5) | b5;
        }
    }
    
    Serial.println("Underwater effect complete");
}

// Apply halftone dots effect (newspaper/screen print style)
void applyHalftoneEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565) {
    Serial.println("Applying halftone effect...");
    
    uint16_t width = imgDsc->header.w;
    uint16_t height = imgDsc->header.h;
    const uint16_t* rgbData = (const uint16_t*)imgDsc->data;
    
    const uint8_t dotSize = 4; // Size of halftone dot pattern
    
    // First, convert to grayscale for intensity
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t i = y * width + x;
            uint16_t pixel = rgbData[i];
            
            uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
            uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
            uint8_t b = (pixel & 0x1F) * 255 / 31;
            
            // Calculate luminance
            uint8_t luma = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
            
            // Create halftone pattern based on position within dot grid
            uint8_t dotX = x % dotSize;
            uint8_t dotY = y % dotSize;
            
            // Calculate distance from center of dot (simplified)
            int8_t dx = dotX - dotSize/2;
            int8_t dy = dotY - dotSize/2;
            uint8_t distSquared = dx*dx + dy*dy;
            
            // Threshold based on luminance - darker areas get bigger dots
            uint8_t threshold = (255 - luma) / 16; // 0-15 range
            
            uint8_t outputValue;
            if (distSquared <= threshold) {
                // Inside dot - use original color but slightly enhanced
                outputValue = luma;
            } else {
                // Outside dot - white/light background
                outputValue = 255;
            }
            
            // Create monochrome halftone or colored version
            // Using cyan-magenta-yellow style for color halftone
            uint8_t finalR, finalG, finalB;
            
            if (outputValue == 255) {
                // Background - white
                finalR = finalG = finalB = 255;
            } else {
                // Keep some color information for colored halftone
                finalR = (r * outputValue) / 255;
                finalG = (g * outputValue) / 255;
                finalB = (b * outputValue) / 255;
                
                // Enhance contrast
                finalR = (finalR * 120 / 100) > 255 ? 255 : (finalR * 120 / 100);
                finalG = (finalG * 120 / 100) > 255 ? 255 : (finalG * 120 / 100);
                finalB = (finalB * 120 / 100) > 255 ? 255 : (finalB * 120 / 100);
            }
            
            // Convert back to RGB565
            uint16_t r5 = (finalR >> 3) & 0x1F;
            uint16_t g6 = (finalG >> 2) & 0x3F;
            uint16_t b5 = (finalB >> 3) & 0x1F;
            
            outputRGB565[i] = (r5 << 11) | (g6 << 5) | b5;
        }
    }
    
    Serial.println("Halftone effect complete");
}

// Apply thermal mood color remapping
// mode: 0=calm(blue/purple), 1=warm(orange/red), 2=energetic(neon), 3=rainy(desaturated)
void applyThermalMood(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565, uint8_t mode) {
    const char* moodNames[] = {"Calm", "Warm", "Energetic", "Rainy"};
    Serial.printf("Applying %s mood...\n", moodNames[mode]);
    
    uint16_t width = imgDsc->header.w;
    uint16_t height = imgDsc->header.h;
    const uint16_t* rgbData = (const uint16_t*)imgDsc->data;
    
    for (uint32_t i = 0; i < width * height; i++) {
        uint16_t pixel = rgbData[i];
        
        // Extract RGB components
        uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
        uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
        uint8_t b = (pixel & 0x1F) * 255 / 31;
        
        // Calculate luminance for intensity mapping
        float luma = (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f;
        
        uint8_t nr, ng, nb;
        
        switch(mode) {
            case 0: // CALM MODE - Blue/Purple gradient
                nr = luma * 120 + 50;  // Soft purple
                ng = luma * 80 + 30;   // Less green
                nb = luma * 200 + 55;  // Strong blue
                break;
                
            case 1: // WARM MODE - Orange/Red gradient
                nr = luma * 200 + 55;  // Strong red
                ng = luma * 120 + 50;  // Medium orange
                nb = luma * 60 + 20;   // Low blue
                break;
                
            case 2: // ENERGETIC MODE - Neon/Electric
                // Neon effect with high saturation
                nr = luma * 255;
                ng = luma * 255;
                nb = luma * 255;
                
                // Add neon color shift based on intensity
                if (luma > 0.7f) {
                    nr = 255;
                    ng = luma * 100 + 155;
                    nb = luma * 200 + 55;
                } else if (luma > 0.4f) {
                    nr = luma * 150 + 105;
                    ng = 255;
                    nb = luma * 180 + 75;
                } else {
                    nr = luma * 180 + 75;
                    ng = luma * 120 + 50;
                    nb = 255;
                }
                break;
                
            case 3: // RAINY MODE - Desaturated cool gray
                // Calculate average for desaturation
                uint8_t avg = (r + g + b) / 3;
                // Shift towards cool blue-gray
                nr = avg * 0.85f;
                ng = avg * 0.90f;
                nb = avg * 1.05f;
                break;
        }
        
        // Clamp values
        nr = nr > 255 ? 255 : nr;
        ng = ng > 255 ? 255 : ng;
        nb = nb > 255 ? 255 : nb;
        
        // Convert back to RGB565
        uint16_t r5 = (nr >> 3) & 0x1F;
        uint16_t g6 = (ng >> 2) & 0x3F;
        uint16_t b5 = (nb >> 3) & 0x1F;
        
        outputRGB565[i] = (r5 << 11) | (g6 << 5) | b5;
    }
    
    Serial.printf("%s mood complete\n", moodNames[mode]);
}

// Perform histogram equalization on RGB565 image
void histogramEqualizationRGB(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565) {
    Serial.println("Performing histogram equalization on RGB channels...");
    
    uint16_t width = imgDsc->header.w;
    uint16_t height = imgDsc->header.h;
    const uint16_t* rgbData = (const uint16_t*)imgDsc->data;
    uint32_t totalPixels = width * height;
    
    // Process each color channel separately
    uint32_t histogramR[256] = {0};
    uint32_t histogramG[256] = {0};
    uint32_t histogramB[256] = {0};
    
    uint32_t cdfR[256] = {0};
    uint32_t cdfG[256] = {0};
    uint32_t cdfB[256] = {0};
    
    // Build histograms for each channel
    for (uint32_t i = 0; i < totalPixels; i++) {
        uint16_t pixel = rgbData[i];
        
        uint8_t r8 = ((pixel >> 11) & 0x1F) * 255 / 31;
        uint8_t g8 = ((pixel >> 5) & 0x3F) * 255 / 63;
        uint8_t b8 = (pixel & 0x1F) * 255 / 31;
        
        histogramR[r8]++;
        histogramG[g8]++;
        histogramB[b8]++;
    }
    
    // Calculate CDFs for each channel
    cdfR[0] = histogramR[0];
    cdfG[0] = histogramG[0];
    cdfB[0] = histogramB[0];
    
    for (int i = 1; i < 256; i++) {
        cdfR[i] = cdfR[i - 1] + histogramR[i];
        cdfG[i] = cdfG[i - 1] + histogramG[i];
        cdfB[i] = cdfB[i - 1] + histogramB[i];
    }
    
    // Find minimum non-zero CDF values
    uint32_t cdfMinR = 0, cdfMinG = 0, cdfMinB = 0;
    for (int i = 0; i < 256; i++) {
        if (cdfR[i] > 0 && cdfMinR == 0) cdfMinR = cdfR[i];
        if (cdfG[i] > 0 && cdfMinG == 0) cdfMinG = cdfG[i];
        if (cdfB[i] > 0 && cdfMinB == 0) cdfMinB = cdfB[i];
    }
    
    // Apply histogram equalization to each channel
    for (uint32_t i = 0; i < totalPixels; i++) {
        uint16_t pixel = rgbData[i];
        
        uint8_t r8 = ((pixel >> 11) & 0x1F) * 255 / 31;
        uint8_t g8 = ((pixel >> 5) & 0x3F) * 255 / 63;
        uint8_t b8 = (pixel & 0x1F) * 255 / 31;
        
        // Equalize each channel
        uint8_t eqR = (uint8_t)(((cdfR[r8] - cdfMinR) * 255) / (totalPixels - cdfMinR));
        uint8_t eqG = (uint8_t)(((cdfG[g8] - cdfMinG) * 255) / (totalPixels - cdfMinG));
        uint8_t eqB = (uint8_t)(((cdfB[b8] - cdfMinB) * 255) / (totalPixels - cdfMinB));
        
        // Convert back to RGB565
        uint16_t r5 = (eqR >> 3) & 0x1F;
        uint16_t g6 = (eqG >> 2) & 0x3F;
        uint16_t b5 = (eqB >> 3) & 0x1F;
        
        outputRGB565[i] = (r5 << 11) | (g6 << 5) | b5;
    }
    
    Serial.println("Histogram equalization complete");
}

// Display image on screen
void displayImageOnScreen(const lv_img_dsc_t *imgDsc, const char *title) {
    lv_obj_clean(lv_scr_act());
    
    // Create dark background
    lv_obj_t *bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bg, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(bg, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(bg, 0, 0);
    lv_obj_set_style_pad_all(bg, 0, 0);
    
    // Create title label with accent background
    lv_obj_t *titleBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(titleBg, LV_PCT(100), 30);
    lv_obj_set_style_bg_color(titleBg, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_border_width(titleBg, 0, 0);
    lv_obj_set_style_radius(titleBg, 0, 0);
    lv_obj_align(titleBg, LV_ALIGN_TOP_MID, 0, 0);
    
    statusLabel = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x00d4ff), 0);
    lv_label_set_text(statusLabel, title);
    lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 0, 8);
    
    // Create image display
    imgDisplay = lv_img_create(lv_scr_act());
    lv_img_set_src(imgDisplay, imgDsc);
    lv_obj_align(imgDisplay, LV_ALIGN_CENTER, 0, 10);
}

// Create grayscale image descriptor from grayscale data
lv_img_dsc_t createGrayscaleImage(const uint8_t* grayData, uint16_t width, uint16_t height) {
    // Convert grayscale to RGB565 for display
    for (uint32_t i = 0; i < width * height; i++) {
        uint8_t gray = grayData[i];
        uint16_t r5 = (gray >> 3) & 0x1F;
        uint16_t g6 = (gray >> 2) & 0x3F;
        uint16_t b5 = (gray >> 3) & 0x1F;
        processingBuffer[i] = (r5 << 11) | (g6 << 5) | b5;
    }
    
    lv_img_dsc_t img_dsc;
    img_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
    img_dsc.header.w = width;
    img_dsc.header.h = height;
    img_dsc.data = (const uint8_t*)processingBuffer;
    img_dsc.data_size = width * height * 2;
    
    return img_dsc;
}

// Create RGB565 image descriptor
lv_img_dsc_t createRGB565Image(const uint16_t* rgbData, uint16_t width, uint16_t height) {
    lv_img_dsc_t img_dsc;
    img_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
    img_dsc.header.w = width;
    img_dsc.header.h = height;
    img_dsc.data = (const uint8_t*)rgbData;
    img_dsc.data_size = width * height * 2;
    
    return img_dsc;
}

// Setup function
void setup() {
    Serial.begin(115200);
    DEV_DEVICE_INIT();
    delay(2000);
    
    Serial.println("ESP32-C6 Image Display with Effects (Enhanced)");
    Serial.println("--- Memory Diagnostic ---");
    
    uint32_t freeHeap = ESP.getFreeHeap();
    Serial.printf("Free heap: %u bytes (%u KB)\n", freeHeap, freeHeap / 1024);
    
    // Init Display
    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
        while (true) {}
    }
    gfx->setRotation(0);
    gfx->fillScreen(RGB565_BLACK);
    
    // Set brightness
    ledcAttachChannel(GFX_BL, 1000, 8, 1);
    ledcWrite(GFX_BL, SCREEN_BRIGHTNESS);
    
    // Init LVGL
    lv_init();
    lv_tick_set_cb(millis_cb);
    
#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif
    
    // Setup display buffer
    uint32_t bufSize = SCREEN_WIDTH * 40;
    lv_color_t *disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!disp_draw_buf) {
        disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
    }
    if (!disp_draw_buf) {
        Serial.println("LVGL buffer allocation failed!");
        while (true) {}
    }
    
    disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    freeHeap = ESP.getFreeHeap();
    Serial.printf("Free heap after LVGL: %u KB\n", freeHeap / 1024);
    
    // Allocate ONLY 2 buffers - shared reusable processing buffer and grayscale buffer
    uint32_t pixelCount = SCREEN_WIDTH * SCREEN_HEIGHT;
    
    processingBuffer = (uint16_t*)malloc(pixelCount * 2);
    grayscaleBuffer = (uint8_t*)malloc(pixelCount);
    
    if (!processingBuffer || !grayscaleBuffer) {
        Serial.println("Failed to allocate buffers!");
        Serial.printf("Processing buffer: %s\n", processingBuffer ? "OK" : "FAILED");
        Serial.printf("Grayscale buffer: %s\n", grayscaleBuffer ? "OK" : "FAILED");
        while (true) {}
    }
    
    freeHeap = ESP.getFreeHeap();
    Serial.printf("Free heap after buffer allocation: %u KB\n", freeHeap / 1024);
    Serial.println("Setup complete! Starting display loop...");
    
    lastSwitchTime = millis();
    displaySplashScreen("Original Image");
    currentState = 0;
}

// Main loop
void loop() {
    uint32_t currentTime = millis();
    
    switch(currentState) {
        case 0: // Splash for original image
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                displayImageOnScreen(&img2, "Original Image");
                currentState = 1;
                lastSwitchTime = currentTime;
                Serial.println("Displaying original image");
            }
            break;
            
        case 1: // Showing original image
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Grayscale Image");
                currentState = 2;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 2: // Splash for grayscale
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                convertToGrayscale(&img2, grayscaleBuffer);
                lv_img_dsc_t grayImg = createGrayscaleImage(grayscaleBuffer, 
                                                            img2.header.w, 
                                                            img2.header.h);
                displayImageOnScreen(&grayImg, "Grayscale Image");
                currentState = 3;
                lastSwitchTime = currentTime;
                Serial.println("Displaying grayscale image");
            }
            break;
            
        case 3: // Showing grayscale image
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Sepia Tone");
                currentState = 4;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 4: // Splash for sepia
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                applySepiaEffect(&img2, processingBuffer);
                lv_img_dsc_t sepiaImg = createRGB565Image(processingBuffer, 
                                                          img2.header.w, 
                                                          img2.header.h);
                displayImageOnScreen(&sepiaImg, "Sepia Tone");
                currentState = 5;
                lastSwitchTime = currentTime;
                Serial.println("Displaying sepia image");
            }
            break;
            
        case 5: // Showing sepia image
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Histogram Equalized");
                currentState = 6;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 6: // Splash for equalized
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                histogramEqualizationRGB(&img2, processingBuffer);
                lv_img_dsc_t eqImg = createRGB565Image(processingBuffer, 
                                                        img2.header.w, 
                                                        img2.header.h);
                displayImageOnScreen(&eqImg, "Histogram Equalized");
                currentState = 7;
                lastSwitchTime = currentTime;
                Serial.println("Displaying equalized image");
            }
            break;
            
        case 7: // Showing equalized image
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Low Poly Effect");
                currentState = 8;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 8: // Splash for low poly
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                applyLowPolyEffect(&img2, processingBuffer);
                lv_img_dsc_t lowPolyImg = createRGB565Image(processingBuffer, 
                                                            img2.header.w, 
                                                            img2.header.h);
                displayImageOnScreen(&lowPolyImg, "Low Poly Effect");
                currentState = 9;
                lastSwitchTime = currentTime;
                Serial.println("Displaying low poly effect");
            }
            break;
            
        case 9: // Showing low poly
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Underwater Effect");
                currentState = 10;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 10: // Splash for underwater
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                applyUnderwaterEffect(&img2, processingBuffer);
                lv_img_dsc_t underwaterImg = createRGB565Image(processingBuffer, 
                                                               img2.header.w, 
                                                               img2.header.h);
                displayImageOnScreen(&underwaterImg, "Underwater Effect");
                currentState = 11;
                lastSwitchTime = currentTime;
                Serial.println("Displaying underwater effect");
            }
            break;
            
        case 11: // Showing underwater
            if (currentTime - lastSwitchTime >= 10000) {//10s due to animation
                displaySplashScreen("Halftone Effect");
                currentState = 12;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 12: // Splash for halftone
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                applyHalftoneEffect(&img2, processingBuffer);
                lv_img_dsc_t halftoneImg = createRGB565Image(processingBuffer, 
                                                             img2.header.w, 
                                                             img2.header.h);
                displayImageOnScreen(&halftoneImg, "Halftone Effect");
                currentState = 13;
                lastSwitchTime = currentTime;
                Serial.println("Displaying halftone effect");
            }
            break;
            
        case 13: // Showing halftone
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Calm Mood");
                currentState = 14;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 14: // Splash for calm mood
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                applyThermalMood(&img2, processingBuffer, 0); // Calm mode
                lv_img_dsc_t calmImg = createRGB565Image(processingBuffer, 
                                                         img2.header.w, 
                                                         img2.header.h);
                displayImageOnScreen(&calmImg, "Calm Mood");
                currentState = 15;
                lastSwitchTime = currentTime;
                Serial.println("Displaying calm mood");
            }
            break;
            
        case 15: // Showing calm mood
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Warm Mood");
                currentState = 16;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 16: // Splash for warm mood
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                applyThermalMood(&img2, processingBuffer, 1); // Warm mode
                lv_img_dsc_t warmImg = createRGB565Image(processingBuffer, 
                                                         img2.header.w, 
                                                         img2.header.h);
                displayImageOnScreen(&warmImg, "Warm Mood");
                currentState = 17;
                lastSwitchTime = currentTime;
                Serial.println("Displaying warm mood");
            }
            break;
            
        case 17: // Showing warm mood
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Energetic Mood");
                currentState = 18;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 18: // Splash for energetic mood
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                applyThermalMood(&img2, processingBuffer, 2); // Energetic mode
                lv_img_dsc_t energeticImg = createRGB565Image(processingBuffer, 
                                                              img2.header.w, 
                                                              img2.header.h);
                displayImageOnScreen(&energeticImg, "Energetic Mood (Neon)");
                currentState = 19;
                lastSwitchTime = currentTime;
                Serial.println("Displaying energetic mood");
            }
            break;
            
        case 19: // Showing energetic mood
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                displaySplashScreen("Rainy Mood");
                currentState = 20;
                lastSwitchTime = currentTime;
            }
            break;
            
        case 20: // Splash for rainy mood
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                applyThermalMood(&img2, processingBuffer, 3); // Rainy mode
                lv_img_dsc_t rainyImg = createRGB565Image(processingBuffer, 
                                                          img2.header.w, 
                                                          img2.header.h);
                displayImageOnScreen(&rainyImg, "Rainy Mood");
                currentState = 21;
                lastSwitchTime = currentTime;
                Serial.println("Displaying rainy mood");
            }
            break;
            
        case 21: // Showing rainy mood
            if (currentTime - lastSwitchTime >= DISPLAY_TIME) {
                // Loop back to beginning
                displaySplashScreen("Original Image");
                currentState = 0;
                lastSwitchTime = currentTime;
                Serial.println("Looping back to start");
            }
            break;
    }
    
    lv_timer_handler();
    delay(5);
}
