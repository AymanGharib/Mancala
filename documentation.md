# Rapport De Soutenance — Mancala 3D (OpenGL / C++)

## 1. Contexte du mini-projet
Le sujet demandé consiste à développer une application 3D interactive du jeu **Mancala** en OpenGL, avec:
- interaction souris/clavier,
- gestion des matériaux, textures et lumières,
- sélection/manipulation d’objets et modes d’affichage,
- détection de collisions,
- personnalisation visuelle (thèmes et assets),
- cohérence visuelle et qualité du rendu.

Ce document est rédigé pour la présentation orale et répond explicitement à la question:  
**“Qu’est-ce qui a été demandé, et qu’est-ce qui a été effectivement réalisé ?”**

---

## 2. Ce que le professeur a demandé vs ce qui a été réalisé

### 2.1 Gestion souris et clavier pour interagir avec la scène
**Demandé**  
Interaction complète avec la scène 3D via souris et clavier.

**Réalisé**  
- Orbite caméra: `RMB + déplacement`.
- Zoom: molette souris.
- Pan caméra: `W/A/S/D/Q/E`.
- Commandes système: `R` (reset), `M` (mode rendu), `T` (thème), `L` (lumière), `H`/`F` (HUD), `ESC` (sortie).

**Preuves techniques**  
- `core/Window.cpp` (callbacks souris/clavier/scroll).  
- `main.cpp` (`processInput`, calcul caméra orbitale).

---

### 2.2 Matériaux, textures, lumières (une ou plusieurs)
**Demandé**  
Affectation/modification des propriétés matérielles, mappage texture, activation de plusieurs sources de lumière.

**Réalisé**  
- Matériaux: entièrement utilisés (ambient/diffuse/specular/shininess) sur plateau/fosses/graines.
- Lumières multiples: 3 lumières ponctuelles + activation/désactivation (`L`).
- Modèle d’éclairage: Blinn-Phong avec atténuation + correction gamma.
- Textures: infrastructure présente (chargement/caching), mais usage partiel dans la scène actuelle.

**Preuves techniques**  
- `Scene/GameObject.h` (envoi des uniformes matériau).  
- `Shaders/phong.fs` (lumière multi-source, attenuation, gamma).  
- `main.cpp` (`setupLights`, `lightingEnabled`, `numLights`).  
- `Rendering/TextureManager.h` (système textures), avec intégration partielle côté rendu.

---

### 2.3 Sélection d’objets + déplacement/cacher + modes d’affichage
**Demandé**  
Sélection des objets, possibilité de déplacement/cacher/afficher, modes d’affichage variés.

**Réalisé**  
- Sélection d’objets: oui (ray casting sur fosses jouables).
- Modes d’affichage: oui (`Wireframe`, `Shaded`, `Textured`, `Shaded+Wire`).
- Déplacement/cacher d’objets par interaction utilisateur: non exposé comme fonctionnalité de manipulation en jeu (même si `setVisible()` existe techniquement).

**Preuves techniques**  
- `Interaction/ObjectPicker.h` (`screenToWorldRay`, `pickObject`).  
- `main.cpp` (`handleMousePicking`).  
- `Rendering/RenderModeManager.h` (cycle et application des modes).  
- `Scene/GameObject.h` (`setVisible` disponible).

---

### 2.4 Détection des collisions
**Demandé**  
Détection de collision lors de la manipulation des objets de la scène.

**Réalisé**  
- Détection de collision orientée interaction/picking implémentée:
  - intersection rayon-sphère (utilisée),
  - intersection rayon-AABB (implémentée, non utilisée dans le flux principal).

**Preuves techniques**  
- `Interaction/ObjectPicker.h` (`raySphereIntersection`, `rayAABBIntersection`).

---

### 2.5 Personnalisation avancée (thèmes + assets)
**Demandé**  
Choix de thèmes visuels cohérents + personnalisation des assets importants.

**Réalisé**  
- 4 thèmes visuels prédéfinis:
  - Classic Wood,
  - Modern Stone,
  - Egyptian Gold,
  - Neon Cyber.
- Changement de thème en temps réel (`T`).
- Application des matériaux thématiques sur plateau, fosses et graines.

**Preuves techniques**  
- `Game/ThemeManager.h` (définition/applique des thèmes).  
- `main.cpp` (`applyThemeToGame`).

---

## 3. Ce qui est réalisé sur le jeu Mancala lui-même

### 3.1 Logique de jeu fonctionnelle
- Plateau 14 emplacements (6 fosses + magasin par joueur).
- 4 graines initiales par fosse non-magasin.
- Validation des coups (pas de fosse vide, pas de magasin, pas de fosse adverse).
- Distribution des graines avec saut du magasin adverse.
- Tour supplémentaire si la dernière graine tombe dans son magasin.
- Règle de capture implémentée.

**Preuves**  
- `Game/MancalaGame.cpp` (`isValidMove`, `executeMove`, `checkCapture`).

### 3.2 Interface de suivi de partie
- Affichage joueur courant, scores, état lumière, gagnant.
- Fenêtres Help et Stats (FPS) avec ImGui.

**Preuves**  
- `main.cpp` (`drawImGuiHUD`).

---

## 4. Architecture technique livrée

### 4.1 Organisation des modules
- `core/`: fenêtre OpenGL et mesh GPU.
- `Rendering/`: caméra, shaders, matériaux, textures, modes.
- `Scene/`: `Transform` + `GameObject`.
- `Game/`: logique Mancala + thèmes.
- `Interaction/`: picking / collisions rayon.
- `Shaders/`: vertex + fragment shader.

### 4.2 Justification des choix
- **OpenGL 3.3 + GLFW + GLAD**: base stable et portable.
- **GLM**: calcul matriciel/vectoriel fiable.
- **Blinn-Phong**: bon compromis réalisme/coût.
- **ImGui**: utile pour HUD et démonstration en soutenance.
- **Primitives procédurales**: simplicité d’intégration dans un mini-projet.

---

## 5. Limites actuelles (à annoncer clairement)
Pour une soutenance crédible, ces points doivent être assumés explicitement:

1. Animation des graines encore simplifiée (déplacement instantané).  
   - `Game/MancalaGame.cpp` (`updateAnimation`).
2. Fin de partie: transfert final des graines restantes vers les magasins non finalisé.  
   - `Game/MancalaGame.cpp` (`checkWinCondition`, commentaire d’implémentation manquante).
3. Pipeline texture complet préparé mais pas pleinement exploité sur les objets du jeu (`hasTexture` non piloté partout).
4. Sélection d’objets opérationnelle, mais pas de mode utilisateur de déplacement/libre-cachage d’objets en runtime.

---

## 6. Démonstration conseillée devant le professeur (2–3 minutes)
1. **Interaction caméra**: RMB drag + molette + WASDQE.  
2. **Jeu**: clic sur fosse valide, montrer changement de tour et capture.  
3. **Rendu**: `M` pour alterner les 4 modes d’affichage.  
4. **Éclairage**: `L` ON/OFF pour comparer rendu éclairé/non éclairé.  
5. **Personnalisation**: `T` pour changer de thème en direct.  
6. **HUD**: montrer Score/Help/Stats avec `H` et `F`.

---

## 7. Conclusion de soutenance
Le projet atteint les objectifs centraux du cahier des charges:  
- application Mancala 3D interactive,  
- architecture logicielle claire,  
- rendu shader multi-lumières,  
- sélection d’objets par ray casting,  
- personnalisation par thèmes,  
- interface utilisateur exploitable en démonstration.

Les limites restantes sont identifiées, localisées dans le code et techniquement maîtrisées, ce qui rend la proposition solide pour un mini-projet académique.
