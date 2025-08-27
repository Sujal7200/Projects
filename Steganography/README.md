# STEG — Streamlit Steganography Toolkit

STEG — Streamlit Steganography Toolkit is a hands-on web app that makes the idea of hiding information inside files both simple and approachable. Instead of digging into low-level code, users can upload images, audio, or video files and embed secret text or documents directly through an easy interface. The app takes care of the tricky parts behind the scenes, like making sure file headers aren’t damaged and giving you control over how the data is hidden with settings such as the starting offset (S), periodicity (L), and mode (fixed or cycle). When it’s time to reveal the hidden message, you just upload the stego file and use the same parameters to extract it. 

A login system keeps things personal, while the public gallery adds a fun way to share and explore what others have created. Built with Streamlit for a clean, responsive UI and MongoDB for managing users, STEG isn’t about bulletproof secrecy but about learning, experimenting, and exploring how steganography really works in practice. It’s perfect for students, hobbyists, and anyone curious about the hidden side of digital files.

> ⚠️ **For learning & demo use.** Do not rely on this project for real-world secrecy. See **Security Notes** below.

---

## Table of Contents
- [Overview](#overview)
- [Key Features](#key-features)
- [How It Works](#how-it-works)
- [Screens & Flow](#screens--flow)
- [Usage Guide](#usage-guide)
- [Capacity & Limits](#capacity--limits)
- [Project Structure](#project-structure)
- [Tech Stack](#tech-stack)
- [Security Notes & TODOs](#security-notes--todos)
- [Roadmap](#roadmap)
- [License](#license)

---

## Overview
**STEG** lets an authenticated user:
- **Embed** a message (typed text and/or uploaded file) into a carrier file using a configurable pattern.
- **Extract** a fixed-length message from a stego file using the same parameters.
- **Browse/Download** publicly saved stego files in a simple gallery.

The app avoids corrupting file headers by skipping the first `max(S, 512)` bits during embed/extract.

---

## Key Features
- 🔐 **Basic Auth** — Username/password check against MongoDB (demo-grade).
- 🧬 **Header-safe Embedding** — Enforces a minimum 512-bit offset.
- ⚙️ **Tunable Parameters** — Choose starting bit **S**, periodicity **L**, and embedding **mode C** (`fixed` or `cycle`).
- 🧾 **Dual Message Sources** — Combine bytes from an uploaded file **and** typed text.
- 📂 **Public Gallery** — Writes stego files to `public_files/` and exposes them for preview & download.
- 🛠️ **Admin Utility** — If the logged-in user is `admin`, a **Clear Gallery** action is available.

---

## How It Works

### Embedding
- Convert carrier bytes and message bytes to bit strings.
- Start at index `S' = max(S, 512)` to avoid file headers.
- Write each message bit into the carrier at positions that advance by a **step**:
  - **C = `fixed`** → step size = `L` bits (e.g., 8).
  - **C = `cycle`** → step cycles through `[8, 16, 28]`.
- Repack bits into bytes and save the modified file to `public_files/stego_<carrier-name>`.

### Extraction
- Start at `S' = max(S, 512)` and read a known number of bits (`message_len_bytes * 8`).
- Use the same stepping rule as embed to reconstruct the hidden bytes and offer them for download.

---

## Screens & Flow
1. **Login** — Users authenticate before accessing Upload/Extract.
2. **Gallery** — Public landing page listing files in `public_files/` (images are previewed inline).
3. **Upload (Embed)** — Choose carrier, add message (file and/or text), set `S`, `L`, `C`, then embed.
4. **Extract** — Upload a stego file, set `S`, `L`, `C`, and the expected **message length (bytes)**.


## Usage Guide

### Embedding Parameters
- **S (Starting Bit)** — First bit index to modify (minimum enforced: 512). The UI auto-suggests by file type; e.g. `jpg/jpeg: 32768`, `png: 8192`, `mp4: 65536`, etc.
- **L (Periodicity)** — Step size in bits (used when `C = fixed`). Common values: `8, 16, 32`.
- **C (Mode)** — `fixed` (constant step `L`) or `cycle` (steps through `8, 16, 28`).

The app pre-checks message size vs. available capacity and warns if the message is too large.

### Extracting
Provide the **exact byte length** of the hidden message. The app reads that many bytes using the same `S/L/C` stepping pattern.

---

## Capacity & Limits
Let `Nbits = len(carrier_bytes) * 8`.

- If **C = fixed**:
  ```
  capacity_bits ≈ max(0, Nbits - S) // L
  ```
- If **C = cycle**:
  ```
  average_step ≈ (8 + 16 + 28) / 3 = 17
  capacity_bits ≈ max(0, Nbits - S) // 17
  ```

> Tip: decrease `S` or `L` to increase capacity, but keep `S ≥` the recommended default for your file type to avoid visible corruption.

---

## Project Structure
```
app.py                 # Streamlit application (this repo's main file)
public_files/          # Output stego files (also used by Gallery)
.streamlit/
  └─ secrets.toml      # Contains MONGO_URI
requirements.txt       # pymongo, streamlit, pillow, numpy
```

---

## Tech Stack
- **Frontend**: Streamlit
- **Backend**: Python
- **Database**: MongoDB (via `pymongo`)
- **Imaging**: Pillow (PIL)

---

## Security Notes & TODOs
- 🔑 **Passwords are stored & checked in plaintext** in this demo. Replace with a secure hash (e.g., `bcrypt`) and salting.
- 🧯 **No rate limiting / lockouts** — add to mitigate brute force.
- 🧪 **Input validation** — validate file types and sizes; consider server-side scanning for malware.
- 🗂️ **Per-user storage** — currently, all stego files are public. Consider per-user/private storage and scoped access.
- 📝 **Provenance** — record parameter metadata (`S`, `L`, `C`, original filename) alongside outputs.
- 🔒 **Secrets management** — ensure `.streamlit/secrets.toml` is not committed.

---

## Roadmap
- [ ] Replace plaintext auth with hashed passwords & sessions
- [ ] Add user roles and per-user galleries
- [ ] Parameter presets per file type with learnable heuristics
- [ ] Optional encryption of message before embed
- [ ] Drag-and-drop multi-file embedding & batch extraction
- [ ] Dockerfile and CI pipeline

---

## License
**Personal License** — Unauthorized copying, distribution, or commercial use is prohibited. For permissions, contact [sujalm7200@gmail.com](mailto:sujalm7200@gmail.com).
