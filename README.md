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

### Passage au monde discret : rasterization

[Lien explicatif](https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage)
