# Rapport Technique Académique — Projet Mancala 3D (OpenGL/C++)

## 1. Vue d’ensemble du projet
Le projet implémente une version tridimensionnelle interactive du jeu **Mancala** en C++ avec OpenGL 3.3 Core Profile. L’application est structurée autour d’un exécutable unique (`Mancala3D`) construit via CMake, avec les dépendances graphiques GLFW, GLAD, GLM, stb_image et Dear ImGui.

L’entrée principale (`main.cpp`) orchestre:
- l’initialisation de la fenêtre et du contexte OpenGL,
- la configuration caméra,
- l’instanciation du moteur de jeu `MancalaGame`,
- le rendu temps réel avec shader Phong,
- l’interaction utilisateur (clavier/souris),
- l’interface HUD avec ImGui.

L’objectif fonctionnel principal est de fournir une expérience de jeu Mancala manipulable en 3D, avec sélection de coups à la souris, gestion des tours, thèmes visuels, modes de rendu et retour d’état en interface.

## 2. Analyse du problème
Le problème traité combine trois dimensions techniques:

- **Simulation logique d’un jeu à règles discrètes**: représenter un plateau à 14 cases (12 fosses + 2 magasins), vérifier la légalité d’un coup, distribuer les graines selon l’ordre de parcours, appliquer les règles de capture, déterminer l’issue de partie.
- **Visualisation 3D interactive**: représenter des objets de jeu (plateau, fosses, graines) comme entités géométriques transformables et éclairées, avec pipeline de rendu stable en temps réel.
- **Interaction utilisateur multi-canal**: navigation caméra (orbite/pan/zoom), sélection objet par ray casting, raccourcis clavier de contrôle du système visuel (thèmes, éclairage, mode de rendu), tableau d’information en surimpression.

Contraintes de conception observées dans l’implémentation:
- recours à des primitives procédurales (`Mesh::createCube`, `Mesh::createSphere`) plutôt qu’à des actifs externes,
- centralisation de certains services en singletons (`ThemeManager`, `RenderModeManager`, `TextureManager`),
- séparation partielle des responsabilités entre logique de jeu, rendu, interaction et infrastructure de fenêtre.

## 3. Architecture du système
### 3.1 Arborescence du projet
Structure pertinente du dépôt (hors dépendances volumineuses et artefacts de build):

```text
.
├── CMakeLists.txt
├── main.cpp
├── core/
│   ├── Mesh.h
│   ├── Mesh.cpp
│   ├── Window.h
│   └── Window.cpp
├── Rendering/
│   ├── Camera.h
│   ├── Camera.cpp
│   ├── Material.h
│   ├── shader.h
│   ├── shader.cpp
│   ├── Texture.h
│   ├── TextureManager.h
│   ├── TextureManager.cpp
│   └── RenderModeManager.h
├── Scene/
│   ├── Transform.h
│   └── GameObject.h
├── Game/
│   ├── MancalaGame.h
│   ├── MancalaGame.cpp
│   └── ThemeManager.h
├── Interaction/
│   ├── ObjectPicker.h
│   └── ObjectPicker.cpp
└── Shaders/
    ├── phong.vs
    └── phong.fs
```

### 3.2 Modules centraux
- `core`: infrastructure système bas niveau (fenêtre GLFW, callbacks, buffers géométriques GPU).
- `Rendering`: abstractions de rendu (caméra, shader, matériau, texture, modes d’affichage).
- `Scene`: modèle entité-scène minimal (`GameObject` + `Transform`).
- `Game`: logique Mancala et personnalisation thématique.
- `Interaction`: conversion souris-écran vers rayon monde et sélection d’objet.
- `Shaders`: programme GLSL vertex/fragment pour l’éclairage et la coloration finale.

### 3.3 Responsabilités des classes
- `Window` (`core/Window.h`, `core/Window.cpp`):
  - création du contexte OpenGL,
  - activation profondeur/MSAA/culling,
  - callbacks clavier/souris/scroll,
  - état d’orbite caméra (`yaw`, `pitch`, `distance`).
- `Mesh` (`core/Mesh.h`, `core/Mesh.cpp`):
  - stockage vertex/index,
  - gestion VAO/VBO/EBO,
  - dessin `glDrawElements`,
  - génération procédurale cube/sphère,
  - calcul normales et bornes.
- `Camera` (`Rendering/Camera.h`):
  - encapsulation des matrices `view` et `projection`,
  - mise à jour position-cible et vecteurs locaux.
- `Shader` (`Rendering/shader.h`, `Rendering/shader.cpp`):
  - chargement fichiers GLSL,
  - compilation/lien programme,
  - setters d’uniformes avec cache de locations.
- `GameObject` (`Scene/GameObject.h`):
  - couple `Transform + Mesh + Material`,
  - envoi des uniformes matériau et matrice modèle,
  - rendu d’une entité visible.
- `MancalaGame` (`Game/MancalaGame.h`, `Game/MancalaGame.cpp`):
  - création du plateau/fosses/graines,
  - gestion des coups et de l’état de partie,
  - règles de capture et changement de joueur,
  - extraction de tous les objets pour rendu.
- `ThemeManager` (`Game/ThemeManager.h`):
  - définition de thèmes prédéfinis,
  - application de matériaux au plateau/fosses/graines.
- `ObjectPicker` (`Interaction/ObjectPicker.h`):
  - ray casting écran->monde,
  - intersections rayon-sphère/rayon-AABB,
  - sélection de l’objet le plus proche touché.
- `RenderModeManager` (`Rendering/RenderModeManager.h`):
  - pilotage de `glPolygonMode`, culling, overlay filaire.
- `TextureManager` (`Rendering/TextureManager.h`):
  - cache de textures,
  - chargement stb_image,
  - textures procédurales unies/damier.

## 4. Description du pipeline de rendu
Pipeline observé à l’exécution dans `main.cpp`:

1. **Préparation frame**:
- acquisition `deltaTime`, polling événements GLFW,
- mise à jour de la caméra orbitale depuis l’état `Window` (yaw/pitch/distance + cible de pan).

2. **Mise à jour logique/animation**:
- appel conditionnel `MancalaGame::updateAnimation` (animation déclarée mais actuellement simplifiée).

3. **Interaction de picking**:
- conversion curseur -> rayon 3D,
- test intersections avec les fosses sélectionnables,
- déclenchement de `executeMove` au clic valide.

4. **Pass principal OpenGL**:
- `glClear` couleur + profondeur,
- activation shader Phong,
- upload des uniformes globaux (`view`, `projection`, `viewPos`, paramètres lumière, drapeau textures),
- rendu de tous les `GameObject` visibles.

5. **Mode rendu spécial SHADED_WIRE**:
- pass solide,
- pass superposé fil de fer via `RenderModeManager::enableWireframeOverlay()`.

6. **Pass UI**:
- frame ImGui,
- dessin fenêtres Score/Help/Stats,
- rendu final ImGui avec profondeur/culling temporairement désactivés pour garantir l’overlay.

## 5. Techniques graphiques utilisées
### 5.1 Modèle de shader
Le shader utilise un pipeline GLSL 330 (`Shaders/phong.vs`, `Shaders/phong.fs`) avec:
- attributs sommet: position, normale, coordonnées UV,
- transformation `model/view/projection`,
- calcul normal via matrice normale `mat3(transpose(inverse(model)))`.

### 5.2 Modèle d’éclairage
Le fragment shader applique un **Blinn-Phong multi-lumières**:
- composante ambiante,
- diffuse (Lambert),
- spéculaire via vecteur half-way,
- addition des contributions de plusieurs lumières.

### 5.3 Gestion des matériaux
Chaque `GameObject` transmet un `Material` (ambient/diffuse/specular/shininess). La struct GLSL `Material` est alignée avec les uniformes envoyés dans `GameObject::render`.

### 5.4 Mappage de textures
Le shader supporte le chemin texturé (`texture_diffuse1`, `useTextures`, `hasTexture`).

Constat d’implémentation:
- le système de textures est techniquement présent (`TextureManager`),
- cependant, dans la boucle principale, aucune liaison de texture d’objet ni `hasTexture=true` n’est observée,
- la couleur finale est donc majoritairement pilotée par `material.diffuse` dans l’état actuel.

### 5.5 Correction gamma
Le fragment shader applique explicitement `pow(result, vec3(1.0/2.2))` tant en mode éclairé qu’en mode sans éclairage, ce qui stabilise la perception de luminance sur écrans standards.

## 6. Système d’interaction
### 6.1 Souris
- **RMB + déplacement**: orbite caméra (`Window::onMouseMove`).
- **Roulette**: zoom par variation de distance (`Window::onScroll`, clamp [2,20]).
- **LMB**: sélection de fosse par ray casting, puis tentative de coup.

### 6.2 Clavier
Traitement continu/déclenché dans `processInput`:
- `W/A/S/D/Q/E`: translation de la cible caméra (pan 3D),
- `R`: reset partie,
- `M`: cycle modes de rendu,
- `T`: cycle thèmes,
- `L`: activation/désactivation éclairage,
- `H`: visibilité fenêtre d’aide,
- `F`: visibilité statistiques,
- `ESC`: fermeture fenêtre (callback `Window::onKey`).

### 6.3 Caméra orbit/pan/zoom
La caméra est recalculée chaque frame à partir de coordonnées sphériques:
- direction dérivée de `yaw/pitch`,
- position = cible - direction normalisée * distance,
- matrice vue par `lookAt(position, target, up)`.

Cette stratégie sépare proprement la navigation (dans `Window`) de la projection/rendu (dans `Camera`).

### 6.4 Sélection d’objet
Le picking fonctionne sur les fosses uniquement:
- extraction des `pitObject` de `MancalaGame::getPits()`,
- test intersection rayon-sphère approximative,
- sélection de l’objet hit le plus proche,
- conversion objet -> index de fosse -> validation de coup.

Le système respecte `ImGui::GetIO().WantCaptureMouse` pour éviter les conflits entre interaction UI et interaction 3D.

## 7. Logique de jeu
### 7.1 Implémentation des règles Mancala
Éléments effectivement implémentés:
- plateau à 14 emplacements (index 0–13),
- 4 graines initiales dans chaque fosse non-magasin,
- impossibilité de jouer un magasin,
- impossibilité de jouer une fosse vide,
- impossibilité de jouer une fosse adverse,
- distribution des graines avec saut du magasin adverse,
- tour supplémentaire si la dernière graine termine dans son magasin,
- capture si la dernière graine arrive dans une fosse vide du joueur et que l’opposée contient des graines.

### 7.2 Gestion des tours
La variable `m_currentPlayer` alterne via `switchPlayer()` lorsqu’aucun tour supplémentaire n’est acquis.

### 7.3 Système d’animation
Un squelette d’animation existe:
- état `m_isAnimating`,
- `updateAnimation(float deltaTime)`,
- structures `m_animatingSeeds`, `m_seedTargets`.

Constat: `updateAnimation` est actuellement un stub (`m_isAnimating=false`), donc les déplacements de graines sont instantanés après distribution (`updateSeedPositions`).

### 7.4 Détection de fin de partie
`checkWinCondition()` détecte l’arrêt lorsqu’un côté (0–5 ou 7–12) est vide puis compare les magasins.

Limite importante observée:
- le commentaire indique le transfert des graines restantes vers les magasins,
- l’implémentation effective de ce transfert n’est pas réalisée (`// ... (implementation)`).

Ainsi, le score final peut être sous-estimé selon l’état des fosses au moment d’arrêt.

## 8. Stratégie de détection de collision
Le projet emploie une **collision géométrique orientée interaction** plutôt qu’une physique complète:

- collision utilisée principalement pour sélection d’objets (picking),
- test principal: **rayon-sphère** avec rayon approximé par `max(scale)/2`,
- test secondaire disponible: **rayon-AABB** (méthode implémentée mais non utilisée dans le flux courant).

Cette stratégie minimise le coût de calcul et convient à une interaction de type pointage/clic sur objets statiques du plateau.

## 9. Fonctionnalités de personnalisation
### 9.1 Changement de thème
Le `ThemeManager` définit quatre thèmes prédéfinis:
- Classic Wood,
- Modern Stone,
- Egyptian Gold,
- Neon Cyber.

Le changement est déclenché par `T` puis appliqué au plateau, fosses et graines via `applyThemeToGame`.

### 9.2 Modification des matériaux d’actifs
L’apparence des objets est contrôlée par les paramètres matériaux (ambient/diffuse/specular/shininess). Les graines reçoivent une couleur cyclique à partir de la palette du thème courant.

### 9.3 Gestion des modes de rendu
`RenderModeManager` propose:
- `WIREFRAME`,
- `SHADED`,
- `TEXTURED`,
- `SHADED_WIRE` (double passe).

Le basculement agit à la fois sur l’état OpenGL (polygon mode/culling) et sur les uniformes shader (`useTextures`).

## 10. Système d’éclairage
### 10.1 Configuration multi-lumières
Trois lumières ponctuelles sont configurées dans `setupLights`:
- lumière principale chaude,
- fill light froide,
- back light de contre-jour.

### 10.2 Activation/désactivation
Le booléen `lightsEnabled` piloté par la touche `L` bascule entre rendu éclairé (Blinn-Phong) et rendu non éclairé (couleur de base gamma-corrigée).

### 10.3 Modèle d’atténuation
Chaque contribution lumineuse est pondérée par une atténuation distance:

\\[
\\text{attenuation} = \\frac{1}{1 + 0.045d + 0.0075d^2}
\\]

Ce choix évite une chute trop abrupte de luminosité tout en conservant une décroissance réaliste pour une scène de petite échelle.

## 11. Interface utilisateur (ImGui)
L’interface est intégrée via `imgui_impl_glfw` et `imgui_impl_opengl3` avec style sombre.

Fenêtres implémentées:
- **Score**: joueur courant, score magasins P1/P2, état éclairage, résultat final.
- **Help**: rappel des commandes caméra/jeu/affichage.
- **Stats**: FPS courant (`ImGui::GetIO().Framerate`).

Conception UI observée:
- fenêtres ancrées à positions fixes,
- redimensionnement automatique,
- capture d’entrées UI respectée avant picking 3D.

## 12. Justification des choix techniques
- **OpenGL 3.3 Core + GLFW/GLAD**: choix robuste, portable et pédagogique pour un mini-projet universitaire en rendu temps réel.
- **GLM**: simplifie les opérations matricielles/vecteurs indispensables à la 3D.
- **Shader Blinn-Phong**: compromis pertinent entre coût de calcul et lisibilité visuelle pour des objets stylisés.
- **Primitives procédurales**: accélère la production et réduit la dépendance à une chaîne d’assets complexe.
- **Singletons pour services globaux**: simplifie l’accès aux états transverses (thèmes, textures, modes) dans une application mono-scène.
- **ImGui**: excellent support de prototypage d’outils d’observation/debug pendant le développement.

## 13. Considérations de performance
Aspects positifs:
- géométrie statique envoyée en GPU (VAO/VBO/EBO),
- état OpenGL configuré pour profondeur/culling/MSAA,
- cache des locations d’uniformes dans `Shader`,
- maillage sphérique des graines modéré (`segments=16`).

Points de vigilance:
- `getAllObjects()` reconstruit un `std::vector` à chaque frame,
- rendu en double passe pour `SHADED_WIRE`,
- picking testé à chaque frame sur toutes les fosses sélectionnables,
- absence d’optimisations de regroupement de draw calls (instancing non utilisé malgré support dans `Mesh::draw`).

## 14. Limitations identifiées
Limitations strictement observées dans le code:
- `selectPit`, `distributeSeedsAnimation`, `calculateSeedPosition`, `loadCubemap`, `getObjectBounds` sont déclarées mais non définies/utilisées dans le flux principal.
- L’animation des graines n’est pas opérationnelle (stub dans `updateAnimation`).
- La règle de fin de partie n’intègre pas encore le transfert effectif des graines restantes vers les magasins.
- Le chemin texturé du shader est partiel dans l’application courante (`hasTexture` non piloté, textures non bindées par objet).
- `wireframeColor` est envoyé dans `main.cpp` mais non consommé dans `phong.fs`.
- Les fosses et magasins sont rendus avec des cubes (commentaire signale un cylindre souhaité ultérieurement).

## 15. Améliorations futures
Améliorations directement alignées sur l’architecture existante:

1. Finaliser les règles Mancala de fin de partie en transférant explicitement les graines résiduelles avant calcul du gagnant.
2. Implémenter une animation de semis continue (interpolation temporelle graine par graine, easing, verrouillage d’entrée pendant animation).
3. Compléter le pipeline texture par objet (`hasTexture`, binding texture slots, matériaux texturés par thème).
4. Remplacer les colliders sphériques approximatifs par AABB/OBB par objet pour une sélection plus précise.
5. Introduire un système de surbrillance d’objet survolé (`hoveredObject`) pour améliorer le retour visuel.
6. Ajouter des meshes dédiés (fosses cylindriques, plateau creusé) et éventuellement import OBJ réellement implémenté.
7. Factoriser la scène vers une hiérarchie plus explicite (gestionnaire d’entités, séparation update/render).

## 16. Conclusion
Le projet présente une base technique solide pour un mini-projet de graphisme interactif: moteur de rendu temps réel fonctionnel, logique de jeu jouable, navigation caméra complète, picking opérationnel, personnalisation visuelle et instrumentation UI.

Sur le plan académique, la solution démontre la maîtrise des composants fondamentaux d’une application OpenGL moderne (pipeline, shaders, transformations, interaction). Les limites relevées concernent surtout la complétude fonctionnelle (animation et certaines règles terminales) et l’activation effective de fonctionnalités déjà préparées (textures complètes, collisions plus fines).

En conséquence, l’implémentation actuelle constitue une version **fonctionnelle et extensible**, adaptée à une soutenance de mini-projet avec perspectives d’amélioration clairement identifiées et techniquement cohérentes avec l’architecture existante.
