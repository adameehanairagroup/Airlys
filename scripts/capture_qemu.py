#!/usr/bin/env python3
import sys
import time
import socket
import json

def qmp_send_keys(sock_path, key_names):
    try:
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s.settimeout(10)
        s.connect(sock_path)
        s.recv(4096)
        s.sendall(json.dumps({"execute": "qmp_capabilities"}).encode() + b"\n")
        s.recv(4096)
        for k in key_names:
            cmd = json.dumps({"execute": "send-key", "arguments": {"keys": [{"type": "qcode", "data": k}]}})
            s.sendall(cmd.encode() + b"\n")
            s.recv(4096)
            time.sleep(0.1)
        s.close()
        print(f"Sent keys {key_names} successfully.")
    except Exception as e:
        print(f"Failed to send keys: {e}")

def qmp_screendump(sock_path, out_file):
    try:
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s.settimeout(10)
        s.connect(sock_path)
        s.recv(4096)  # QMP greeting
        s.sendall(json.dumps({"execute": "qmp_capabilities"}).encode() + b"\n")
        s.recv(4096)  # Capabilities accepted
        cmd = json.dumps({"execute": "screendump", "arguments": {"filename": out_file}})
        s.sendall(cmd.encode() + b"\n")
        s.recv(4096)  # Screendump completed
        s.close()
        print(f"Successfully captured screenshot to: {out_file}")
    except Exception as e:
        print(f"Failed to capture {out_file}: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: capture_qemu.py <qmp_socket_path> <action_or_output>")
        sys.exit(1)
    sock = sys.argv[1]
    arg = sys.argv[2]
    if arg == "enter":
        qmp_send_keys(sock, ["ret"])
    elif arg == "login":
        qmp_send_keys(sock, ["a", "i", "r", "l", "y", "s", "ret"])
    elif arg == "start" or arg == "super":
        qmp_send_keys(sock, ["meta_l"])
    else:
        qmp_screendump(sock, arg)
