Ce projet C++ permet de faire de la synthèse d'images (au format .ppm) grâce au raytracing. Cette version est destinée à tourner sur un GPU. Plus particulièrement, il fonctionne grâce à OpenAcc, et nécessite donc une carte graphique NVIDIA.

# Lancer le programme
Pour lancer le programme (une fois que la description de l'image à générer a été faite au programme dans le code), il faut d'abord compiler.
Cependant, un compilateur standard pour le C++ tel que gcc ne fonctionnera pas la plupart du temps.
Il vous faut installer NVIDIA HPC SDK, un compilateur qui permettra aux commandes OpenAcc de fonctionner.
Avec un compilateur classique le programme devrait fonctionner mais s'exécutera uniquement sur le CPU, et la génération sera beaucoup plus lente, trop lente pour être utilisable si votre scène contient plus que quelques centaines de primitives (sphères et triangles).

NVIDIA HPC SDK ne possède pas de version native pour Windows, si vous utilisez un tel appareil, vous devrez installer préalablement un environnement linux (ex : WSL).

Une fois fois cela fait, vous pouvez compiler en exécutant dans un terminal dont le dosser courant contient main.cpp la commande suivante : \
`nvc++ -acc -O3 -gpu=votre_compute_capability,fastmath -Minfo=accel main.cpp -o nom_de_l_executable`
- `-acc` indique au compilateur qu'on utilise OpenAcc ;
- `-O3` (falcultatif) indique au compilateur de procéder automatiquement à une optimisation des performances (l'exécution sera plus rapide) ;
- `-gpu=votre_compute_capability,fastmath` (facultatif) indique au compilateur sur quelle carte graphique sera exécuté le programme. Cela permet au compilateur d'effectuer des optimisations spécifiques à votre machine. Pour connaitre quelle option mettre, vous pouvez aller sur [cette page](https://developer.nvidia.com/cuda/gpus) et chercher votre carte graphique pour connaitre sa *compute capability*. Sinon, vous pouvez indiquer ccnative et le compilateur essaiera de détecter seul quelle est votre carte. L'option a mettre est alors cc suivi des numéros de cette dernière sans le ".". Ajouter l'option fastmath accélère certaines opérations sur les flottants, je vous conseille donc de mettre l'option aussi ;
- `-Minfo=accel` (facultatif) affiche des informations dans la console sur le travail effectué par le compilateur.

Il ne vous reste plus qu'à exécuter l'exécutable généré. Sous linux par exemple :
`./nom_de_l_executable`

-------------------------------------------------------------------

# Décrire une image au programme

Plusieurs exemples sont donnés dans le fichier Scenes.hpp (qui fonctionne avec la fonction `classic` de main.cpp).
En particulier, `lambertian_exemple` est commenté de manière à décrire les différentes étapes de la génération d'une image.

## Instructions minimales
```c++
vector<Object> world;

float im_ratio = 1.f;
int im_width = 1024;
Camera cam(im_ratio, im_width);

Material mat = create_lambertian(Color(1, 0, 0));
Object sphere = create_sphere(Point3(0, 1, 1), 1, mat);
world.push_back(sphere);

vector<Color> fb = render(cam, world);

ofstream fout("images/mon_image.ppm");
write_result(fout, cam, fb);
```

On commence par créer une liste `world` qui contiendra toutes les primitives dans la scène.
Il faut aussi créer une caméra, pour laquelle il faut au moins spécifier la résolution et la largeur (en pixels) de l'image à générer. Les options pour paramétrer la caméra sont détaillées plus bas.

On peut alors créer des objets pour la scène (au moins un, sinon une erreur se produira lorsque vous essaierez de lancer le programme).
Dans cet exemple, `mat` est un lambertien de couleur rouge, et `sphere`  est une sphère de centre (1, 0, 0) et de rayon 1.
`world.push_back(sphere)` permet d'ajouter la sphère créée à la scène.

Une fois qu'on a décrit la scène au programme, on appelle la fonction render qui prend la caméra et la scène en arguments et qui retourne l'image sous la forme d'un *framebuffer*, c'est-à-dire d'une liste de taille `LARGEURxHAUTEUR` contenant à l'indice i le pixel de coordonnées `(i // LARGEUR, i % LARGEUR)` sur l'image (le pixel de coordonnées `(0, 0)` est en haut à gauche).

Enfin, pour enregistrer l'image au format .ppm, on utilise la fonction `write_result` qui prend en paramètre une `ofstream` dans lequel le résultat doit être mis, la caméra et le *framebuffer* dans lequel est enregistré l'image.

## Le type `Vec3`

Avant de décrire plus en détail les paramètres de génération, je vais vous introduire le type `Vec3`.
Celui-ci représente un vecteur à trois dimensions standard, que l'on peut additionner, multiplier (par un scalaire ou par un aure vecteur, dans ce second cas il s'agira d'une multiplication coordonnée par coordonnée).

Pour mieux comprendre le rôle des différents paramètres, l'alias `Point3` permet de distinguer les points et les vecteurs.
Cependant il faut noter que pour l'ordinateur, il ne s'agit pas de deux types distincts, ne vous étonnez donc pas si le compilateur accepte sans rechigner que vous inversiez les valeurs `cam.center` et `cam.look_direction`!
Le seul indice de votre erreur sera le rendu qui ne correspondra pas à vos attentes.

De même pour représenter les couleurs, un alias du type `Vec3` est utilisé : `Color`. Les couleurs sont dans le format rvb, mais chaque composante est ramenée à une valeur comprise entre 0 et 1.
Ainsi, la couleur $(R, V, B)$ dans le format RVB ($R, V, B \in ⟦0, 256⟧$) est représentée par l'objet `Color(R/256, V/256, B/256)` dans le code.

## Les réglages de la caméra

Les attributs des objets de types `Camera` sont les suivants :
- `center` - position du centre de la caméra ;
- `look_direction` - direction dans laquelle la caméra pointe ;
- `up` - le haut pour la caméra, donc pour l'image générée (si la caméra était votre tête, changer `up` reviendrait à tourner cette dernière) ;
- `v_fov` - l'angle de vue, c'est-à-dire l'angle de centre `center` formé avec deux points extrèmes haut et bas de ce qui est vu par la caméra (si la caméra était votre tête, diminuer `v_fov` reviendrait à plisser des yeux) ;
- `ratio` - le rapport largeur de l'image sur hauteur de l'image générée ;
- `im_width` - la largeur (en pixels) de l'image générée ;
- `samples_per_pixel` - le nombre de rayons lancés dans la scène pour chaque pixel de l'image. Plus ce nombre est grand, moins l'image sera bruitée. La complexité de l'algorithme est linéaire en ce paramètre ;
- `max_depth` - les rayons lancés peuvent rebondir sur les objets de la scène. Pour évier qu'un rayon rebondisse indéfiniment sans atteindre de source de lumière et sans se perdre à l'infini, une limite de rebond est fixé pour chaque rayon. Plus ce nombre sera élevé, plus l'image sera réaliste (en particulier les reflets s'améliorent). Il faut aussi noter qu'un réglage trop bas de ce paramètre rendra aussi une image très sombre. La complexité dans le pire cas de l'algorithme est linéaire en ce paramètre. Cependant augmenter ce paramètre au-delà d'un certain seuil (qui dépend de l'abondance des sources de lumière et d'à quel point la scène est "ouverte" sur l'arrière-plan) aura peu d'influence sur le temps d'exécution (de même que sur le rendu) ;
- `defocus_angle` - le rayon angulaire du disque centré autour de `center` dans lequel l'origine des rayons lancés est choisi. Il sert à créer une profondeur de champ et à simuler la mise au point sur un plan en particulier. Plus ce rayon est grand, plus la distance au point sur lequel est fait la mise au point augmente le flou. Une valeur inférieure ou égale à 0 supprime la profondeur de champ, et tous les objets seront nets, quelle que soit leur position ;
- `focus_dist` - la distance au centre de la caméra (dans la direction d'observation) du point sur lequel la mise au point est fait ;
- `bg_type` - le type d'arrière-plan, définissant la couleur d'un rayon qui se perd à l'infini.

Pour créer un nouvel arrière-plan, il faut aller dans `Camera.hpp`, puis :
- ajouter un type dans `BackgroundType` ;
- ajouter un cas à la fonction `background_color` ;
- régler l'attribut cam.bg_type au type ajouté dans `BackgroundType`.
Par défaut, la couleur de l'arrière-plan renvoyé ne peut dépendre que des attributs enregistrés dans la caméra et de la direction du rayon qui se perd à l'infini.
Cependant, pour ajouter d'autres facteurs pris en compte il suffit d'ajouter un paramètre à `background_color` et de mettre à jour l'appel à celle-ci dans les fonctions `rayColorBVH` et `rayColorLin` du fichier `Raytracer.hpp`.

## Les types de matériaux

Les matériaux déterminent le comportement d'un rayon lumineux au contact d'un objet.
Il y en as quatre types.

### Le type `Lambertian`

Les objets fait de ce matériau diffusent la lumière de la même manière quelque soit la direction d'observation.
On peut régler l'attribut `albedo` qui correspond à la couleur du matériau.
La fonction `create_lambertian` prend en paramètre une couleur et retourne un matériau de type `Lambertian` de cette couleur.

### Le type `Metal`

Les objets fait de ce matériau se comportent comme des métaux.
On peut régler l'attribut `albedo` qui correspond à la couleur du matériau, et l'attribut `fuzz` qui règle l'effet de flou.
`fuzz` peut prendre des valeurs entre $0$ et $1$ ; plus le réglage est proche de $0$, plus le métal réfléchi parfaitement la lumière et plus il est proche de $1$, plus le métal diffuse la lumière.
La fonction `create_metal` prend en paramètre une couleur et une valeur pour `fuzz` et retourne un matériau de type `Metal` avec ces attributs pour `albedo` et `fuzz`.

### Le type `Dielectric`

Les objets fait de ce matériau sont transparents.
On peut régler l'attribut `n` qui est l'indice de réfraction du milieu intérieur à l'objet relativement au milieu extérieur (nInt/nExt).
La fonction `create_dielectric` prend en paramètre un indice de réfraction et retourne un matériau de type `Dielectric` d'incicde réfraction relatif correspondant.

### Le type `Diffuse_light`

Les objets fait de ce matériau émettent de la lumière.
On peut régler l'attribut `color` à la couleur de la lumière souhaitée.
La fonction `create_diffuse_light` prend en paramètre une couleur et retourne un matériau de type `Diffuse_light` de la couleur donnée.

### Créer un nouveau type de matériau

1. Ajouter un identificateur `MON_MATERIAU` dans `MATERIAL_TYPE` ;
2. Créer une structure `Mon_materiau` avant la définition de `Material` dans `Material.hpp`. Elle doit contenir un nombre quelconque d'attributs, un ou plusieurs constructeur, une méthode `scatter` si un rayon peut rebondir (réfraction ou réflexion) après avoir intersecté un matériau de ce type, et une méthode `emitted` si le matériau émet de la lumière. On décrit ce que doivent faire ces méthodes plus loin.
3. Dans la structure `Material`, ajouter la ligne `Mon_materiau mon_materiau;` dans l'union d'attributs ;
4. Mettre à jour la méthode `Material::operator=` pour ajouter le cas de la copie de votre type de matériau ;
5. Si vous avez une méthode `scatter` pour votre matériau, ajouter le cas de votre matériau dans `Materiau::scatter` ;
6. Si vous avez une méthode `emitted` pour votre matériau, ajouter le cas de votre matériau dans `Material::emitted` ;
7. Créer une version de `create_mon_materiau` pour chaque constructeur de `Mon_materiau` qui automatisera la création d'un objet de type `Mon_materiau`.

Pour les méthodes `scatter` et `emitted`, il est très important d'écrire un prototype lors de la création de `struct`, puis d'écrire votre fonction en-dessous de la déclaration avec `struct`, en écrivant à la ligne précédente `#pragma acc routine seq`.
C'est une instruction pour que le compilateur sache comment traiter votre méthode lors de la parallélisation.

La méthode `scatter` prend au minimum deux paramètres - `Color &attenuation` et `Ray &scattered` - et retourne un booléen.
Elle est appelée quand un rayon intersecte un objet de type dont le matériau est de type `Mon_materiau`. \
Son rôle est de remplir `attenuation` avec la couleur qui serait prise par un rayon de lumière blanche confondu avec le rayon incident suite au contact avec ce matériau, de remplir `scattered` avec le nouveau rayon qui provient de cet objet s'il y a lieu (réfraction du rayon incident par exemple), et de retourner un booléen indiquant s'il existe bien un rayon rebondit (ce qui revient à indiquer si le rayon incident a absorbé le rayon incident).
Remarquez que pour un matériau lumineux, ne pas avoir de rayon qui rebondit peut signifier que le rayon incident était en réalité émis par l'objet : je rappelle que le raytracing est basé sur le retour inverse de la lumière, et que les rayons sont lancé depuis la caméra vers les sources lumineuses pour simuler le comportement inverse. \
Les autres paramètres que vous pouvez utiliser sont par défaut le rayon incident, l'instance de `HitRecord` associée à la collision (voir plus loin), et le générateur d'aléatoire associé au pixel.
Cependant vous pouvez utiliser d'autres paramètres, il faut juste préalablement les ajouter aux paramètres de la méthode `scatter` de `Material` et mettre à jour l'appel de celle-ci dans les fonctions `rayColorBVH` et `rayColorLin` du fichier `Raytracer.hpp`.
Si vous utilisez un paramère de type HitRecord, pensez à écrire le code de votre méthode après la structure `HitRecord` dans le fichier.

La méthode `emitted` doit retourner une couleur, celle émise par votre matériau.
Par défaut, elle ne peut prendre aucun paramètre, mais on peut en ajouter de la même manière que pour la méthode `scatter`.

## Les types de primitives

Les primitives sont la forme des objets.
Une primitive est par défaut un triangle, une sphère.
J'ai également ajouté une primitive quad qui permet de faire des parallélogrammes sans passer par deux triangles (en plus d'accélérer la description de certaines scènes, les calculs sont deux fois plus rapides).

Toutes les fonctions de création de primitives peuvent prendre 3 paramètres tetaw, tetay, tetaz en plus de ceux qui seront décrits qui indique que l'objet doit subir une rotation autour de l'axe respectivement des abscisses, des ordonnées et des côtes.

### Les sphères

Ces primitives sont, comme leur nom l'indique, des sphères (creuses donc).
La structure qui leur est associée est `Sphere`.
Elles sont définies par un centre `center`, un rayon `radius` et un matériau `mat`.

La fonction `create_sphere` prend en arguments ces trois paramètres et retourne une instance de `Sphere` correspondante.

### Les triangles

Ces primitives sont, comme leur nom l'indique, des triangles (plats donc).
La structure qui leur est associée est `Triangle`.
Elles sont définies par une origine `origin`, deux côtés `u` et `v`, un vecteur normal `normal` (pour l'orientation) et un matériau `mat`.

Une version de la fonction `create_triangle` prend en arguments une origine de type `Point3`, deux côtés de type `Vec3` et un matériau et retourne une instance de `Triangle` correspondante (`normal` est alors définie par `u ∧ v`). \
Une deuxième version prend en argument trois points, une normale et un matériau et retourne une instance de `Triangle` correspondant au triangle dont les sommets sont les points donnés en argument, orienté suivant le vecteur normal donné en argument dans le matériau donné en argument.

### Les parallélogrammes

Ces primitives sont, comme leur nom l'indique, des parallélogrammes (plats donc).
La structure qui leur est associée est `Quad`.
Elles sont définies par une origine `origin`, deux côtés `u` et `v`, un vecteur normal `normal` (pour l'orientation) et un matériau `mat`.

La fonction `create_quad` prend en arguments une origine de type `Point3`, deux côtés de type `Vec3` et un matériau et retourne une instance de `Quad` correspondante (`normal` est alors définie par `u ∧ v`). \


### Ajouter des primitives

Il est possible d'ajouter des primitives, mais a priori ça ne devrait pas être utile grâce aux meshs (voir section suivante).
Si vous souhaitez tout de même en ajouter, voici un résumé de ce qu'il faut faire :
1. Ajouter un type `OBJ_MA_PRIMITIVE` dans `ObjectType` dans `Object.hpp` ;
2. Créer un structure `MA_PRIMITIVE` avant la définition de `Object` dans `Object.hpp`, qui doit posséder au moins les attributs `AABB bbox` et `Material mat` ;
3. Écrire les méthodes `hit` et le ou les constructeurs (pour `hit`, de la même manière que pour les méthodes `scatter` et `emitted` des matériaux il faut utiliser l'instruction `#pragma acc routine seq`). Celles-ci doivent impérativement renseigner l'instance de `HitRecord` passée en paramètre (obligatoire) s'il y a intersection ; 
4. Dans la structure `Object` ajouter la ligne `Ma_primitive ma_primitive;` dans l'union d'attributs ;
5. Ajouter les cas associés à `Ma_primitive` dans les méthodes `&operator=`, `hit_stat` et `init_aabb` de `Object` ;
6. Créer une version de `create_ma_primitive` pour chaque constructeur de `Ma_primitive` que vous avez écrite ;

## Les meshs (maillages)

Pour rendre des objets quelconques, on utilise des maillages, ou meshs.
Un mesh est un ensemble de triangles assemblés de manière à former un objet quelconque.

Pour ce raytracer, on utilise la classe `Meshreader` pour rendre des meshs.
La structure `MeshReader` est utilisée pour gérer les interactions entre les fichiers `.obj` et le programme.
Pour ajouter un mesh enregistré dans un fichier .obj à la scène, il faut créer une instance de la classe `MeshReader` avec en argument le chemin depuis le dossier courant jusqu'au fichier `.obj` et le matériau du mesh.
Trois arguments supplémentaires `tetax`, `tetay` et `tetaz` peuvent être ajoutés, représentant l'angle de rotation autour de l'axe respectivement des abscisses, des ordonnées et des côtes.

Pour qu'un fichier `.obj` soit correctement lu, il faut s'assurer :
1. Que tous la forme soit constituée uniquement de triangles
2. Que le format des triangles soit `f som//norm som//norm som//norm` ou `f som/text/norm som/text/norm som/text/norm`

Enfin, pour ajouter les triangles du mesh à la scène stockée dans le vecteur `world`, il faut utiliser la méthode convert de `MeshReader` à qui on passe `world` en argument.

## La génération de nombres aléatoires avec `RNG`

Si vous souhaitez apporter des modifications au projet, il vous faudra probablement utiliser de l'aléatoire.
L'aléatoire est géré avec la structure `RNG` (du fichier `Random.hpp`).
Celle-ci utilise l'algorithme *xoshift32*.

Pour initialiser un nouveau générateur, il faut créer un instance de `RNG` puis donner une valeur non nulle à l'attribut `state`.
C'est la graine du générateur.
Vous pouvez utiliser ensuite les différentes méthodes de `RNG`.
Pour la liste de celles-ci et ce qu'elles génèrent, je vous invite à regarder directement dans `Random.hpp`, les commentaires sont très clairs.

Si vous souhaitez faire de la cryptographie, vous devrez utiliser un autre générateur d'aléatoire que `RNG`, qui ne serait pas sécurisé.
Mais *a priori* ça ne devrait pas être nécessaire dans le cadre d'un raytracer !

## La structure `HitRecord`

`HitRecord` sert à enregistrer toutes les informations liées aux intersections.
Les différents attributs sont commentés dans le fichier `Material.hpp`.
Les attributs `normal` et `front_face` doivent être renseignés à l'aide de la méthode `set_sens_normal`.
Des attributs peuvent y être ajoutés sans problème, à condition de remplir ceux-ci dans les instances de `HitRecord` là où c'est nécessaire.
