# LinuxMediaLibrary (LML)

A high-performance, enterprise-grade media playout, mixing, real-time graphics (CG), and broadcast automation framework developed specifically for modern Linux operating systems. 

**LinuxMediaLibrary (LML)** combines low-latency GPU-accelerated video rendering, robust multi-format hardware ingest/output, vector-based real-time graphics generation, and network streaming into a unified, high-performance C++ API.

---

## 🚀 Key Modules & Capabilities

* **High-Performance Playout Engine (`LFile`)**: Modular media playout engine using FFmpeg, supporting dynamic rate adjustment, precise millisecond seeks, multiple audio track switching, advanced FFmpeg audio/video filter chains, and embedded timecode decoding.
* **Live Ingestion (`LLive`)**: Native support for professional SDI/HDMI hardware capture cards via the **Blackmagic DeckLink SDK** and low-latency network streams via **NewTek NDI®**.
* **Real-Time Video Switcher & Mixer (`LSwitcher` & `LMixer`)**: Multi-channel broadcast switcher with Program/Preview bus support, seamless alpha transitions (Stingers), dynamic resizing, and layered video compositing.
* **Professional Character Generator (`LCharacter`)**: Real-time vector-graphics engine powered by **Cairo** and **ThorVG** for rendering crisp, high-resolution lower-thirds, crawls, tickers, dynamic text, and Lottie animations on playout layers.
* **Dynamic Lottie Animation Engine (`LAnimation`)**: Supports multi-layered dynamic JSON Lottie templates, allowing run-time data injection such as updating text keys, replacing image assets for dynamic branding, and adjusting Hex color properties.
* **Frame Grabber Sink (`LFrame`)**: Inherits from `LSink`. Connects directly to any source `LObject` to capture raw video/audio frames at runtime, performing automated YUV-to-RGBA color conversion and resampling audio samples into standardized Signed 16-bit (S16) PCM data.
* **Hardware Playout Output Engine (`LOutput`)**: Dynamic playout output engine interfacing with Blackmagic Design DeckLink cards to output video/audio over physical SDI/HDMI outputs, with automated format/resolution mapping and synchronous presentation.
* **Metadata Reader & Format Analyzer (`LReader`)**: A high-speed media inspector that uses FFmpeg to quickly extract media file container formats, video/audio codecs, total duration, bitrates, audio channels, aspect ratios, color spaces, chroma subsampling, and bit depths without starting playback.
* **Generator & Fallback Signal Source (`LSignal`)**: A real-time signal and color generator inheriting from `LObject`. Allows setting solid background colors (via Hex strings) or static standby images to serve as a color black or fallback background in switcher channels.
* **Modular Preview Controller (`LPreview`)**: A framework-agnostic video preview controller. Dynamically enables audio/video rendering and overlays on a native window handle (`void* parent`), supporting both Qt (`QWidget*`) and GTK (`GtkWidget*`) parent widgets at runtime.
* **Broadcast Recorder (`LRecorder`)**: Synchronous, high-quality, real-time encoding of audio/video outputs into industry-standard container formats (MP4, MKV, MOV, MXF).
* **Playout Streaming (`LStream`)**: Built-in broadcast streamer supporting RTMP, SRT, and NDI® stream generation.
* **GPU-Accelerated Video Previews**: Built-in, ultra-low-latency video and audio preview widgets supporting both **Qt (QOpenGLWidget)** and **GTK4 (GtkOverlay + GdkTexture)** with real-time VU meters and timecode overlays.

---

## 🛠 Technology Stack

* **Core Language**: Modern C++ (C++17 / C++20)
* **Media Processing**: FFmpeg (libavcodec, libavformat, libavfilter, libswscale, libswresample, libavutil)
* **Hardware Ingest & Playout**: Blackmagic DeckLink SDK (v12.x+)
* **Network Video**: NewTek NDI® SDK (v6.x)
* **Vector Graphics & Rendering**: ThorVG, Cairo Graphics
* **Audio Playback**: SDL2 (Simple DirectMedia Layer)
* **UI Integration**: Qt6 Framework (Core, Gui, Widgets, OpenGLWidgets) & GTK4 Framework (GIO, GObject, GTK4)

---

## 📦 System Dependencies

While the distributed library directory (`LinuxMediaLibrary-Git`) contains pre-packaged dynamic libraries for Qt6, FFmpeg, SDL2, NDI, and GTK4 to minimize client-side configuration, the target system **MUST** provide the following external system dependencies:

### ⚠️ Mandatorily Required External Dependencies (NOT Bundled in Package)

1. **Blackmagic Desktop Video Driver & Service**
   * **Purpose**: Required for SDI/HDMI ingest and playout using DeckLink hardware. The unbundled driver daemon (`Desktop Video` service) and the Linux kernel module (`blackmagic.ko`) must be installed on the client machine.
   * **Installation**: Download the Desktop Video installer for Linux from [Blackmagic Support](https://www.blackmagicdesign.com/support/).

2. **Display Server (X11 / Wayland)**
   * **Purpose**: Required to create graphical display contexts and preview windows. A display server must be active on the target machine (cannot run in pure headless server environments without a virtual framebuffer like Xvfb).

3. **Audio Server (PulseAudio / PipeWire / ALSA)**
   * **Purpose**: Required for SDL2 to play out physical audio signals to system output speakers or devices.

4. **GTK4 Shared System Assets (`libgtk-4-common`)**
   * **Purpose**: While the GTK4 binary `libgtk-4.so` is bundled, GTK4 dynamically queries system font configurations, standard system icon packs, and GSettings schemas (window manager styles) from `/usr/share/` and `/usr/lib/`.
   * **Installation**:
     ```bash
     # Ubuntu / Debian
     sudo apt update && sudo apt install libgtk-4-1
     ```

---

## 💻 Repository & Build Structure

* **`LinuxMediaLibrary/`**: Main build workspace, containing the compiled shared framework `libLinuxMediaLibrary.so`.
* **`LinuxMediaLibrary/header/LinuxMediaLibrary/`**: Public framework headers exposed for consumer integration.
* **`LinuxMediaLibrary/Example/`**: Standalone, modular example integrations showing the power of the LML framework:
  * **`PreviewGTK`**: Pure GTK4 GPU-accelerated video/audio player featuring elegant dark neon styling, VU meters, and overlay toggles.
  * **`LiveDevice`**: Qt6-based ingest monitor displaying low-latency signals queried dynamically from physical DeckLink capture cards.
  * **`Switcher`**: Full multi-input broadcast switcher demo showcasing Program/Preview transitions, NDI streams, and CG overlay blending.
  * **`Playout` / `Output` / `Recorder` / `Mixer`**: Focused single-module showcase apps.

---

## 💡 Integration Best Practices

### 1. Qt6 Integration & Shared OpenGL Contexts
When integrating the GPU-accelerated **`LVideoWidget` (Qt6)** into multi-window Qt applications, you **MUST** enable shared OpenGL contexts before instantiating the `QApplication`. This ensures smooth hardware-accelerated playback across multiple video previews without context conflicts:
```cpp
int main(int argc, char *argv[]) {
    // Enable shared OpenGL contexts (Mandatory for LML Qt widgets)
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
```

### 2. GTK4 Performance
For **`LVideoWidgetGTK`**, ensure your Linux desktop environment has active OpenGL rendering enabled. You can force GPU acceleration in GTK4 application startups by setting the environment variable:
```bash
GSK_RENDERER=gl ./YourGTKApplication
```

---

## 🎯 Target Applications

* **Television Playout & Broadcast Automation Systems**
* **Live Video Production Switchers**
* **Digital Signage & Dynamic Ad Insertion Controllers**
* **Real-time Character Generators (CG) & Title Overlay Inserters**
* **Interactive IP/NDI Stream Converters & Multi-viewers**

---

## 📄 License

This software and its APIs are proprietary and developed for high-end broadcast automation. Unauthorized copying, distribution, or modifications are strictly prohibited.
