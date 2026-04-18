import serial
import time

PORT = "/dev/ttyACM0"  # change if needed (/dev/ttyUSB0, COMx on Windows)
BAUD = 921600
MAX_FRAMES = 20


def read_line(ser):
    return ser.readline().decode(errors="ignore").strip()


def main():
    ser = serial.Serial(PORT, BAUD, timeout=3)
    time.sleep(2)  # board reset time

    saved = 0
    while saved < MAX_FRAMES:
        line = read_line(ser)
        if not line:
            continue

        if line.startswith("FRAME "):
            try:
                size = int(line.split()[1])
            except:
                continue

            data = ser.read(size)
            if len(data) != size:
                print(f"[ERR] expected {size}, got {len(data)}")
                continue

            # consume trailing newline + FRAME_END line
            ser.readline()
            end = read_line(ser)

            if end != "FRAME_END":
                print(f"[WARN] missing FRAME_END, got: {end}")

            fn = f"frame_{saved:03d}.jpg"
            with open(fn, "wb") as f:
                f.write(data)
            print(f"[OK] saved {fn} ({size} bytes)")
            saved += 1
        else:
            print("[LOG]", line)

    ser.close()


if __name__ == "__main__":
    main()
