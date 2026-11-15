# OpenMP Roleplay

Proyek server GTA San Andreas Roleplay berbasis [open.mp](https://open.mp/).
Repository GitHub: [dissaramdhani/openmp-roleplay](https://github.com/dissaramdhani/openmp-roleplay).

## Status

Phase 0 telah selesai: fondasi repository, standar kontribusi, konfigurasi
workspace, dokumentasi, dan CI awal sudah tersedia. Phase 1 telah selesai:
setup dan verifikasi environment, termasuk koneksi MySQL localhost, telah
lulus. Checklist dan bukti verifikasi tersedia pada Issue #5. Phase 2 telah
selesai: fondasi arsitektur Pawn-first, registrasi modul, dokumentasi, dan
pemeriksaan dependency telah lulus quality gate; buktinya tersedia pada Issue
#9 dan PR #8. Phase 3 sedang dikerjakan: loader konfigurasi JSON bertipe untuk
environment development/production sedang dibangun pada Issue #12. Fitur
gameplay dan schema database belum diimplementasikan.

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
├── config/               # Contoh konfigurasi JSON per environment
├── docs/                 # Dokumentasi developer
├── gamemodes/            # Entry point dan core Pawn
├── native/               # C++ configuration plugin dan tes
├── scripts/              # Tooling dan validasi repository
├── tests/                # Pengujian otomatis
├── README.md
└── CONTRIBUTING.md
```

Fondasi GameMode dan aturan arsitektur Pawn ditetapkan pada Phase 2. Phase 3
menambahkan konfigurasi aplikasi dan plugin Infrastructure C++; sistem gameplay
tetap ditambahkan pada phase selanjutnya.

## Memulai

1. Pasang Visual Studio Code dan Git.
2. Buka folder repository di VS Code dan pasang extension yang direkomendasikan.
3. Baca [panduan lingkungan pengembangan](docs/development-environment.md).
4. Baca [panduan core architecture](docs/architecture.md).
5. Ikuti [panduan kontribusi](CONTRIBUTING.md).

Compiler Pawn, open.mp server, MySQL, dan toolchain C/C++ harus dipasang serta
diverifikasi sebelum pengembangan dan pengujian yang memerlukannya. Extension
VS Code saja tidak menyediakan toolchain tersebut.

## Validasi lokal

Dengan Node.js terpasang, jalankan:

```sh
npm ci
npm run lint
npm test
node scripts/validate-workspace-config.js
node scripts/check-pawn-architecture.js
```

CI menjalankan pemeriksaan workspace, lint dan tes JavaScript, pemeriksaan
dependency arsitektur Pawn, serta compile GameMode dengan Pawn dari rilis
open.mp yang dipin dan diverifikasi checksum-nya. CI juga membangun plugin
konfigurasi Windows x86 dan menjalankan tes C++.

Lihat [panduan konfigurasi](docs/configuration.md) untuk pemilihan environment,
schema v1, pengelolaan secret, dan API konfigurasi Pawn.
