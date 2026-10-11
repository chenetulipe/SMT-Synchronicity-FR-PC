import os, shutil, sys
from fontTools.ttLib import TTFont
from fontTools.ttLib.tables._g_l_y_f import Glyph, GlyphComponent

def patch_font(src_path, dst_path):
    print(f'Patching {os.path.basename(src_path)} -> {os.path.basename(dst_path)}...')
    font = TTFont(src_path)
    
    glyf = font['glyf']
    hmtx = font['hmtx']
    cmap = font.getBestCmap()
    
    # Clean non-conflicting character mapping
    char_map = {
        # --- Fullwidth & Special ---
        'Ｊ': 'J',
        '？': '?',
        
        # --- Memo & Message ---
        'メ': 'Me',
        'モ': 'mo',
        'ッ': '',
        'セ': 'ssage',
        'ー': '',
        'ジ': '',
        
        # --- Demonica OS ---
        'デ': 'De',
        'ニ': 'nica OS',
        'カ': '',
        
        # --- Dwarf ---
        'ド': 'Dwarf',
        'ワ': '',
        'フ': '',
        
        # --- Alice ---
        'ア': '',
        'リ': 'Alice',
        'ス': '',
        
        # --- Jack Frost & Black Frost ---
        'ク': '',
        'ャ': 'Jack ',
        'じ': 'Black ',
        'ゃ': '',
        'あ': '',
        'く': '',
        'ロ': '',
        'ト': 'Frost',
        
        # --- Lantern / Terminal / Bifrons ---
        'ラ': 'Lantern',
        'ン': '',
        'タ': '',
        'コ': 'Terminal',
        'ピ': '',
        'ュ': '',
        '端': '',
        '末': '',
        'ビ': 'Bifrons',
    }
    
    empty_glyph = Glyph()
    empty_glyph.numberOfContours = 0
    
    for ch, replacement_str in char_map.items():
        code = ord(ch)
        if code not in cmap:
            gname = f'uni{code:04X}'
            cmap[code] = gname
            font['glyf'][gname] = Glyph()
            font['hmtx'][gname] = (0, 0)
        else:
            gname = cmap[code]
            
        if not replacement_str:
            glyf[gname] = copy_glyph(empty_glyph)
            hmtx[gname] = (0, 0)
            continue
            
        total_adv = 0
        components = []
        for c in replacement_str:
            c_code = ord(c)
            if c_code not in cmap:
                continue
            c_gname = cmap[c_code]
            adv, lsb = hmtx[c_gname]
            
            comp = GlyphComponent()
            comp.glyphName = c_gname
            comp.flags = 0x0202 # ARGS_ARE_XY_VALUES | SCALED_COMPONENT_OFFSET
            comp.x = total_adv
            comp.y = 0
            components.append(comp)
            total_adv += adv
            
        if components:
            comp_g = Glyph()
            comp_g.numberOfContours = -1
            comp_g.components = components
            glyf[gname] = comp_g
            hmtx[gname] = (total_adv, 0)
            
    font.save(dst_path)
    print(f'[OK] Saved {dst_path}')

def copy_glyph(g):
    import copy
    return copy.deepcopy(g)

font_dir = r'Shin Megami Tensei - Synchronicity (English)\Shin Megami Tensei - Synchronicity (English)\data\font'

# Ensure game is not locking files
os.system('taskkill /f /im megaten.exe 2>nul')

patch_font(os.path.join(font_dir, 'cinecaption227.TTF.bak'), os.path.join(font_dir, 'cinecaption227.TTF'))
patch_font(os.path.join(font_dir, 'nukamiso_2004_beta08.ttf.bak'), os.path.join(font_dir, 'nukamiso_2004_beta08.ttf'))
print('All dialogue fonts patched successfully!')
