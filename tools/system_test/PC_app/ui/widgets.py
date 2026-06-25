from collections import deque

from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor, QPainter, QPen
from PyQt5.QtWidgets import QFrame, QHBoxLayout, QLabel, QSizePolicy, QVBoxLayout, QWidget


class TemperatureChart(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.samples = deque(maxlen=120)
        self.setMinimumHeight(150)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

    def add_sample(self, value):
        self.samples.append(float(value))
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        rect = self.rect().adjusted(10, 10, -10, -12)
        painter.fillRect(self.rect(), QColor("#FFFFFF"))
        painter.setPen(QPen(QColor("#E2E8F0"), 1))
        for index in range(5):
            y = rect.top() + index * rect.height() / 4
            painter.drawLine(rect.left(), int(y), rect.right(), int(y))

        painter.setPen(QPen(QColor("#CBD5E1"), 1))
        painter.drawRect(rect)
        if len(self.samples) < 2:
            painter.setPen(QColor("#94A3B8"))
            painter.drawText(rect, Qt.AlignCenter, "Waiting for temperature data")
            return

        values = list(self.samples)
        minimum, maximum = min(values), max(values)
        if maximum - minimum < 1.0:
            minimum -= 0.5
            maximum += 0.5

        def to_point(index, value):
            x = rect.left() + index * rect.width() / (len(values) - 1)
            y = rect.bottom() - ((value - minimum) / (maximum - minimum)) * rect.height()
            return int(x), int(y)

        painter.setPen(QPen(QColor("#2563EB"), 2))
        last = to_point(0, values[0])
        for index, value in enumerate(values[1:], start=1):
            current = to_point(index, value)
            painter.drawLine(last[0], last[1], current[0], current[1])
            last = current

        label_rect = rect.adjusted(6, 4, -6, -4)
        painter.setPen(QColor("#64748B"))
        painter.drawText(label_rect, Qt.AlignTop | Qt.AlignRight, f"{maximum:.1f}")
        painter.drawText(label_rect, Qt.AlignBottom | Qt.AlignRight, f"{minimum:.1f}")


class MetricCard(QFrame):
    def __init__(self, title, value="--", unit="", parent=None):
        super().__init__(parent)
        self.setObjectName("card")

        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 14, 16, 14)
        layout.setSpacing(6)

        title_label = QLabel(title)
        title_label.setObjectName("muted")

        value_row = QHBoxLayout()
        value_row.setSpacing(6)
        self.value_label = QLabel(value)
        self.value_label.setObjectName("metricValue")
        self.unit_label = QLabel(unit)
        self.unit_label.setObjectName("metricUnit")
        self.unit_label.setAlignment(Qt.AlignBottom)
        value_row.addWidget(self.value_label)
        value_row.addWidget(self.unit_label)
        value_row.addStretch()

        self.caption_label = QLabel("")
        self.caption_label.setObjectName("muted")
        layout.addWidget(title_label)
        layout.addLayout(value_row)
        layout.addWidget(self.caption_label)

    def set_value(self, value, caption=""):
        self.value_label.setText(value)
        self.caption_label.setText(caption)
