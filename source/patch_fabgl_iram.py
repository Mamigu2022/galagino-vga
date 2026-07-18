"""
patch_fabgl_iram.py - Quita IRAM_ATTR de funciones no criticas de FabGL
Uso: python patch_fabgl_iram.py <ruta_fabgl_src>
"""
import os, sys, re

# Archivos y sus funciones SEGURAS para mover a Flash
# Formato: { archivo: [lista de nombres de funcion] }
PATCHES = {
    "displaycontroller.cpp": [
        "resetPaintState", "getSprite", "hideSprites", "showSprites",
        "execPrimitive", "getActualBrushColor", "getActualPenColor",
        "lineTo", "updateAbsoluteClippingRect", "drawRect", "fillRect",
        "fillEllipse", "renderGlyphsBuffer", "drawPath", "fillPath",
        "absDrawThickLine", "drawBitmap", "absDrawBitmap",
    ],
    "vgacontroller.cpp": [
        "setPixelAt", "absDrawLine", "rawFillRow", "rawInvertRow",
        "swapRows", "drawEllipse", "clear", "VScroll", "HScroll",
        "drawGlyph", "invertRect", "swapFGBG", "copyRect",
        "rawDrawBitmap_Native", "rawDrawBitmap_Mask",
        "rawDrawBitmap_RGBA2222", "rawDrawBitmap_RGBA8888",
    ],
    "graphicsadapter.cpp": [
        "drawScanline_PC_Text_40x25_16Colors",
        "drawScanline_PC_Text_80x25_16Colors",
        "drawScanline_PC_Graphics_320x200_4Colors",
        "drawScanline_PC_Graphics_640x200_2Colors",
        "drawScanline_PC_Graphics_HGC_720x348",
    ],
    "cvbs16controller.cpp": [
        "drawScanlineX1", "drawScanlineX2", "drawScanlineX3",
    ],
    "fabutils.cpp": [
        "merge", "intersection",
    ],
}

def patch_file(filepath, func_names):
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    original = content
    total = 0

    for fn in func_names:
        # Quita IRAM_ATTR cuando va seguido del nombre de funcion
        # Cubre: "IRAM_ATTR FuncName" y "IRAM_ATTR ClassName::FuncName"
        pattern = r'IRAM_ATTR(\s+(?:\w+::)?' + re.escape(fn) + r'\b)'
        new, n = re.subn(pattern, r'\1', content)
        if n:
            content = new
            total += n
            print(f"    [-] IRAM_ATTR removed from {fn} ({n}x)")

    if total:
        bak = filepath + ".bak"
        if not os.path.exists(bak):
            with open(bak, 'w', encoding='utf-8') as f:
                f.write(original)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"  -> {total} cambios guardados (backup: {os.path.basename(bak)})")
    else:
        print(f"  -> Sin cambios")

    return total

def main():
    if len(sys.argv) < 2:
        print("Uso: python patch_fabgl_iram.py <ruta_fabgl_src>")
        sys.exit(1)

    src_dir = sys.argv[1]
    if not os.path.isdir(src_dir):
        print(f"Error: directorio no encontrado: {src_dir}")
        sys.exit(1)

    total = 0
    for fname, funcs in PATCHES.items():
        # Busca el archivo recursivamente
        for root, _, files in os.walk(src_dir):
            if fname in files:
                fpath = os.path.join(root, fname)
                print(f"\n{fname}:")
                total += patch_file(fpath, funcs)
                break
        else:
            print(f"\n{fname}: NO ENCONTRADO")

    print(f"\nTotal: {total} cambios")
    if total:
        print("\nAhora ejecuta:")
        print("  rmdir /s /q .pio\\build\\release_ttgo_vga32_bt\\libb80")
        print("  pio run -e release_ttgo_vga32_bt")

if __name__ == "__main__":
    main()