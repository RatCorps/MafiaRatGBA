# MafiaRatGBA

Jogo de estratégia tática para **Game Boy Advance**, feito em C++ com a biblioteca
[Butano](https://github.com/GValiente/butano). O projeto jogável fica em `teste_do_pato/`.

> Ratos mafiosos num tabuleiro. Mova o cursor, selecione uma unidade, veja o alcance
> de movimento e a seta do caminho, e mova pelo grid.

---

## 1. Pré-requisitos (instalar uma vez)

Você precisa da **toolchain do devkitPro** — ela **não** está no repositório (é um
programa de sistema, cada um instala o seu).

1. Instale o **devkitPro**: https://devkitpro.org/wiki/Getting_Started
2. Pelo `dkp-pacman`, instale o grupo de GBA (traz `arm-none-eabi-g++`, `gbafix`, etc.):

   ```bash
   sudo dkp-pacman -S gba-dev
   ```

3. Garanta que as variáveis `DEVKITPRO` / `DEVKITARM` estão no ambiente
   (o instalador normalmente já configura; em geral apontam pra `/opt/devkitpro`).

Um emulador de GBA pra rodar o resultado (ex.: **mGBA**): https://mgba.io

---

## 2. Clonar (com o Butano junto)

O Butano é um **submódulo** git. Clone com `--recursive` pra ele vir junto:

```bash
git clone --recursive https://github.com/RatCorps/MafiaRatGBA.git
```

Se você clonou **sem** o `--recursive`, puxe o submódulo depois:

```bash
git submodule update --init --recursive
```

---

## 3. Buildar

```bash
cd teste_do_pato
make            # compila e gera teste_do_pato.gba
make clean      # limpa o build/ (força regenerar graficos/audio)
```

O `Makefile` acha o Butano sozinho (procura por `butano.mak`). Se precisar forçar:

```bash
make LIBBUTANO=/caminho/para/butano/butano
```

---

## 4. Rodar

Abra o ROM gerado num emulador:

```bash
mgba teste_do_pato/teste_do_pato.gba
```

### Controles

| Botão | Ação |
|-------|------|
| **D-pad** | move o cursor pelo grid (uma casa por toque) |
| **A** | seleciona a unidade sob o cursor → mostra o alcance e a seta |
| **A** (com unidade selecionada) | move a unidade pra uma casa alcançável |

---

## 5. Estrutura

```
MafiaRatGBA/
├── butano/            # submódulo (a biblioteca; não editar aqui)
├── teste_do_pato/     # o jogo
│   ├── src/           # main.cpp, things.cpp/.h (engine de entidades + grid + BFS)
│   ├── graphics/      # sprites e tiles em .bmp indexado + .json do Butano
│   └── Makefile
└── README.md
```

### Fluxo de gráficos (resumo)

O Butano só processa **`.bmp` indexado** (4/8 bpp). Transparência = **índice 0** da paleta.
Cada `.bmp` tem um `.json` ao lado dizendo o tipo (`sprite`, `regular_bg_tiles`,
`bg_palette`...). Depois do `make`, os símbolos aparecem em `build/` (ex.:
`bn_sprite_items_sheet.h`).

---

## Problemas comuns

- **`Could not find butano.mak`** → você esqueceu o submódulo. Rode
  `git submodule update --init --recursive`.
- **`Could not find .../base_tools`** → devkitPro/devkitARM não instalado ou
  `DEVKITPRO` não setado.
- **Autocomplete não acha `sprite_items` / `bg_palette_items`** → rode `make` primeiro
  (esses headers são gerados em `build/`) e reinicie o language server do editor.
