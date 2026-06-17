Ce projet C++ permet de faire de la synthèse d'images (au format .ppm) grâce au raytracing. Cette version est destinée à tourner sur un GPU. Plus particulièrement, il fonctionne grâce à OpenAcc, et nécessite donc une carte graphique NVIDIA.

# Lancer le programme
Pour lancer le programme (une fois que la description de l'image à générer a été faite au programme dans le code), il faut d'abord compiler.
Cependant, un compilateur standard pour le C++ tel que gcc ne fonctionnera pas la plupart du temps.
Il vous faut installer NVIDIA HPC SDK, un compilateur qui permettra aux commandes OpenAcc de fonctionner.
Avec un compilateur classique le programme devrait fonctionner mais s'exécutera uniquement sur le CPU, et la génération sera beaucoup plus lente, trop lente pour être utilisable si votre scène contient plus que quelques centaines de primitives (sphères et triangles).

NVIDIA HPC SDK ne possède pas de version native pour Windows, si vous utilisez un tel appareil, vous devrez installer préalablement un environnement linux (ex : WSL).

Une fois fois cela fait, vous pouvez compiler en exécutant dans un terminal dont le dosser courant contient main.cpp la commande suivante : \
`nvc++ -acc -O3 -gpu=votre_compute_capability,fastmath -Minfo=accel main.cpp -o nom_de_l_executable`
- `-acc` indique au compilateur qu'on utilise OpenAcc
- `-O3` (falcultatif) indique au compilateur de procéder automatiquement à une optimisation des performances (l'exécution sera plus rapide)
- `-gpu=votre_compute_capability,fastmath` (facultatif) indique au compilateur sur quelle carte graphique sera exécuté le programme. Cela permet au compilateur d'effectuer des optimisations spécifiques à votre machine. Pour connaitre quelle option mettre, vous pouvez aller sur [cette page](https://developer.nvidia.com/cuda/gpus) et chercher votre carte graphique pour connaitre sa *compute capability*. Sinon, vous pouvez indiquer ccnative et le compilateur essaiera de détecter seul quelle est votre carte. L'option a mettre est alors cc suivi des numéros de cette dernière sans le ".". Ajouter l'option fastmath accélère certaines opérations sur les flottants, je vous conseille donc de mettre l'option aussi
- `-Minfo=accel` (facultatif) affiche des informations dans la console sur le travail effectué par le compilateur

Il ne vous reste plus qu'à exécuter l'exécutable généré. Sous linux par exemple :
`./nom_de_l_executable`

-------------------------------------------------------------------

# Décrire une image au programme

Plusieurs exemples sont donnés dans le fichier Scenes.hpp (qui fonctionne avec la fonction `classic` de main.cpp).
En particulier, `lambertian_exemple` est commenté de manière à décrire les différentes étapes de description.

## Instructions minimales

```cpp {1}
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
