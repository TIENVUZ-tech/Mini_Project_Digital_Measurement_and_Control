from datetime import datetime

from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtWidgets import (
    QApplication,
    QButtonGroup,
    QComboBox,
    QDoubleSpinBox,
    QFrame,
    QGridLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QProgressBar,
    QTextEdit,
    QVBoxLayout,
    QWidget,
)

import protocol
from serial_manager import SerialManager
from ui.widgets import MetricCard, TemperatureChart


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.serial = SerialManager()
        self.mode = protocol.MODE_AUTO
        self.report_mode = protocol.REPORT_ONE_SHOT
        self.temperature = None
        self.setpoint = None
        self.pending_setpoint = None
        self.fan_state = protocol.ACT_OFF
        self.heater_state = protocol.ACT_OFF
        self.fan_pwm = None
        self.heater_pwm = None
        self.frame_log_visible = True

        self.setWindowTitle("Greenhouse PC App")
        self.resize(1280, 780)

        self._build_ui()
        self._wire_signals()
        self.refresh_ports()
        self._sync_connection_state(False)
        self._sync_mode_buttons()
        self._sync_report_buttons()
        self._update_dashboard()

        self.port_timer = QTimer(self)
        self.port_timer.setInterval(3000)
        self.port_timer.timeout.connect(self.refresh_ports)
        self.port_timer.start()

    def _build_ui(self):
        central = QWidget()
        self.setCentralWidget(central)

        root = QVBoxLayout(central)
        root.setContentsMargins(16, 16, 16, 12)
        root.setSpacing(12)

        root.addWidget(self._build_header())

        content = QHBoxLayout()
        content.setSpacing(12)

        left = QVBoxLayout()
        left.setSpacing(12)
        left.addLayout(self._build_metrics())
        left.addWidget(self._build_chart_card())
        left.addWidget(self._build_log_card(), 1)

        right = QVBoxLayout()
        right.setSpacing(10)
        right.addWidget(self._build_control_card())
        right.addWidget(self._build_pid_card())
        right.addStretch()

        left_widget = QWidget()
        left_widget.setLayout(left)
        right_widget = QWidget()
        right_widget.setLayout(right)
        right_widget.setMinimumWidth(430)
        right_widget.setMaximumWidth(500)

        content.addWidget(left_widget, 3)
        content.addWidget(right_widget, 1)
        root.addLayout(content, 1)

        self.statusBar().showMessage("Ready")

    def _build_header(self):
        frame = QFrame()
        frame.setObjectName("topBar")
        layout = QHBoxLayout(frame)
        layout.setContentsMargins(18, 14, 18, 14)
        layout.setSpacing(14)

        title_block = QVBoxLayout()
        title_block.setSpacing(2)
        title = QLabel("Greenhouse Control Panel")
        title.setObjectName("title")
        subtitle = QLabel("Serial dashboard for temperature control, actuator commands, PID tuning, and frame logs")
        subtitle.setObjectName("subtitle")
        title_block.addWidget(title)
        title_block.addWidget(subtitle)

        self.connection_pill = QLabel("Disconnected")
        self.connection_pill.setObjectName("pill")
        self.connection_pill.setProperty("status", "bad")
        self.connection_pill.setAlignment(Qt.AlignCenter)

        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(120)

        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["115200", "57600", "38400", "19200", "9600"])
        self.baud_combo.setCurrentText("115200")

        self.refresh_button = QPushButton("Refresh")
        self.refresh_button.setIcon(QApplication.style().standardIcon(QApplication.style().SP_BrowserReload))

        self.connect_button = QPushButton("Connect")
        self.connect_button.setProperty("variant", "success")

        layout.addLayout(title_block, 1)
        layout.addWidget(self.connection_pill)
        layout.addWidget(QLabel("Port"))
        layout.addWidget(self.port_combo)
        layout.addWidget(QLabel("Baud"))
        layout.addWidget(self.baud_combo)
        layout.addWidget(self.refresh_button)
        layout.addWidget(self.connect_button)

        return frame

    def _build_metrics(self):
        grid = QGridLayout()
        grid.setSpacing(12)

        self.temp_card = MetricCard("Temperature", "--", "deg C")
        self.setpoint_card = MetricCard("Setpoint", "--", "deg C")
        self.fan_card = MetricCard("Fan", "OFF", "")
        self.heater_card = MetricCard("Heater", "OFF", "")

        grid.addWidget(self.temp_card, 0, 0)
        grid.addWidget(self.setpoint_card, 0, 1)
        grid.addWidget(self.fan_card, 0, 2)
        grid.addWidget(self.heater_card, 0, 3)

        return grid

    def _make_card(self, margins=(16, 14, 16, 14), spacing=10):
        frame = QFrame()
        frame.setObjectName("card")
        layout = QVBoxLayout(frame)
        layout.setContentsMargins(*margins)
        layout.setSpacing(spacing)
        return frame, layout

    def _make_group(self, title, layout_cls=QHBoxLayout):
        box = QGroupBox(title)
        layout = layout_cls(box)
        layout.setContentsMargins(10, 12, 10, 8)
        layout.setSpacing(8)
        return box, layout

    def _make_button(self, text, variant=None):
        button = QPushButton(text)
        if variant:
            button.setProperty("variant", variant)
        return button

    def _build_chart_card(self):
        frame, layout = self._make_card()

        row = QHBoxLayout()
        title = QLabel("Temperature Trend")
        title.setObjectName("sectionTitle")
        self.last_update_label = QLabel("No data yet")
        self.last_update_label.setObjectName("muted")
        row.addWidget(title)
        row.addStretch()
        row.addWidget(self.last_update_label)

        self.chart = TemperatureChart()

        layout.addLayout(row)
        layout.addWidget(self.chart)

        return frame

    def _build_log_card(self):
        self.log_card, layout = self._make_card()

        row = QHBoxLayout()
        title = QLabel("Frame Log")
        title.setObjectName("sectionTitle")
        self.toggle_log_button = QPushButton("Hide")
        self.clear_log_button = QPushButton("Clear")
        row.addWidget(title)
        row.addStretch()
        row.addWidget(self.toggle_log_button)
        row.addWidget(self.clear_log_button)

        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)

        layout.addLayout(row)
        layout.addWidget(self.log_text, 1)

        return self.log_card

    def _build_control_card(self):
        frame, layout = self._make_card((14, 12, 14, 12), 8)

        title = QLabel("System Control")
        title.setObjectName("sectionTitle")
        layout.addWidget(title)

        mode_row = QHBoxLayout()
        self.auto_button = QPushButton("Auto")
        self.manual_button = QPushButton("Manual")
        self.mode_group = QButtonGroup(self)
        self.mode_group.setExclusive(True)
        self.mode_group.addButton(self.auto_button, protocol.MODE_AUTO)
        self.mode_group.addButton(self.manual_button, protocol.MODE_MANUAL)
        mode_row.addWidget(self.auto_button)
        mode_row.addWidget(self.manual_button)
        layout.addLayout(mode_row)

        setpoint_box, setpoint_layout = self._make_group("Temperature Setpoint")
        self.setpoint_spin = QDoubleSpinBox()
        self.setpoint_spin.setRange(0.0, 50.0)
        self.setpoint_spin.setDecimals(1)
        self.setpoint_spin.setSingleStep(0.5)
        self.setpoint_spin.setValue(40.0)
        self.apply_setpoint_button = QPushButton("Apply")
        setpoint_layout.addWidget(self.setpoint_spin, 1)
        setpoint_layout.addWidget(self.apply_setpoint_button)
        layout.addWidget(setpoint_box)

        report_box, report_layout = self._make_group("Report Mode", QGridLayout)
        self.oneshot_button = QPushButton("One Shot")
        self.stream_button = QPushButton("Stream")
        self.stop_stream_button = QPushButton("Stop Stream")
        self.stop_stream_button.setProperty("variant", "warning")
        report_layout.addWidget(self.oneshot_button, 0, 0)
        report_layout.addWidget(self.stream_button, 0, 1)
        report_layout.addWidget(self.stop_stream_button, 1, 0, 1, 2)
        layout.addWidget(report_box)

        actuator_box, actuator_layout = self._make_group("Manual Actuators", QGridLayout)
        self.fan_on_button = self._make_button("Fan ON", "success")
        self.fan_off_button = self._make_button("Fan OFF", "danger")
        self.heater_on_button = self._make_button("Heater ON", "success")
        self.heater_off_button = self._make_button("Heater OFF", "danger")
        for button, row, col in (
            (self.fan_on_button, 0, 0),
            (self.fan_off_button, 0, 1),
            (self.heater_on_button, 1, 0),
            (self.heater_off_button, 1, 1),
        ):
            actuator_layout.addWidget(button, row, col)
        layout.addWidget(actuator_box)

        pwm_row = QHBoxLayout()
        pwm_label = QLabel("Heater PWM")
        self.heater_pwm_bar = QProgressBar()
        self.heater_pwm_bar.setRange(0, 100)
        self.heater_pwm_bar.setValue(0)
        self.heater_pwm_bar.setFormat("%p%")
        pwm_row.addWidget(pwm_label)
        pwm_row.addWidget(self.heater_pwm_bar, 1)
        layout.addLayout(pwm_row)

        raw_box, raw_layout = self._make_group("Raw Sender")
        self.raw_send_edit = QLineEdit()
        self.raw_send_edit.setPlaceholderText("AA 0F 02 00 00")
        self.raw_send_button = QPushButton("Send")
        raw_layout.addWidget(self.raw_send_edit, 1)
        raw_layout.addWidget(self.raw_send_button)
        layout.addWidget(raw_box)

        return frame

    def _build_pid_card(self):
        frame, layout = self._make_card((14, 12, 14, 12), 8)

        title = QLabel("PID Tuning")
        title.setObjectName("sectionTitle")
        layout.addWidget(title)

        grid = QGridLayout()
        grid.setHorizontalSpacing(8)
        grid.setVerticalSpacing(6)
        self.kp_spin, self.ki_spin, self.kd_spin = (
            self._make_gain_spin(value) for value in (2.5, 0.1, 0.4)
        )
        for row, (label, spin) in enumerate((
            ("Kp", self.kp_spin),
            ("Ki", self.ki_spin),
            ("Kd", self.kd_spin),
        )):
            grid.addWidget(QLabel(label), row, 0)
            grid.addWidget(spin, row, 1)

        self.apply_pid_button = QPushButton("Apply PID")
        layout.addLayout(grid)
        layout.addWidget(self.apply_pid_button)

        return frame

    def _make_gain_spin(self, value):
        spin = QDoubleSpinBox()
        spin.setRange(0.0, 100.0)
        spin.setDecimals(3)
        spin.setSingleStep(0.1)
        spin.setValue(value)
        return spin

    def _wire_signals(self):
        for button, handler in (
            (self.refresh_button, self.refresh_ports),
            (self.connect_button, self.toggle_connection),
            (self.toggle_log_button, self.toggle_frame_log),
            (self.clear_log_button, self.log_text.clear),
            (self.apply_setpoint_button, self.apply_setpoint),
            (self.apply_pid_button, self.apply_pid),
            (self.raw_send_button, self.send_raw_data),
        ):
            button.clicked.connect(handler)

        for button, mode in ((self.auto_button, protocol.MODE_AUTO), (self.manual_button, protocol.MODE_MANUAL)):
            button.clicked.connect(lambda checked=False, mode=mode: self.set_mode(mode))

        for button, report_mode in (
            (self.oneshot_button, protocol.REPORT_ONE_SHOT),
            (self.stream_button, protocol.REPORT_STREAM),
            (self.stop_stream_button, protocol.REPORT_ONE_SHOT),
        ):
            button.clicked.connect(lambda checked=False, report_mode=report_mode: self.set_report_mode(report_mode))

        for button, actuator, state in (
            (self.fan_on_button, protocol.ACTUATOR_FAN, protocol.ACT_ON),
            (self.fan_off_button, protocol.ACTUATOR_FAN, protocol.ACT_OFF),
            (self.heater_on_button, protocol.ACTUATOR_HEATER, protocol.ACT_ON),
            (self.heater_off_button, protocol.ACTUATOR_HEATER, protocol.ACT_OFF),
        ):
            button.clicked.connect(
                lambda checked=False, actuator=actuator, state=state: self.set_actuator(actuator, state)
            )

        self.raw_send_edit.returnPressed.connect(self.send_raw_data)

        self.serial.frame_received.connect(self.handle_frame)
        self.serial.raw_log.connect(self.handle_raw_log)
        self.serial.connection_changed.connect(self._sync_connection_state)
        self.serial.connection_lost.connect(self.handle_connection_lost)

    def refresh_ports(self):
        current = self.port_combo.currentText()
        ports = SerialManager.list_ports()

        self.port_combo.blockSignals(True)
        self.port_combo.clear()
        if ports:
            self.port_combo.addItems(ports)
            if current in ports:
                self.port_combo.setCurrentText(current)
        else:
            self.port_combo.addItem("No ports")
        self.port_combo.blockSignals(False)

    def toggle_frame_log(self):
        self.frame_log_visible = not self.frame_log_visible
        self.log_text.setVisible(self.frame_log_visible)
        self.toggle_log_button.setText("Hide" if self.frame_log_visible else "Show")

    def toggle_connection(self):
        if self.serial.is_connected():
            self.serial.disconnect_port()
            self._append_event("Disconnected")
            return

        port = self.port_combo.currentText()
        if not port or port == "No ports":
            self._append_event("No serial port available", level="warn")
            return

        baudrate = int(self.baud_combo.currentText())
        if self.serial.connect_port(port, baudrate):
            self._append_event(f"Connected to {port} at {baudrate} baud")
            self.set_report_mode(protocol.REPORT_ONE_SHOT)
        else:
            self._append_event(f"Could not open {port}", level="error")
            QMessageBox.warning(self, "Connection failed", f"Could not open {port}.")

    def set_mode(self, mode):
        if self._send(protocol.build_set_mode(mode), "SET_MODE"):
            self.mode = mode
            self._sync_mode_buttons()

    def apply_setpoint(self):
        value = self.setpoint_spin.value()
        if self._send(protocol.build_set_setpoint(value), "SET_SETPOINT"):
            self.pending_setpoint = value
            self.setpoint = value
            self._update_dashboard()

    def set_report_mode(self, report_mode):
        if self._send(protocol.build_set_report_mode(report_mode), "SET_REPORT_MODE"):
            self.report_mode = report_mode
            self._sync_report_buttons()

    def set_actuator(self, actuator, state):
        if self.mode != protocol.MODE_MANUAL:
            self._append_event("Switch to Manual before actuator control", level="warn")
            return

        if self._send(protocol.build_manual_actuator(actuator, state), "MANUAL_ACTUATOR"):
            self._append_event("Waiting for MCU actuator status", level="info")
            self._send(protocol.build_request_status(), "REQUEST_STATUS")

    def apply_pid(self):
        kp = self.kp_spin.value()
        ki = self.ki_spin.value()
        kd = self.kd_spin.value()
        self._send(protocol.build_set_pid(kp, ki, kd), "SET_PID")

    def send_raw_data(self):
        text = self.raw_send_edit.text().strip()
        if not text:
            self._append_event("Raw data is empty", level="warn")
            return

        try:
            data = self._parse_hex_text(text)
        except ValueError as exc:
            QMessageBox.warning(self, "Invalid raw data", str(exc))
            return

        self._send(data, "RAW")

    def _parse_hex_text(self, text):
        cleaned = text.replace("0x", "").replace("0X", "")
        for separator in (",", ";", ":", "-", "_", "\r", "\n", "\t"):
            cleaned = cleaned.replace(separator, " ")

        compact = "".join(cleaned.split())
        if not compact:
            raise ValueError("Enter at least one byte in hexadecimal format.")

        if len(compact) % 2:
            raise ValueError("Hex data must contain an even number of digits.")

        try:
            return bytes.fromhex(compact)
        except ValueError as exc:
            raise ValueError("Hex data contains invalid characters.") from exc

    def _send(self, frame, label):
        if not self.serial.is_connected():
            self._append_event(f"{label} not sent: serial is disconnected", level="warn")
            return False

        ok = self.serial.send(frame)
        if ok:
            self.statusBar().showMessage(f"Sent {label}", 2500)
        else:
            self._append_event(f"{label} send failed", level="error")
        return ok

    def handle_frame(self, frame):
        gid = frame["gid"]
        mid = frame["mid"]
        payload = frame["payload"]
        name = protocol.frame_name(gid, mid)

        if gid == protocol.GID_SENSOR and mid == protocol.MID_SENSOR_REPORT:
            decoded = protocol.decode_sensor_report(payload)
            if decoded:
                self.temperature = decoded["temperature"]
                self._handle_reported_setpoint(decoded["temp_setpoint"])
                self._append_event(
                    f"{name}: temperature={self.temperature:.2f}, setpoint={self.setpoint:.2f}"
                )
                self._record_sample()
                self._update_dashboard()
            return

        if gid == protocol.GID_SYSTEM and mid == protocol.MID_STATUS_REPORT and len(payload) >= 10:
            decoded = protocol.decode_status_report(payload)
            if decoded:
                self.temperature = decoded["temperature"]
                self._handle_reported_setpoint(decoded["temp_setpoint"])
                self.fan_state = decoded["fan_state"]
                self.heater_state = decoded["heater_state"]
                self.fan_pwm = decoded["fan_pwm"]
                self.heater_pwm = decoded["heater_pwm"]
                self._append_event(f"{name}: status updated")
                self._record_sample()
                self._update_dashboard()
            return

        if gid == protocol.GID_ERROR:
            decoded = protocol.decode_error_report(payload)
            if decoded:
                self._append_event(
                    f"{name}: code={decoded['error_code']} actuator={decoded['actuator_index']}",
                    level="error",
                )
            else:
                self._append_event(f"{name}: {payload.hex(' ').upper()}", level="error")
            return

        status = protocol.decode_status_response(payload)
        if status is not None:
            text = protocol.status_name(gid, mid, status)
            level = "ok" if "updated" in text.lower() or text.endswith("OK") else "warn"
            if gid == protocol.GID_SYSTEM and mid == protocol.MID_SET_SETPOINT:
                if status == protocol.RSP_SET_SETPOINT_FAIL:
                    self.pending_setpoint = None
            self._append_event(f"{name}: {text}", level=level)
        else:
            self._append_event(f"{name}: payload={payload.hex(' ').upper()}")

    def handle_raw_log(self, direction, data):
        color = "#93C5FD" if direction == "TX" else "#86EFAC"
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        hex_data = data.hex(" ").upper()
        self.log_text.append(
            f'<span style="color:#64748B">{timestamp}</span> '
            f'<span style="color:{color}; font-weight:700">{direction}</span> '
            f"<span>{hex_data}</span>"
        )

    def handle_connection_lost(self):
        self._append_event("Serial connection lost", level="error")
        QMessageBox.warning(self, "Connection lost", "The serial connection was closed unexpectedly.")

    def _sync_connection_state(self, connected):
        self.connection_pill.setText("Connected" if connected else "Disconnected")
        self.connection_pill.setProperty("status", "ok" if connected else "bad")
        self._repolish(self.connection_pill)

        self.connect_button.setText("Disconnect" if connected else "Connect")
        self.connect_button.setProperty("variant", "danger" if connected else "success")
        self._repolish(self.connect_button)

        self.port_combo.setEnabled(not connected)
        self.baud_combo.setEnabled(not connected)
        self.refresh_button.setEnabled(not connected)
        self._sync_controls_enabled()

        self.statusBar().showMessage("Connected" if connected else "Disconnected", 2500)

    def _sync_controls_enabled(self):
        connected = self.serial.is_connected()
        for widget in (
            self.auto_button,
            self.manual_button,
            self.setpoint_spin,
            self.apply_setpoint_button,
            self.oneshot_button,
            self.stream_button,
            self.stop_stream_button,
            self.kp_spin,
            self.ki_spin,
            self.kd_spin,
            self.apply_pid_button,
            self.raw_send_edit,
            self.raw_send_button,
        ):
            widget.setEnabled(connected)

        manual_enabled = connected and self.mode == protocol.MODE_MANUAL
        for widget in (
            self.fan_on_button,
            self.fan_off_button,
            self.heater_on_button,
            self.heater_off_button,
        ):
            widget.setEnabled(manual_enabled)

    def _sync_mode_buttons(self):
        self._set_active(self.auto_button, self.mode == protocol.MODE_AUTO)
        self._set_active(self.manual_button, self.mode == protocol.MODE_MANUAL)
        self._sync_controls_enabled()

    def _sync_report_buttons(self):
        self._set_active(self.oneshot_button, self.report_mode == protocol.REPORT_ONE_SHOT)
        self._set_active(self.stream_button, self.report_mode == protocol.REPORT_STREAM)

    def _set_active(self, button, active):
        button.setProperty("active", "true" if active else "false")
        self._repolish(button)

    def _repolish(self, widget):
        widget.style().unpolish(widget)
        widget.style().polish(widget)
        widget.update()

    def _record_sample(self):
        if self.temperature is None:
            return
        self.chart.add_sample(self.temperature)
        self.last_update_label.setText(datetime.now().strftime("Updated %H:%M:%S"))

    def _handle_reported_setpoint(self, reported_setpoint):
        if self.pending_setpoint is None:
            self.setpoint = reported_setpoint
            return

        if abs(reported_setpoint - self.pending_setpoint) <= 0.05:
            self.setpoint = reported_setpoint
            self.pending_setpoint = None
            return

        self.setpoint = self.pending_setpoint

    def _setpoint_editor_has_focus(self):
        return (
            self.setpoint_spin.hasFocus()
            or self.setpoint_spin.lineEdit().hasFocus()
        )

    def _update_dashboard(self):
        if self.temperature is None:
            self.temp_card.set_value("--", "Waiting for MCU")
        else:
            self.temp_card.set_value(f"{self.temperature:.1f}", "Live sensor")

        if self.setpoint is None:
            self.setpoint_card.set_value("--", "Not received")
        else:
            caption = "Waiting for MCU update" if self.pending_setpoint is not None else "Target temperature"
            self.setpoint_card.set_value(f"{self.setpoint:.1f}", caption)
            can_sync_editor = (
                self.pending_setpoint is None
                and not self._setpoint_editor_has_focus()
            )
            if can_sync_editor and abs(self.setpoint_spin.value() - self.setpoint) > 0.05:
                self.setpoint_spin.setValue(self.setpoint)

        fan_caption = "State from MCU"
        heater_caption = "PWM unknown" if self.heater_pwm is None else f"PWM {self.heater_pwm}%"
        self.fan_card.set_value(self._state_text(self.fan_state), fan_caption)
        self.heater_card.set_value(self._state_text(self.heater_state), heater_caption)

        if self.heater_pwm is None and self.heater_state == protocol.ACT_ON:
            pwm_value = 100
        else:
            pwm_value = self.heater_pwm or 0
        self.heater_pwm_bar.setValue(max(0, min(100, int(pwm_value))))

    def _state_text(self, state):
        return "ON" if state == protocol.ACT_ON else "OFF"

    def _append_event(self, message, level="info"):
        colors = {
            "info": "#E2E8F0",
            "ok": "#BBF7D0",
            "warn": "#FDE68A",
            "error": "#FCA5A5",
        }
        timestamp = datetime.now().strftime("%H:%M:%S")
        color = colors.get(level, colors["info"])
        self.log_text.append(
            f'<span style="color:#64748B">{timestamp}</span> '
            f'<span style="color:{color}">{message}</span>'
        )

    def closeEvent(self, event):
        self.serial.disconnect_port()
        super().closeEvent(event)
