# 💧 Smart Jar Test System (PDAM Automation) with ESP32

![Project Status](https://img.shields.io/badge/Status-Active-success)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![Language](https://img.shields.io/badge/Language-C%2B%2B%2FArduino-orange)



## 📖 Overview

Proyek ini adalah sistem otomatisasi alat **Jar Test** yang digunakan oleh PDAM (Perusahaan Daerah Air Minum) untuk menentukan dosis koagulan yang tepat dalam proses pengolahan air.

Sistem ini menggunakan **ESP32** sebagai otak utama untuk melakukan dua fungsi sekaligus:
1.  **Kontrol Motor DC:** Mengatur kecepatan pengadukan (RPM) untuk simulasi proses koagulasi dan flokulasi.
2.  **Monitoring Sensor:** Membaca parameter kualitas air (seperti Turbidity, pH, atau Suhu) secara real-time.

Code dalam proyek ini menerapkan prinsip **Modular Programming**, di mana logika motor dan sensor dipisahkan ke dalam file berbeda agar mudah dikelola.

---

## ✨ Features

* **Precise Motor Control:** Pengaturan kecepatan motor (PWM) untuk fase pengadukan cepat dan lambat.
* **Real-time Monitoring:** Pembacaan data sensor secara terus menerus.
* **Dual Core Processing (Optional):** Memanfaatkan fitur ESP32 untuk menjalankan kontrol motor dan pembacaan sensor secara paralel (jika menggunakan FreeRTOS).
* **Serial Data Output:** Data monitoring dapat dilihat melalui Serial Monitor/Plotter.

---

## 📂 File Structure

Struktur file dalam proyek ini dibagi menjadi dua bagian utama untuk kebersihan kode:

```text
├── 📄 main.ino           # File utama (Setup & Loop, memanggil fungsi dari file lain)
├── 📄 motor_control.ino  # Berisi fungsi kontrol kecepatan motor (PWM/Driver Logic)
└── 📄 sensor_read.ino    # Berisi logika pembacaan sensor & kalibrasi
````

-----

## 🛠️ Hardware Requirements

| Component | Description |
| :--- | :--- |
| **Microcontroller** | ESP32 Dev Kit V1 |
| **Actuator** | DC Motor (High Torque) |
| **Motor Driver** | L298N / BTS7960 (sesuaikan dengan driver Anda) |
| **Sensors** | Turbidity Sensor, pH Sensor, or DS18B20 (Temp) |
| **Power Supply** | 12V DC Adapter (for Motors) & 5V (for ESP32) |

-----

## 🔌 Pin Configuration (Wiring)

> **Catatan:** Silakan sesuaikan pin di bawah ini dengan konfigurasi aktual pada kode Anda.

### Motor Driver

| ESP32 Pin | Driver Pin | Function |
| :--- | :--- | :--- |
| GPIO 26 | IN1 | Motor Direction A |
| GPIO 27 | IN2 | Motor Direction B |
| GPIO 14 | ENA | PWM Speed Control |

### Sensors

| ESP32 Pin | Sensor Pin | Function |
| :--- | :--- | :--- |
| GPIO 34 | Analog Out | Turbidity Sensor |
| GPIO 35 | Analog Out | pH Sensor |

-----

## 🚀 Installation & Usage

1.  **Clone Repository**
    ```bash
    git clone [https://github.com/username/project-name.git](https://github.com/username/project-name.git)
    ```
2.  **Open in Arduino IDE**
      * Buka file `main.ino`. Pastikan file `motor_control.ino` dan `sensor_read.ino` berada dalam satu folder yang sama agar terbuka sebagai *tabs*.
3.  **Install Libraries**
      * Pastikan library sensor yang dibutuhkan sudah terinstall (misal: `OneWire`, `DFRobot_PH`, dll).
4.  **Upload**
      * Pilih Board: **DOIT ESP32 DEVKIT V1**.
      * Pilih Port yang sesuai.
      * Klik **Upload**.

-----

## 📝 Code Explanation

### 1\. Motor Logic (`motor_control`)

Bagian ini menangani sinyal PWM. Dalam proses Jar Test, biasanya terdapat fase:

  * **Rapid Mixing (Koagulasi):** Kecepatan tinggi (\~100-150 RPM) selama 1-2 menit.
  * **Slow Mixing (Flokulasi):** Kecepatan rendah (\~20-40 RPM) selama 10-20 menit.

### 2\. Sensor Logic (`sensor_read`)

Menggunakan ADC (Analog to Digital Converter) pada ESP32. Karena ESP32 memiliki resolusi ADC 12-bit (0-4095), kode ini mencakup konversi nilai analog mentah menjadi satuan yang dapat dibaca (NTU untuk kekeruhan, atau nilai pH).

-----

## 🤝 Contributing

Kontribusi sangat diterima\! Jika Anda ingin meningkatkan algoritma PID motor atau menambahkan fitur IoT (seperti Blynk/MQTT), silakan buat *Pull Request*.

## 📄 License

Project ini dilisensikan di bawah [MIT License](https://www.google.com/search?q=LICENSE).

```

---

### Tips Tambahan untuk Anda:

1.  **Cara Menggunakan:** Copy teks di dalam kotak kode di atas, lalu buat file baru bernama `README.md` di dalam folder proyek Anda (atau langsung di GitHub/GitLab).
2.  **Edit Bagian `[ ]`:** Pastikan Anda mengedit bagian Pin Configuration dan Hardware sesuai dengan alat yang benar-benar Anda pakai.
3.  **Image:** Jika Anda punya foto alatnya, ganti `` dengan link gambar atau upload fotonya ke folder repo dan link-kan path-nya.

**Langkah Selanjutnya:**
Apakah Anda ingin saya membantu **menggabungkan (merge)** kedua file kode Anda (sensor & motor) agar berjalan lancar menggunakan **FreeRTOS (Multitasking)**? Ini sangat berguna agar pembacaan sensor tidak mengganggu kestabilan putaran motor.
```
