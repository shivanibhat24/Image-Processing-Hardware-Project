### Micro Image Processing Suite

<div align="center">

![Made with Love](https://img.shields.io/badge/Made%20with-💖-ff69b4?style=for-the-badge)
![ESP32-C6](https://img.shields.io/badge/ESP32--C6-💫-blueviolet?style=for-the-badge)
![LVGL](https://img.shields.io/badge/LVGL-🎭-success?style=for-the-badge)
![Image Magic](https://img.shields.io/badge/Image%20Magic-🪄-orange?style=for-the-badge)

**Transform your tiny display into a pocket-sized art gallery!**

[✨ Features](#-features) • [🚀 Quick Start](#-quick-start) • [🎪 Effects Gallery](#-effects-gallery) • [💝 Why You'll Love It](#-why-youll-love-it)

---

</div>

## 🌟 What is This Magical Thing?

Welcome to the **most adorable** image processing showcase for ESP32-C6! This isn't just code—it's a *carnival of creativity* packed into a tiny microcontroller! 🎪✨

Your 172x320 display becomes a canvas where images dance through **11 stunning effects** with splash screens so pretty you'll want to frame them! 🖼️

---

## ✨ Features

<table>
<tr>
<td width="50%">

### 🎨 **Visual Magic**
- 🌈 **11 Unique Effects** that'll make you go "WOW!"
- 🎭 Beautiful animated splash screens
- 🌊 Real-time animated underwater caustics
- 💫 Smooth transitions between effects
- 🎪 Auto-cycling showcase mode

</td>
<td width="50%">

### 🧠 **Smart Tech**
- 🚀 Memory-optimized buffer reuse
- ⚡ ESP32-C6 powered performance
- 🎯 LVGL-based smooth rendering
- 🔄 Efficient RGB565 processing
- 💾 Only 2 buffers for all effects!

</td>
</tr>
</table>

---

## 🎪 Effects Gallery

### 🖼️ The Classics
| Effect | Vibe | What It Does |
|--------|------|--------------|
| 🎨 **Original** | *Pure Beauty* | Your image in all its glory! |
| ⚫ **Grayscale** | *Timeless Elegance* | Classic black & white magic |
| 📜 **Sepia Tone** | *Vintage Vibes* | Like a photograph from 1920! |
| 📊 **Histogram Equalized** | *Maximum Drama* | Cranks up that contrast to 11! |

### 🎭 The Show-Stoppers
| Effect | Vibe | What It Does |
|--------|------|--------------|
| 🔷 **Low Poly** | *Geometric Glam* | Triangulated art style! |
| 🌊 **Underwater** | *Deep Sea Dreams* | Animated caustics & shimmer! *(10 sec show!)* |
| 🔘 **Halftone** | *Pop Art Power* | Newspaper dots meet modern style |

### 💖 The Mood Ring Collection
| Mood | Vibe | Color Palette |
|------|------|---------------|
| 😌 **Calm** | *Zen Vibes* | Soothing blues & purples 💙💜 |
| 🔥 **Warm** | *Cozy Feels* | Sunset oranges & reds 🧡❤️ |
| ⚡ **Energetic** | *Party Mode!* | Electric neon explosion! 💚💛💙 |
| 🌧️ **Rainy** | *Moody Blues* | Desaturated cool grays 🩶 |

---
### Input Image

![img2](https://github.com/user-attachments/assets/14163fa8-3973-4043-8d06-2699b467a322)

## 🚀 Quick Start

### 📦 What You Need
```
🔧 ESP32-C6 Dev Module
📺 172x320 LCD Display (ESP32-C6-LCD-1.47)
💻 Arduino IDE v3.2.0+
📚 LVGL Library
```

### ⚡ Installation Lightning Round

1. **Clone this cutie!** 🐙
   ```bash
   git clone https://github.com/yourusername/esp32-image-effects-wonderland.git
   ```

2. **Install LVGL** 📚
   ```bash
   # In Arduino IDE: Library Manager → Search "lvgl" → Install
   ```

3. **Select Your Board** 🎯
   ```
   Tools → Board → ESP32 Arduino → ESP32C6 Dev Module
   ```

4. **Upload & Watch The Magic!** ✨
   ```
   Hit that upload button and prepare to be amazed!
   ```

---

## 🎮 How It Works

```
🌟 Splash Screen (1.5s)
    ↓
🖼️ Show Effect (3s)
    ↓
🌟 Next Splash (1.5s)
    ↓
🔄 Repeat Forever!
```

**Special Guest Star:** Underwater effect gets a *10-second encore* because it's just THAT good! 🌊

---

## 💝 Why You'll Love It

### 🎯 **Perfect For:**
- 📸 Digital photo frames with PERSONALITY
- 🎨 Art installations that WOW
- 🎓 Learning image processing (but make it *fun!*)
- 💡 Portfolio projects that SHINE
- 🎪 Tech demos that turn heads

### 🌈 **Special Sauce:**
- ✅ Memory-efficient (reuses buffers like a BOSS)
- ✅ Beautiful UI/UX (those splash screens though! 😍)
- ✅ Production-ready code (no sketchy hacks here!)
- ✅ Easy to customize (add your own effects!)
- ✅ Well-documented (you're reading it! 📖)

---

## 🛠️ Customize Your Magic

### 🎨 Add Your Own Effect

```cpp
void applyYourCrazyEffect(const lv_img_dsc_t* imgDsc, uint16_t* outputRGB565) {
    Serial.println("Unleashing creativity...");
    // Your magical code here! ✨
    // Process those pixels like a wizard! 🧙‍♂️
}
```

### ⏱️ Tweak The Timing
```cpp
#define DISPLAY_TIME 3000   // Make it longer! ⏰
#define SPLASH_TIME 1500    // Or shorter! ⚡
```

### 🎨 Change The Colors
```cpp
// In displaySplashScreen():
lv_obj_set_style_bg_color(bg, lv_color_hex(0xYOURCOLOR), 0);
```

---

## 🤓 Technical Deep Dive

### Memory Management Magic 🧙‍♂️
```
📊 Processing Buffer: 172×320×2 = 110KB (RGB565)
⚫ Grayscale Buffer: 172×320×1 = 55KB  (8-bit)
💾 Total: ~165KB for ALL effects!
```

**How?** We reuse buffers like a sustainable coding wizard! ♻️

### Effect Processing Pipeline
```
Original Image (RGB565)
    ↓
[Processing Buffer] ← All transformations happen here!
    ↓
Display (LVGL)
```

---

## 🌈 Color Formats Explained

```cpp
RGB565 Breakdown:
RRRRR GGGGGG BBBBB
  5     6      5   = 16 bits per pixel

Why RGB565? 
✅ Perfect for embedded displays
✅ Good color range (65,536 colors!)
✅ Memory efficient
✅ Fast processing
```

---

## 📸 Effect Showcase

```
┌─────────────────────────────────────────┐
│                                         │
│  Original → Grayscale → Sepia Tone     │
│     ↓           ↓           ↓           │
│  [Your Image Cycling Through Effects]  │
│     ↓           ↓           ↓           │
│  Low Poly → Underwater → Halftone      │
│                                         │
│  Then: Calm → Warm → Energy → Rainy   │
│                                         │
│         🔄 Loop Forever! 🔄            │
└─────────────────────────────────────────┘
```

---

## 🎓 Learn While You Play

Each effect teaches you something cool:

- 🎨 **Grayscale**: Color space conversion (ITU-R BT.601)
- 📊 **Histogram Eq**: Statistical image enhancement
- 🔷 **Low Poly**: Block averaging & geometric art
- 🌊 **Underwater**: Multi-layered sine wave animation
- 🔘 **Halftone**: Pattern-based dithering
- 💭 **Moods**: Color theory & emotional design

---

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| 💥 Heap allocation failed | Check your board selection! |
| 🖼️ No image showing | Verify `img2` external declaration |
| 🌈 Weird colors | Check RGB565 conversion functions |
| ⚡ Slow performance | Normal! Effects are compute-heavy |
| 📺 Display not responding | Check pin definitions in PINS header |

---

## 🎁 Bonus Features

- 🎪 **Splash Screens**: Animated spinner, gradient backgrounds, neon accents!
- ⚡ **Smart Memory**: Clears buffers between effects
- 🎨 **Serial Debug**: Track every effect in real-time
- 🔄 **Infinite Loop**: Never stops being awesome!

---

## 📚 Code Stats

```
📊 Lines of Code: ~850
🎨 Effects: 11
💾 Buffer Count: 2
⏱️ Full Cycle Time: ~45 seconds
🎭 Splash Screens: 11
💖 Cuteness Level: MAXIMUM
```

---

## 🌟 Future Dreams

- [ ] 🎮 Button controls for manual switching
- [ ] 📱 Bluetooth effect selection
- [ ] 🎨 Custom effect parameters via Serial
- [ ] 🖼️ Multiple image support
- [ ] 💾 Save favorite effects to flash
- [ ] 🎵 Sound-reactive effects (imagine!)

---

## 🙏 Credits & Love

Built with 💖 by passionate makers for passionate makers!

### 🛠️ Powered By:
- **ESP32-C6**: The tiny powerhouse
- **LVGL**: Graphics magic maker
- **Arduino**: Making hardware accessible
- **You**: For appreciating good code! 🎉

---

## 📜 License

```
MIT License - Because sharing is caring! 💝

Feel free to:
✨ Use it
🔧 Modify it  
🎁 Share it
💖 Love it
```

---

<div align="center">

## 🎪 **Ready to Make Some Magic?** 🎪

### ⭐ Star this repo if it made you smile! ⭐

### 🎨 Fork it, customize it, make it YOURS! 🎨

---

**Made with 💖, ☕, and countless hours of pixel-perfect tweaking**

*Remember: Life's too short for boring displays!* ✨
