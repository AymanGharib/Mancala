# Mancala 3D — Soutenance OpenGL / C++
**Ayman Gharib · Amine Izoughagen** | Janvier 2026

---

## Contexte

Application 3D interactive du jeu **Mancala** développée en C++ avec OpenGL 3.3 Core Profile. L'application couvre l'ensemble de la chaîne graphique temps réel : pipeline de rendu, éclairage, interaction utilisateur, logique de jeu et personnalisation visuelle.

**Stack technique :** OpenGL 3.3 · GLFW · GLAD · GLM · stb_image · Dear ImGui · CMake

---

## 1. Interaction Souris & Clavier

| Action | Contrôle |
|--------|----------|
| Orbite caméra | Clic droit + glisser |
| Zoom | Molette souris |
| Pan (déplacement latéral) | `W` `A` `S` `D` `Q` `E` |
| Jouer un coup | Clic gauche sur une fosse |
| Reset la partie | `R` |
| Changer mode de rendu | `M` |
| Changer de thème | `T` |
| Toggle éclairage | `L` |
| Toggle aide / stats | `H` / `F` |
| Quitter | `ESC` |

La caméra est calculée en **coordonnées sphériques** (yaw/pitch/distance) recalculées à chaque frame vers une matrice `lookAt`. Les callbacks sont gérés dans `core/Window.cpp`, la logique dans `main.cpp → processInput`.

---

## 2. Matériaux, Textures & Lumières

### Matériaux
Chaque objet de la scène (plateau, fosses, graines) possède un `Material` complet avec quatre paramètres envoyés au shader via uniformes :

```
ambient · diffuse · specular · shininess
```

Les matériaux changent dynamiquement selon le thème actif.

**Fichiers :** `Scene/GameObject.h`, `Rendering/Material.h`

---

### Éclairage — Modèle Blinn-Phong

3 lumières ponctuelles configurées dans `setupLights()` :

| # | Position | Couleur | Rôle |
|---|----------|---------|------|
| 1 | (3, 5, 3) | Blanc chaud | Lumière principale |
| 2 | (−3, 4, 3) | Blanc froid | Fill (débouche les ombres) |
| 3 | (0, 3, −5) | Blanc neutre | Back light (séparation du fond) |

Calcul dans `phong.fs` pour chaque lumière :

```glsl
// Composantes Blinn-Phong
ambient  = Ka × lightColor
diffuse  = Kd × max(dot(N, L), 0) × lightColor
specular = Ks × pow(max(dot(N, H), 0), shininess)

// Atténuation distance
attenuation = 1.0 / (1.0 + 0.045*d + 0.0075*d²)

// Correction gamma finale
result = pow(color, vec3(1.0/2.2))
```

Activation/désactivation en temps réel avec `L`. Quand désactivé : rendu en couleurs plates (gamma-corrigé seulement), ce qui illustre directement l'impact du modèle d'éclairage.

**Fichiers :** `Shaders/phong.vs`, `Shaders/phong.fs`, `main.cpp → setupLights`

---

### Textures
Infrastructure complète implémentée (`Rendering/TextureManager.h`) : chargement via stb_image, caching GPU, textures procédurales unies/damier. Le shader supporte `hasTexture` / `useTextures`. Dans la version actuelle, les matériaux pilotent la couleur finale — le binding par objet est préparé mais pas activé.

---

## 3. Sélection d'Objets & Modes d'Affichage

### Sélection par Ray Casting

À chaque frame, un rayon est projeté depuis la caméra à travers le pixel sous le curseur :

```
Pixel (x,y)
  → NDC  : ndcX = 2x/W − 1,  ndcY = 1 − 2y/H
  → Eye  : inverse(proj) × rayClip
  → World: normalize( inverse(view) × rayEye )
  → Test intersection rayon-sphère sur chaque fosse
  → Sélectionne le hit le plus proche
```

Le clic n'est traité que si `!ImGui::GetIO().WantCaptureMouse` pour éviter les conflits avec l'UI.

**Fichiers :** `Interaction/ObjectPicker.h`, `main.cpp → handleMousePicking`

---

### Modes d'Affichage (Touche `M`)

| Mode | Comportement |
|------|-------------|
| `SHADED` | Rendu solide + Blinn-Phong (défaut) |
| `WIREFRAME` | `glPolygonMode(GL_LINE)` · culling OFF |
| `TEXTURED` | Solide + flag `useTextures=true` |
| `SHADED_WIRE` | Double passe : solide puis filaire superposé |

**Fichier :** `Rendering/RenderModeManager.h`

---

### Visibilité des objets
`GameObject::setVisible()` est disponible. Les graines apparaissent/disparaissent selon l'état du plateau (fosses vides = pas de graines rendues). La manipulation libre en runtime n'est pas exposée comme feature indépendante du jeu.

---

## 4. Détection de Collisions

Deux méthodes implémentées dans `Interaction/ObjectPicker.h` :

**Rayon-Sphère** *(utilisée pour le picking des fosses)*
```
OC = centre − origine_rayon
d² = |OC|² − dot(OC, dir)²
Si d² ≤ r²  →  HIT  à  t = dot(OC,dir) − √(r²−d²)
```
Le rayon de la sphère est approximé par `max(scale) / 2`.

**Rayon-AABB** *(implémentée, disponible)*  
Méthode des slabs sur les 3 axes — `rayAABBIntersection()` dans ObjectPicker.

Cette approche est suffisante pour un jeu de plateau statique : coût minimal, précision adéquate pour un pointage/clic.

---

## 5. Personnalisation — Thèmes Visuels

4 thèmes prédéfinis, cycle avec `T` :

| Thème | Plateau | Graines | Ambiance |
|-------|---------|---------|----------|
| **Classic Wood** | Brun chaud | Teintes chaudes | Shininess 16 — doux |
| **Modern Stone** | Gris pierre | Gris neutres | Shininess 8 — mat |
| **Egyptian Gold** | Doré/bronze | Or + bleu | Shininess 64 — brillant |
| **Neon Cyber** | Noir profond | Cyan + magenta | Shininess 128 — néon |

Chaque thème modifie les `Material` du plateau, des fosses et des graines via `ThemeManager::applyThemeToGame()`. Le changement est **instantané** à l'exécution.

**Fichiers :** `Game/ThemeManager.h`, `main.cpp → applyThemeToGame`

---

## 6. Logique du Jeu Mancala

### Plateau — 14 emplacements

```
Joueur 2  →   12   11   10    9    8    7
            ┌────┬────┬────┬────┬────┬────┐
  [13] P2   │    │    │    │    │    │    │   [6] P1
            └────┴────┴────┴────┴────┴────┘
              0    1    2    3    4    5
Joueur 1  →
```

- Index 6  = magasin Joueur 1 · Index 13 = magasin Joueur 2
- 4 graines initiales dans chaque fosse non-magasin

### Règles implémentées

- Validation du coup : fosse non vide, fosse du joueur actif, pas un magasin
- Distribution des graines en sens antihoraire avec **saut du magasin adverse**
- **Tour supplémentaire** si la dernière graine tombe dans son propre magasin
- **Capture** : dernière graine dans fosse vide du joueur → capture + fosse opposée (`oppose = 12 − index`)
- Détection de fin de partie quand un côté est entièrement vide

**Fichiers :** `Game/MancalaGame.cpp` — `isValidMove`, `executeMove`, `checkCapture`, `checkWinCondition`

---

## 7. Architecture

```
core/          →  Window (GLFW), Mesh (VAO/VBO/EBO), génération procédurale
Rendering/     →  Camera, Shader, Material, Texture, TextureManager, RenderModeManager
Scene/         →  Transform (TRS), GameObject (Transform + Mesh + Material)
Game/          →  MancalaGame (état + règles), ThemeManager (4 thèmes)
Interaction/   →  ObjectPicker (ray casting, rayon-sphère, rayon-AABB)
Shaders/       →  phong.vs · phong.fs (Blinn-Phong multi-lumières + gamma)
```

**Choix justifiés**
- OpenGL 3.3 Core + GLFW/GLAD : base portable et pédagogiquement claire
- GLM : calcul matriciel/vectoriel standard
- Blinn-Phong : compromis optimal réalisme/performance pour une scène stylisée
- Primitives procédurales : aucune dépendance à des assets externes
- ImGui : HUD fonctionnel visible directement en soutenance

---

## 8. Limites Assumées

| Point | État |
|-------|------|
| Animation des graines | Déplacement instantané — stub dans `updateAnimation` |
| Fin de partie | Transfert final des graines non implémenté dans `checkWinCondition` |
| Textures par objet | Pipeline prêt, `hasTexture` non piloté sur les objets du jeu |
| Déplacement libre d'objets | `setVisible()` existe, pas exposé comme feature runtime indépendante |

Ces limites sont toutes **localisées** dans le code et **techniquement identifiées**.

---

## 9. Démonstration — Ordre suggéré

```
1. Lancer l'app           →  vérifier FPS dans Stats (F)
2. Orbite + zoom          →  clic droit glisser · molette
3. Jouer 2-3 coups        →  montrer distribution, changement de tour, capture
4. Modes de rendu (M×4)   →  Shaded → Wireframe → Textured → Shaded+Wire
5. Éclairage (L)          →  ON/OFF : contraste avec/sans Blinn-Phong
6. Thèmes (T×4)           →  Classic → Stone → Gold → Neon
7. HUD                    →  Score (toujours visible) · Help (H) · Stats (F)
8. Reset (R)              →  partie relancée proprement
```

---

## Conclusion

Le projet couvre l'ensemble des objectifs du cahier des charges : pipeline de rendu temps réel fonctionnel, éclairage Blinn-Phong multi-sources, interaction complète souris/clavier, sélection d'objets par ray casting, détection de collision, personnalisation par thèmes et interface HUD. Les limites restantes sont identifiées et localisées — ce qui témoigne d'une maîtrise technique réelle du projet.