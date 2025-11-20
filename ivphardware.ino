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
#define DISPLAY_TIME 5000  // 5 seconds per image
#define SPLASH_TIME 3000   // 3 seconds for splash screen

// LVGL Display
lv_display_t *disp;

// Image display objects
static lv_obj_t *imgDisplay;
static lv_obj_t *statusLabel;

// Image processing buffers
static uint8_t *grayscaleBuffer = NULL;
static uint16_t *rgb565Buffer = NULL;
static uint16_t *equalizedRGB565Buffer = NULL;

// Current state
static uint8_t currentState = 0; // 0=splash, 1=original, 2=grayscale, 3=equalized
static uint32_t lastSwitchTime = 0;

// Forward declarations
void convertToGrayscale(const lv_img_dsc_t* imgDsc, uint8_t* grayData);
void histogramEqualizationRGB(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565);
void displayImageOnScreen(const lv_img_dsc_t *imgDsc, const char *title);
void displaySplashScreen(const char *nextImage);
lv_img_dsc_t createGrayscaleImage(const uint8_t* grayData, uint16_t width, uint16_t height);
lv_img_dsc_t createRGB565Image(const uint16_t* rgbData, uint16_t width, uint16_t height);

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
        rgb565Buffer[i] = (r5 << 11) | (g6 << 5) | b5;
    }
    
    lv_img_dsc_t img_dsc;
    img_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
    img_dsc.header.w = width;
    img_dsc.header.h = height;
    img_dsc.data = (const uint8_t*)rgb565Buffer;
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
    
    Serial.println("ESP32-C6 Image Display with Splash");
    
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
    
    // Allocate processing buffers
    uint32_t pixelCount = SCREEN_WIDTH * SCREEN_HEIGHT;
    grayscaleBuffer = (uint8_t*)malloc(pixelCount);
    rgb565Buffer = (uint16_t*)malloc(pixelCount * 2);
    equalizedRGB565Buffer = (uint16_t*)malloc(pixelCount * 2);
    
    if (!grayscaleBuffer || !rgb565Buffer || !equalizedRGB565Buffer) {
        Serial.println("Failed to allocate processing buffers!");
        while (true) {}
    }
    
    Serial.println("Setup complete! Starting display loop...");
    lastSwitchTime = millis();
    
    // Show initial splash screen
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
                Serial.println("Splash: Grayscale");
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
                displaySplashScreen("Equalized Image");
                currentState = 4;
                lastSwitchTime = currentTime;
                Serial.println("Splash: Equalized");
            }
            break;
            
        case 4: // Splash for equalized
            if (currentTime - lastSwitchTime >= SPLASH_TIME) {
                histogramEqualizationRGB(&img2, equalizedRGB565Buffer);
                lv_img_dsc_t eqImg = createRGB565Image(equalizedRGB565Buffer, 
                                                        img2.header.w, 
                                                        img2.header.h);
                displayImageOnScreen(&eqImg, "Histogram Equalized");
                currentState = 5;
                lastSwitchTime = currentTime;
                Serial.println("Displaying equalized image");
            }
            break;
            
        case 5: // Showing equalized image
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
