# Kompresi Gambar Quadtree

Proyek ini adalah implementasi sederhana dari algoritma kompresi gambar lossy menggunakan struktur data Quadtree dalam C++. Program ini membaca sebuah gambar, mengompresnya berdasarkan kriteria error yang dipilih dan threshold yang ditentukan, lalu menyimpan gambar hasil kompresi.

Proyek ini menggunakan library [CImg](https://cimg.eu/) untuk pemrosesan gambar.

## Fitur

- Kompresi gambar menggunakan algoritma Quadtree.
- Mendukung beberapa metode perhitungan error untuk mengontrol kualitas kompresi:
  - **Variance**: Berdasarkan varians warna dalam sebuah blok.
  - **Mean Absolute Deviation (MAD)**: Berdasarkan rata-rata selisih absolut warna dari rata-rata blok.
  - **Max Pixel Difference**: Berdasarkan selisih absolut maksimum warna piksel dari rata-rata blok.
  - **Entropy**: Berdasarkan entropy (keacakan) warna dalam sebuah blok.
- Input interaktif untuk menentukan:
  - Path gambar input.
  - Metode error yang digunakan.
  - Nilai threshold (batas error untuk pembagian blok).
  - Ukuran blok minimum (dalam piksel persegi) sebelum pembagian dihentikan.
  - Path gambar output.
- Menampilkan statistik kompresi (waktu eksekusi, ukuran file sebelum/sesudah, rasio kompresi, dll.).

## Dependensi

Untuk mengompilasi dan menjalankan proyek ini, Anda memerlukan:

1. **Compiler C++** yang mendukung C++20 (misalnya, g++ versi 10 atau lebih baru).
2. **Make**: Utilitas build standar.
3. **Library CImg**: Sudah disertakan dalam direktori `src/`.
4. **Library Pendukung CImg**:
   - **libpng**: Untuk membaca/menulis file PNG.
   - **zlib**: Dependensi untuk libpng.
   - **libpthread**: Untuk dukungan multithreading (digunakan oleh CImg).

**Instalasi Dependensi (Contoh untuk Debian/Ubuntu):**

```bash
sudo apt update
sudo apt install build-essential make libpng-dev zlib1g-dev
```

## Cara Kompilasi

1. Clone repositori ini (jika belum).
2. Navigasi ke direktori root proyek di terminal Anda.
3. Jalankan perintah `make`:

   ```bash
   make
   ```

   atau

   ```bash
   make all
   ```

   Ini akan mengompilasi kode sumber dan menempatkan file executable (`main`) di dalam direktori `bin/`.

## Cara Menjalankan

Ada dua cara untuk menjalankan program setelah kompilasi:

1. **Menggunakan Make:**

   ```bash
   make run
   ```

   Perintah ini akan menjalankan executable `bin/main`.

2. **Langsung Menjalankan Executable:**

   ```bash
   ./bin/main
   ```

Program akan meminta Anda secara interaktif untuk memasukkan detail berikut:

- Path gambar input.
- Pilihan metode error (angka 1-4).
- Nilai threshold (angka desimal >= 0, perhatikan rentang efektif yang disarankan).
- Ukuran blok minimum (angka integer >= 1).
- Path gambar output.

## Anggota

| Nama          | Nomor ID |
| ------------- | -------- |
| Felix Chandra | 13523012 |
| Ahmad Ibrahim | 13523089 |
