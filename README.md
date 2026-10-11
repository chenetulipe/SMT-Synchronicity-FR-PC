<p align="center">
  <img src="assets/logo.png" width="340" alt="Shin Megami Tensei Synchronicity Prologue Logo">
</p>

# Shin Megami Tensei: Synchronicity Prologue (Traduction Française)

Projet de localisation française intégrale pour le jeu PC **Shin Megami Tensei: Synchronicity Prologue**, compatible avec les versions japonaises officielles (v1.00 et v1.01) ainsi qu'avec la version anglaise.

[![Version](https://img.shields.io/badge/Version-1.0-blue)](#guide-dinstallation)
[![Statut](https://img.shields.io/badge/Statut-Traduction_Complete-brightgreen)](#contenu-de-la-traduction)
[![Licence](https://img.shields.io/badge/Licence-CC_BY--NC--SA_4.0-lightgrey)](#architecture-technique-et-legalite)
[![Plateforme](https://img.shields.io/badge/Plateforme-PC_Windows-informational)](#guide-dinstallation)
[![Format de patch](https://img.shields.io/badge/Patch-bsdiff_binaire-success)](#architecture-technique-et-legalite)

Développé par le studio **Team Ladybug** (*Record of Lodoss War: Deedlit in Wonder Labyrinth*, *Touhou Luna Nights*) sous la supervision d'**Atlus**, *Synchronicity Prologue* est un jeu d'action et de plateformes de type Metroidvania sorti fin 2017 pour célébrer le lancement de *Shin Megami Tensei: Strange Journey Redux*.

Le jeu met en scène Jack Frost et Pyro Jack au cœur du Schwarzwelt dans une mission d'investigation en territoire hostile. N'ayant jamais fait l'objet d'une sortie officielle en dehors du Japon, ce projet propose une adaptation complète en langue française, respectant l'univers de la licence Megaten et intégrant une gestion intégrale des caractères accentués.

---

## Sommaire

* [Contenu de la traduction](#contenu-de-la-traduction)
* [Architecture technique et légalité](#architecture-technique-et-legalite)
* [Guide d'installation](#guide-dinstallation)
* [Commandes du jeu](#commandes-du-jeu)
* [Discord Rich Presence](#discord-rich-presence)
* [Crédits](#credits)

---

## Contenu de la traduction

La traduction couvre l'ensemble des éléments du jeu :

* **Traduction des dialogues (`dial.dat`)** : Traduction intégrale de l'ensemble des dialogues du jeu en français.
* **Graphismes et interface (`megaten.exe`)** : 541 textures adaptées en français, incluant l'écran-titre, les jauges de santé et de magie (PV / PM), les indications d'affinités élémentaires, la mini-carte, les menus d'inventaire et les écrans de Game Over.
* **Typographie accentuée (`cinecaption227.TTF`)** : Intégration vectorielle des accents et caractères spécifiques français (é, è, ê, à, ç, œ, etc.) sans altérer le style d'affichage original.
* **Lanceur de configuration (`synchronicity.exe`)** : Traduction complète de l'utilitaire de réglages (attribution des touches, gestion des manettes, volume sonore, affichage fenêtré et plein écran, synchronisation verticale).
* **Manuel officiel (PDF)** : Traduction complète du manuel de jeu officiel au format PDF haute définition, incluant la présentation de l'histoire, la configuration requise, les commandes détaillées, la lecture de l'interface en jeu, le fonctionnement des terminaux de sauvegarde et les règles de permutation entre Jack Frost et Pyro Jack.
* **Module Discord Rich Presence (`winmm.dll`)** : Affichage dynamique de votre progression sur votre profil Discord.

---

## Architecture technique et légalité

Le patcher est conçu pour respecter scrupuleusement la propriété intellectuelle de l'éditeur :

1. **Aucun binaire sous copyright distribué** : Ce correctif ne contient ni le fichier `megaten.exe` original, ni de fichiers de musiques ou de sons d'Atlus.
2. **Patchs différentiels binaires bsdiff** : Les modifications sont distribuées sous la forme de fichiers de différences (`.bsdiff`) de moins de 2 Mo au total. Ces correctifs s'appliquent directement sur votre propre copie légale locale du jeu.
3. **Détection automatique de version** : Le patcher analyse la signature SHA-256 de votre jeu et applique le delta approprié selon que vous possédiez la version japonaise originale v1.01 ou la version anglaise.
4. **Sécurité des données** : Une copie de sauvegarde (`megaten.exe.bak`) est créée automatiquement avant la moindre modification.
5. **Proxy système ouvert** : La DLL `winmm.dll` est un proxy open-source assurant le relais des appels de menus et la communication Discord RPC sans modification permanente des bibliothèques de votre système d'exploitation.

---

## Guide d'installation

### Prérequis

* Un ordinateur équipé de Windows 10 ou Windows 11.
* Python 3.8 ou version supérieure installé sur la machine.
* Une copie locale du jeu (dossier original contenant `synchronicity.exe` et le sous-dossier `data`, ou archive `.rar` / `.zip` correspondante).

### Procédure d'application

1. Téléchargez l'archive `SMT-Synchronicity-FR-Patcher-v1.0.zip` depuis la page des publications du projet.
2. Décompressez le dossier du patcher sur votre ordinateur.
3. Lancez le fichier `lancer_patcher.bat`.
4. L'interface s'ouvre dans votre navigateur par défaut (`http://localhost:8765`).
5. Renseignez l'emplacement de votre jeu (ou sélectionnez directement l'archive d'origine du jeu).
6. Cliquez sur le bouton "Appliquer le Patch FR". L'opération s'effectue en une fraction de seconde.
7. Lancez `synchronicity.exe` pour démarrer votre partie.

---

## Commandes du jeu

### Configuration Clavier par défaut

| Action | Touche assignée |
|---|---|
| Déplacement (Haut / Bas / Gauche / Droite) | Flèches directionnelles |
| Attaque principale (Tir) | Z |
| Saut | X |
| Changer de protagoniste (Jack Frost / Pyro Jack) | C |
| Compétence magique | V |
| Course (Dash) | Double pression sur Gauche ou Droite |
| Menu Pause | Échap / Entrée |

La configuration des touches peut être redéfinie à tout moment via l'onglet Commandes du lanceur `synchronicity.exe`.

### Prise en charge des manettes

Le jeu reconnaît les manettes Xbox, PlayStation et les périphériques USB standards (XInput et DirectInput). La configuration des boutons s'effectue dans le menu Manette du lanceur.

---

## Discord Rich Presence

L'intégration Discord fournie par le module `winmm.dll` affiche sur votre profil :

* Le nom du jeu : *Shin Megami Tensei: Synchronicity Prologue*
* Le personnage actuellement actif (Jack Frost ou Pyro Jack)
* Le secteur du labyrinthe actuellement exploré
* La durée de la session en cours

Cette fonctionnalité s'active automatiquement au lancement du jeu lorsque le client Discord est ouvert sur votre ordinateur.

---

## Crédits

* **gc-ultimate117** : [https://github.com/gc-ultimate117](https://github.com/gc-ultimate117)
* **chenetulipe** : [https://github.com/chenetulipe](https://github.com/chenetulipe)
* **Atlus / Sega** : Création de la franchise *Shin Megami Tensei* et supervision du titre original.
* **Team Ladybug / Den-fami Nico Game Magazine** : Développement original de *Synchronicity Prologue*.
* **bsdiff** : Algorithme de patch différentiel binaire par Colin Percival.

Projet non officiel à visée communautaire et sans but lucratif. Tous les droits relatifs à la licence et à l'univers de Shin Megami Tensei appartiennent à Atlus / Sega.
