"""
serial_manager.py
PyQt5 Serial Communication Manager
"""

import time
import serial
import serial.tools.list_ports

from PyQt5.QtCore import (
    QObject,
    QThread,
    pyqtSignal
)

import protocol


# ============================================================
# RX WORKER THREAD
# ============================================================

class SerialWorker(QObject):

    frame_received = pyqtSignal(dict)

    raw_log = pyqtSignal(
        str,
        bytes
    )

    connection_lost = pyqtSignal()

    def __init__(self, serial_port):
        super().__init__()

        self.serial_port = serial_port

        self.running = True

        self.rx_buffer = bytearray()

    # --------------------------------------------------------

    def stop(self):
        self.running = False

    # --------------------------------------------------------

    def run(self):

        while self.running:

            try:

                count = self.serial_port.in_waiting

                if count:

                    data = self.serial_port.read(count)

                    if data:

                        self.rx_buffer.extend(data)

                        self._process_buffer()

                else:

                    time.sleep(0.01)

            except Exception:

                self.connection_lost.emit()

                return

    # --------------------------------------------------------

    def _process_buffer(self):

        while True:

            while (
                self.rx_buffer
                and self.rx_buffer[0]
                != protocol.FRAME_SOF
            ):
                self.rx_buffer.pop(0)

            if not self.rx_buffer:
                return

            frame, consumed = protocol.parse_frame(
                bytes(self.rx_buffer)
            )

            if frame is None:

                if (
                    consumed == 0
                    and len(self.rx_buffer)
                    < protocol.FRAME_HEADER_SIZE
                ):
                    return

                if consumed == 0:

                    self.rx_buffer.pop(0)

                    continue

                return

            raw = bytes(
                self.rx_buffer[:consumed]
            )

            del self.rx_buffer[:consumed]

            self.raw_log.emit(
                "RX",
                raw
            )

            self.frame_received.emit(frame)


# ============================================================
# SERIAL MANAGER
# ============================================================

class SerialManager(QObject):

    frame_received = pyqtSignal(dict)

    raw_log = pyqtSignal(
        str,
        bytes
    )

    connection_changed = pyqtSignal(bool)

    connection_lost = pyqtSignal()

    # --------------------------------------------------------

    def __init__(self):

        super().__init__()

        self.serial_port = None

        self.thread = None

        self.worker = None

    # ========================================================
    # PORTS
    # ========================================================

    @staticmethod
    def list_ports():

        return [
            p.device
            for p in serial.tools.list_ports.comports()
        ]

    # ========================================================
    # CONNECT
    # ========================================================

    def connect_port(
        self,
        port_name,
        baudrate=115200
    ):

        self.disconnect_port()

        try:

            self.serial_port = serial.Serial(
                port_name,
                baudrate,
                timeout=0.05
            )

        except Exception:

            return False

        self.thread = QThread()

        self.worker = SerialWorker(
            self.serial_port
        )

        self.worker.moveToThread(
            self.thread
        )

        self.thread.started.connect(
            self.worker.run
        )

        self.worker.frame_received.connect(
            self.frame_received.emit
        )

        self.worker.raw_log.connect(
            self.raw_log.emit
        )

        self.worker.connection_lost.connect(
            self._on_connection_lost
        )

        self.thread.start()

        self.connection_changed.emit(True)

        return True

    # ========================================================
    # DISCONNECT
    # ========================================================

    def disconnect_port(self):

        if self.worker:

            self.worker.stop()

        if self.thread:

            self.thread.quit()

            self.thread.wait()

            self.thread = None

        self.worker = None

        if self.serial_port:

            try:
                self.serial_port.close()
            except:
                pass

            self.serial_port = None

        self.connection_changed.emit(False)

    # ========================================================
    # STATUS
    # ========================================================

    def is_connected(self):

        return (
            self.serial_port is not None
            and self.serial_port.is_open
        )

    # ========================================================
    # SEND
    # ========================================================

    def send(self, data: bytes):

        if not self.is_connected():
            return False

        try:

            self.serial_port.write(data)

            self.raw_log.emit(
                "TX",
                data
            )

            return True

        except Exception:

            self._on_connection_lost()

            return False

    # ========================================================
    # LOST
    # ========================================================

    def _on_connection_lost(self):

        self.disconnect_port()

        self.connection_lost.emit()