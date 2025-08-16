# ESP32 Retro Gaming Console

A retro-style gaming console built with ESP32 featuring dual joysticks, OLED display, NeoPixel LEDs, and buzzer for classic arcade game experiences.

## Features

- 128x64 OLED display for game graphics and menus
- Dual analog joysticks for 2-player games
- 16 NeoPixel RGB LED strip for visual effects
- Buzzer for sound effects and music
- Pause/Resume and Reset buttons
- High score storage in EEPROM
- Game menu with 4 classic games: Pong, Snake, Breakout, Tetris
- Splash screen with rainbow animation
- Compatible with multiple ESP32 Arduino core versions

## Hardware Requirements

### Components
- ESP32 development board
- SSD1306 OLED display (128x64, I2C)
- 2x Analog joysticks with push buttons
- WS2812B NeoPixel LED strip (16 LEDs)
- Passive buzzer
- 2x Push buttons
- Breadboard and jumper wires
- 10kΩ resistors (for pull-ups if needed)

### Pin Connections

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| OLED SDA | GPIO 21 | I2C Data |
| OLED SCL | GPIO 22 | I2C Clock |
| NeoPixels | GPIO 27 | Data line |
| Buzzer | GPIO 14 | PWM output |
| Joy1 X-axis | GPIO 34 | ADC1 |
| Joy1 Y-axis | GPIO 35 | ADC1 |
| Joy1 Button | GPIO 32 | INPUT_PULLUP |
| Joy2 X-axis | GPIO 33 | ADC1 |
| Joy2 Y-axis | GPIO 25 | ADC2 |
| Joy2 Button | GPIO 26 | INPUT_PULLUP |
| Pause Button | GPIO 13 | INPUT_PULLUP |
| Reset Button | GPIO 4 | INPUT_PULLUP |

## Software Requirements

### Arduino IDE Setup
1. Install ESP32 board package
2. Install required libraries:
   - Adafruit GFX Library
   - Adafruit SSD1306
   - Adafruit NeoPixel

### Library Installation
```
Tools → Manage Libraries → Search and install:
- Adafruit GFX Library by Adafruit
- Adafruit SSD1306 by Adafruit  
- Adafruit NeoPixel by Adafruit
```

## Installation

1. Clone or download this repository
2. Open `retro_gaming_console.ino` in Arduino IDE
3. Select your ESP32 board and port
4. Upload the code to your ESP32

## Usage

### Controls
- **Joystick 1 (Player 1)**: Menu navigation and game control
- **Joystick 2 (Player 2)**: Second player control (game dependent)
- **Pause Button (GPIO 13)**: Pause/resume current game
- **Reset Button (GPIO 4)**: Reset score and game state

### Navigation
1. **Splash Screen**: Press Joy1 button to continue
2. **Game Menu**: Use Joy1 Y-axis to navigate, Joy1 button to select
3. **In Game**: Use joysticks for gameplay, pause/reset buttons for control

### Game Features
- Real-time score display
- High score tracking (saved to EEPROM)
- Visual feedback via NeoPixel animations
- Audio feedback via buzzer
- Pause/resume functionality

## Code Structure

```
├── Hardware Setup
│   ├── OLED Display (SSD1306)
│   ├── NeoPixel LED Strip
│   ├── Dual Joysticks
│   └── Buzzer (PWM)
├── Game States
│   ├── Splash Screen
│   ├── Menu System
│   └── Game Playing
├── Input Handling
│   ├── Joystick Reading
│   ├── Button Debouncing
│   └── Navigation Logic
└── Game Framework
    ├── Score Management
    ├── Sound Effects
    └── Visual Effects
```

## Customization

### Adding New Games
1. Add game name to `games[]` array
2. Update `GAME_COUNT` constant
3. Implement game logic in `gameLoopStep()` function
4. Add game-specific input handling

### Modifying Visual Effects
- Edit `rainbowAnimation()` for startup effects
- Modify `pixelsFill()` calls for different color schemes
- Adjust LED brightness in `setup()` with `pixels.setBrightness()`

### Sound Customization
- Modify frequencies and durations in `toneBeep()` calls
- Add new sound effects by calling `toneBeep(frequency, duration)`

## Troubleshooting

### Common Issues

**Compilation Errors with LEDC functions:**
- The code includes compatibility for multiple ESP32 core versions
- If errors persist, update ESP32 board package to latest version

**OLED Not Working:**
- Check I2C connections (SDA/SCL)
- Verify OLED address (usually 0x3C)
- Ensure adequate power supply

**Joystick Issues:**
- Check ADC pin assignments
- Calibrate center values (around 2048 for 12-bit ADC)
- Avoid using ADC2 pins when WiFi is active

**NeoPixel Not Lighting:**
- Verify data pin connection
- Check power requirements (5V for WS2812B)
- Ensure proper ground connections

## Technical Specifications

- **Frame Rate**: ~30 FPS (33ms frame time)
- **Display**: 128x64 monochrome OLED
- **LEDs**: 16 individually addressable RGB
- **Audio**: PWM-generated tones via buzzer
- **Storage**: EEPROM for persistent high scores
- **Input**: Analog joysticks + digital buttons

## Future Enhancements

- [ ] Implement actual game logic for Pong, Snake, Breakout, Tetris
- [ ] Add multiplayer support
- [ ] Implement game saves
- [ ] Add more sound effects and music
- [ ] Create custom graphics and sprites
- [ ] Add WiFi connectivity for online features
- [ ] Implement game difficulty levels

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---

**Happy Gaming! 🎮**
