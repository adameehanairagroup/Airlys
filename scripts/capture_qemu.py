#!/usr/bin/env python3
import sys
import time
import socket
import json

def qmp_execute(sock_path, cmd_dict):
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.settimeout(10)
    s.connect(sock_path)
    s.recv(4096)  # Greeting
    s.sendall(json.dumps({"execute": "qmp_capabilities"}).encode() + b"\n")
    s.recv(4096)  # Ack
    s.sendall(json.dumps(cmd_dict).encode() + b"\n")
    resp = s.recv(4096)
    s.close()
    return resp

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
            time.sleep(0.08)
        s.close()
        print(f"Sent keys {key_names} successfully.")
    except Exception as e:
        print(f"Failed to send keys: {e}")

def qmp_send_combo(sock_path, combo_keys):
    try:
        keys_payload = [{"type": "qcode", "data": k} for k in combo_keys]
        cmd = {"execute": "send-key", "arguments": {"keys": keys_payload}}
        qmp_execute(sock_path, cmd)
        print(f"Sent combo {combo_keys} successfully.")
    except Exception as e:
        print(f"Failed to send combo {combo_keys}: {e}")

def qmp_type_string(sock_path, text):
    try:
        for ch in text:
            if ch == ' ':
                qmp_send_keys(sock_path, ['spc'])
            elif ch == '\n':
                qmp_send_keys(sock_path, ['ret'])
            elif ch == '-':
                qmp_send_keys(sock_path, ['minus'])
            else:
                qmp_send_keys(sock_path, [ch.lower()])
            time.sleep(0.05)
        print(f"Typed string '{text}' successfully.")
    except Exception as e:
        print(f"Failed to type string: {e}")

def qmp_open_about_terminal(sock_path):
    try:
        print("Opening Application / Terminal for About System...")
        # 1. First try KRunner (Alt+F2)
        qmp_send_combo(sock_path, ["alt", "f2"])
        time.sleep(1.5)
        qmp_type_string(sock_path, "konsole\n")
        time.sleep(4)

        # 2. Also try standard shortcut Ctrl+Alt+T
        qmp_send_combo(sock_path, ["ctrl", "alt", "t"])
        time.sleep(3)

        # 3. Type fastfetch in Konsole terminal
        qmp_type_string(sock_path, "fastfetch\n")
        time.sleep(4)
        print("Executed fastfetch command in live terminal.")
    except Exception as e:
        print(f"Failed to open About system: {e}")

def qmp_screendump(sock_path, out_file):
    try:
        cmd = {"execute": "screendump", "arguments": {"filename": out_file}}
        qmp_execute(sock_path, cmd)
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
    elif arg == "esc":
        qmp_send_keys(sock, ["esc"])
    elif arg == "about":
        qmp_open_about_terminal(sock)
    else:
        qmp_screendump(sock, arg)

