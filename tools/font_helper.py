from fontTools.ttLib import TTFont
from fontTools.ttLib.tables._g_l_y_f import Glyph, GlyphComponent
import copy, os

def create_composite_glyph(font, char_sequence):
    """
    Creates a new TrueType composite glyph combining multiple characters (e.g. 'Ja', 'ck', 'Frost')
    """
    cmap = font.getBestCmap()
    hmtx = font['hmtx']
    glyf = font['glyf']
    
    total_advance = 0
    components = []
    
    for ch in char_sequence:
        code = ord(ch)
        if code not in cmap:
            continue
        gname = cmap[code]
        adv, lsb = hmtx[gname]
        
        comp = GlyphComponent()
        comp.glyphName = gname
        comp.flags = 0x0204 # ARGS_ARE_XY_VALUES | ROUND_XY_TO_GRID | MORE_COMPONENTS
        comp.x = total_advance
        comp.y = 0
        components.append(comp)
        
        total_advance += adv

    if not components:
        return None, 0

    components[-1].flags = 0x0200 # remove MORE_COMPONENTS flag on last component
    
    comp_glyph = Glyph()
    comp_glyph.numberOfContours = -1 # indicates composite glyph in TrueType
    comp_glyph.components = components
    
    return comp_glyph, total_advance

print('Composite glyph helper ready!')
