#!/usr/bin/env python3
"""
Gera o JSON do Butano para um spritesheet .bmp.

O Butano ja fatia a folha em grid sozinho: basta o tamanho de cada sprite
(width/height). Uma folha WxH gera (W/w)*(H/h) frames num unico sprite_item,
acessados por indice em runtime:

    bn::sprite_items::<nome>.create_sprite(x, y, indice);

Uso:
    python3 gerar_spritesheet_json.py sheet.bmp          # sprites 16x16
    python3 gerar_spritesheet_json.py sheet.bmp 32 32    # sprites 32x32
"""

import struct
import sys
from pathlib import Path

VALID = {8, 16, 32, 64}  # tamanhos de sprite validos no GBA


def bmp_size(path):
    """Le largura/altura direto do header BMP (sem dependencias externas)."""
    with open(path, "rb") as f:
        if f.read(2) != b"BM":
            raise ValueError(f"{path} nao e um BMP")
        f.seek(18)
        w, h = struct.unpack("<ii", f.read(8))
    return w, abs(h)  # altura pode ser negativa (top-down)


def gerar(bmp_path, sw=16, sh=16):
    if sw not in VALID or sh not in VALID:
        raise ValueError(f"tamanho de sprite invalido: {sw}x{sh} (validos: {sorted(VALID)})")

    sheet_w, sheet_h = bmp_size(bmp_path)
    if sheet_w % sw or sheet_h % sh:
        raise ValueError(f"folha {sheet_w}x{sheet_h} nao e divisivel por {sw}x{sh}")

    frames = (sheet_w // sw) * (sheet_h // sh)
    out = Path(bmp_path).with_suffix(".json")
    out.write_text(
        "{\n"
        '    "type": "sprite",\n'
        f'    "width": {sw},\n'
        f'    "height": {sh}\n'
        "}\n"
    )
    print(f"{out.name}: folha {sheet_w}x{sheet_h}, sprites {sw}x{sh} -> {frames} frames (indices 0..{frames - 1})")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("uso: python3 gerar_spritesheet_json.py <arquivo.bmp> [width] [height]")
        sys.exit(1)

    bmp = sys.argv[1]
    w = int(sys.argv[2]) if len(sys.argv) > 2 else 16
    h = int(sys.argv[3]) if len(sys.argv) > 3 else 16
    gerar(bmp, w, h)
