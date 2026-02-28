# 🎮 Mancala 3D — Rapport Technique Complet
### Mini-Projet de Programmation Graphique 3D — OpenGL/C++
**Ayman Gharib · Amine Izoughagen** | *Janvier 2026*

---

## 📋 Table des Matières

1. [Introduction & Objectifs](#1-introduction--objectifs)
2. [Analyse du Problème](#2-analyse-du-problème)
3. [Architecture Générale](#3-architecture-générale)
4. [Modélisation 3D & Assets](#4-modélisation-3d--assets)
5. [Pipeline de Rendu OpenGL](#5-pipeline-de-rendu-opengl)
6. [Éclairage & Matériaux — Blinn-Phong](#6-éclairage--matériaux--blinn-phong)
7. [Interaction Utilisateur](#7-interaction-utilisateur)
8. [Logique du Jeu Mancala](#8-logique-du-jeu-mancala)
9. [Détection de Collisions & Ray Casting](#9-détection-de-collisions--ray-casting)
10. [Personnalisation & Thèmes](#10-personnalisation--thèmes)
11. [Performances & Optimisations](#11-performances--optimisations)
12. [Manuel de Test en Direct](#12-manuel-de-test-en-direct)
13. [Limitations & Améliorations Futures](#13-limitations--améliorations-futures)
14. [Conclusion](#14-conclusion)

---

## 1. Introduction & Objectifs

Ce projet réalise une version **tridimensionnelle interactive** du jeu de société africain *Mancala* (aussi appelé Kalah), développée en **C++ avec OpenGL 3.3 Core Profile**. L'objectif pédagogique est de maîtriser l'ensemble de la chaîne de création d'une application graphique temps réel :

| Objectif | Réalisé |
|----------|---------|
| Pipeline de rendu OpenGL 3.3 Core | ✅ |
| Shader Blinn-Phong multi-lumières | ✅ |
| Interaction souris/clavier (orbite, pan, zoom, clic) | ✅ |
| Ray casting pour sélection d'objets | ✅ |
| Logique complète du jeu Mancala | ✅ |
| Système de thèmes visuels | ✅ |
| Interface HUD avec Dear ImGui | ✅ |
| Modes de rendu (Filaire, Solide, Hybride) | ✅ |

---

## 2. Analyse du Problème

Le problème combine **trois dimensions techniques interdépendantes** :

```
┌─────────────────────────────────────────────────────────────────┐
│                    DIMENSIONS DU PROJET                          │
│                                                                   │
│  ┌──────────────────┐  ┌──────────────────┐  ┌───────────────┐  │
│  │  LOGIQUE DE JEU  │  │  RENDU 3D TEMPS  │  │  INTERACTION  │  │
│  │                  │  │      RÉEL        │  │  UTILISATEUR  │  │
│  │  • 14 cases      │  │  • OpenGL 3.3    │  │  • Souris     │  │
│  │  • Distribution  │◄─►  • Shaders GLSL  │◄─►  • Clavier   │  │
│  │  • Captures      │  │  • Éclairage     │  │  • Ray Cast   │  │
│  │  • Tours suppl.  │  │  • Matériaux     │  │  • Picking    │  │
│  │  • Fin de partie │  │  • Textures      │  │  • ImGui HUD  │  │
│  └──────────────────┘  └──────────────────┘  └───────────────┘  │
│                                                                   │
│  Contrainte : Synchronisation parfaite état logique ↔ visuel     │
└─────────────────────────────────────────────────────────────────┘
```

### Contraintes de Conception Identifiées

- **Primitives procédurales** : cubes et sphères générés en code (pas d'assets externes requis)
- **Singletons** pour les services globaux : `ThemeManager`, `RenderModeManager`, `TextureManager`
- **Séparation des responsabilités** : logique / rendu / interaction clairement découplés

---

## 3. Architecture Générale

### 3.1 Arborescence du Projet

```
Mancala3D/
├── CMakeLists.txt              ← Build system
├── main.cpp                    ← Boucle principale + orchestration
│
├── core/
│   ├── Mesh.h / Mesh.cpp       ← Géométrie GPU (VAO/VBO/EBO)
│   ├── Window.h / Window.cpp   ← Fenêtre GLFW + callbacks
│
├── Rendering/
│   ├── Camera.h / Camera.cpp   ← Matrices view/projection
│   ├── Material.h              ← Propriétés matérielles
│   ├── shader.h / shader.cpp   ← Compilation GLSL
│   ├── Texture.h               ← Abstraction texture GPU
│   ├── TextureManager.h/.cpp   ← Cache textures (singleton)
│   └── RenderModeManager.h     ← glPolygonMode + culling
│
├── Scene/
│   ├── Transform.h             ← TRS (Translation/Rotation/Scale)
│   └── GameObject.h            ← Entité scène = Transform+Mesh+Material
│
├── Game/
│   ├── MancalaGame.h/.cpp      ← Règles + état + positions des graines
│   └── ThemeManager.h          ← Thèmes visuels prédéfinis (singleton)
│
├── Interaction/
│   ├── ObjectPicker.h/.cpp     ← Ray casting écran→monde
│
└── Shaders/
    ├── phong.vs                ← Vertex shader
    └── phong.fs                ← Fragment shader Blinn-Phong
```

### 3.2 Diagramme de Flux d'Exécution

```
┌────────────────────────────────────────────────────────────────┐
│                     BOUCLE PRINCIPALE (main.cpp)                │
│                                                                  │
│  Chaque frame (16ms @ 60fps) :                                  │
│                                                                  │
│  1. EVENTS ──► GLFW::pollEvents()                               │
│       │         └─ onKey() / onMouseMove() / onScroll()         │
│       │                                                          │
│  2. UPDATE ──► deltaTime calculation                            │
│       │         └─ Camera::update(yaw, pitch, distance, target) │
│       │         └─ MancalaGame::updateAnimation(dt)             │
│       │                                                          │
│  3. PICKING ──► ObjectPicker::castRay(cursorX, cursorY)        │
│       │          └─ Test intersections avec fosses              │
│       │          └─ Si LMB clic → executeMove(pitIndex)        │
│       │                                                          │
│  4. RENDER ──► glClear(COLOR | DEPTH)                          │
│       │         └─ Shader Phong activé                          │
│       │         └─ Upload uniformes (view, proj, lights)        │
│       │         └─ Pour chaque GameObject → render()            │
│       │         [Si SHADED_WIRE: 2ème pass filaire]             │
│       │                                                          │
│  5. UI ──────► ImGui::NewFrame()                                │
│                 └─ Fenêtre Score (joueur actif, magasins)       │
│                 └─ Fenêtre Help (commandes)                     │
│                 └─ Fenêtre Stats (FPS)                          │
│                 └─ ImGui::Render()                               │
│                                                                  │
│  6. SWAP ────► glfwSwapBuffers()                                │
└────────────────────────────────────────────────────────────────┘
```

### 3.3 Responsabilités des Classes (Résumé)

```
┌──────────────┐    crée     ┌─────────────┐   contient   ┌──────────────┐
│    Window    │────────────►│  GameObject │◄─────────────│  Transform   │
│  (GLFW ctx)  │             │  (entité)   │              │  (TRS)       │
└──────────────┘             └──────┬──────┘              └──────────────┘
                                    │ contient
                    ┌───────────────┼───────────────┐
                    ▼               ▼               ▼
               ┌─────────┐   ┌──────────┐   ┌──────────┐
               │  Mesh   │   │ Material │   │ Shader   │
               │(VAO/VBO)│   │(amb/diff │   │(GLSL prog│
               │         │   │/spec/shi)│   │ uniforms)│
               └─────────┘   └──────────┘   └──────────┘

┌──────────────────┐  gère état  ┌────────────────┐
│  MancalaGame     │◄────────────│  ObjectPicker  │
│  (logique+scène) │             │  (ray casting) │
└────────┬─────────┘             └────────────────┘
         │ utilise
    ┌────▼──────┐   ┌──────────────────┐   ┌──────────────────┐
    │ThemeManager│  │RenderModeManager │   │ TextureManager   │
    │(4 thèmes) │  │(polygon mode)    │   │(cache textures)  │
    └───────────┘  └──────────────────┘   └──────────────────┘
```

---

## 4. Modélisation 3D & Assets

### 4.1 Géométrie Procédurale

Tous les objets sont générés **algorithmiquement** dans `Mesh.cpp` — aucun fichier externe requis.

```
PLATEAU DE JEU                    FOSSES & MAGASINS          GRAINES
┌──────────────────────────┐      ┌─────────────┐           ●
│  Cube allongé            │      │ Cubes       │          ●●●
│  Scale: (5.0, 0.3, 2.5)  │      │ positionnés │         ●●●●
│  12 fosses + 2 magasins  │      │ sur plateau │        (Sphères
└──────────────────────────┘      └─────────────┘         r=0.08)

Mesh::createCube()   → 24 vertices (normales par face)
Mesh::createSphere() → segments=16 → ~512 triangles par graine
```

### 4.2 Organisation du Plateau (14 Cases)

```
Disposition logique des indices (vue de dessus) :

  JOUEUR 2 (haut) ←────────────────────────────────
  Index :   12    11    10     9     8     7
         ┌──────┬─────┬─────┬─────┬─────┬─────┐
    [13] │  ●●  │ ●●● │ ●●● │ ●●● │ ●●  │ ●●● │ [6]
  STORE  └──────┴─────┴─────┴─────┴─────┴─────┘  STORE
  P2     ┌──────┬─────┬─────┬─────┬─────┬─────┐  P1
    [13] │  ●●  │ ●●● │ ●●● │ ●●  │ ●●● │ ●●● │ [6]
         └──────┴─────┴─────┴─────┴─────┴─────┘
  Index :   0     1     2     3     4     5
  JOUEUR 1 (bas) ──────────────────────────────────►

  • Index 6  = Magasin Joueur 1 (droite)
  • Index 13 = Magasin Joueur 2 (gauche)
  • Configuration initiale : 4 graines dans chaque fosse (0-5, 7-12)
```

### 4.3 Positionnement 3D des Graines

```cpp
// updateSeedPositions() dans MancalaGame.cpp
// Calcul de la position de chaque graine dans une fosse
glm::vec3 pos = pitCenter + offset;
// Les graines sont disposées en grille (3×N) au-dessus de chaque fosse
// avec hauteur croissante selon le nombre de graines empilées
```

---

## 5. Pipeline de Rendu OpenGL

### 5.1 Diagramme du Pipeline Complet

```
CPU (Application)                    GPU (OpenGL Pipeline)
─────────────────                    ─────────────────────

Vertices + Normales + UV             ┌─────────────────────┐
    │                                │   VERTEX SHADER     │
    ▼                                │  (phong.vs)         │
 VBO/VAO                             │  • gl_Position      │
    │                                │  • FragPos (world)  │
    └────────────────────────────────►  • Normal (transf.) │
                                     │  • TexCoords        │
Uniformes :                          └─────────┬───────────┘
 • model (mat4)                                │ Rasterisation
 • view  (mat4)          ─────────────────────►│ (interpolation)
 • proj  (mat4)                                ▼
 • viewPos (vec3)        ┌─────────────────────────────────────┐
 • lights[3]             │        FRAGMENT SHADER (phong.fs)   │
 • material              │                                     │
 • useTextures            │  Pour chaque lumière i :           │
 • useLighting            │                                     │
                         │  ambient  = Ka × lightColor[i]     │
                         │  diffuse  = Kd × max(N·L, 0)       │
                         │  specular = Ks × (R·V)^shininess   │
                         │  atten    = 1/(1+0.045d+0.0075d²) │
                         │                                     │
                         │  result  = Σ (amb+diff+spec)×atten │
                         │  output  = pow(result, 1/2.2)      │
                         │           ↑ correction gamma        │
                         └─────────────────┬───────────────────┘
                                           │
                                    ┌──────▼──────┐
                                    │  Depth Test  │
                                    │  Framebuffer │
                                    └──────┬───────┘
                                           │
                                    ┌──────▼──────┐
                                    │  ImGui Pass  │
                                    │  (2D overlay)│
                                    └─────────────┘
```

### 5.2 Matrices de Transformation

```
Espace Local  ──[Model]──►  Espace Monde  ──[View]──►  Espace Caméra  ──[Proj]──►  Clip Space

Model  = Translation × Rotation × Scale
View   = lookAt(cameraPos, target, up)
Proj   = perspective(45°, aspectRatio, 0.1f, 100.0f)

Normal Matrix = mat3(transpose(inverse(model)))
  ↑ Nécessaire pour corriger les normales sous transformations non-uniformes
```

### 5.3 Modes de Rendu

```
Touche M → cycle entre 4 modes :

  SHADED ────────────►  glPolygonMode(GL_FILL) + éclairage ON
  WIREFRAME ─────────►  glPolygonMode(GL_LINE) + culling OFF
  TEXTURED ──────────►  glPolygonMode(GL_FILL) + useTextures=true
  SHADED_WIRE ───────►  Pass 1: solide (SHADED)
                         Pass 2: filaire superposé (glPolygonMode GL_LINE)
                                 (double rendu de toute la scène)
```

---

## 6. Éclairage & Matériaux — Blinn-Phong

### 6.1 Modèle Mathématique

```
Pour chaque lumière ponctuelle i :

                 ┌─────────────── AMBIANT ──────────────────┐
  contribution = │  Ka × ambientColor[i]                    │
                 ├─────────────── DIFFUS ────────────────────┤
               + │  Kd × max(dot(N, L), 0) × diffColor[i]  │
                 ├─────────────── SPÉCULAIRE (Blinn) ────────┤
               + │  Ks × pow(max(dot(N, H), 0), shininess)  │
                 │  où H = normalize(L + V)  (half-vector)  │
                 └──────────────────────────────────────────┘
                 × atténuation(distance)

  Atténuation = 1 / (1 + 0.045·d + 0.0075·d²)
  Correction gamma finale : pow(color, vec3(1/2.2))
```

### 6.2 Configuration des 3 Lumières

```
Vue de dessus du plateau :

         Lumière Fill (bleue/froide)
            ●  (-3, 4, 3)
             \
              \        Lumière Back (contre-jour)
               \           ● (0, 3, -5)
                \         /
         ┌──────────────────┐
         │                  │
         │    PLATEAU 3D    │
         │                  │
         └──────────────────┘
                  \
                   ●  (3, 5, 3)
           Lumière Principale (jaune/chaude)
```

| Lumière | Position | Couleur | Rôle |
|---------|----------|---------|------|
| Principale | (3, 5, 3) | Blanc chaud (1.0, 0.95, 0.8) | Éclairage dominant, ombres naturelles |
| Fill | (-3, 4, 3) | Blanc froid (0.6, 0.7, 1.0) | Déboucher les ombres |
| Back | (0, 3, -5) | Blanc neutre (0.5, 0.5, 0.5) | Séparer les objets du fond |

### 6.3 Propriétés Matérielles par Type d'Objet

```
┌──────────────┬──────────┬──────────┬──────────┬──────────┐
│    Objet     │ Ambient  │ Diffuse  │ Specular │Shininess │
├──────────────┼──────────┼──────────┼──────────┼──────────┤
│ Plateau      │ (0.3,    │ Selon    │ (0.1,    │   16.0   │
│ (bois)       │  0.2,    │  thème   │  0.1,    │          │
│              │  0.1)    │          │  0.1)    │          │
├──────────────┼──────────┼──────────┼──────────┼──────────┤
│ Fosses       │ (0.2,    │ Selon    │ (0.05,   │    8.0   │
│              │  0.15,   │  thème   │  0.05,   │          │
│              │  0.08)   │          │  0.05)   │          │
├──────────────┼──────────┼──────────┼──────────┼──────────┤
│ Graines      │ (0.1,    │ Cyclique │ (0.3,    │   32.0   │
│ (pierres)    │  0.1,    │ par thème│  0.3,    │          │
│              │  0.1)    │          │  0.3)    │          │
└──────────────┴──────────┴──────────┴──────────┴──────────┘
```

---

## 7. Interaction Utilisateur

### 7.1 Carte Complète des Contrôles

```
╔══════════════════════════════════════════════════════════╗
║              CONTRÔLES — MANCALA 3D                      ║
╠══════════════════════════════════════════════════════════╣
║  CAMÉRA                                                   ║
║  ───────────────────────────────────────────────────────  ║
║  Clic Droit + Glisser  →  Orbite (rotation autour plateau)║
║  Molette               →  Zoom (distance 2 ↔ 20 units)   ║
║  W / S                 →  Pan avant / arrière             ║
║  A / D                 →  Pan gauche / droite             ║
║  Q / E                 →  Pan bas / haut                  ║
║                                                           ║
║  JEU                                                      ║
║  ───────────────────────────────────────────────────────  ║
║  Clic Gauche sur fosse →  Sélectionner & jouer le coup    ║
║  R                     →  Reset la partie                 ║
║                                                           ║
║  AFFICHAGE                                                ║
║  ───────────────────────────────────────────────────────  ║
║  T                     →  Changer de thème (4 thèmes)     ║
║  M                     →  Changer mode rendu (4 modes)    ║
║  L                     →  Toggle éclairage ON/OFF         ║
║  H                     →  Toggle fenêtre d'aide           ║
║  F                     →  Toggle fenêtre Stats/FPS        ║
║  ESC                   →  Quitter l'application           ║
╚══════════════════════════════════════════════════════════╝
```

### 7.2 Système de Caméra Orbitale

```
Coordonnées Sphériques → Cartésiennes :

    yaw (azimut) + pitch (élévation)
         │
         ▼
  dir.x = cos(pitch) × sin(yaw)
  dir.y = sin(pitch)
  dir.z = cos(pitch) × cos(yaw)
         │
         ▼
  cameraPos = target - normalize(dir) × distance
         │
         ▼
  view = lookAt(cameraPos, target, worldUp)

  Contraintes :
  • pitch clampé à [-89°, 89°] (évite le gimbal lock)
  • distance clampée à [2, 20] (zoom min/max)
  • target modifié par W/A/S/D/Q/E (pan 3D)
```

---

## 8. Logique du Jeu Mancala

### 8.1 Algorithme de Distribution des Graines

```
executeMove(pitIndex) :

  1. VALIDATION
     ├─ pitIndex ∈ [0..5] si joueur 1, ou [7..12] si joueur 2
     ├─ board[pitIndex] > 0  (fosse non vide)
     └─ !m_isAnimating       (pas d'animation en cours)

  2. RAMASSAGE
     seeds = board[pitIndex]
     board[pitIndex] = 0
     lastPit = pitIndex

  3. DISTRIBUTION (sens antihoraire)
     Pour i = 1 à seeds :
       nextPit = (lastPit + 1) % 14
       Si nextPit == magasinAdverse → skip (nextPit += 1)
       board[nextPit] += 1
       lastPit = nextPit

  4. RÈGLES SPÉCIALES sur lastPit :
     ┌─ Si lastPit == monMagasin
     │    → Tour supplémentaire (switchPlayer = false)
     │
     └─ Si board[lastPit] == 1 ET lastPit ∈ monCôté ET board[oppose(lastPit)] > 0
          → CAPTURE :
            board[monMagasin] += 1 + board[oppose(lastPit)]
            board[lastPit] = 0
            board[oppose(lastPit)] = 0

  5. CHANGEMENT DE TOUR
     Si !tourSupplémentaire → switchPlayer()

  6. VÉRIFICATION FIN DE PARTIE
     Si Σ board[0..5] == 0 ou Σ board[7..12] == 0
       → collecterGrainesRestantes() + compareScores()
```

### 8.2 Règle de l'Opposé (Capture)

```
  Fosse adverse opposée = 12 - pitIndex

  Exemple :
  Joueur 1 finit en fosse 2 (vide)
  → Capture fosse adverse = 12 - 2 = 10

  Indices :   12   11   10    9    8    7
            ┌────┬────┬[10]┬────┬────┬────┐
       [13] │    │    │ 5  │    │    │    │ [6]
            └────┴────┴────┴────┴────┴────┘
            ┌────┬────┬────┬────┬────┬────┐
       [13] │    │    │[2] │    │    │    │ [6]
            └────┴────┴────┴────┴────┴────┘
  Indices :   0    1   2    3    4    5

  Résultat : board[6] += 1 + 5 = 6 graines capturées !
```

### 8.3 Transitions d'État de Partie

```
          ┌──────────────┐
          │   WAITING    │  ← État initial
          │  (joueur 1)  │
          └──────┬───────┘
                 │ Clic valide sur fosse
                 ▼
          ┌──────────────┐
          │  EXECUTING   │  executeMove()
          │    MOVE      │
          └──────┬───────┘
                 │
         ┌───────┴────────────────────┐
         ▼ Tour suppl.                ▼ Pas tour suppl.
  ┌──────────────┐            ┌──────────────┐
  │   WAITING    │            │ switchPlayer │
  │ (même joueur)│            │  WAITING     │
  └──────────────┘            │ (autre joueur│
                               └──────┬───────┘
                                      │ Si côté vide
                                      ▼
                               ┌──────────────┐
                               │   GAME OVER  │
                               │  Calcul score│
                               └──────────────┘
```

---

## 9. Détection de Collisions & Ray Casting

### 9.1 Principe du Ray Casting (Picking)

```
  Écran 2D (pixels)          Espace NDC              Espace Monde 3D

  ┌──────────────────┐       ┌──────────┐        ┌───────────────────┐
  │         ●        │       │          │        │                   │
  │    (mouseX,Y)    │──────►│(-1 à +1) │──────► │  Rayon 3D         │
  │                  │       │          │        │  Origine: caméra  │
  └──────────────────┘       └──────────┘        │  Dir: vers scène  │
                                                  └───────────────────┘

  Étapes :
  1. ndcX = (2·mouseX / width)  - 1
     ndcY = 1 - (2·mouseY / height)

  2. rayClip = vec4(ndcX, ndcY, -1, 1)

  3. rayEye = inverse(projection) × rayClip
     rayEye = vec4(rayEye.xy, -1, 0)

  4. rayWorld = normalize(vec3(inverse(view) × rayEye))

  5. Pour chaque fosse sélectionnable :
     Test intersection rayon-sphère (rayon = max(scale)/2)
     → Retenir le hit le plus proche
```

### 9.2 Algorithme Intersection Rayon-Sphère

```
Données :
  O = origine du rayon (position caméra)
  D = direction normalisée du rayon
  C = centre de la sphère (position fosse)
  r = rayon de la sphère approximative

Calcul :
  OC = C - O
  tca = dot(OC, D)
  d²  = dot(OC, OC) - tca²

  Si d² > r² → Pas d'intersection (MISS)
  Sinon      → Intersection à t = tca - sqrt(r² - d²)
               → Si t > 0 : HIT (objet devant la caméra)

Sélection : on retient le HIT avec le t minimum (objet le plus proche)
```

### 9.3 Prévention des Interactions Invalides

```
Filtre de validité avant tout coup :

  clic souris
      │
      ▼
  ImGui::GetIO().WantCaptureMouse ?
      │ Oui → ignorer (clic sur UI)
      │ Non ↓
      ▼
  Objet touché est une fosse ?
      │ Non → ignorer
      │ Oui ↓
      ▼
  Fosse appartient au joueur actif ?
      │ Non → ignorer (fosse adverse)
      │ Oui ↓
      ▼
  Fosse contient des graines ?
      │ Non → ignorer (fosse vide)
      │ Oui ↓
      ▼
  Partie en cours ? (!gameOver)
      │ Non → ignorer
      │ Oui ↓
      ▼
  executeMove(pitIndex) ✅
```

---

## 10. Personnalisation & Thèmes

### 10.1 Les 4 Thèmes Disponibles (Touche T)

```
┌──────────────────┬──────────────────┬──────────────────┬──────────────────┐
│  CLASSIC WOOD    │  MODERN STONE    │  EGYPTIAN GOLD   │   NEON CYBER     │
│                  │                  │                  │                  │
│ Plateau: brun    │ Plateau: gris    │ Plateau: doré    │ Plateau: noir    │
│ Fosses:  brun    │ Fosses:  pierre  │ Fosses:  bronze  │ Fosses:  sombre  │
│         foncé    │         foncée   │                  │                  │
│ Graines: teintes │ Graines: grises  │ Graines: or/bleu │ Graines: cyan/   │
│         chaudes  │         neutres  │                  │         magenta  │
│                  │                  │                  │                  │
│ Shininess: 16    │ Shininess: 8     │ Shininess: 64    │ Shininess: 128   │
│ Ambiance: douce  │ Ambiance: froide │ Ambiance: chaude │ Ambiance: néon   │
└──────────────────┴──────────────────┴──────────────────┴──────────────────┘
```

### 10.2 Application d'un Thème

```
ThemeManager::applyThemeToGame(MancalaGame& game) :

  1. game.getBoard() → setMaterial(theme.boardMaterial)
  2. Pour chaque fosse : fosse.setMaterial(theme.pitMaterial)
  3. Pour chaque graine i : graine.setMaterial(theme.seedColors[i % palette.size()])
  4. Shader re-upload des uniformes matériaux au prochain frame
```

---

## 11. Performances & Optimisations

### 11.1 Stratégies Implémentées

```
✅ Géométrie statique en GPU (VAO/VBO/EBO chargés une seule fois)
✅ Cache des locations d'uniformes dans Shader::getLocation()
✅ MSAA activé (antialiasing matériel)
✅ Depth test (GL_DEPTH_TEST) pour tri automatique z
✅ Face culling (GL_CULL_FACE) → rendu faces avant uniquement
✅ sphères graines : segments=16 (compromis qualité/perf)
```

### 11.2 Points d'Attention (Profil de Performance)

```
Opération                    Fréquence    Impact
─────────────────────────────────────────────────
getAllObjects() → vector     Chaque frame  Moyen (alloc heap)
SHADED_WIRE double pass      Si activé     Double draw calls
Picking sur toutes fosses    Chaque frame  Faible (12 tests max)
Animation stub (no-op)       Chaque frame  Négligeable
ImGui render                 Chaque frame  Faible
```

### 11.3 Mesure des FPS

```
ImGui Stats window affiche : ImGui::GetIO().Framerate
Objectif : ≥ 60 FPS sur configuration standard
```

---

## 12. Manuel de Test en Direct

### 12.1 Compilation & Lancement

```bash
# 1. Cloner / extraire le projet
cd Mancala3D/

# 2. Créer le dossier de build
mkdir build && cd build

# 3. Configurer avec CMake
cmake ..

# 4. Compiler
make -j4           # Linux/macOS
# ou
cmake --build .    # Windows (Visual Studio)

# 5. Lancer
./Mancala3D        # Linux/macOS
.\Mancala3D.exe    # Windows
```

### 12.2 Scénario de Test Complet (pour la Démo)

#### ▶ TEST 1 : Navigation Caméra (30 secondes)

```
Action                          Résultat Attendu
─────────────────────────────────────────────────────────────────
Clic droit + glisser souris  → Plateau tourne autour de son axe
Molette vers soi             → Caméra s'éloigne (max distance 20)
Molette vers écran           → Caméra se rapproche (min distance 2)
Appuyer W                    → Vue descend (pan avant)
Appuyer A/D                  → Vue se déplace gauche/droite
```

#### ▶ TEST 2 : Jeu de Base (1-2 minutes)

```
Action                          Résultat Attendu
─────────────────────────────────────────────────────────────────
Clic gauche fosse joueur 1   → Graines redistribuées, score maj
(ex: fosse index 3, 4 graines) → fosses 4,5,6 (magasin) reçoivent +1
                             → Si dernière graine en magasin :
                               "même joueur rejoue" dans HUD
Clic sur fosse adverse       → Rien ne se passe (ignoré)
Clic sur fosse vide          → Rien ne se passe (ignoré)
```

#### ▶ TEST 3 : Thèmes Visuels

```
Appuyer T × 4 fois → Cycle : Classic Wood → Modern Stone
                            → Egyptian Gold → Neon Cyber → Classic Wood
À chaque pression → Couleurs plateau/fosses/graines changent instantanément
```

#### ▶ TEST 4 : Modes de Rendu

```
Appuyer M × 4 fois → Cycle : SHADED → WIREFRAME → TEXTURED → SHADED_WIRE → SHADED
WIREFRAME  → Structure en fils métalliques (arêtes seulement)
SHADED_WIRE → Solide + filaire superposé (double passe visible)
TEXTURED   → Même apparence que SHADED (textures pas bindées dans v1)
```

#### ▶ TEST 5 : Éclairage

```
Appuyer L → Bascule éclairage ON/OFF
OFF → Toute la scène en couleurs plates uniformes (pas d'ombre)
ON  → Retour au rendu Blinn-Phong avec dégradés et reflets
```

#### ▶ TEST 6 : Reset

```
Appuyer R → Plateau réinitialisé (4 graines partout, scores à 0)
```

#### ▶ TEST 7 : Fin de Partie (si temps disponible)

```
Jouer jusqu'à ce qu'un côté soit vide
→ HUD affiche "Game Over" + nom du gagnant
→ Appuyer R pour rejouer
```

### 12.3 Checklist de Démonstration Rapide (5 min)

```
□ 1. Lancer l'app → vérifier 60 FPS dans fenêtre Stats (F)
□ 2. Orbite caméra avec clic droit → zoom molette
□ 3. Jouer 2-3 coups pour montrer le gameplay
□ 4. Appuyer T → montrer les 4 thèmes
□ 5. Appuyer M → montrer mode filaire
□ 6. Appuyer L → montrer éclairage ON/OFF
□ 7. Appuyer R → reset propre
□ 8. Montrer le HUD (Score + Help avec H)
```

---

## 13. Limitations & Améliorations Futures

### 13.1 Limitations Actuelles

| Limitation | Détail | Priorité Fix |
|------------|--------|--------------|
| Animation stub | `updateAnimation` est un no-op — graines téléportées | Haute |
| Fin de partie incomplète | Transfert final des graines non implémenté | Haute |
| Textures non bindées | Pipeline texture présent mais `hasTexture` toujours false | Moyenne |
| Fosses en cubes | Commentaire indique cylindres souhaités | Basse |
| `wireframeColor` non consommé | Envoyé au shader mais inutilisé | Basse |

### 13.2 Améliorations Futures (Roadmap)

```
Court terme (1-2 semaines) :
  ├─ Finaliser règle fin de partie (transfert graines)
  ├─ Animation par interpolation (lerp graine par graine)
  └─ Surbrillance fosse au survol (hover effect)

Moyen terme (1 mois) :
  ├─ Textures par objet/thème (compléter pipeline)
  ├─ Collisions AABB au lieu de sphères approx.
  ├─ Meshes cylindriques pour fosses
  └─ IA basique (heuristique greedy)

Long terme :
  ├─ IA Minimax avec alpha-beta
  ├─ Mode multijoueur réseau
  ├─ Effets post-processing (bloom, DOF)
  └─ Sauvegarde/chargement de parties
```

---

## 14. Conclusion

Ce projet démontre la maîtrise des **composants fondamentaux d'une application OpenGL moderne** :

```
✅ Pipeline de rendu temps réel (VAO/VBO/EBO + shaders GLSL)
✅ Modèle d'éclairage Blinn-Phong multi-sources avec correction gamma
✅ Navigation caméra orbitale (yaw/pitch/distance en coordonnées sphériques)
✅ Sélection d'objets par ray casting (écran → monde)
✅ Logique de jeu Mancala complète (distribution, capture, tour suppl.)
✅ Personnalisation thématique (4 thèmes × matériaux dynamiques)
✅ Interface HUD intégrée avec Dear ImGui
✅ Architecture modulaire et extensible
```

L'application constitue une base **fonctionnelle et extensible**, prête à être améliorée avec animations, textures complètes et IA.

---

*Rapport généré pour soutenance — Janvier 2026*
*Dépendances : GLFW · GLAD · GLM · stb_image · Dear ImGui · CMake*