from serial.tools import list_ports
import serial

PICO_VID = 0x2E8A
PICO_PID = 0x000A
PICO_BAUDRATE = 115200


class dac8554_api:

    def __init__(self):
        port = None

        for p in list_ports.comports():
            if p.vid == PICO_VID and p.pid == PICO_PID:
                port = p.device
                break

        if port is None:
            raise RuntimeError("Cannot find DAC8554 CLI interface")

        self.serial = serial.Serial(port, PICO_BAUDRATE, timeout=1)
        self.serial.read_until(b"DAC> ")

    def send(self, command):
        self.serial.write((command + "\n").encode())
        self.serial.flush()

        return self.serial.read_until(b"DAC> ").decode(errors="replace")

    def write_single(self, inst, channel, value):
        if inst < 0 or inst > 2:
            raise ValueError("Instrument number must be 0, 1 or 2")

        if value < 0 or value > 65535:
            raise ValueError("Value must be between 0 and 65535")

        return self.send( f"write_single --inst {inst} --ch {channel} 0x{value:04X}")

    def write_all(self, inst, A, B, C, D):
        if inst < 0 or inst > 2:
            raise ValueError("Instrument number must be 0, 1 or 2")

        for value in (A, B, C, D):
            if value < 0 or value > 65535:
                raise ValueError("All values must be between 0 and 65535")

        return self.send(   f"write_all --inst {inst} "
                            f"--ch A 0x{A:04X} "
                            f"--ch B 0x{B:04X} "
                            f"--ch C 0x{C:04X} "
                            f"--ch D 0x{D:04X}")

    def read_config(self):
        return self.send("read_config")

    def zero(self):
        return self.send("zero")

    def close(self):
        if hasattr(self, "serial") and self.serial.is_open:
            self.serial.close()

    def __del__(self):
        self.close()