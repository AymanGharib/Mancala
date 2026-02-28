# Rapport de mini-projet : Application 3D OpenGL du jeu de Mancala

**1. Présentation du projet**

Ce mini-projet académique consiste à concevoir une application 3D interactive du jeu de Mancala en OpenGL. L’objectif est de combiner programmation graphique avancée et conception d’interfaces interactives, tout en respectant des exigences de réalisme visuel, de cohérence esthétique et de robustesse fonctionnelle. L’application propose un plateau 3D manipulable, des animations de distribution des graines, des effets d’éclairage multiples et une interface utilisateur intégrée via ImGui.

**2. Analyse du problème**

Le jeu de Mancala impose une logique de règles séquentielles et un état de jeu évolutif. Sur le plan graphique, il requiert une représentation 3D fidèle d’un plateau à cavités, des objets déplacés et des interactions naturelles. Le problème technique se décompose en deux axes :

- Modéliser un environnement 3D immersif et cohérent avec des matériaux, textures et éclairages réalistes.
- Implémenter une interaction utilisateur précise (souris/clavier) et une logique de jeu correcte (tour par tour, capture, fin de partie).

**3. Architecture & conception du système**

L’architecture est volontairement modulaire afin d’isoler les responsabilités, faciliter la maintenance et justifier les choix techniques.

- **Modules principaux**
- `Renderer` : gestion du pipeline OpenGL, shaders, états de rendu.
- `Scene` : graphe de scène, hiérarchie des objets, transformations.
- `GameLogic` : règles du Mancala, état de jeu, système de tours.
- `Input` : gestion souris/clavier, navigation caméra.
- `Picking` : sélection d’objets par ray casting.
- `ThemeManager` / `RenderModeManager` / `TextureManager` : personnalisation visuelle.
- `UI` : fenêtres ImGui, métriques et aide.

- **Pipeline de rendu**
- Étape de géométrie : transformation modèle-vue-projection via GLM.
- Étape de fragment : calculs d’illumination Phong / Blinn-Phong.
- Post-traitement minimal : correction gamma et réglages de rendu.

- **Structure de la scène**
- Plateau principal avec cavités paramétriques.
- Graines comme instances d’objets 3D.
- Trois sources lumineuses positionnées (clé, remplissage, contre-jour).
- Caméra orbitale centrée sur le plateau.

**4. Techniques graphiques utilisées**

- **Shaders (Phong / Blinn-Phong)**
- Modélisation locale de l’illumination avec composantes ambiante, diffuse et spéculaire.
- Blinn-Phong utilisé pour des reflets plus stables à faible coût.

- **Modèle d’éclairage**
- Sources ponctuelles avec atténuation en fonction de la distance.
- Configuration tri-point pour un rendu équilibré et lisible.

- **Matériaux et textures**
- Paramètres matériaux (diffuse, specular, shininess) ajustables.
- Mapping de textures pour différencier plateau, cavités et graines.

- **Correction gamma**
- Améliore la perception des contrastes et la fidélité des couleurs.

- **Modes de rendu**
- Shaded, wireframe, et shaded+wire pour l’analyse structurelle.

**5. Système d’interaction**

- **Contrôles souris**
- Rotation orbitale de la caméra.
- Sélection d’objets via clic avec ray casting.
- Déplacement d’objets sélectionnés sous contraintes.

- **Contrôles clavier**
- Activation/désactivation des lumières.
- Changement de mode de rendu.
- Réinitialisation de la vue et de la scène.

- **Système de caméra**
- Caméra orbitale avec zoom et limites pour conserver la visibilité du plateau.

- **Picking (ray casting)**
- Calcul d’un rayon en espace monde depuis le curseur.
- Intersections testées avec volumes englobants des objets.

**6. Implémentation de la logique de jeu**

- **Règles du Mancala**
- Distribution anti-horaire des graines.
- Capture selon la règle de dernière graine.
- Tour supplémentaire si la dernière graine tombe dans le magasin.

- **Système de tours**
- Alternance stricte entre joueurs.
- Validation des actions selon l’état du plateau.

- **Gestion des animations**
- Interpolation temporelle pour déplacements de graines.
- Synchronisation animation-logique pour éviter les incohérences.

- **Détection de fin de partie**
- Fin lorsque toutes les cavités d’un joueur sont vides.
- Calcul du score final et désignation du vainqueur.

**7. Stratégie de détection de collisions**

- **Contraintes d’objets**
- Empêche le chevauchement des graines lors du déplacement.
- Maintient les objets dans les limites du plateau.

- **Justification**
- Choix d’une approche par volumes simples pour limiter le coût computationnel.
- Adaptée à la nature discrète et topologique du plateau de Mancala.

**8. Personnalisation & thèmes**

- **Système de thèmes**
- Styles prédéfinis cohérents (bois classique, pierre, métal, etc.).
- Paramètres globaux regroupés dans `ThemeManager`.

- **Modification d’assets**
- Variation des matériaux et textures des objets clés.
- Ajustement des couleurs et des propriétés spéculaires.

- **Réaffectation de textures**
- `TextureManager` permet le rechargement dynamique sans redémarrage.

**9. Interface utilisateur (ImGui)**

- **Fenêtre de score**
- Affichage des scores en temps réel.

- **Panneau d’aide**
- Récapitulatif des commandes et règles.

- **Panneau de statistiques**
- Nombre de coups, durée de la partie, état des tours.

**10. Système d’éclairage**

- **Configuration multi-lumière**
- Key light : principale, accentue le relief.
- Fill light : réduit les ombres dures.
- Back light : séparation du sujet et profondeur.

- **Activation/désactivation**
- Contrôles pour activer une ou plusieurs sources.

- **Justification**
- La configuration tri-point équilibre la lisibilité des formes et la crédibilité du rendu.

**11. Effets visuels & réalisme**

- **Reflets spéculaires**
- Rendent les matériaux plus plausibles.

- **Atténuation**
- Contribue à la perception de profondeur.

- **Perception de profondeur**
- Combinaison d’ombres, atténuation et perspective.

**12. Justification des choix techniques**

- **Pourquoi OpenGL**
- Contrôle bas niveau du pipeline, adéquat pour une démonstration pédagogique.

- **Pourquoi Blinn-Phong**
- Bon compromis entre réalisme et performance, adapté à un projet temps réel.

- **Pourquoi le ray casting**
- Sélection précise et indépendante du rendu, extensible à d’autres objets.

- **Pourquoi une architecture modulaire**
- Facilite l’extension des fonctionnalités, la maintenance et la réutilisation.

**13. Considérations de performance**

- Réduction des appels de rendu par regroupement d’objets.
- Utilisation d’instances pour les graines répétitives.
- Limitation des tests de collision aux objets pertinents.

**14. Compilation et exécution**

Pré-requis : CMake, compilateur C++17, OpenGL, GLFW, GLAD, ImGui, GLM.

Exemple de compilation :

```bash
cmake -S . -B build
cmake --build build --config Release
```

Exemple d’exécution :

```bash
./build/Release/Mancala3D
```

**15. Conclusion**

Le projet démontre la capacité à intégrer une logique de jeu non triviale avec une architecture de rendu 3D moderne. L’approche retenue met l’accent sur la rigueur technique, la qualité visuelle et l’ergonomie utilisateur, tout en justifiant les choix d’implémentation selon des critères académiques.

**16. Améliorations possibles**

- Ajout d’ombres projetées pour renforcer la profondeur.
- Intégration d’animations procédurales plus fines.
- Mode multijoueur en réseau.
- Sauvegarde et reprise de partie.