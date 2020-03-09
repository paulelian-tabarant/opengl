# OpenGL : notes

## Stockage des données de géométrie

### Vertices

![VAO expliqué](img-notes/vertex_array_objects.png)

Attention : respecter l'ordre suivant

* Initialisation du VAO
* Initialisation et assignation des données au VBO
* Initialisation et assignation des données à l'EBO
* Définition du `glVertexAttribPointer` avec les repères de lecture puis activation (`glEnableVertexAttribArray`)

## Caméra

Définie par 3 vecteurs :

* Position (origine du repère)
* Front (direction vers laquelle la caméra pointe)
* Up (sens de la caméra, vecteur d'élévation)

## Utilisation de shaders

### Bonnes pratiques

* toujours initialiser les variables shader APRÈS appel à la fonction `.use()`
* toujours normaliser les vecteurs de direction dans les calculs de lumière au sein des shaders
* éviter les opérations `inverse()` sur des matrices (ralentissements) : préférable de faire les calculs au préalable sur CPU
* Faire attention à ce que toutes les uniform du vertex shader sont définies correctement, sinon il est possible que rien ne soit affiché à l'écran

### Passage au monde discret : rasterization

[Lien explicatif](https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage)

### Outils pratiques

~~~C++
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
~~~

`offsetof` est une directive de préprocesseur qui permet de calculer le nombre d'octets en mémoire entre deux champs d'un type composé `struct`. Pratique pour spécifier un offset pour un attribut de Vertex à aller lire dans un buffer.

#### Assimp

![Schéma d'une scène Assimp](img-notes/assimp_structure.png)

Importation d'un nouveau modèle situé au chemin `path`
~~~C++
Assimp::Importer importer;
aiScene *scene = importer.ReadFile(path, <options>);
~~~

`<options>` permet de rajouter des *flags* comme `aiProcess_Triangulate` pour trianguler tous les éléments (si autres primitives que triangles sont dans le fichier, comme des quads) ou `aiProcess_FlipUVs` pour inverser l'axe Y au moment de l'application des coordonnées de texture, ou encore `aiProcess_GenNormals` pour calculer automatiquement les normales à partir des positions et des indices.
