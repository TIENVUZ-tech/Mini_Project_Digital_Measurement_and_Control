# Phát triển hệ thống giám sát và điều khiển nhiệt độ cho nhà kính sử dụng STM32

Dự án xây dựng hệ thống giám sát và điều khiển nhiệt độ nhà kính dựa trên vi điều khiển STM32F103C8T6. Điểm đặc biệt của dự án là phần mềm nhúng được thiết kế theo hướng tiếp cận lập trình thanh ghi (bare-metal) thay vì sử dụng các thư viện cấp cao có sẵn (như HAL hay Arduino framework). Hệ thống bao gồm Firmware trên vi điều khiển và phần mềm điều khiển trung tâm trên máy tính (Python/PyQt5).

---

## 🎯 Mục tiêu dự án
* **Thiết kế trình điều khiển thiết bị (Driver):** Tự xây dựng các thư viện giao tiếp cấp thấp hoàn toàn bằng cách can thiệp trực tiếp vào thanh ghi của vi điều khiển.
* **Thuật toán điều khiển:** Ứng dụng thuật toán PID rời rạc với tích phân hình thang để điều khiển nhiệt độ với độ chính xác cao.
* **Giao tiếp và Giám sát:** Thiết lập cơ chế truyền thông hai chiều (UART) bất đồng bộ giữa vi điều khiển và máy tính (PC) bằng giao thức tự định nghĩa.

## ⚙️ Kiến trúc phần mềm
Hệ thống được tổ chức theo kiến trúc phân tầng rõ ràng, đảm bảo nguyên tắc Low Coupling và High Cohesion:
1. **Application Layer:** Chứa toàn bộ logic nghiệp vụ (xử lý lệnh, điều khiển PID, trạng thái hệ thống).
2. **Platform Layer (HAL):** Lớp trừu tượng phần cứng tự xây dựng, cung cấp API cho tầng Application (sensor_hal, actuator_hal, comm_hal, soft_timer).
3. **MCU Drivers Layer:** Giao tiếp trực tiếp với thanh ghi phần cứng STM32 (GPIO, USART, TIMER).
4. **Common Layer:** Cấu trúc dữ liệu dùng chung (Frame, Hàng đợi, Macro).

## 🚀 Các tính năng chính
* **Cơ chế SoftTimer:** Sử dụng bộ định thời phần mềm (phi chiếm quyền) thay thế hoàn toàn cho hàm `delay()`, tránh việc block CPU.
* **Máy trạng thái hữu hạn (FSM):** Xử lý nhận dữ liệu UART chống nghẽn và chống rớt gói tin bằng cơ chế Timeout tự động.
* **Chế độ vận hành:** * *Auto:* Tự động điều chỉnh nhiệt độ bám sát Setpoint qua thuật toán PID.
  * *Manual:* Điều khiển trực tiếp cơ cấu chấp hành qua PC.
* **Chế độ an toàn (Safe Mode):** Tự động ngắt toàn bộ cơ cấu chấp hành nếu phát hiện cảm biến lỗi hoặc nhiệt độ nằm ngoài vùng an toàn (-40°C đến 80°C).

## 🛠 Phần cứng sử dụng
* **Vi điều khiển:** STM32F103C8T6 (Bluepill)
* **Cảm biến:** DHT22 (Nhiệt độ & Độ ẩm, giao tiếp 1-Wire)
* **Cơ cấu chấp hành:** Motor DC 12V (Quạt), Điện trở sưởi 3.3V, Module đệm công suất MOSFET
* **Truyền thông:** Module USB to TTL (Baudrate: 115200 bps)

## 📂 Cấu trúc thư mục
```text
📦 PROJECT
 ┣ 📂 comps
 ┃ ┣ 📂 application   # Logic điều khiển, Protocol Handler, PID
 ┃ ┣ 📂 common        # Hàng đợi, định nghĩa Frame, cấu trúc dữ liệu
 ┃ ┣ 📂 mcu_drivers   # Driver cấp thanh ghi (GPIO, USART, PWM)
 ┃ ┗ 📂 platform      # Hardware Abstraction Layer (HAL API)
 ┣ 📂 docs            # Tài liệu dự án, báo cáo
 ┣ 📂 external        # Thư viện bên ngoài (dự phòng mở rộng)
 ┗ 📂 tools           # Script build, cấu hình Keil/STM32CubeIDE/CMake