import sys
sys.stdout.reconfigure(encoding='utf-8')

# Target strings
targets = [
    ('ジャックフロスト', 'Jack Frost'),
    ('ジャックランタン', 'Jack Lantern'),
    ('じゃあくフロスト', 'Black Frost'),
    ('ビフロンス', 'Bifrons'),
    ('ドワーフ', 'Dwarf'),
    ('アリス', 'Alice'),
    ('デカラビア', 'Decarabia'),
    ('メッセージ', 'Message'),
    ('メモ', 'Memo'),
    ('コンピューター端末', 'Terminal'),
    ('デモニカOS', 'Demonica OS'),
    ('？？？？', '????'),
    ('Ｊ', 'J')
]

# We can also test 'Pyro Jack' as alternative
all_chars = sorted(list(set(''.join(t[0] for t in targets))))

def find_valid_partitions(target_str, n_parts):
    # Partition target_str into n_parts substrings (some can be empty)
    # n_parts is len of Japanese string
    results = []
    def helper(s, parts_left):
        if parts_left == 1:
            results.append([s])
            return
        for i in range(len(s) + 1):
            head = s[:i]
            tail = s[i:]
            for sub in helper_sub(tail, parts_left - 1):
                results.append([head] + sub)
    def helper_sub(s, parts_left):
        if parts_left == 1:
            return [[s]]
        res = []
        for i in range(len(s) + 1):
            head = s[:i]
            tail = s[i:]
            for sub in helper_sub(tail, parts_left - 1):
                res.append([head] + sub)
        return res
    
    return helper_sub(target_str, n_parts)

# Precompute candidate partitions for small strings:
# Memo (2 chars): "Memo" -> 5 partitions
# Alice (3 chars): "Alice" -> 21 partitions
# Bifrons (5 chars): "Bifrons" -> 462 partitions
# Decarabia (5 chars): "Decarabia" -> 715 partitions
# Message (5 chars): "Message" -> 462 partitions
# Dwarf (4 chars): "Dwarf" -> 56 partitions
# Demonica OS (6 chars): "Demonica OS" -> 4368 partitions

print("Precomputing candidate partitions for strings...")

# For characters in multiple strings, we can solve string by string:
# Order of strings:
# 1. '？？？？' -> '?' each
# 2. 'Ｊ' -> 'J'
# 3. 'メモ' (メ, モ) -> test 5 partitions
# 4. 'メッセージ' (メ, ッ, セ, ー, ジ) -> filters based on メ
# 5. 'ドワーフ' (ド, ワ, ー, フ) -> filters based on ー
# 6. 'アリス' (ア, リ, ス)
# 7. 'ビフロンス' (ビ, フ, ロ, ン, ス) -> filters on フ, ス
# 8. 'デカラビア' (デ, カ, ラ, ビ, ア) -> filters on ビ, ア
# 9. 'デモニカOS' (デ, モ, ニ, カ, O, S) -> filters on デ, モ, カ
# 10. 'コンピューター端末' (コ, ン, ピ, ュ, ー, タ, ー, 端, 末) -> filters on ン, ー
# 11. 'ジャックフロスト' (ジ, ャ, ッ, ク, フ, ロ, ス, ト) -> filters on ジ, ッ, フ, ロ, ス
# 12. 'ジャックランタン' (ジ, ャ, ッ, ク, ラ, ン, タ, ン) -> filters on ジ, ャ, ッ, ク, ラ, ン, タ
# 13. 'じゃあくフロスト' (じ, ゃ, あ, く, フ, ロ, ス, ト) -> filters on フ, ロ, ス, ト

def solve_recursive(str_idx, current_map):
    if str_idx == len(targets):
        return current_map
    
    raw, target = targets[str_idx]
    
    # Check if all chars in raw are already assigned
    all_assigned = all(c in current_map for c in raw)
    if all_assigned:
        formed = ''.join(current_map[c] for c in raw)
        if formed == target:
            res = solve_recursive(str_idx + 1, current_map)
            if res: return res
        return None

    # Otherwise generate partitions of target
    partitions = find_valid_partitions(target, len(raw))
    for part in partitions:
        # Check compatibility with current_map
        valid = True
        new_map = current_map.copy()
        for c, p in zip(raw, part):
            if c in new_map:
                if new_map[c] != p:
                    valid = False
                    break
            else:
                new_map[c] = p
        
        if valid:
            res = solve_recursive(str_idx + 1, new_map)
            if res: return res

    return None

print("Starting solver...")
res_map = solve_recursive(0, {'？': '?', 'Ｊ': 'J'})

if not res_map:
    # Try with Pyro Jack
    targets[1] = ('ジャックランタン', 'Pyro Jack')
    res_map = solve_recursive(0, {'？': '?', 'Ｊ': 'J'})

print("Solver result:", res_map is not None)
if res_map:
    print("\nSUCCESS! FOUND 100% VALID CHAR MAP:")
    for k, v in sorted(res_map.items()):
        print(f"    '{k}': '{v}',")
        
    print("\nVerification:")
    for raw, expected in targets:
        rendered = ''.join(res_map[c] for c in raw)
        print(f"  {raw:12} -> \"{rendered}\" | Expected: \"{expected}\" | OK={rendered == expected}")
