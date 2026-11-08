# <span style="color: #003262;"> • Arduino Block Stacker Game

An interactive, physical arcade game built with an Arduino Uno, a 4-module LED matrix, an I2C OLED display, and a joystick module. Stack the moving blocks as high as you can without missing!

---

## <span style="color: #26619C;">• Features
* **LED Matrix Display:** Uses a 4-module MD_MAX72xx LED matrix configured in Parola hardware mode with optimized low brightness (`INTENSITY 0`) to run safely on a 9V battery.
* **OLED Score & Status Screen:** A 128x64 I2C OLED display tracking levels, game status, and final scores in real-time (with 180° rotation and dual-color zone alignment).
* **Joystick Control:** Precise block placement and game restarts via an integrated joystick push button (`SW`).

---

## <span style="color: #26619C;">• Hardware Requirements

| Component | Quantity | Notes |
| :--- | :---: | :--- |
| **Arduino Uno** | 1 | Microcontroller board |
| **MD_MAX72xx LED Matrix** | 4 Modules | Parola HW type (cascaded) |
| **I2C OLED Display (128x64)** | 1 | Address `0x3C` |
| **Joystick Module** | 1 | With integrated button (`SW`) |
| **9V Battery & Snap Connector** | 1 | Connected to `Vin` for portable play |
| **Jumper Wires** | As needed | Male-to-male / male-to-female |

---

## <span style="color: #26619C;">• Wiring Guide

### 1. Power Configuration (9V Battery)
* **Red Wire (+)** $\rightarrow$ Connected to Arduino **`Vin`**. *Never plug directly into 5V!*
* **Black Wire (-)** $\rightarrow$ Connected to Arduino **`GND`**.

### 2. LED Matrix Connections (SPI)
* **DATA (DIN)** $\rightarrow$ Pin 10
* **CLK** $\rightarrow$ Pin 13
* **CS** $\rightarrow$ Pin 12
* **VCC & GND** $\rightarrow$ 5V and GND

### 3. OLED Display Connections (I2C)
* **SDA** $\rightarrow$ Arduino **A4**
* **SCL** $\rightarrow$ Arduino **A5**
* **VCC & GND** $\rightarrow$ 5V and GND

### 4. Joystick Connections
* **SW (Button)** $\rightarrow$ Pin 2 (configured with internal pull-up)

---

## <span style="color: #26619C;">• Required Libraries
Make sure you have installed the following libraries via the Arduino IDE Library Manager:
* [`MD_MAX72xx`](https://github.com/MajicDesigns/MD_MAX72XX)
* [`Adafruit_GFX`](https://github.com/adafruit/Adafruit-GFX-Library)
* [`Adafruit_SSD1306`](https://github.com/adafruit/Adafruit-SSD1306)

---

## <span style="color: #26619C;">• How to Play
1. Power on the device.
2. The OLED displays the startup screen, followed by **"Level: 1"** and **"Stacking..."**.
3. Press the **Joystick Button** to place the moving block onto the tower.
4. If your block overlaps with the layer below, you advance to the next level and the block shrinks!
5. Missing completely triggers **GAME OVER** and shows your final score percentage. Press the button again to restart.