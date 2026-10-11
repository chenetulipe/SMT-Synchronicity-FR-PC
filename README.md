# Shin Megami Tensei: Synchronicity Prologue (Traduction Francaise)

Projet de localisation francaise integrale pour le jeu PC **Shin Megami Tensei: Synchronicity Prologue**, compatible avec les versions japonaises officielles (v1.00 et v1.01) ainsi qu'avec la version anglaise.

[![Version](https://img.shields.io/badge/Version-1.0-blue.svg)](#guide-dinstallation)
[![Statut](https://img.shields.io/badge/Statut-Traduction%20compl%C3%A8te-brightgreen.svg)](#contenu-de-la-traduction)
[![Licence](https://img.shields.io/badge/Licence-CC%20BY--NC--SA%204.0-lightgrey.svg)](#architecture-technique-et-legalite)
[![Plateforme](https://img.shields.io/badge/Plateforme-PC%20(Windows)-informational.svg)](#guide-dinstallation)
[![Format de patch](https://img.shields.io/badge/Patch-bsdiff%20binaire-success.svg)](#architecture-technique-et-legalite)

Développé par le studio **Team Ladybug** (*Record of Lodoss War: Deedlit in Wonder Labyrinth*, *Touhou Luna Nights*) sous la supervision d'**Atlus**, *Synchronicity Prologue* est un jeu d'action et de plateformes de type Metroidvania sorti fin 2017 pour célébrer le lancement de *Shin Megami Tensei: Strange Journey Redux*.

Le jeu met en scène Jack Frost et Pyro Jack au coeur du Schwarzwelt dans une mission d'investigation en territoire hostile. N'ayant jamais fait l'objet d'une sortie officielle en dehors du Japon, ce projet propose une adaptation complete en langue francaise, respectant le lore de la licence Megaten et intégrant une gestion intégrale des caracteres accentues.

---

## Sommaire

* [Contenu de la traduction](#contenu-de-la-traduction)
* [Architecture technique et legalite](#architecture-technique-et-legalite)
* [Guide d'installation](#guide-dinstallation)
* [Commandes du jeu](#commandes-du-jeu)
* [Discord Rich Presence](#discord-rich-presence)
* [Credits](#credits)

---

## Contenu de la traduction

La traduction couvre l'ensemble des elements du jeu :

* **Dialogues et narration (`dial.dat`)** : Traduction intégrale des cinematiques, des echanges avec les boss, des dialogues avec les demons allies et des annonces systeme du Demonica OS.
* **Graphismes et interface (`megaten.exe`)** : 541 textures adaptees en francais, incluant l'ecran titre, les jauges de sante et de magie (PV / PM), les indications d'affinites elementaires, la mini-carte, les menus d'inventaire et les ecrans de Game Over.
* **Typographie accentuee (`cinecaption227.TTF`)** : Integration vectorielle des accents et caracteres specifiques francais (e accent aigu, e accent grave, e accent circonflexe, cedille, etc.) sans alterer le style d'affichage original.
* **Lanceur de configuration (`synchronicity.exe`)** : Traduction complete de l'utilitaire de reglages (attribution des touches, gestion des manettes, volume sonore, affichage fenetre et plein ecran, synchronisation verticale).
* **Manuel officiel (PDF)** : Le livret de jeu officiel completement traduit en francais haute definition, avec illustrations et explications detaillees sur le systeme de faiblesses elementaires.
* **Module Discord Rich Presence (`winmm.dll`)** : Affichage dynamique de votre progression sur votre profil Discord.

---

## Architecture technique et legalite

Le patcher est concu pour respecter scrupuleusement la propriete intellectuelle de l'editeur :

1. **Aucun binaire sous copyright distribue** : Ce correctif ne contient ni le fichier `megaten.exe` original, ni de fichiers de musiques ou de sons d'Atlus.
2. **Patchs differentiels binaires bsdiff** : Les modifications sont distribuees sous la forme de fichiers de differences (`.bsdiff`) de moins de 2 Mo au total. Ces correctifs s'appliquent directement sur votre propre copie legale locale du jeu.
3. **Detection automatique de version** : Le patcher analyse la signature SHA-256 de votre jeu et applique le delta approprie selon que vous possediez la version japonaise originale v1.01 ou la version anglaise.
4. **Securite des donnees** : Une copie de sauvegarde (`megaten.exe.bak`) est creee automatiquement avant la moindre modification.
5. **Proxy systeme ouvert** : La DLL `winmm.dll` est un proxy open-source assurant le relais des appels de menus et la communication Discord RPC sans modification permanente des bibliotheques de votre systeme d'exploitation.

---

## Guide d'installation

### Prerequis

* Un ordinateur equipe de Windows 10 ou Windows 11.
* Python 3.8 ou version superieure installe sur la machine.
* Une copie locale du jeu (dossier original contenant `synchronicity.exe` et le sous-dossier `data`, ou archive `.rar` / `.zip` correspondante).

### Procedure d'application

1. Telechargez l'archive `SMT-Synchronicity-FR-Patcher-v1.0.zip` depuis la page des publications du projet.
2. Decompressez le dossier du patcher sur votre ordinateur.
3. Lancez le fichier `lancer_patcher.bat`.
4. L'interface s'ouvre dans votre navigateur par defaut (`http://localhost:8765`).
5. Renseignez l'emplacement de votre jeu (ou selectionnez directement l'archive d'origine du jeu).
6. Cliquez sur le bouton "Appliquer le Patch FR". L'operation s'effectue en une fraction de seconde.
7. Lancez `synchronicity.exe` pour demarrer votre partie.

---

## Commandes du jeu

### Configuration Clavier par defaut

| Action | Touche assignee |
|---|---|
| Deplacement (Haut / Bas / Gauche / Droite) | Fleches directionnelles |
| Attaque principale (Tir) | Z |
| Saut | X |
| Changer de protagoniste (Jack Frost / Pyro Jack) | C |
| Competence magique | V |
| Course (Dash) | Double pression sur Gauche ou Droite |
| Menu Pause | Echap / Entree |

La configuration des touches peut etre redefinie a tout moment via l'onglet Commandes du lanceur `synchronicity.exe`.

### Prise en charge des manettes

Le jeu reconnait les manettes Xbox, PlayStation et les peripheriques USB standards (XInput et DirectInput). La configuration des boutons s'effectue dans le menu Manette du lanceur.

---

## Discord Rich Presence

L'integration Discord fournie par le module `winmm.dll` affiche sur votre profil :

* Le nom du jeu : *Shin Megami Tensei: Synchronicity Prologue*
* Le personnage actuellement actif (Jack Frost ou Pyro Jack)
* Le secteur du labyrinthe actuellement explore
* La duree de la session en cours

Cette fonctionnalite s'active automatiquement au lancement du jeu lorsque le client Discord est ouvert sur votre ordinateur.

---

## Credits

* **Atlus / Sega** : Creation de la franchise *Shin Megami Tensei* et supervision du titre.
* **Team Ladybug / Den-fami Nico Game Magazine** : Developpement original de *Synchronicity Prologue*.
* **Equipe de Traduction FR** : Traduction des textes et du manuel, lettrage graphique, programmation du patcher et module Discord RPC.
* **bsdiff** : Algorithme de patch differentiel binaire par Colin Percival.

Projet non officiel a visee communautaire et sans but lucratif. Tous les droits relatifs a la licence et a l'univers de Shin Megami Tensei appartiennent a Atlus / Sega.
