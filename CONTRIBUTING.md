# Panduan Kontribusi

## Alur kerja

1. Periksa scope dan acceptance criteria yang berlaku untuk phase.
2. Jangan mulai phase berikutnya sebelum phase sebelumnya memenuhi Quality Gate.
3. Buat perubahan sekecil mungkin dan pisahkan concern antar-layer.
4. Tambahkan atau perbarui pengujian dan dokumentasi yang terkait langsung.
5. Jalankan validasi yang relevan dan cantumkan hasilnya pada pull request.
6. Jangan menandai pekerjaan selesai tanpa bukti build, pengujian, dan verifikasi
   yang diwajibkan untuk scope tersebut.

## Standar kode dan keamanan

- Ikuti `.editorconfig` dan pengaturan workspace VS Code.
- Gunakan Conventional Commits, misalnya `feat(account): add registration`.
- Pertahankan arah dependency Clean Architecture: Domain tidak bergantung pada
  Infrastructure.
- Validasi state, izin, kepemilikan, jarak, saldo, cooldown, dan frekuensi aksi
  di server.
- Gunakan BCrypt untuk password; jangan menyimpan atau mencatat password maupun
  hash.
- Gunakan prepared statements dan parameter binding untuk query SQL.
- Jangan memasukkan secret, kredensial, data pemain, atau dump database ke Git.
- Hindari operasi blocking pada game thread dan dokumentasikan trade-off teknis
  yang relevan.

## Pull request

Sertakan:

- Ringkasan perubahan dan alasan.
- Phase, requirement, atau issue yang ditangani.
- Perintah pengujian dan hasilnya.
- Pertimbangan keamanan dan performa.
- Dampak pada konfigurasi, database, dan dokumentasi.

Jangan menyatakan Quality Gate lulus jika ada kriteria wajib yang belum
diverifikasi.
