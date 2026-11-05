#!/usr/bin/env python3
"""Generate the deliberately unsettling cheeseburger textures for the Mustard and Mayonnaise plugin.

Each surface (bun, cheese, patty) gets a colour map and a normal map covered in tight clusters of
pits, lotus-seed-pod style. Textures tile seamlessly. Pure standard library (zlib PNG writer),
seeded, so the output is reproducible:

    scripts/generate_burger_textures.py plugins/mustardmayo/assets
"""

import math
import random
import struct
import sys
import zlib
from pathlib import Path

SIZE = 512


def write_png(path, pixels, size=SIZE):
    """pixels: flat list of (r, g, b) tuples, row-major."""
    raw = bytearray()
    for y in range(size):
        raw.append(0)  # filter: none
        for x in range(size):
            raw.extend(pixels[y * size + x])

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    Path(path).write_bytes(png)


def value_noise(rng, cells):
    """Smooth tileable noise in [0, 1], as a function of pixel coordinates."""
    grid = [[rng.random() for _ in range(cells)] for _ in range(cells)]
    scale = cells / SIZE

    def sample(x, y):
        gx, gy = x * scale, y * scale
        x0, y0 = int(gx) % cells, int(gy) % cells
        x1, y1 = (x0 + 1) % cells, (y0 + 1) % cells
        fx, fy = gx - int(gx), gy - int(gy)
        fx, fy = fx * fx * (3 - 2 * fx), fy * fy * (3 - 2 * fy)
        top = grid[y0][x0] * (1 - fx) + grid[y0][x1] * fx
        bottom = grid[y1][x0] * (1 - fx) + grid[y1][x1] * fx
        return top * (1 - fy) + bottom * fy

    return sample


def clustered_holes(rng, clusters, per_cluster, radius_range, spread):
    """Holes packed tightly in clusters: list of (x, y, r)."""
    holes = []
    for _ in range(clusters):
        cx, cy = rng.uniform(0, SIZE), rng.uniform(0, SIZE)
        placed = []
        attempts = 0
        target = rng.randint(*per_cluster)
        while len(placed) < target and attempts < target * 40:
            attempts += 1
            r = rng.uniform(*radius_range)
            angle = rng.uniform(0, 2 * math.pi)
            dist = spread * math.sqrt(rng.random())
            x, y = cx + math.cos(angle) * dist, cy + math.sin(angle) * dist
            # Keep a thin wall between neighbouring pits: the packing is what makes it unsettling.
            if all(math.hypot(x - px, y - py) > r + pr + 1.0 for px, py, pr in placed):
                placed.append((x, y, r))
        holes.extend(placed)
    return holes


def render(holes, base, pit, rim_tint, noise, noise_amount):
    """Returns (height, darkness) maps from the hole list."""
    height = [0.0] * (SIZE * SIZE)
    dark = [0.0] * (SIZE * SIZE)
    for hx, hy, r in holes:
        reach = int(r * 1.45) + 2
        for dy in range(-reach, reach + 1):
            for dx in range(-reach, reach + 1):
                d = math.hypot(dx + (hx % 1), dy + (hy % 1)) / r
                if d > 1.45:
                    continue
                i = ((int(hy) + dy) % SIZE) * SIZE + (int(hx) + dx) % SIZE
                if d < 1.0:  # the pit: deep, smooth bowl
                    depth = -(1 - d * d) ** 0.7
                    height[i] = min(height[i], depth)
                    dark[i] = max(dark[i], (1 - d) ** 0.5)
                else:  # raised lip around it
                    lip = 0.25 * math.sin((d - 1) / 0.45 * math.pi)
                    height[i] = max(height[i], lip) if height[i] >= 0 else height[i]
                    dark[i] = max(dark[i], -0.25 * lip)  # rims slightly lighter
    colours = []
    for y in range(SIZE):
        for x in range(SIZE):
            i = y * SIZE + x
            n = 1 + (noise(x, y) - 0.5) * noise_amount
            k = dark[i]
            if k >= 0:
                c = [base[j] * n * (1 - k) + pit[j] * k for j in range(3)]
            else:
                c = [base[j] * n + (rim_tint[j] - base[j]) * -k for j in range(3)]
            colours.append(tuple(max(0, min(255, int(v))) for v in c))
    return height, colours


def normal_map(height, strength):
    out = []
    for y in range(SIZE):
        for x in range(SIZE):
            hl = height[y * SIZE + (x - 1) % SIZE]
            hr = height[y * SIZE + (x + 1) % SIZE]
            hu = height[((y - 1) % SIZE) * SIZE + x]
            hd = height[((y + 1) % SIZE) * SIZE + x]
            nx, ny, nz = (hl - hr) * strength, (hu - hd) * strength, 1.0
            length = math.sqrt(nx * nx + ny * ny + nz * nz)
            out.append(tuple(int((v / length * 0.5 + 0.5) * 255) for v in (nx, ny, nz)))
    return out


SURFACES = {
    # name: clusters, holes per cluster, hole radius (px), cluster spread (px), colours, noise
    "bun": dict(clusters=22, per_cluster=(40, 70), radius=(3.5, 9), spread=48,
                base=(196, 128, 58), pit=(38, 18, 8), rim=(232, 176, 104), noise=(10, 0.35)),
    "cheese": dict(clusters=24, per_cluster=(30, 50), radius=(2.5, 6.5), spread=36,
                   base=(250, 178, 40), pit=(120, 60, 6), rim=(255, 214, 110), noise=(6, 0.15)),
    "patty": dict(clusters=34, per_cluster=(30, 50), radius=(2, 5.5), spread=30,
                  base=(92, 48, 30), pit=(22, 6, 4), rim=(140, 80, 52), noise=(16, 0.5)),
}


def main():
    out_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "plugins/mustardmayo/assets")
    out_dir.mkdir(parents=True, exist_ok=True)
    for name, s in SURFACES.items():
        rng = random.Random(name)  # stable per surface
        holes = clustered_holes(rng, s["clusters"], s["per_cluster"], s["radius"], s["spread"])
        noise = value_noise(rng, s["noise"][0])
        height, colours = render(holes, s["base"], s["pit"], s["rim"], noise, s["noise"][1])
        write_png(out_dir / f"{name}_color.png", colours)
        write_png(out_dir / f"{name}_normal.png", normal_map(height, 3.0))
        print(f"{name}: {len(holes)} pits")


if __name__ == "__main__":
    main()
