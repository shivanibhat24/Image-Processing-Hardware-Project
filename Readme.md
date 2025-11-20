
## Image Preparation

Convert any PNG/JPG to RGB565 C array using LVGL's online image converter:  
https://lvgl.io/tools/imageconverter

- Color format: **True color / RGB565**
- Output format: **C array**
- Name the variable `img2`

Save the generated `.c` file in the sketch folder and make sure the corresponding header is included.

## Behavior

| Phase               | Duration | Screen Content                     |
|---------------------|----------|------------------------------------|
| Splash (Original)   | 3 s      | Animated spinner + "Original Image"|
| Original Image      | 5 s      | Full-color picture                 |
| Splash (Grayscale)  | 3 s      | Spinner + "Grayscale Image"        |
| Grayscale Image     | 5 s      | Monochrome version                 |
| Splash (Equalized)  | 3 s      | Spinner + "Histogram Equalized"    |
| Equalized Image     | 5 s      | Enhanced contrast version          |
| → loops forever     |          |                                    |

## Memory Usage (approx.)

- LVGL draw buffer: ~27 KB (172×40×2 bytes)
- Grayscale buffer: 55 KB
- Equalized buffer: 55 KB
- RGB565 conversion buffer: 110 KB

Total ≈ 250 KB → fits comfortably in ESP32-C6 SPIRAM or internal RAM.

## Customization

- Change `DISPLAY_TIME` and `SPLASH_TIME` macros at the top of the sketch
- Adjust `SCREEN_BRIGHTNESS` (0-255)
- Replace `img2` with any other RGB565 image of the same or smaller resolution

## Input Image
![img2](https://github.com/user-attachments/assets/1d065644-b299-4840-b384-e90500c676bf)

## Output 
<img width="233" height="367" alt="image" src="https://github.com/user-attachments/assets/595a6a38-1b67-4361-855a-f008ec400ed0" />
<img width="241" height="341" alt="image" src="https://github.com/user-attachments/assets/371da70f-91b8-4251-81ad-f01ab80c1a4e" />
<img width="226" height="359" alt="image" src="https://github.com/user-attachments/assets/d3e91500-37e8-4059-913e-672b5bd93317" />




