# Philosophers - Projet 42

## Description

Le problème des philosophes est un problème classique en informatique qui illustre les défis de la synchronisation dans la programmation concurrente. Des philosophes sont assis autour d'une table ronde, alternant entre manger, dormir et penser. Ils ont besoin de deux fourchettes pour manger, mais il n'y a qu'une fourchette entre chaque paire de philosophes adjacents.

## Structure du Projet

```
philosopher/
├── Makefile
├── include/
│   └── philo.h          # Définitions et prototypes
├── src/
│   ├── main.c           # Point d'entrée et gestion des arguments
│   ├── utils.c          # Fonctions utilitaires
│   ├── init.c           # Initialisation des structures
│   ├── philo.c          # Logique des philosophes
│   └── monitor.c        # Surveillance (mort/repas terminés)
├── bin/                 # Fichiers objets (généré)
└── philo               # Exécutable (généré)
```

## Compilation et Utilisation

### Compilation
```bash
make            # Compile le projet
make clean      # Supprime les fichiers objets
make fclean     # Supprime tout (objets + exécutable)
make re         # Recompile entièrement
```

### Utilisation
```bash
./philo [nb_philo] [time_to_die] [time_to_eat] [time_to_sleep] [nb_meals]
```

**Paramètres :**
- `nb_philo` : Nombre de philosophes (et de fourchettes)
- `time_to_die` : Temps en ms avant qu'un philosophe meure de faim
- `time_to_eat` : Temps en ms pour manger
- `time_to_sleep` : Temps en ms pour dormir
- `nb_meals` : (Optionnel) Nombre de repas par philosophe

**Exemples :**
```bash
./philo 4 410 200 200        # 4 philosophes, simulation infinie
./philo 5 800 200 200 7      # 5 philosophes, 7 repas chacun
./philo 1 800 200 200        # 1 philosophe (impossible de manger)
```

## Architecture du Code

### Structures de Données

```c
// Structure représentant un philosophe
typedef struct s_philo
{
    int             id;              // Numéro du philosophe (1-n)
    int             meals_eaten;     // Nombre de repas consommés
    long long       last_meal_time;  // Timestamp du dernier repas
    pthread_t       thread;          // Thread du philosophe
    pthread_mutex_t *left_fork;      // Fourchette de gauche
    pthread_mutex_t *right_fork;     // Fourchette de droite
    struct s_data   *data;          // Référence aux données partagées
} t_philo;

// Structure contenant les données partagées
typedef struct s_data
{
    int             nb_philo;        // Nombre de philosophes
    int             time_to_die;     // Temps avant la mort
    int             time_to_eat;     // Temps pour manger
    int             time_to_sleep;   // Temps pour dormir
    int             nb_meals;        // Nombre de repas requis (-1 si infini)
    int             dead;            // Flag indiquant si un philosophe est mort
    long long       start_time;      // Timestamp de début de simulation
    pthread_mutex_t *forks;          // Tableau des fourchettes (mutex)
    pthread_mutex_t print_mutex;     // Mutex pour l'affichage
    pthread_mutex_t dead_mutex;      // Mutex pour le flag de mort
    t_philo         *philos;         // Tableau des philosophes
} t_data;
```

### Schéma de la Table

```
        Philo 1
         🍴 1
    🍴 4       🍴 2
  Philo 4     Philo 2
    🍴 3       🍴 3
        Philo 3

Chaque philosophe a accès à deux fourchettes :
- Philo N : fourchettes N et (N+1)%nb_philo
```

## Détail des Modules

### 1. main.c - Point d'Entrée
```c
// Vérifie la validité des arguments
static int check_args(int argc, char **argv);

// Libère toutes les ressources
void cleanup(t_data *data);

int main(int argc, char **argv);
```

**Responsabilités :**
- Validation des arguments de la ligne de commande
- Initialisation des structures de données
- Lancement de la simulation
- Nettoyage des ressources

### 2. utils.c - Fonctions Utilitaires
```c
// Conversion string vers int
int ft_atoi(const char *str);

// Obtient le temps actuel en millisecondes
long long get_time(void);

// Sleep précis en millisecondes
void ft_usleep(int ms);

// Affichage thread-safe des statuts
void print_status(t_philo *philo, char *status);
```

### 3. init.c - Initialisation
```c
// Initialise les données principales
int init_data(t_data *data, char **argv);

// Initialise les fourchettes (mutex)
int init_forks(t_data *data);

// Initialise les philosophes
int init_philos(t_data *data);
```

**Processus d'initialisation :**
1. Parse les arguments
2. Crée les mutex pour les fourchettes
3. Initialise chaque philosophe avec ses fourchettes
4. Configure les mutex de synchronisation

### 4. philo.c - Logique des Philosophes
```c
// Acquisition des fourchettes (évite les deadlocks)
static void take_forks(t_philo *philo);

// Processus de manger
static void eat(t_philo *philo);

// Processus dormir et penser
static void sleep_and_think(t_philo *philo);

// Routine principale de chaque philosophe
void *philo_routine(void *arg);

// Lance tous les threads
int start_simulation(t_data *data);
```

**Cycle de vie d'un philosophe :**
1. **Penser** → Réfléchit (pas d'action spécifique)
2. **Prendre les fourchettes** → Acquire les mutex
3. **Manger** → Met à jour `last_meal_time` et `meals_eaten`
4. **Dormir** → Sleep pendant `time_to_sleep`
5. **Répéter** jusqu'à la mort ou fin des repas

### 5. monitor.c - Surveillance
```c
// Vérifie si un philosophe est mort de faim
int check_death(t_data *data);

// Vérifie si tous ont terminé leurs repas
int check_meals(t_data *data);
```

## Gestion de la Synchronisation

### Prévention des Deadlocks
```c
// Les philosophes pairs et impairs prennent les fourchettes dans un ordre différent
if (philo->id % 2 == 0)
{
    pthread_mutex_lock(philo->right_fork);  // Pair : droite puis gauche
    pthread_mutex_lock(philo->left_fork);
}
else
{
    pthread_mutex_lock(philo->left_fork);   // Impair : gauche puis droite  
    pthread_mutex_lock(philo->right_fork);
}
```

### Protection des Données Critiques
- **`print_mutex`** : Évite le mélange des messages de sortie
- **`dead_mutex`** : Protège le flag de mort partagé
- **`forks[]`** : Chaque fourchette est un mutex

### Gestion du Temps
```c
long long get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
```

## Exemples de Tests

### Test de Base
```bash
./philo 4 410 200 200
```
**Sortie attendue :**
```
0 1 has taken a fork
0 1 has taken a fork
0 1 is eating
0 3 has taken a fork
0 3 has taken a fork
0 3 is eating
200 1 is sleeping
200 3 is sleeping
...
```

### Test avec Limite de Repas
```bash
./philo 5 800 200 200 7
```
La simulation s'arrête quand tous les philosophes ont mangé 7 fois.

### Test de Mort
```bash
./philo 4 310 200 200
```
Un philosophe devrait mourir car `time_to_die < time_to_eat + time_to_sleep`.

### Tests de Validation
```bash
# Aucun philosophe ne devrait mourir
./philo 1 800 200 200
./philo 5 800 200 200
./philo 5 800 200 200 7

# Un philosophe devrait mourir
./philo 4 310 200 200
./philo 4 200 205 200
```

## Messages de Sortie

Format : `[timestamp_ms] [philo_id] [action]`

**Actions possibles :**
- `has taken a fork`
- `is eating`
- `is sleeping`
- `is thinking`
- `died`

## Debugging et Outils

### Vérification des Data Races
```bash
valgrind --tool=helgrind ./philo 4 410 200 200
```

### Vérification des Fuites Mémoire
```bash
valgrind --leak-check=full ./philo 4 410 200 200
```

### Analyse des Performances
```bash
# Compter les repas
./philo 4 300 100 100 10 | grep "is eating" | wc -l

# Observer les premiers événements
./philo 4 300 100 100 | head -20
```

## Règles de la Norme 42

- ✅ Pas plus de 25 lignes par fonction
- ✅ Pas plus de 5 fonctions par fichier
- ✅ Variables et fonctions nommées selon les conventions
- ✅ Pas de variables globales
- ✅ Fonctions statiques pour les fonctions internes
- ✅ Commentaires en français
- ✅ Makefile avec règles standard

## Algorithme de Résolution

1. **Initialisation** : Créer n philosophes et n fourchettes
2. **Démarrage** : Lancer n threads (un par philosophe)
3. **Surveillance** : Thread principal surveille mort/fin des repas
4. **Synchronisation** : Utiliser des mutex pour éviter les races
5. **Terminaison** : Attendre tous les threads et nettoyer

## Défis Techniques Relevés

- **Deadlock** : Résolu par l'ordre différent d'acquisition des fourchettes
- **Race Conditions** : Protégées par des mutex appropriés
- **Precision Temporelle** : `ft_usleep()` pour un timing précis
- **Thread Safety**