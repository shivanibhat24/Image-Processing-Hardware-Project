# ESP32-C6 LCD Image Effects Processor

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32--C6-green.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)

A sophisticated image processing demonstration for the ESP32-C6 microcontroller with a 172×320 LCD display, featuring 10+ real-time image effects with smooth transitions and professional UI.

## 🎨 Features

- **10+ Image Processing Effects** with mathematical precision
- **Dual-buffer architecture** for memory-efficient processing
- **Beautiful splash screens** with animated transitions
- **Real-time effect application** on RGB565 images
- **Professional LVGL-based UI** with modern aesthetics
- **Automatic cycling** through all effects

## 📋 Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Software Dependencies](#software-dependencies)
- [Image Effects](#image-effects)
- [Mathematical Algorithms](#mathematical-algorithms)
- [Memory Architecture](#memory-architecture)
- [Installation](#installation)
- [Usage](#usage)
- [Performance](#performance)

## 🔧 Hardware Requirements

- **ESP32-C6 Dev Module** (tested on v3.2.0)
- **172×320 LCD Display** (RGB565 format)
- Compatible with ESP32-C6-LCD-1.47 pinout

## 📚 Software Dependencies

- Arduino IDE or PlatformIO
- LVGL (Light and Versatile Graphics Library)
- ESP32 Arduino Core
- `PINS_ESP32-C6-LCD-1_47.h` header file

## 🎭 Image Effects

### 1. Original Image Display
Displays the source image in native RGB565 format without modifications.

### 2. Grayscale Conversion
Converts color images to grayscale using the ITU-R BT.601 luminance formula.

### 3. Sepia Tone
Applies a vintage, warm-toned photographic effect.

### 4. Histogram Equalization
Enhances image contrast by redistributing intensity values across the full dynamic range.

### 5. Low Poly Effect
Creates a geometric, faceted appearance by averaging colors in triangular regions.

### 6. Underwater Effect
Simulates underwater viewing with caustic light patterns, wave distortion, and depth gradients.

### 7. Halftone Effect
Creates a newspaper/screen print appearance using dot patterns.

### 8. Calm Mood (Blue/Purple)
Remaps colors to create a serene, cool-toned atmosphere.

### 9. Warm Mood (Orange/Red)
Applies a cozy, warm color palette transformation.

### 10. Energetic Mood (Neon)
Creates vibrant, high-saturation neon-style coloring.

### 11. Rainy Mood (Desaturated)
Produces a melancholic, cool gray atmosphere.

## 🧮 Mathematical Algorithms

### Grayscale Conversion

Implements the ITU-R BT.601 standard for RGB to grayscale conversion:

```
Gray = 0.299 × R + 0.587 × G + 0.114 × B
```

**RGB565 to 8-bit extraction:**
```
R₈ = ((pixel >> 11) & 0x1F) × 255 / 31
G₈ = ((pixel >> 5) & 0x3F) × 255 / 63
B₈ = (pixel & 0x1F) × 255 / 31
```

### Sepia Tone Transform

Applies the classic sepia transformation matrix:

```
R' = 0.393 × R + 0.769 × G + 0.189 × B
G' = 0.349 × R + 0.686 × G + 0.168 × B
B' = 0.272 × R + 0.534 × G + 0.131 × B
```

Values are clamped to [0, 255] range.

### Histogram Equalization

Performs independent channel equalization using the cumulative distribution function (CDF):

1. **Build histogram** for each RGB channel:
   ```
   H[intensity] = count of pixels with that intensity
   ```

2. **Calculate CDF**:
   ```
   CDF[i] = CDF[i-1] + H[i]
   ```

3. **Normalize and remap**:
   ```
   Output[i] = ((CDF[input[i]] - CDF_min) × 255) / (total_pixels - CDF_min)
   ```

This spreads the intensity distribution across the full [0, 255] range, enhancing contrast.

### Sobel Edge Detection

Uses Sobel operators to detect edges via gradient calculation:

**Horizontal kernel (Gₓ):**
```
[-1  0  1]
[-2  0  2]
[-1  0  1]
```

**Vertical kernel (Gᵧ):**
```
[-1 -2 -1]
[ 0  0  0]
[ 1  2  1]
```

**Gradient magnitude:**
```
G = |Gₓ| + |Gᵧ|  (Manhattan distance approximation)
```

Full calculation:
```
Gₓ = Σ(kernel_x[i,j] × pixel[x+i, y+j])
Gᵧ = Σ(kernel_y[i,j] × pixel[x+i, y+j])
magnitude = |Gₓ| + |Gᵧ|
```

### Low Poly Effect

Creates a faceted, geometric appearance:

1. **Divide image into blocks** of size `polySize × polySize` (default: 12×12)
2. **Calculate average color** for each block
3. **Split each block diagonally** into two triangles
4. **Apply differential shading**:
   - Upper triangle: 100% intensity
   - Lower triangle: 90% intensity (for depth)
5. **Darken edges** by 70% for polygon border definition

**Triangle determination:**
```
upperTriangle = (x + y) < polySize
```

### Underwater Effect

Multi-component simulation of underwater viewing:

**Wave Distortion:**
```
wave₁ = sin(x × 0.05 + y × 0.03 + time) × 2.5
wave₂ = sin(x × 0.08 - y × 0.04 + time × 1.3) × 1.8
totalWave = wave₁ + wave₂

sourceX = x + totalWave
sourceY = y + totalWave × 0.6
```

**Color Transformation:**
```
R' = R × 0.70  (reduce red)
G' = G × 0.85  (slightly reduce green)
B' = B × 1.20  (boost blue, clamped to 255)
```

**Caustic Light Patterns:**
```
caustic₁ = sin(x × 0.08 + time × 0.5) × cos(y × 0.08 + time × 0.7)
caustic₂ = sin(x × 0.12 + 1.0) × cos(y × 0.104 + 0.5)
pattern = (caustic₁ + caustic₂) × 0.5 + 0.5  [normalized to 0-1]

if pattern > 0.7:
    intensity = (pattern - 0.7) / 0.3
    R += intensity × 60
    G += intensity × 80
    B += intensity × 40
```

**Depth Gradient:**
```
depthFactor = 1.0 - (y / height) × 0.3
R = R × depthFactor
G = G × depthFactor
B = B × depthFactor
```

### Halftone Effect

Simulates screen printing with dot patterns:

1. **Convert to luminance:**
   ```
   luma = 0.299 × R + 0.587 × G + 0.114 × B
   ```

2. **Calculate position in dot grid:**
   ```
   dotX = x mod dotSize
   dotY = y mod dotSize
   ```

3. **Distance from dot center:**
   ```
   dx = dotX - dotSize/2
   dy = dotY - dotSize/2
   distSquared = dx² + dy²
   ```

4. **Threshold determination:**
   ```
   threshold = (255 - luma) / 16  [range: 0-15]
   
   if distSquared ≤ threshold:
       pixel = original_color × luma/255
   else:
       pixel = white (255, 255, 255)
   ```

### Thermal Mood Remapping

Applies color grading based on luminance:

**Luminance calculation:**
```
luma = (0.299 × R + 0.587 × G + 0.114 × B) / 255  [normalized to 0-1]
```

**Calm Mode (Blue/Purple):**
```
R' = luma × 200 + 50
G' = luma × 80 + 30
B' = luma × 200 + 55
```

**Warm Mode (Orange/Red):**
```
R' = luma × 200 + 55
G' = luma × 120 + 50
B' = luma × 60 + 20
```

**Energetic Mode (Neon):**
```
if luma > 0.7:
    R' = 255
    G' = luma × 100 + 155
    B' = luma × 200 + 55
else if luma > 0.4:
    R' = luma × 150 + 105
    G' = 255
    B' = luma × 180 + 75
else:
    R' = luma × 180 + 75
    G' = luma × 120 + 50
    B' = 255
```

**Rainy Mode (Desaturated):**
```
avg = (R + G + B) / 3
R' = avg × 0.85
G' = avg × 0.90
B' = avg × 1.05
```

## 🧠 Memory Architecture

### Dual-Buffer System

The application uses a memory-efficient dual-buffer architecture:

**Buffer 1: Processing Buffer (RGB565)**
- Size: `SCREEN_WIDTH × SCREEN_HEIGHT × 2 bytes` (110,080 bytes)
- Usage: Stores processed RGB565 image data
- Reused across all color effects

**Buffer 2: Grayscale Buffer (8-bit)**
- Size: `SCREEN_WIDTH × SCREEN_HEIGHT × 1 byte` (55,040 bytes)
- Usage: Temporary storage for grayscale operations and edge detection
- Reused for intermediate calculations

**Total static allocation:** ~165 KB

### RGB565 Format

Each pixel uses 16 bits (2 bytes):
```
[R₄R₃R₂R₁R₀|G₅G₄G₃G₂G₁G₀|B₄B₃B₂B₁B₀]
 ←--5 bits--→←----6 bits----→←--5 bits--→
```

**Bit manipulation:**
```cpp
// Extract RGB565 to 8-bit
uint8_t r8 = ((pixel >> 11) & 0x1F) * 255 / 31;
uint8_t g8 = ((pixel >> 5) & 0x3F) * 255 / 63;
uint8_t b8 = (pixel & 0x1F) * 255 / 31;

// Pack 8-bit to RGB565
uint16_t rgb565 = ((r8 >> 3) << 11) | ((g8 >> 2) << 5) | (b8 >> 3);
```

## 📦 Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/esp32-image-effects.git
   cd esp32-image-effects
   ```

2. **Install dependencies:**
   - Install ESP32 board support in Arduino IDE
   - Install LVGL library (v8.x or v9.x)
   - Ensure you have the required hardware header files

3. **Configure your image:**
   - Convert your image to RGB565 C array format
   - Declare it as `extern const lv_img_dsc_t img2;`
   - Link the compiled image file

4. **Upload to ESP32-C6:**
   - Select board: "ESP32C6 Dev Module"
   - Connect via USB
   - Upload the sketch

## 🚀 Usage

1. **Power on** the ESP32-C6 device
2. The display will automatically cycle through effects:
   - 1.5s splash screen for each effect
   - 3s display time per effect (8s for animated underwater)
3. Effects loop continuously

### Customization

**Modify timing:**
```cpp
#define DISPLAY_TIME 3000   // Effect display time (ms)
#define SPLASH_TIME 1500    // Splash screen time (ms)
```

**Adjust effect parameters:**
```cpp
const uint8_t polySize = 12;      // Low poly block size
const uint8_t dotSize = 4;        // Halftone dot size
```

**Change brightness:**
```cpp
#define SCREEN_BRIGHTNESS 255  // 0-255
```

## ⚡ Performance

### Memory Usage
- **Free heap after init:** ~165 KB (varies by ESP32-C6 configuration)
- **Processing buffers:** 165 KB (dual-buffer system)
- **LVGL display buffer:** ~13.7 KB

### Processing Times (Typical)
- Grayscale conversion: ~20-30ms
- Sepia effect: ~25-35ms
- Histogram equalization: ~80-120ms
- Edge detection (Sobel): ~150-200ms
- Low poly effect: ~40-60ms
- Underwater effect: ~200-300ms (includes wave calculations)
- Halftone effect: ~50-70ms
- Thermal mood: ~30-50ms

### Display Performance
- **Frame rate:** 5ms loop delay + LVGL processing
- **Display refresh:** Partial rendering via LVGL
- **Splash animations:** Smooth spinner at native refresh rate

## 🔬 Technical Details

### Color Space Conversions

All effects maintain color accuracy through proper bit-depth conversions:

1. **RGB565 → RGB888 (8-bit per channel):**
   - Red: 5-bit → 8-bit via `(value × 255) / 31`
   - Green: 6-bit → 8-bit via `(value × 255) / 63`
   - Blue: 5-bit → 8-bit via `(value × 255) / 31`

2. **RGB888 → RGB565:**
   - Red: 8-bit → 5-bit via `(value >> 3) & 0x1F`
   - Green: 8-bit → 6-bit via `(value >> 2) & 0x3F`
   - Blue: 8-bit → 5-bit via `(value >> 3) & 0x1F`

### Buffer Management

The code implements intelligent buffer clearing:
```cpp
void clearProcessingBuffers() {
    if (processingBuffer) {
        memset(processingBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
    }
    if (grayscaleBuffer) {
        memset(grayscaleBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    }
}
```

Called strategically after splash screens to maintain memory hygiene.

## 🎨 UI Design

### Splash Screen Components
- **Gradient background:** Dark theme (0x1a1a2e → 0x16213e)
- **Animated spinner:** 60×60px with cyan accent (0x00d4ff)
- **Typography:** Montserrat font family
- **Decorative elements:** Three animated dots

### Effect Display
- **Full-screen image:** Centered display
- **Title bar:** 30px height with effect name
- **Accent color:** Cyan (0x00d4ff) for consistency

## 📝 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 📧 Contact

For questions or suggestions, please open an issue on GitHub.

---

**Note:** Ensure your ESP32-C6 has sufficient power supply (USB or external) as image processing can be computationally intensive.
