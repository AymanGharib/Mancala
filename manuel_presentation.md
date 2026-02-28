# Manuel De Présentation — Mancala 3D

Date de référence: **28/02/2026**  
Présentation prévue: **01/03/2026**

## 1. Objectif de la démo
Montrer, de manière vérifiable, ce qui est réellement implémenté dans le projet par rapport au sujet: interaction 3D, rendu, logique Mancala, interface, personnalisation.

## 2. Démarrage rapide
Lancer l’application:

```bash
cmake -S . -B build
cmake --build build
./build/bin/Mancala3D
```

Sur Windows:

```bash
.\build\bin\Mancala3D.exe
```

## 3. Script de démo (action => résultat)

### 3.1 Interaction caméra (souris + clavier)
- Faire `RMB + drag` => la caméra orbite autour du plateau.
- Faire `molette` => zoom avant/arrière.
- Faire `W/A/S/D/Q/E` => déplacement du point cible (pan).

Preuve code:
- `core/Window.cpp` (gestion souris/clavier orbit/zoom).
- `main.cpp` (recalcul position caméra depuis `yaw/pitch/distance`).

### 3.2 Interaction de jeu (sélection d’objet)
- Cliquer `LMB` sur une fosse valide (côté joueur courant) => les graines sont distribuées selon les règles.
- Cliquer une fosse invalide (vide, magasin, côté adverse) => aucun mouvement.

Preuve code:
- `main.cpp` (`handleMousePicking`).
- `Interaction/ObjectPicker.h` (`screenToWorldRay`, `pickObject`).
- `Game/MancalaGame.cpp` (`isValidMove`, `executeMove`).

### 3.3 Logique Mancala
- Faire un coup qui se termine dans son magasin => le joueur rejoue.
- Faire un coup de capture (dernière graine dans fosse vide du joueur, opposée non vide) => capture vers magasin.
- Continuer jusqu’à fin de partie => affichage du gagnant dans la fenêtre Score.

Preuve code:
- `Game/MancalaGame.cpp` (`executeMove`, `checkCapture`, `checkWinCondition`).
- `main.cpp` (`drawImGuiHUD`).

### 3.4 Modes d’affichage
- Appuyer sur `M` plusieurs fois => cycle:
  - `Wireframe`
  - `Shaded`
  - `Textured`
  - `Shaded + Wireframe`

Preuve code:
- `Rendering/RenderModeManager.h` (`cycleMode`, `applyMode`).
- `main.cpp` (`renderScene` double passe pour `SHADED_WIRE`).

### 3.5 Éclairage (multi-light + toggle)
- Appuyer sur `L` => ON/OFF éclairage.
- Montrer la différence visuelle entre mode éclairé et non éclairé.

Preuve code:
- `main.cpp` (`setupLights`, upload `numLights` et positions/couleurs/intensités).
- `Shaders/phong.fs` (`lightingEnabled`, Blinn-Phong, atténuation).

### 3.6 Thèmes et personnalisation visuelle
- Appuyer sur `T` => changement de thème global.
- Montrer changement matériau plateau/fosses/graines.

Preuve code:
- `Game/ThemeManager.h` (thèmes prédéfinis).
- `main.cpp` (`applyThemeToGame`).

### 3.7 Interface ImGui
- Montrer fenêtre `Score`: joueur courant, scores, état éclairage, gagnant.
- Montrer fenêtre `Help`: raccourcis.
- Montrer fenêtre `Stats`: FPS.
- Touche `H` pour help, `F` pour stats.

Preuve code:
- `main.cpp` (`drawImGuiHUD`).

## 4. Tableau “Exigence du sujet -> État réel”

### 4.1 Gestion souris/clavier pour interagir avec la scène
- État: **Implémenté**.
- Preuve: orbit, zoom, pan, clic sélection (`core/Window.cpp`, `main.cpp`).

### 4.2 Matériaux + textures + activation d’une ou plusieurs lumières
- Matériaux: **Implémenté** (`Scene/GameObject.h`, `Game/ThemeManager.h`).
- Lumières multiples + toggle: **Implémenté** (`main.cpp`, `Shaders/phong.fs`).
- Textures: **Partiellement implémenté**.
  - Le manager texture existe (`Rendering/TextureManager.h`),
  - mais le flux de rendu courant n’associe pas encore textures objets complètes (`hasTexture` non piloté globalement).

### 4.3 Sélection d’objets avec déplacement/cacher/modes d’affichage
- Sélection: **Implémenté** (picking sur fosses, `Interaction/ObjectPicker.h`, `main.cpp`).
- Modes d’affichage: **Implémenté** (`Rendering/RenderModeManager.h`).
- Déplacement/cacher d’objets en interaction utilisateur: **Non implémenté dans l’UI actuelle**.
  - Cacher existe techniquement via `setVisible`, mais pas exposé comme fonctionnalité utilisateur de manipulation.

### 4.4 Détection des collisions lors de la manipulation
- État: **Implémentation orientée picking** (rayon-sphère; rayon-AABB disponible mais non utilisé dans le flux principal).
- Preuve: `Interaction/ObjectPicker.h`.

### 4.5 Personnalisation avancée (thèmes + assets)
- Thèmes prédéfinis: **Implémenté** (`Game/ThemeManager.h`).
- Personnalisation des assets importants (matériaux plateau/fosses/graines): **Implémenté**.

## 5. Points à annoncer honnêtement au professeur
- L’animation des graines est prévue mais actuellement simplifiée (déplacement instantané):
  - `Game/MancalaGame.cpp` (`updateAnimation` stub).
- La règle de fin de partie n’intègre pas encore explicitement le transfert final de graines restantes vers magasins:
  - `Game/MancalaGame.cpp` (`checkWinCondition`, commentaire `// ... (implementation)`).
- Le pipeline texture est préparé mais pas exploité complètement sur les objets du jeu.
- Les objets sont générés procéduralement (cubes/sphères), pas via assets Blender intégrés dans cette version.

## 6. Pitch oral (45 secondes)
“Ce projet implémente un Mancala 3D interactif en OpenGL avec caméra orbitale, picking souris, logique de tour et de capture, rendu Phong multi-lumières, thèmes visuels et HUD ImGui. Nous avons privilégié une architecture modulaire séparant fenêtre, rendu, scène, interaction et logique de jeu. Les exigences majeures sont couvertes: interaction, éclairage, sélection, modes de rendu et personnalisation. Les limites actuelles sont connues et documentées: animation des graines simplifiée, pipeline texture partiel et finalisation de la règle terminale de collecte des graines.”

## 7. Plan de secours si une démo échoue
- Si clic ne répond pas: montrer `Help` + expliquer que seul le côté du joueur courant est valide.
- Si performance variable: passer en `Shaded` ou `Wireframe` via `M`.
- Si question sur preuves: ouvrir directement les fichiers référencés ci-dessus.
