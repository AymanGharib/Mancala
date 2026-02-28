# Rapport de mini-projet : Application 3D OpenGL du jeu de Mancala

**1. Pr�sentation du projet**

Ce mini-projet acad�mique consiste � concevoir une application 3D interactive du jeu de Mancala en OpenGL. L�objectif est de combiner programmation graphique avanc�e et conception d�interfaces interactives, tout en respectant des exigences de r�alisme visuel, de coh�rence esth�tique et de robustesse fonctionnelle. L�application propose un plateau 3D manipulable, des animations de distribution des graines, des effets d��clairage multiples et une interface utilisateur int�gr�e via ImGui.

**2. Analyse du probl�me**

Le jeu de Mancala impose une logique de r�gles s�quentielles et un �tat de jeu �volutif. Sur le plan graphique, il requiert une repr�sentation 3D fid�le d�un plateau � cavit�s, des objets d�plac�s et des interactions naturelles. Le probl�me technique se d�compose en deux axes :

- Mod�liser un environnement 3D immersif et coh�rent avec des mat�riaux, textures et �clairages r�alistes.
- Impl�menter une interaction utilisateur pr�cise (souris/clavier) et une logique de jeu correcte (tour par tour, capture, fin de partie).

**3. Architecture & conception du syst�me**

L�architecture est volontairement modulaire afin d�isoler les responsabilit�s, faciliter la maintenance et justifier les choix techniques.

- **Modules principaux**
- `Renderer` : gestion du pipeline OpenGL, shaders, �tats de rendu.
- `Scene` : graphe de sc�ne, hi�rarchie des objets, transformations.
- `GameLogic` : r�gles du Mancala, �tat de jeu, syst�me de tours.
- `Input` : gestion souris/clavier, navigation cam�ra.
- `Picking` : s�lection d�objets par ray casting.
- `ThemeManager` / `RenderModeManager` / `TextureManager` : personnalisation visuelle.
- `UI` : fen�tres ImGui, m�triques et aide.

- **Pipeline de rendu**
- �tape de g�om�trie : transformation mod�le-vue-projection via GLM.
- �tape de fragment : calculs d�illumination Phong / Blinn-Phong.
- Post-traitement minimal : correction gamma et r�glages de rendu.

- **Structure de la sc�ne**
- Plateau principal avec cavit�s param�triques.
- Graines comme instances d�objets 3D.
- Trois sources lumineuses positionn�es (cl�, remplissage, contre-jour).
- Cam�ra orbitale centr�e sur le plateau.

**4. Techniques graphiques utilis�es**

- **Shaders (Phong / Blinn-Phong)**
- Mod�lisation locale de l�illumination avec composantes ambiante, diffuse et sp�culaire.
- Blinn-Phong utilis� pour des reflets plus stables � faible co�t.

- **Mod�le d��clairage**
- Sources ponctuelles avec att�nuation en fonction de la distance.
- Configuration tri-point pour un rendu �quilibr� et lisible.

- **Mat�riaux et textures**
- Param�tres mat�riaux (diffuse, specular, shininess) ajustables.
- Mapping de textures pour diff�rencier plateau, cavit�s et graines.

- **Correction gamma**
- Am�liore la perception des contrastes et la fid�lit� des couleurs.

- **Modes de rendu**
- Shaded, wireframe, et shaded+wire pour l�analyse structurelle.

**5. Syst�me d�interaction**

- **Contr�les souris**
- Rotation orbitale de la cam�ra.
- S�lection d�objets via clic avec ray casting.
- D�placement d�objets s�lectionn�s sous contraintes.

- **Contr�les clavier**
- Activation/d�sactivation des lumi�res.
- Changement de mode de rendu.
- R�initialisation de la vue et de la sc�ne.

- **Syst�me de cam�ra**
- Cam�ra orbitale avec zoom et limites pour conserver la visibilit� du plateau.

- **Picking (ray casting)**
- Calcul d�un rayon en espace monde depuis le curseur.
- Intersections test�es avec volumes englobants des objets.

**6. Impl�mentation de la logique de jeu**

- **R�gles du Mancala**
- Distribution anti-horaire des graines.
- Capture selon la r�gle de derni�re graine.
- Tour suppl�mentaire si la derni�re graine tombe dans le magasin.

- **Syst�me de tours**
- Alternance stricte entre joueurs.
- Validation des actions selon l��tat du plateau.

- **Gestion des animations**
- Interpolation temporelle pour d�placements de graines.
- Synchronisation animation-logique pour �viter les incoh�rences.

- **D�tection de fin de partie**
- Fin lorsque toutes les cavit�s d�un joueur sont vides.
- Calcul du score final et d�signation du vainqueur.

**7. Strat�gie de d�tection de collisions**

- **Contraintes d�objets**
- Emp�che le chevauchement des graines lors du d�placement.
- Maintient les objets dans les limites du plateau.

- **Justification**
- Choix d�une approche par volumes simples pour limiter le co�t computationnel.
- Adapt�e � la nature discr�te et topologique du plateau de Mancala.

**8. Personnalisation & th�mes**

- **Syst�me de th�mes**
- Styles pr�d�finis coh�rents (bois classique, pierre, m�tal, etc.).
- Param�tres globaux regroup�s dans `ThemeManager`.

- **Modification d�assets**
- Variation des mat�riaux et textures des objets cl�s.
- Ajustement des couleurs et des propri�t�s sp�culaires.

- **R�affectation de textures**
- `TextureManager` permet le rechargement dynamique sans red�marrage.

**9. Interface utilisateur (ImGui)**

- **Fen�tre de score**
- Affichage des scores en temps r�el.

- **Panneau d�aide**
- R�capitulatif des commandes et r�gles.

- **Panneau de statistiques**
- Nombre de coups, dur�e de la partie, �tat des tours.

**10. Syst�me d��clairage**

- **Configuration multi-lumi�re**
- Key light : principale, accentue le relief.
- Fill light : r�duit les ombres dures.
- Back light : s�paration du sujet et profondeur.

- **Activation/d�sactivation**
- Contr�les pour activer une ou plusieurs sources.

- **Justification**
- La configuration tri-point �quilibre la lisibilit� des formes et la cr�dibilit� du rendu.

**11. Effets visuels & r�alisme**

- **Reflets sp�culaires**
- Rendent les mat�riaux plus plausibles.

- **Att�nuation**
- Contribue � la perception de profondeur.

- **Perception de profondeur**
- Combinaison d�ombres, att�nuation et perspective.

**12. Justification des choix techniques**

- **Pourquoi OpenGL**
- Contr�le bas niveau du pipeline, ad�quat pour une d�monstration p�dagogique.

- **Pourquoi Blinn-Phong**
- Bon compromis entre r�alisme et performance, adapt� � un projet temps r�el.

- **Pourquoi le ray casting**
- S�lection pr�cise et ind�pendante du rendu, extensible � d�autres objets.

- **Pourquoi une architecture modulaire**
- Facilite l�extension des fonctionnalit�s, la maintenance et la r�utilisation.

**13. Consid�rations de performance**

- R�duction des appels de rendu par regroupement d�objets.
- Utilisation d�instances pour les graines r�p�titives.
- Limitation des tests de collision aux objets pertinents.

**14. Compilation et ex�cution**

Pr�-requis : CMake, compilateur C++17, OpenGL, GLFW, GLAD, ImGui, GLM.

Exemple de compilation :

```bash
cmake -S . -B build
cmake --build build --config Release
```

Exemple d�ex�cution :

```bash
./build/Release/Mancala3D
```

**15. Conclusion**

Le projet d�montre la capacit� � int�grer une logique de jeu non triviale avec une architecture de rendu 3D moderne. L�approche retenue met l�accent sur la rigueur technique, la qualit� visuelle et l�ergonomie utilisateur, tout en justifiant les choix d�impl�mentation selon des crit�res acad�miques.

**16. Am�liorations possibles**

- Ajout d�ombres projet�es pour renforcer la profondeur.
- Int�gration d�animations proc�durales plus fines.
- Mode multijoueur en r�seau.
- Sauvegarde et reprise de partie.