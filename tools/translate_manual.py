import fitz
from pathlib import Path

def translate_manual(pdf_in_path, pdf_out_path):
    doc = fitz.open(pdf_in_path)
    page = doc[0]

    bg_color = (51/255, 49/255, 50/255)
    gold_color = (255/255, 203/255, 4/255)
    white_color = (1, 1, 1)
    yellow_color = (255/255, 242/255, 0)
    cyan_color = (0/255, 185/255, 242/255)
    red_color = (237/255, 28/255, 36/255)
    dark_color = (51/255, 49/255, 50/255)
    black_fill = (0, 0, 0)

    f_reg = r"C:\Windows\Fonts\segoeui.ttf"
    f_bold = r"C:\Windows\Fonts\segoeuib.ttf"

    # Surgical redactions
    redactions = [
        # Main Title
        (fitz.Rect(40, 35, 555, 78), bg_color),

        # Section Titles
        (fitz.Rect(48, 78, 140, 102), bg_color),   # Histoire
        (fitz.Rect(48, 142, 145, 166), bg_color),  # Demarrage
        (fitz.Rect(48, 253, 136, 276), bg_color),  # Commandes
        (fitz.Rect(48, 375, 150, 398), bg_color),  # Interface
        (fitz.Rect(48, 556, 130, 580), bg_color),  # Sauvegarde
        (fitz.Rect(288, 556, 490, 580), bg_color), # Changement de heros
        (fitz.Rect(48, 707, 180, 730), bg_color),  # Config requise
        (fitz.Rect(288, 707, 390, 730), bg_color), # Quitter

        # Story text
        (fitz.Rect(60, 97, 550, 137), bg_color),

        # Demarrage bullets (strictly left of launcher screenshot at 277)
        (fitz.Rect(65, 168, 275, 242), bg_color),

        # Commandes note & subtitles
        (fitz.Rect(130, 258, 260, 275), bg_color),
        (fitz.Rect(275, 275, 395, 290), bg_color),
        (fitz.Rect(275, 328, 385, 343), bg_color),

        # Commandes table text (strictly inside the black cells, preserving outer & inner borders)
        # Table outer box: [64.9, 279.6, 269.9, 360.6], lines at x=95.1, 125.2
        (fitz.Rect(65.5, 280.2, 124.6, 293.0), black_fill), # Row 1 full left span
        (fitz.Rect(65.5, 293.7, 94.5, 360.0), black_fill),  # Col 1 rows 2-6
        (fitz.Rect(95.6, 293.7, 124.6, 360.0), black_fill), # Col 2 rows 2-6
        (fitz.Rect(125.8, 280.2, 269.3, 360.0), black_fill), # Col 3 all rows

        # Interface pointers around screenshot
        (fitz.Rect(58, 394, 295, 410), bg_color),
        (fitz.Rect(140, 538, 230, 552), bg_color),

        # Interface table text (strictly inside cells, line at x=359.1)
        (fitz.Rect(293.3, 416.0, 358.5, 538.0), black_fill),
        (fitz.Rect(359.7, 416.0, 539.3, 538.0), black_fill),

        # Sauvegarde text
        (fitz.Rect(60, 659, 285, 698), bg_color),

        # Speech bubble text only (oval is [342.9, 583.9, 517.1, 599.9])
        (fitz.Rect(347.0, 585.5, 514.0, 598.0), gold_color),

        # Character switching text
        (fitz.Rect(375, 616, 545, 698), bg_color),

        # Specs table (strictly inside cells, lines at x=84.5, 187.1, 221.8)
        (fitz.Rect(65.5, 734.4, 84.0, 773.4), black_fill),
        (fitz.Rect(85.0, 734.4, 186.5, 773.4), black_fill),
        (fitz.Rect(187.6, 734.4, 221.2, 773.4), black_fill),
        (fitz.Rect(222.4, 734.4, 282.8, 773.4), black_fill),

        # Quitter text & warning
        (fitz.Rect(300, 728, 545, 780), bg_color),
    ]

    for rect, fill in redactions:
        page.add_redact_annot(rect, fill=fill)
    page.apply_redactions()

    # 1. Main Title
    page.insert_textbox(fitz.Rect(40, 42, 552, 76), "SHIN MEGAMI TENSEI: SYNCHRONICITY PROLOGUE",
                        fontfile=f_bold, fontsize=18, color=white_color, align=fitz.TEXT_ALIGN_CENTER)

    # 2. Section Titles
    titles = [
        (fitz.Point(50, 95), "HISTOIRE"),
        (fitz.Point(50, 160), "DÉMARRAGE DU JEU"),
        (fitz.Point(50, 270), "COMMANDES"),
        (fitz.Point(50, 392), "INTERFACE DE JEU"),
        (fitz.Point(50, 574), "SAUVEGARDE"),
        (fitz.Point(292, 574), "CHANGEMENT DE HÉROS"),
        (fitz.Point(50, 724), "CONFIGURATION REQUISE"),
        (fitz.Point(292, 724), "QUITTER LE JEU"),
    ]
    for pt, txt in titles:
        page.insert_text(pt, txt, fontfile=f_bold, fontsize=12.5, color=gold_color)

    # 3. Story text
    story_lines = [
        "Un jour, un mystérieux message parvint sur l'ordinateur... « Lance le programme ci-joint et forge l'avenir... »",
        "Ce logiciel, baptisé « Synchro », liait la conscience de son hôte aux démons du Schwarzwelt pour les contrôler.",
        "Quel était le dessein derrière ce programme... ? Et quelle aventure allait ainsi débuter au coeur du Schwarzwelt ?",
    ]
    for i, line in enumerate(story_lines):
        page.insert_text(fitz.Point(66, 108.5 + i * 11.5), line, fontfile=f_reg, fontsize=7.6, color=white_color)

    # 4. Demarrage bullets
    bullets = [
        ("• Lancez « synchronicity.exe » dans le dossier", white_color),
        ("  pour afficher le lanceur de jeu.", white_color),
        ("• « Key Config » configure les touches.", white_color),
        ("• « Sound Volume » ajuste le volume sonore.", white_color),
        ("• Cliquez sur « RUN » pour lancer le jeu !", yellow_color),
    ]
    b_y = [174.0, 184.0, 197.0, 211.0, 225.0]
    for (b_txt, b_col), y in zip(bullets, b_y):
        page.insert_text(fitz.Point(75, y), b_txt, fontfile=f_reg, fontsize=7.1, color=b_col)

    # 5. Commandes Header & Subtitles
    page.insert_text(fitz.Point(138, 269), "※ Configuration modifiable dans le menu « Commandes » du lanceur",
                     fontfile=f_reg, fontsize=5.2, color=white_color)
    page.insert_text(fitz.Point(277, 287), "Configuration initiale de la manette",
                     fontfile=f_bold, fontsize=6.8, color=white_color)
    page.insert_text(fitz.Point(277, 338), "Configuration initiale du clavier",
                     fontfile=f_bold, fontsize=6.8, color=white_color)

    # Commandes Table
    cmd_rows = [
        ("Flèches", "", "Déplacement du curseur / personnage"),
        ("X", "1", "Confirmer  /  Attaque"),
        ("Z", "2", "Annuler  /  Saut"),
        ("C", "3", "Magie  (maintenir pour permuter)"),
        ("V", "4", "Changer de personnage"),
        ("Espace", "5", "Pause"),
    ]
    y_start = 286.0
    row_h = 13.5
    for i, (k_col, j_col, a_col) in enumerate(cmd_rows):
        y = y_start + i * row_h
        page.insert_text(fitz.Point(68, y), k_col, fontfile=f_bold, fontsize=7.2, color=white_color)
        if j_col:
            page.insert_text(fitz.Point(107, y), j_col, fontfile=f_bold, fontsize=7.2, color=white_color)
        page.insert_text(fitz.Point(129, y), a_col, fontfile=f_reg, fontsize=6.8, color=white_color)

    # 6. Interface Pointers
    page.insert_text(fitz.Point(60, 405), "Lune & Niveau", fontfile=f_reg, fontsize=5.8, color=white_color)
    page.insert_text(fitz.Point(118, 405), "PV / PM", fontfile=f_reg, fontsize=5.8, color=white_color)
    page.insert_text(fitz.Point(155, 405), "Magie", fontfile=f_reg, fontsize=5.8, color=white_color)
    page.insert_text(fitz.Point(202, 405), "Héros actif", fontfile=f_reg, fontsize=5.8, color=white_color)
    page.insert_text(fitz.Point(252, 405), "Macca", fontfile=f_reg, fontsize=5.8, color=white_color)
    page.insert_text(fitz.Point(145, 547), "Personnage contrôlé", fontfile=f_reg, fontsize=5.8, color=white_color)

    # Interface Table on the right
    hud_rows = [
        ("Personnage contrôlé", "Le héros actuellement dirigé par le joueur."),
        ("PV (HP)", "Diminuent lors des dégâts. À 0 PV, la partie est terminée."),
        ("PM (MP)", "Énergie requise pour lancer les compétences magiques."),
        ("Magie", "Sort actif (maintenez la touche Magie pour permuter)."),
        ("Phase lunaire", "Évolue en marchant. Plus elle est pleine, plus les PM montent."),
        ("Niveau", "Gagner de l'EXP renforce le niveau, l'attaque et la défense."),
        ("Macca (argent)", "Monnaie obtenue sur les démons, pour acheter des objets."),
    ]
    y_hud = 423.5
    for i, (name, desc) in enumerate(hud_rows):
        y = y_hud + i * 13.7
        page.insert_text(fitz.Point(296, y), name, fontfile=f_bold, fontsize=6.7, color=white_color)
        page.insert_text(fitz.Point(363, y), desc, fontfile=f_reg, fontsize=5.8, color=white_color)

    # Last HUD row
    page.insert_text(fitz.Point(296, 526.0), "Héros actif", fontfile=f_bold, fontsize=6.7, color=white_color)
    page.insert_text(fitz.Point(363, 521.0), "Après avoir rallié Lantern, permutez librement entre Frost", fontfile=f_reg, fontsize=5.8, color=white_color)
    page.insert_text(fitz.Point(363, 531.0), "et Lantern avec la touche « Changer de personnage ».", fontfile=f_reg, fontsize=5.8, color=white_color)

    # 7. Sauvegarde Text
    save_lines = [
        "Placez-vous devant la stèle d'un terminal et appuyez sur la flèche [Haut]",
        "pour sauvegarder votre progression dans le Schwarzwelt.",
        "En cas de Game Over, vous reprendrez depuis votre dernière sauvegarde.",
    ]
    for i, line in enumerate(save_lines):
        page.insert_text(fitz.Point(62, 672 + i * 10.5), line, fontfile=f_reg, fontsize=7.0, color=white_color)

    # 8. Character Switching Speech Bubble & Explanation
    page.insert_text(fitz.Point(358, 594.0), "Appuyez sur Changer pour permuter !",
                     fontfile=f_bold, fontsize=7.0, color=(0.2, 0.19, 0.19))

    switch_y = 626.0
    page.insert_text(fitz.Point(380, switch_y), "Frost résiste à la glace, ", fontfile=f_bold, fontsize=6.8, color=cyan_color)
    page.insert_text(fitz.Point(454, switch_y), "Lantern résiste au feu :", fontfile=f_bold, fontsize=6.8, color=red_color)

    switch_lines = [
        "adaptez votre héros aux attaques",
        "pour annuler tous les dégâts !",
        "De plus, Lantern flotte et atteint des",
        "zones en hauteur inaccessibles à Frost.",
        "Analysez la situation et utilisez le bon",
        "héros pour progresser dans l'aventure.",
    ]
    for i, line in enumerate(switch_lines):
        page.insert_text(fitz.Point(380, 637.5 + i * 11.0), line, fontfile=f_reg, fontsize=6.8, color=white_color)

    # 9. System Requirements Table
    spec_rows = [
        ("OS", "Windows 7 ou supérieur", "Mémoire", "500 Mo de RAM"),
        ("CPU", "Intel Core 2 Duo", "DirectX", "Version 9.0c"),
        ("GPU", "Direct3D 9 compatible", "Disque", "80 Mo libres"),
    ]
    y_spec = 741.0
    for i, (k1, v1, k2, v2) in enumerate(spec_rows):
        y = y_spec + i * 13.5
        page.insert_text(fitz.Point(66, y), k1, fontfile=f_bold, fontsize=6.8, color=white_color)
        page.insert_text(fitz.Point(88, y), v1, fontfile=f_reg, fontsize=6.7, color=white_color)
        page.insert_text(fitz.Point(189, y), k2, fontfile=f_bold, fontsize=6.8, color=white_color)
        page.insert_text(fitz.Point(224, y), v2, fontfile=f_reg, fontsize=6.7, color=white_color)

    # 10. Quitter Text & Note
    quit_lines = [
        "Sélectionnez EXIT dans le menu Pause ou pressez la touche Échap",
        "pour revenir à l'écran-titre.",
        "Dans le menu principal, choisissez EXIT pour fermer le jeu.",
    ]
    for i, line in enumerate(quit_lines):
        page.insert_text(fitz.Point(304, 739.0 + i * 10.0), line, fontfile=f_reg, fontsize=7.0, color=white_color)

    page.insert_text(fitz.Point(304, 773.0), "※ En cas de blocage imprévu du jeu, appuyez simultanément sur Alt et F4 pour forcer la fermeture.",
                     fontfile=f_reg, fontsize=4.8, color=white_color)

    doc.save(pdf_out_path)
    print(f"French PDF saved successfully to: {pdf_out_path}")

if __name__ == "__main__":
    src_pdf = r"C:\Users\nolan\Downloads\pdf syncro\Manuel Shin Megami Tensei Synchronicity.pdf"
    out_pdf = r"C:\Users\nolan\Downloads\pdf syncro\Manuel Shin Megami Tensei Synchronicity - FR.pdf"
    translate_manual(src_pdf, out_pdf)
