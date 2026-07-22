#!/usr/bin/env python3
# =============================================================
#  gen_sfx.py  -  Synthesize all chess SFX with the Python
#  standard library only (no downloads, fully offline).
#
#  Run:  python3 gen_sfx.py
#  Output: move.wav capture.wav check.wav castle.wav
#          promote.wav gameover.wav click.wav
# =============================================================
import wave, struct, math

SR = 44100  # sample rate


def _write(name, samples):
    """Write a mono 16-bit PCM WAV, clipping safely to [-1, 1]."""
    with wave.open(name, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        frames = bytearray()
        for s in samples:
            if s > 1.0:
                s = 1.0
            if s < -1.0:
                s = -1.0
            frames += struct.pack("<h", int(s * 32767))
        w.writeframes(bytes(frames))
    print("wrote", name, "(%d samples)" % len(samples))


def env(i, n, attack=0.005, release=0.25):
    """Simple attack/exponential-decay envelope, 0..1."""
    t = i / SR
    dur = n / SR
    a = min(1.0, t / attack) if attack > 0 else 1.0
    # exponential decay over the release tail
    d = math.exp(-(t) / release)
    # fade the very end to avoid a click
    tail = min(1.0, (dur - t) / 0.01) if dur - t < 0.01 else 1.0
    return a * d * max(0.0, tail)


def tone(freq, dur, vol=0.5, attack=0.005, release=0.25, harmonics=(1.0,)):
    n = int(SR * dur)
    out = []
    for i in range(n):
        t = i / SR
        v = 0.0
        for k, amp in enumerate(harmonics, start=1):
            v += amp * math.sin(2 * math.pi * freq * k * t)
        out.append(v * vol * env(i, n, attack, release))
    return out


def sweep(f0, f1, dur, vol=0.5, release=0.2):
    n = int(SR * dur)
    out = []
    for i in range(n):
        t = i / SR
        f = f0 + (f1 - f0) * (i / n)
        out.append(math.sin(2 * math.pi * f * t) * vol * env(i, n, 0.004, release))
    return out


def noise_click(dur, vol=0.4):
    """Short filtered noise burst for a soft UI click (deterministic)."""
    n = int(SR * dur)
    out = []
    prev = 0.0
    seed = 1234567
    for i in range(n):
        # simple LCG so it's reproducible without importing random
        seed = (1103515245 * seed + 12345) & 0x7FFFFFFF
        white = (seed / 0x3FFFFFFF) - 1.0
        prev = prev * 0.6 + white * 0.4  # low-pass -> softer
        out.append(prev * vol * env(i, n, 0.001, 0.03))
    return out


def mix(*layers):
    n = max(len(l) for l in layers)
    out = [0.0] * n
    for l in layers:
        for i, s in enumerate(l):
            out[i] += s
    return out


# ---- move: soft low wooden "tock" ----------------------------
_write("move.wav", tone(220, 0.14, vol=0.55, release=0.09,
                        harmonics=(1.0, 0.3, 0.12)))

# ---- capture: sharper, brighter double-thud ------------------
_write("capture.wav", mix(
    tone(160, 0.16, vol=0.5, release=0.08, harmonics=(1.0, 0.5, 0.25)),
    tone(320, 0.10, vol=0.35, release=0.05),
))

# ---- check: tense two-note alert -----------------------------
_write("check.wav", tone(660, 0.10, vol=0.4, release=0.08) +
                    tone(880, 0.16, vol=0.45, release=0.12))

# ---- castle: two quick wooden taps (rook + king) -------------
_write("castle.wav", tone(240, 0.09, vol=0.5, release=0.06) +
                     [0.0] * int(SR * 0.03) +
                     tone(200, 0.11, vol=0.5, release=0.07))

# ---- promote: rising triumphant sweep ------------------------
_write("promote.wav", mix(
    sweep(440, 990, 0.35, vol=0.4, release=0.3),
    tone(660, 0.35, vol=0.2, release=0.3, attack=0.05),
))

# ---- gameover: descending 3-note chime -----------------------
_write("gameover.wav",
       tone(523, 0.22, vol=0.42, release=0.2) +
       tone(415, 0.22, vol=0.42, release=0.2) +
       tone(311, 0.40, vol=0.45, release=0.35))

# ---- click: soft UI tick -------------------------------------
_write("click.wav", noise_click(0.05, vol=0.5))

print("All SFX generated.")
