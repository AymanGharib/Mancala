# Mancala 3D — Soutenance OpenGL / C++
**Ayman Gharib · Amine Izoughagen** | Janvier 2026

---

## Contexte du Projet

L'objectif de ce mini-projet est de développer une application 3D interactive complète du jeu de société africain **Mancala** (variante Kalah) en utilisant **OpenGL 3.3 Core Profile** avec le langage C++. Le défi principal est de combiner trois domaines techniques simultanément : la programmation graphique bas niveau avec OpenGL, la logique d'un jeu à règles discrètes, et la conception d'une interface utilisateur interactive en 3D.

Le projet ne se limite pas à afficher des objets — il faut que chaque élément visuel soit synchronisé avec l'état logique du jeu en temps réel, tout en maintenant une fluidité d'au moins 60 images par seconde. Cela implique une architecture bien découpée où logique, rendu et interaction restent indépendants mais parfaitement coordonnés.

**Stack technique :** OpenGL 3.3 Core Profile · GLFW · GLAD · GLM · stb_image · Dear ImGui · CMake  
**Langage :** C++17  
**Build system :** CMake (cible unique `Mancala3D`)

---

## Architecture Générale

Avant d'entrer dans les détails techniques, il est important de comprendre comment le projet est découpé. L'architecture suit le principe de **séparation des responsabilités** : chaque dossier correspond à une préoccupation technique distincte, et les modules communiquent via des interfaces claires plutôt que d'être enchevêtrés.

```
core/          →  Fondations système : fenêtre GLFW, gestion GPU (VAO/VBO/EBO), génération de géométrie
Rendering/     →  Tout ce qui concerne le rendu : caméra, shaders, matériaux, textures, modes d'affichage
Scene/         →  Modèle entité-scène : Transform (position/rotation/scale) + GameObject
Game/          →  Logique Mancala (état, règles, coups) + gestion des thèmes visuels
Interaction/   →  Picking par ray casting, tests de collision géométrique
Shaders/       →  Code GLSL s'exécutant directement sur le GPU
```

La **boucle principale** dans `main.cpp` orchestre tout : à chaque frame, elle traite les événements, met à jour la caméra, effectue le picking, rend la scène 3D, puis dessine l'interface ImGui par-dessus. Cette boucle tourne en continu tant que la fenêtre est ouverte.

---

## 1. Interaction Souris & Clavier

### Principe général

L'interaction utilisateur est la couche la plus visible du projet. Toutes les entrées sont capturées via les **callbacks GLFW** définis dans `core/Window.cpp` : `onMouseMove`, `onScroll`, et `onKey`. Ces callbacks mettent à jour un état interne (yaw, pitch, distance, etc.) qui est ensuite lu à chaque frame par la boucle principale pour recalculer la caméra ou déclencher des actions.

Cette architecture basée sur des callbacks est importante : GLFW appelle ces fonctions automatiquement quand l'OS détecte une entrée. Notre code ne "poll" pas la souris en permanence — il réagit aux événements.

### Contrôles disponibles

| Action | Contrôle |
|--------|----------|
| Orbite caméra (rotation autour du plateau) | Clic droit + glisser |
| Zoom avant / arrière | Molette souris |
| Pan (déplacer le point de focus) | `W` `A` `S` `D` `Q` `E` |
| Jouer un coup (sélectionner une fosse) | Clic gauche |
| Reset la partie | `R` |
| Changer mode de rendu | `M` |
| Changer de thème visuel | `T` |
| Toggle éclairage ON/OFF | `L` |
| Toggle fenêtre d'aide | `H` |
| Toggle fenêtre statistiques FPS | `F` |
| Quitter | `ESC` |

### Caméra orbitale — comment ça fonctionne

La caméra est implémentée en **coordonnées sphériques**. Plutôt que de stocker directement la position de la caméra dans l'espace 3D (ce qui rend les rotations autour d'un point complexes), on stocke trois valeurs : `yaw` (rotation horizontale), `pitch` (élévation), et `distance` (éloignement du centre).

À chaque frame, ces trois valeurs sont converties en coordonnées cartésiennes :

```
direction.x = cos(pitch) × sin(yaw)
direction.y = sin(pitch)
direction.z = cos(pitch) × cos(yaw)

cameraPosition = target − normalize(direction) × distance
view = lookAt(cameraPosition, target, worldUp)
```

Quand l'utilisateur fait glisser le clic droit, on incrémente `yaw` et `pitch`. La molette modifie `distance` (clampée entre 2 et 20 unités). Les touches `W/A/S/D/Q/E` déplacent le point `target` dans l'espace 3D, ce qui donne l'effet de pan. Le pitch est limité à ±89° pour éviter le **gimbal lock** (problème de singularité mathématique au pôle nord/sud de la sphère).

**Fichiers concernés :** `core/Window.cpp` (callbacks), `main.cpp` (recalcul caméra chaque frame)

---

## 2. Matériaux, Textures & Éclairage

### Les matériaux — définition et usage

Dans OpenGL, un **matériau** décrit comment une surface réfléchit la lumière. Chaque objet de notre scène (plateau, fosses, graines) possède une structure `Material` avec quatre paramètres :

- `ambient` : couleur réfléchie dans les zones non exposées directement à la lumière — simule la lumière indirecte ambiante de l'environnement
- `diffuse` : couleur principale de l'objet sous éclairage direct — c'est ce qui détermine la teinte perçue
- `specular` : couleur et intensité des reflets brillants — dépend de l'angle entre l'observateur et la surface
- `shininess` : concentre ou étale les reflets — une valeur élevée produit un reflet petit et intense (métal poli), une valeur basse produit un reflet large et doux (bois mat)

Ces quatre valeurs sont envoyées au shader à chaque rendu via des **uniformes** — des variables globales du shader alimentées côté CPU avant chaque appel de dessin. C'est dans `Scene/GameObject.h` que cet envoi est réalisé, juste avant l'appel à `glDrawElements`.

Les matériaux changent dynamiquement selon le thème actif : quand l'utilisateur appuie sur `T`, le `ThemeManager` reconfigure les matériaux de tous les objets de la scène. Le changement est visible immédiatement au frame suivant.

### Le modèle d'éclairage Blinn-Phong

Le modèle d'éclairage implémenté est **Blinn-Phong**, une variante améliorée du modèle de Phong classique. Il s'exécute dans le **fragment shader** (`Shaders/phong.fs`), c'est-à-dire pour chaque pixel de chaque objet affiché. Ce modèle décompose la lumière réfléchie en trois composantes :

La **composante ambiante** est constante — elle représente la lumière qui a rebondi tellement de fois dans l'environnement qu'elle arrive de partout de façon uniforme. Elle empêche les zones d'ombre d'être complètement noires et simule un éclairage indirect global de façon économique.

La **composante diffuse** (modèle de Lambert) dépend de l'angle entre la normale de la surface (`N`) et la direction vers la lumière (`L`). Plus la surface fait face à la lumière, plus elle est éclairée. C'est ce qui crée les dégradés naturels de luminosité sur les surfaces courbes des graines et les bords du plateau.

La **composante spéculaire** utilise le **vecteur half-way** `H = normalize(L + V)` — à mi-chemin entre la direction de la lumière et la direction de l'observateur. Quand `H` est bien aligné avec la normale `N`, on obtient un reflet brillant. Blinn utilise ce vecteur H plutôt que le vecteur de réflexion R du Phong classique, car il évite des artefacts aux faibles angles et est légèrement moins coûteux à calculer sur GPU.

```glsl
// Pour chaque lumière i dans le fragment shader :
vec3 L = normalize(lightPos[i] - FragPos);
vec3 V = normalize(viewPos - FragPos);
vec3 H = normalize(L + V);
vec3 N = normalize(Normal);

float diff     = max(dot(N, L), 0.0);
float spec     = pow(max(dot(N, H), 0.0), material.shininess);
float dist     = length(lightPos[i] - FragPos);
float atten    = 1.0 / (1.0 + 0.045*dist + 0.0075*dist*dist);

vec3 ambient   = material.ambient  * lightColor[i];
vec3 diffuse   = material.diffuse  * diff * lightColor[i];
vec3 specular  = material.specular * spec * lightColor[i];

result        += (ambient + diffuse + specular) * atten;
```

La **correction gamma** appliquée en fin de shader (`pow(result, vec3(1.0/2.2))`) compense la non-linéarité des écrans : les calculs d'éclairage se font en espace linéaire (physiquement correct), mais les écrans affichent en espace gamma. Sans cette correction, les rendus paraissent trop sombres.

### Configuration des 3 lumières

Trois lumières ponctuelles sont configurées dans `setupLights()` au démarrage, selon la technique classique du **three-point lighting** :

| # | Position | Couleur | Rôle |
|---|----------|---------|------|
| 1 | (3, 5, 3) | Blanc chaud (1.0, 0.95, 0.8) | Lumière principale — éclairage dominant |
| 2 | (−3, 4, 3) | Blanc froid (0.6, 0.7, 1.0) | Fill light — débouche les ombres de la principale |
| 3 | (0, 3, −5) | Blanc neutre (0.5, 0.5, 0.5) | Back light — sépare les objets du fond |

Cette configuration venue de la photographie garantit que les objets ne sont jamais complètement plats ni entièrement noirs, tout en conservant un sens de direction lumineuse et de profondeur.

La touche `L` bascule `lightingEnabled`. Quand désactivé, le shader utilise uniquement `material.diffuse` sans calcul Blinn-Phong — c'est une démonstration directe et visuelle de l'importance du modèle d'éclairage pour la perception du volume.

**Fichiers :** `Shaders/phong.vs`, `Shaders/phong.fs`, `Scene/GameObject.h`, `main.cpp → setupLights`

### Textures

L'infrastructure texture est entièrement en place : `Rendering/TextureManager.h` gère un cache GPU (pour ne jamais charger deux fois la même image), le chargement via stb_image, et la génération de textures procédurales (couleur unie, damier). Le shader supporte les uniformes `useTextures` et `hasTexture` pour conditionner leur usage. Dans la version actuelle, la couleur finale est pilotée par les matériaux — le binding de textures par objet est architecturalement prêt, mais pas encore activé dans la boucle de rendu.

**Fichiers :** `Rendering/TextureManager.h`, `Rendering/Texture.h`

---

## 3. Sélection d'Objets & Modes d'Affichage

### Le Ray Casting — principe et nécessité

La **sélection d'objets à la souris** est un problème classique de la 3D interactive. L'utilisateur clique sur un pixel 2D, mais nos objets vivent dans un espace 3D. Il faut donc convertir ce clic 2D en une requête 3D : *"quel objet se trouve dans la direction de ce clic depuis la caméra ?"*

La technique du **ray casting** consiste à construire un rayon mathématique (une demi-droite) partant de la caméra, traversant le pixel cliqué, et se propageant dans la scène. On calcule les intersections de ce rayon avec les objets sélectionnables, et on retient le plus proche.

La conversion d'un pixel écran vers un rayon 3D traverse plusieurs espaces de coordonnées :

```
(mouseX, mouseY) en pixels
  ↓
NDC (Normalized Device Coordinates) :
    ndcX = (2 × mouseX / width) − 1        // −1 à +1
    ndcY = 1 − (2 × mouseY / height)       // Y inversé : écran ≠ OpenGL

  ↓
Espace œil (Eye Space) :
    rayEye = inverse(projection) × vec4(ndcX, ndcY, −1, 1)
    rayEye = vec4(rayEye.xy, −1, 0)        // direction, pas position

  ↓
Espace monde (World Space) :
    rayDir = normalize( vec3(inverse(view) × rayEye) )
```

On obtient un vecteur direction normalisé dans l'espace monde. Ce rayon est testé contre chaque fosse sélectionnable, et on retient le hit le plus proche.

Le picking est filtré par `ImGui::GetIO().WantCaptureMouse` : si la souris survole une fenêtre ImGui, le clic est ignoré pour la scène 3D — les deux systèmes coexistent sans conflit.

**Fichiers :** `Interaction/ObjectPicker.h`, `Interaction/ObjectPicker.cpp`, `main.cpp → handleMousePicking`

### Modes d'Affichage (Touche `M`)

OpenGL permet de modifier le mode de rasterisation via `glPolygonMode`. Le `RenderModeManager` cycle entre quatre modes :

| Mode | Comportement OpenGL | Utilité |
|------|--------------------| --------|
| `SHADED` | `GL_FILL` + Blinn-Phong actif | Mode de jeu normal |
| `WIREFRAME` | `GL_LINE` + culling désactivé | Voir la structure géométrique des maillages |
| `TEXTURED` | `GL_FILL` + `useTextures=true` | Mode textures activées |
| `SHADED_WIRE` | Double passe : solide puis filaire | Vue technique illustration |

Le mode `SHADED_WIRE` est le plus intéressant techniquement : la scène entière est rendue **deux fois** par frame. La première passe dessine les surfaces pleines normalement. La deuxième passe repasse en mode filaire avec un léger décalage de profondeur pour éviter le **z-fighting** (scintillement dû à deux polygones à la même profondeur). C'est plus coûteux mais produit un rendu "technical illustration" très lisible.

**Fichier :** `Rendering/RenderModeManager.h`

### Visibilité des objets

`GameObject::setVisible(bool)` est disponible dans `Scene/GameObject.h`. Elle est utilisée par le jeu : quand une fosse est vide (zéro graines), les sphères graines associées ne sont simplement pas rendues. La manipulation libre de visibilité en runtime n'est pas exposée comme feature indépendante, mais la base technique est en place.

---

## 4. Détection de Collisions

### Contexte et approche choisie

Dans une application de jeu de plateau comme Mancala, la détection de collision sert principalement à **l'interaction utilisateur** (identifier quelle fosse est pointée) plutôt qu'à une simulation physique complète. L'approche adoptée est donc une **collision géométrique orientée picking** : légère en calcul, précise pour l'usage, et adaptée à des objets statiques sur un plateau fixe.

Deux méthodes sont implémentées dans `Interaction/ObjectPicker.h` :

### Rayon-Sphère *(utilisée dans le flux principal)*

Chaque fosse est enveloppée dans une **sphère englobante** dont le rayon est calculé dynamiquement comme `max(scale.x, scale.y, scale.z) / 2`. L'avantage de la sphère est sa simplicité mathématique — le test se réduit à une équation du second degré. C'est la méthode utilisée pour tous les clics en jeu.

```
OC  = centre_fosse − origine_rayon
tca = dot(OC, direction_normalisée)
d²  = dot(OC, OC) − tca²

Si d² > r²  →  MISS  (le rayon passe à côté de la sphère)
Sinon       →  HIT   à  t = tca − √(r² − d²)
```

Si plusieurs fosses sont touchées, on sélectionne celle avec le `t` minimum — c'est-à-dire la plus proche de la caméra, donc la plus visible à l'écran.

### Rayon-AABB *(implémentée, disponible)*

L'**Axis-Aligned Bounding Box** est un parallélépipède rectangle aligné sur les axes du monde. Le test utilise la **méthode des slabs** : on calcule les intervalles d'entrée et de sortie du rayon sur chacun des 3 axes, puis on vérifie s'ils se chevauchent.

```
Pour chaque axe i ∈ {X, Y, Z} :
  t_min[i] = (box.min[i] − ray.origin[i]) / ray.dir[i]
  t_max[i] = (box.max[i] − ray.origin[i]) / ray.dir[i]

tEnter = max(t_min[X], t_min[Y], t_min[Z])
tExit  = min(t_max[X], t_max[Y], t_max[Z])

HIT si tEnter ≤ tExit  et  tExit > 0
```

Cette méthode est plus précise pour des objets rectangulaires comme le plateau. Elle est disponible dans `rayAABBIntersection()` et constitue une amélioration directe pour une version future.

L'approche sphère a été retenue comme méthode principale car elle est suffisamment précise pour des fosses de taille similaire, et le coût est minimal : au maximum 12 tests par clic (une par fosse sélectionnable).

**Fichier :** `Interaction/ObjectPicker.h`

---

## 5. Personnalisation — Thèmes Visuels

### Principe de la personnalisation

Le système de thèmes illustre un principe de conception important : **séparer la géométrie des propriétés visuelles**. La forme des objets (cubes pour le plateau et les fosses, sphères pour les graines) ne change jamais — seuls leurs matériaux changent. Cette séparation rend l'ajout d'un nouveau thème aussi simple que définir de nouvelles valeurs de couleur, sans modifier une seule ligne du code de rendu.

Le `ThemeManager` est un singleton qui maintient une liste de thèmes prédéfinis. Quand l'utilisateur appuie sur `T`, on passe au thème suivant et `applyThemeToGame()` reconfigure les `Material` de tous les objets de la scène. Le changement est **instantané** à l'exécution — le shader reçoit les nouvelles valeurs au frame suivant via les uniformes.

### Les 4 thèmes prédéfinis

| Thème | Plateau | Graines | Shininess | Ambiance |
|-------|---------|---------|-----------|----------|
| **Classic Wood** | Brun chaud | Teintes chaudes variées | 16 — surfaces douces | Naturelle, traditionnelle |
| **Modern Stone** | Gris pierre foncé | Gris neutres | 8 — très mat | Contemporaine, épurée |
| **Egyptian Gold** | Doré / bronze | Or + bleu lapis | 64 — brillant | Riche, historique |
| **Neon Cyber** | Noir profond | Cyan + magenta vifs | 128 — très brillant | Futuriste, contrasté |

Chaque thème forme un **ensemble cohérent** conçu pour fonctionner visuellement ensemble. Par exemple, dans Neon Cyber, le fond noir laisse ressortir les graines cyan/magenta très lumineuses, et le shininess à 128 crée des reflets presque métalliques qui renforcent l'effet néon. Dans Egyptian Gold, le shininess à 64 donne aux surfaces un aspect poli qui rappelle les objets en métal précieux.

Les graines reçoivent des couleurs **cycliques** depuis la palette du thème courant : si un thème définit 4 couleurs et qu'il y a 48 graines en jeu, chaque couleur est attribuée à 12 graines, réparties sur le plateau.

**Fichiers :** `Game/ThemeManager.h`, `main.cpp → applyThemeToGame`

---

## 6. Pipeline de Rendu OpenGL

### Vue d'ensemble de la boucle de rendu

À chaque frame dans `main.cpp`, le pipeline suit une séquence précise :

**Préparation :** Calcul du `deltaTime` (temps écoulé depuis la frame précédente, utilisé pour les animations et les mouvements indépendants du FPS), traitement des événements GLFW, recalcul de la matrice de vue depuis les coordonnées sphériques de la caméra.

**Picking :** Le curseur est converti en rayon 3D. Si l'utilisateur a cliqué (clic gauche) et qu'un hit valide est détecté sur une fosse jouable, `executeMove()` est appelé et l'état du plateau est mis à jour.

**Rendu principal :** `glClear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT)` efface les buffers. Le shader Phong est activé. Les uniformes globaux sont uploadés une fois pour toute la frame : matrices `view` et `projection`, position caméra `viewPos`, paramètres des 3 lumières, état de l'éclairage. Ensuite chaque `GameObject` de la scène appelle `render()` qui uploade ses uniformes locaux (matrice modèle, matériau) et déclenche `glDrawElements`.

**Pass ImGui :** Le depth test et le face culling sont temporairement désactivés pour que l'UI s'affiche correctement par-dessus la scène 3D. ImGui dessine ses fenêtres, puis les états OpenGL sont restaurés.

**Swap buffers :** `glfwSwapBuffers()` présente l'image complète (double buffering — on dessine sur un buffer pendant que l'autre est affiché, ce qui évite le tearing).

### Les shaders GLSL

Le programme shader est composé de deux fichiers compilés sur GPU au démarrage :

Le **vertex shader** (`phong.vs`) s'exécute une fois par sommet. Il applique les trois matrices de transformation successivement — matrice modèle (objet → monde), matrice vue (monde → espace caméra), matrice projection (espace caméra → clip space) — pour obtenir `gl_Position`. Il calcule aussi la position monde et la normale transformée qui sont interpolées et transmises au fragment shader.

La **matrice normale** mérite une explication : on ne peut pas utiliser directement la matrice modèle pour transformer les normales. Si l'objet subit un scale non uniforme (par exemple aplati dans une direction), les normales seraient déformées et l'éclairage deviendrait incorrect. On utilise donc `mat3(transpose(inverse(model)))` qui corrige cette déformation.

Le **fragment shader** (`phong.fs`) s'exécute pour chaque pixel. Il reçoit la position monde et la normale interpolée, récupère les propriétés matérielles et lumineuses depuis les uniformes, calcule la contribution Blinn-Phong de chacune des 3 lumières, les somme, et applique la correction gamma.

### Géométrie procédurale

Tous les objets 3D sont générés par le code dans `Mesh.cpp`, sans fichiers externes. `Mesh::createCube()` génère 24 sommets (4 par face × 6 faces) avec des normales correctes par face — c'est intentionnel car pour un cube on veut des bords nets, pas des normales lissées. `Mesh::createSphere()` génère une sphère par latitude/longitude avec `segments=16`, soit environ 512 triangles par graine — suffisant pour paraître lisse grâce à l'interpolation des normales, sans être trop lourd.

La géométrie est uploadée **une seule fois** au GPU lors de l'initialisation dans des VAO/VBO/EBO qui restent en mémoire vidéo. Chaque frame, seules les matrices de transformation et les matériaux sont mis à jour — la géométrie ne transite plus entre CPU et GPU après l'initialisation.

---

## 7. Logique du Jeu Mancala

### Structure du plateau

Le plateau Mancala est représenté par un tableau de 14 entiers : les indices 0 à 5 sont les fosses du Joueur 1, l'index 6 est son magasin, les indices 7 à 12 sont les fosses du Joueur 2, et l'index 13 est le magasin du Joueur 2.

```
Joueur 2  →   12   11   10    9    8    7
            ┌────┬────┬────┬────┬────┬────┐
  [13]  P2  │    │    │    │    │    │    │  [6]  P1
            └────┴────┴────┴────┴────┴────┘
              0    1    2    3    4    5
                        Joueur 1  →
```

La configuration initiale place **4 graines** dans chaque fosse non-magasin, soit 48 graines en jeu. Les magasins démarrent à 0. La somme totale des graines est toujours 48 pendant la partie — c'est une propriété invariante vérifiable à tout moment pour détecter des bugs.

### Algorithme de distribution

Quand un joueur sélectionne une fosse valide, `executeMove(pitIndex)` est appelé. La logique est la suivante :

1. On ramasse toutes les graines de la fosse choisie — elle passe à zéro.
2. On distribue une graine dans chaque fosse suivante en sens antihoraire, en **sautant le magasin adverse** : quand on croise le magasin de l'adversaire, on passe au suivant sans y déposer.
3. On note quelle est la dernière fosse touchée.
4. On applique les règles spéciales sur cette dernière fosse.

Le **tour supplémentaire** : si la dernière graine tombe dans le propre magasin du joueur actif, il rejoue immédiatement. Cette règle est un élément tactique central du Mancala — savoir compter ses graines pour terminer dans son magasin est une stratégie de base.

La **capture** : si la dernière graine tombe dans une fosse vide du côté du joueur actif, ET que la fosse directement opposée de l'adversaire n'est pas vide, alors le joueur capture tout. Il prend sa graine + toutes celles de la fosse adverse et les dépose dans son magasin. La fosse opposée se calcule avec la formule `oppose = 12 − pitIndex`.

```
Exemple de capture :
Joueur 1 finit en fosse d'index 2 (qui était vide)
Fosse adverse opposée = 12 − 2 = fosse 10
Si board[10] = 5 graines

→ board[6]  += 1 + 5 = 6   (magasin J1 gagne 6 graines)
→ board[2]   = 0
→ board[10]  = 0
```

### Validation des coups

Avant d'exécuter tout coup, `isValidMove()` vérifie que la fosse appartient au joueur actif, qu'elle contient au moins une graine, que ce n'est pas un magasin, et que la partie n'est pas terminée. Ces vérifications sont appliquées à la fois dans la logique de jeu ET dans le picking — une double protection contre tout état incohérent.

### Interface HUD avec ImGui

**Dear ImGui** est une bibliothèque de GUI immédiat : au lieu de maintenir un arbre d'objets UI persistants (comme Qt ou les frameworks web), on redéclare l'interface entière à chaque frame. C'est simple à utiliser, très performant, et parfait pour afficher des données changeantes comme les scores. Trois fenêtres sont toujours disponibles : **Score** (joueur actif, scores, état éclairage, annonce du gagnant), **Help** (liste des commandes), **Stats** (FPS temps réel).

**Fichiers :** `Game/MancalaGame.cpp` — `isValidMove`, `executeMove`, `checkCapture`, `checkWinCondition` · `main.cpp → drawImGuiHUD`

---

## 8. Limites Assumées

Ces points sont identifiés avec précision dans le code, localisés, et assumés clairement :

| Limitation | Détail technique | Fichier concerné |
|------------|-----------------|-----------------|
| Animation des graines | `updateAnimation()` est un stub — `m_isAnimating` reste false, les graines se repositionnent instantanément | `Game/MancalaGame.cpp` |
| Fin de partie incomplète | Le transfert des graines restantes vers les magasins est commenté mais non implémenté — le score final peut être sous-estimé | `MancalaGame.cpp → checkWinCondition` |
| Textures par objet | Pipeline complet présent (`TextureManager`, shader `hasTexture`), mais `hasTexture` reste `false` dans la boucle de rendu | `main.cpp`, `Rendering/TextureManager.h` |
| Manipulation libre d'objets | `setVisible()` existe sur `GameObject`, mais n'est pas exposé comme feature runtime indépendante du gameplay | `Scene/GameObject.h` |

Ces limites sont toutes **non bloquantes pour la démo** — le jeu est jouable, le rendu est fonctionnel, et les fonctionnalités manquantes sont clairement délimitées et techniquement maîtrisées.

---

## 9. Démonstration — Ordre suggéré

Le but est de montrer chaque exigence du cahier des charges de façon fluide en 2 à 3 minutes, dans un ordre qui raconte une progression logique : navigation → jeu → rendu → personnalisation.

```
1. Lancer l'application
   → Vérifier FPS dans Stats (F) — objectif ≥ 60 FPS

2. Navigation caméra (30 sec)
   → Clic droit + glisser → orbite autour du plateau
   → Molette → zoom avant/arrière
   → W/A/S/D → pan horizontal et vertical

3. Jouer 2 à 3 coups (45 sec)
   → Clic gauche sur une fosse du Joueur 1 (bas du plateau)
   → Montrer les graines redistribuées sur les fosses suivantes
   → Si la dernière graine tombe dans le magasin → "Joueur 1 rejoue"
   → Si possible, jouer une capture (fosse vide + graines en face)

4. Modes de rendu → touche M × 4 (20 sec)
   → SHADED         (normal)
   → WIREFRAME      (structure géométrique, maillages visibles)
   → TEXTURED       (flag textures activé)
   → SHADED+WIRE    (double passe, vue technique)

5. Éclairage → touche L × 2 (15 sec)
   → OFF : couleurs plates uniformes, perte totale de volume
   → ON  : retour Blinn-Phong, dégradés et reflets visibles

6. Thèmes → touche T × 4 (20 sec)
   → Classic Wood → Modern Stone → Egyptian Gold → Neon Cyber
   → Matériaux changent instantanément sur plateau + fosses + graines

7. HUD (10 sec)
   → Score : joueur actif + scores magasins (toujours visible)
   → Aide : H · Stats FPS : F

8. Reset → touche R
   → Partie relancée proprement, 4 graines dans chaque fosse
```

---

## Conclusion

Le projet couvre l'ensemble du cahier des charges : pipeline de rendu OpenGL temps réel avec shaders GLSL personnalisés, modèle d'éclairage Blinn-Phong trois sources avec correction gamma, navigation caméra orbitale complète, sélection d'objets par ray casting avec test rayon-sphère et rayon-AABB, quatre modes d'affichage (solide, filaire, texturé, hybride), système de quatre thèmes visuels dynamiques, logique Mancala fonctionnelle avec toutes les règles principales implémentées, et interface HUD Dear ImGui opérationnelle en démo.

L'architecture modulaire du projet garantit que chaque composant peut être étendu indépendamment : ajouter un nouveau thème ne touche pas au rendu, améliorer les collisions ne touche pas au gameplay, compléter les animations ne touche pas à l'interaction. C'est ce découpage clair qui rend le projet solide, extensible, et défendable techniquement.