# OpenMP Roleplay

Proyek server GTA San Andreas Roleplay berbasis [open.mp](https://open.mp/).
Repository GitHub: [dissaramdhani/openmp-roleplay](https://github.com/dissaramdhani/openmp-roleplay).

## Status

Phase 0 telah selesai: fondasi repository, standar kontribusi, konfigurasi
workspace, dokumentasi, dan CI awal sudah tersedia. Phase 1 sedang berlangsung
dan dilacak pada Issue #5. Toolchain, termasuk koneksi MySQL localhost, telah
diverifikasi; Phase 1 menunggu review dan merge PR #6 sebelum ditandai selesai.
Fitur gameplay belum diimplementasikan.

## Teknologi

- Pawn untuk GameMode dan logika gameplay.
- C/C++ untuk plugin dan integrasi native.
- MySQL untuk penyimpanan persisten.
- BCrypt untuk hashing dan verifikasi password.
- JavaScript/Node.js untuk API, dashboard, monitoring, dan tooling.
- GitHub Actions untuk CI.

## Prinsip pengembangan

- Terapkan Clean Architecture sesuai kemampuan dan pola yang cocok untuk Pawn.
- Validasi seluruh input dan aksi penting di server; jangan percaya client.
- Jangan menyimpan secret atau password plaintext di repository maupun log.
- Gunakan prepared statements dan parameter binding untuk akses database.
- Hindari operasi blocking pada game thread, termasuk operasi BCrypt.
- Setiap phase wajib melewati Quality Gate sebelum phase berikutnya dimulai.

## Struktur repository

```text
.
├── .github/
│   └── workflows/       # GitHub Actions
├── .vscode/              # Rekomendasi extension, settings, dan tasks
├── docs/                 # Dokumentasi developer
├── scripts/              # Tooling dan validasi repository
├── README.md
└── CONTRIBUTING.md
```

Direktori untuk GameMode, plugin, konfigurasi, dan sistem roleplay akan
ditambahkan sesuai scope phase terkait; struktur arsitektur aplikasi ditetapkan
pada Phase 2.

## Memulai

1. Pasang Visual Studio Code dan Git.
2. Buka folder repository di VS Code dan pasang extension yang direkomendasikan.
3. Baca [panduan lingkungan pengembangan](docs/development-environment.md).
4. Ikuti [panduan kontribusi](CONTRIBUTING.md).

Compiler Pawn, open.mp server, MySQL, dan toolchain C/C++ harus dipasang serta
diverifikasi sebelum pengembangan dan pengujian yang memerlukannya. Extension
VS Code saja tidak menyediakan toolchain tersebut.

## Validasi lokal

Dengan Node.js terpasang, jalankan:

```sh
node scripts/validate-workspace-config.js
```

Validasi ini memeriksa sintaks JSON file konfigurasi workspace VS Code. CI awal
menjalankan pemeriksaan yang sama.
