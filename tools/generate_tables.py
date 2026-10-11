import sys

names = [
    ('ジャックフロスト', 'Jack Frost'),
    ('ジャックランタン', 'Pyro Jack'),
    ('じゃあくフロスト', 'Black Frost'),
    ('フロストエース', 'Frost Ace'),
    ('ライホーくん', 'Raiho'),
    ('ビフロンス', 'Bifrons'),
    ('デカラビア', 'Decarabia'),
    ('ドワーフ', 'Dwarf'),
    ('アリス', 'Alice'),
    ('コンピューター端末', 'Terminal'),
    ('メッセージ', 'Message'),
    ('メモ', 'Memo'),
    ('デモニカOS', 'Demonica OS'),
    ('デモニカスーツ', 'Demonica Suit'),
    ('デモニカ', 'Demonica'),
    ('モスマン', 'Mothman'),
    ('ゴブリン', 'Goblin'),
    ('悪魔', 'Demon'),
    ('ピクシー', 'Pixie'),
    ('ガキ', 'Preta'),
    ('スライム', 'Slime'),
    ('セト', 'Seth'),
    ('マーラ', 'Mara'),
    ('クー・フーリン', 'Cu Chulainn'),
    ('フロスト', 'Frost'),
    ('ランタン', 'Lantern'),
    ('Ｊ', 'J'),
    ('？？？？', '????'),
    ('？？？', '???'),
    ('？？', '??')
]

with open('tools/replacements.h', 'w', encoding='utf-8') as f:
    f.write('#pragma once\n\n')
    f.write('struct Replacement {\n')
    f.write('    const wchar_t* from_w;\n')
    f.write('    const wchar_t* to_w;\n')
    f.write('    const char* from_a;\n')
    f.write('    const char* to_a;\n')
    f.write('};\n\n')
    f.write('static struct Replacement g_replacements[] = {\n')
    for jp, en in names:
        u16_escaped = ''.join(f'\\x{ord(c):04X}' for c in jp)
        sjis_bytes = jp.encode('shift_jis', errors='replace')
        sjis_escaped = ''.join(f'\\x{b:02X}' for b in sjis_bytes)
        f.write(f'    // {jp} -> {en}\n')
        f.write(f'    {{ L"{u16_escaped}", L"{en}", "{sjis_escaped}", "{en}" }},\n')
    f.write('    { NULL, NULL, NULL, NULL }\n')
    f.write('};\n')
print('Generated tools/replacements.h successfully!')
