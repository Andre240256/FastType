# FastTyping ⌨️

A terminal-based typing trainer application written in C for Linux. Improve your typing speed and accuracy with real-time feedback and detailed statistics.

```
###########################################################
                                                        
  ▄▄▄▄▄▄▄                 ▄▄▄▄▄▄▄                          
 █▀██▀▀▀           █▄    █▀▀██▀▀▀▀                         
   ██             ▄██▄      ██               ▀▀ ▄        ▄▄
   ███▀▄▀▀█▄ ▄██▀█ ██       ██   ██ ██ ████▄ ██ ████▄ ▄████
 ▄ ██  ▄█▀██ ▀███▄ ██       ██   ██▄██ ██ ██ ██ ██ ██ ██ ██
 ▀██▀ ▄▀█▄███▄▄██▀▄██       ▀██▄▄▄▀██▀▄████▀▄██▄██ ▀█▄▀████
                                   ██  ██                ██
                                 ▀▀▀   ▀               ▀▀▀ 
###########################################################
```

---

## 📋 Table of Contents

- [Features](#-features)
- [Quick Start](#-quick-start)
- [Installation](#-installation)
- [How to Play](#-how-to-play)
- [Configuration](#-configuration)
- [Technical Documentation](#-technical-documentation)
- [Project Structure](#-project-structure)
- [Building from Source](#-building-from-source)
- [License](#-license)

---

## ✨ Features

- **Real-time typing feedback** - Instant visual indication of correct/incorrect keystrokes
- **WPM Calculation** - Accurate words-per-minute measurement
- **Per-character statistics** - Track your performance for each letter individually
- **Difficulty levels** - Easy, Normal, Hard, and Impossible modes
- **Customizable themes** - Multiple background color options
- **Progressive learning** - Unlock new letters as you improve
- **Persistent statistics** - Your progress is saved between sessions
- **Dictionary-based words** - Uses system dictionary for realistic typing practice

---

## 🚀 Quick Start

### Prerequisites

- Linux operating system
- GCC compiler
- CMake (version 3.10 or higher)
- System dictionary file (`/usr/share/dict/words`)

### Install & Run

```bash
# Clone the repository
git clone https://github.com/Andre240256/FastType.git
cd KeyTraining

# Build the project
mkdir -p build && cd build
cmake ..
make

# Run the application
./ctt
```

---

## 📦 Installation

### Option 1: Using CMake (Recommended)

```bash
mkdir -p build
cd build
cmake ..
make
```

The executable `ctt` will be created in the `build/` directory.

### Option 2: Using Make directly

```bash
make
```

### Dependencies

| Dependency | Purpose |
|------------|---------|
| GCC | C compiler |
| CMake 3.10+ | Build system |
| `/usr/share/dict/words` | Word dictionary for typing exercises |

On Debian/Ubuntu, install the dictionary with:
```bash
sudo apt install wamerican
```

---

## 🎮 How to Play

### Starting the Game

1. Run `./ctt` (or `./build/ctt` from the project root)
2. Press `s` to start a match
3. Type the displayed words as fast and accurately as you can

### Controls

| Key | Action |
|-----|--------|
| `s` | Start a new game |
| `c` | Continue playing (when in menu) |
| `y` | Play again (after losing) |
| `ESC` | Open menu |
| `Ctrl + Q` | Quit application |

### Visual Symbols

| Symbol | Meaning |
|--------|---------|
| `·` | Space character to type |
| `↵` | Enter/newline to type |

### Color Feedback

- **Gray** - Characters waiting to be typed
- **White** - Correctly typed characters
- **Red** - Incorrectly typed characters

---

## ⚙️ Configuration

Configuration is stored in `src/configs/status.txt`. Each line follows the format: `ID VALUE;`

### Configuration Options

| ID | Setting | Values |
|----|---------|--------|
| 1 | Unlocked Letters | Any combination of letters (e.g., `enitrl`) |
| 2 | Background Color | `black`, `blue`, `red`, `cyan`, or default (dark gray) |
| 3 | Difficulty | `easy`, `normal`, `hard`, `impossible` |

### Example Configuration

```
1 enitrl;
2 black;
3 easy;
```

### Difficulty Levels

| Level | Lives |
|-------|-------|
| Easy | Unlimited |
| Normal | 7 |
| Hard | 3 |
| Impossible | 1 |

---

## 🔧 Technical Documentation

### Architecture Overview

The application follows a modular architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────┐
│                       main.c                            │
│              (State Machine Controller)                 │
├─────────────┬─────────────┬─────────────┬──────────────┤
│  terminal/  │   stats/    │   debug/    │   configs/   │
│ (I/O Layer) │ (Analytics) │  (Logging)  │   (Data)     │
└─────────────┴─────────────┴─────────────┴──────────────┘
```

### State Machine

The application uses a finite state machine to manage different screens:

**States:**
- `STATE_START_MENU` - Initial welcome screen
- `STATE_GAME` - Main typing game
- `STATE_WINNER` - Victory screen with statistics
- `STATE_LOSER` - Defeat screen
- `STATE_MENU` - Statistics menu
- `STATE_EXIT` - Application termination
- `STATE_ERROR` - Error handling state

### Modules

#### Terminal Module (`src/terminal/`)

Handles all terminal I/O operations using ANSI escape sequences and POSIX terminal control:

- **Raw Mode** - Disables canonical mode for immediate key input
- **Alternative Screen Buffer** - Preserves user's terminal history
- **Cursor Control** - Block/bar cursor styles
- **Color Management** - ANSI color codes for theming

Key functions:
```c
void enableRawMode();      // Enable character-by-character input
void disableRawMode();     // Restore original terminal settings
void clearScreen();        // Clear terminal display
void goHome();             // Move cursor to top-left
void setBeckgroundColor(); // Set background color theme
```

#### Statistics Module (`src/stats/`)

Tracks and persists user performance data:

- **Per-character WPM** - Rolling average of typing speed for each key
- **Precision tracking** - Right/wrong keystroke counts
- **Binary persistence** - Saves stats to `userdata.bin`

Data structure:
```c
typedef struct {
    double samples[MAX_SAMPLES];  // Last 50 timing samples
    int sampleIndex;              // Circular buffer index
    int count;                    // Number of samples collected
    double currentWpm;            // Current WPM for this character
    int wrongCount;               // Total wrong keystrokes
    int rightCount;               // Total correct keystrokes
    double precision;             // Accuracy ratio
} CharStats;
```

#### Debug Module (`src/debug/`)

Simple logging facility for development:

```c
void logDebug(const char * format, ...);  // Printf-style logging to debug.txt
```

### Word Loading Algorithm

1. **First Pass** - Scan dictionary to count valid words (3-10 letters, only unlocked characters)
2. **Random Selection** - Generate sorted random indices
3. **Second Pass** - Extract words at selected indices
4. **Shuffle** - Randomize word order using Fisher-Yates algorithm

### WPM Calculation

Words per minute is calculated using the standard formula:

```
WPM = (characters / 5) / minutes
```

Where 5 characters equals one "word" by convention.

### Terminal Control

The application uses:
- **termios** - POSIX terminal control for raw input mode
- **ANSI escape sequences** - For colors, cursor control, and screen management
- **Alternative screen buffer** - `\033[?1049h` / `\033[?1049l`

---

## 📁 Project Structure

```
KeyTraining/
├── CMakeLists.txt          # CMake build configuration
├── Makefile                # Alternative make build
├── README.md               # This file
├── debug.txt               # Debug log output
├── build/                  # Build output directory
│   ├── ctt                 # Compiled executable
│   └── src/configs/        # Runtime config copies
└── src/
    ├── main.c              # Main application & state machine
    ├── configs/
    │   ├── initialMsg.txt  # ASCII art welcome banner
    │   ├── status.txt      # User configuration
    │   └── userdata.bin    # Persistent statistics (generated)
    ├── terminal/
    │   ├── terminal.c      # Terminal I/O implementation
    │   └── include/
    │       └── terminal.h  # Terminal API & color definitions
    ├── stats/
    │   ├── stats.c         # Statistics tracking & persistence
    │   └── include/
    │       └── stats.h     # Statistics data structures
    └── debug/
        ├── debug.c         # Debug logging implementation
        └── include/
            └── debug.h     # Debug API
```

---

## 🛠️ Building from Source

### Compiler Flags

The project uses the following compiler flags for quality assurance:
- `-Wall` - Enable all common warnings
- `-Wextra` - Enable extra warnings
- `-pedantic` - Enforce strict ISO C compliance
- `-g` - Include debug symbols

### CMake Build

```bash
mkdir -p build && cd build
cmake ..
make
```

### Clean Build

```bash
cd build
make clean
rm -rf *
cmake ..
make
```

### Debug Build

The default build includes debug symbols. For a release build, modify `CMakeLists.txt` to remove the `-g` flag.

---

## 🐛 Troubleshooting

### Dictionary not found

If you see "Error opening dataset: /usr/share/dict/words", install a dictionary:

```bash
# Debian/Ubuntu
sudo apt install wamerican

# Fedora
sudo dnf install words

# Arch Linux
sudo pacman -S words
```

### Terminal display issues

Ensure your terminal supports:
- ANSI escape sequences
- UTF-8 encoding (for symbols like `·` and `↵`)
- 256 colors

### Statistics not saving

Check write permissions for `src/configs/userdata.bin`.

---

## 📊 Future Improvements

- [ ] Windows compatibility
- [ ] Custom word lists
- [ ] Multiplayer mode
- [ ] More theme options
- [ ] Sound feedback
- [ ] Timed challenges

---

## 📝 License

This project is open source. Feel free to use, modify, and distribute.

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---

**Happy Typing! ⌨️💨**
