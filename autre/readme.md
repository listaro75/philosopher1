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
    int             eating;          // Flag indiquant si le philosophe mange actuellement
    long long       last_meal_time;  // Timestamp du dernier repas
    pthread_mutex_t meal_mutex;      // Mutex pour protéger les données de repas
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

## Architecture des Threads et Mutex

### Vue d'Ensemble de la Concurrence

Le programme utilise **n+1 threads** pour n philosophes :
- **n threads philosophes** : Un thread par philosophe qui exécute le cycle manger/dormir/penser
- **1 thread monitor** : Surveille en continu les conditions de fin (mort ou repas terminés)

```
┌─────────────────────────────────────────────────────────────┐
│                    Thread Principal                          │
│  ┌─────────────────┐    ┌────────────────────────────────┐  │
│  │   Initialisation │    │         Nettoyage              │  │
│  │   - Structures   │    │   - pthread_join()             │  │
│  │   - Mutex        │    │   - Destruction des mutex      │  │
│  │   - Threads      │    │   - Libération mémoire         │  │
│  └─────────────────┘    └────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
           │                                    ▲
           ▼                                    │
┌─────────────────────────────────────────────────────────────┐
│                  Threads Créés                              │
│                                                             │
│  ┌──────────────┐  ┌──────────────┐         ┌─────────────┐ │
│  │   Thread     │  │   Thread     │   ...   │   Thread    │ │
│  │ Philosophe 1 │  │ Philosophe 2 │         │ Monitor     │ │
│  │              │  │              │         │             │ │
│  │ philo_routine│  │ philo_routine│         │ monitor()   │ │
│  └──────────────┘  └──────────────┘         └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Architecture des Mutex

Le programme utilise **plusieurs types de mutex** pour protéger différentes ressources partagées :

```c
// 1. FOURCHETTES (n mutex) - Une par fourchette
pthread_mutex_t forks[n];
// Protection : Accès exclusif aux fourchettes
// Utilisé par : Threads philosophes pour manger

// 2. AFFICHAGE (1 mutex global)
pthread_mutex_t print_mutex;
// Protection : Messages de sortie non mélangés
// Utilisé par : Tous les threads pour printf()

// 3. FLAG DE MORT (1 mutex global)
pthread_mutex_t dead_mutex;
// Protection : Variable 'dead' partagée
// Utilisé par : Thread monitor (écriture) et philosophes (lecture)

// 4. DONNÉES DE REPAS (n mutex) - Un par philosophe
pthread_mutex_t meal_mutex[i];
// Protection : last_meal_time, meals_eaten, eating
// Utilisé par : Thread philosophe (écriture) et monitor (lecture)
```

### Fonctionnement Détaillé des Threads

#### Thread Monitor - Surveillance Continue

```c
void *monitor(void *pointer)
{
    t_data *data = (t_data *)pointer;
    
    while (1)  // Boucle infinie de surveillance
    {
        // 1. Vérifier si un philosophe est mort
        if (check_death(data) == 1)
            break;
            
        // 2. Vérifier si tous ont fini de manger
        if (check_meals(data) == 1)
            break;
            
        // Petit délai pour éviter la surcharge CPU
        usleep(1000);
    }
    return (NULL);
}
```

**Fonctions de vérification :**

```c
int check_death(t_data *data)
{
    for (int i = 0; i < data->nb_philo; i++)
    {
        // Accès thread-safe au temps du dernier repas
        pthread_mutex_lock(&data->philos[i].meal_mutex);
        long long last_meal = data->philos[i].last_meal_time;
        int eating = data->philos[i].eating;
        pthread_mutex_unlock(&data->philos[i].meal_mutex);
        
        // Vérifier la mort UNIQUEMENT si le philosophe ne mange pas
        if ((get_time() - last_meal >= data->time_to_die) && eating == 0)
        {
            // Marquer la fin de la simulation
            pthread_mutex_lock(&data->dead_mutex);
            data->dead = 1;
            pthread_mutex_unlock(&data->dead_mutex);
            
            // Affichage thread-safe de la mort
            pthread_mutex_lock(&data->print_mutex);
            printf("%lld %d died\n", get_time() - data->start_time, data->philos[i].id);
            pthread_mutex_unlock(&data->print_mutex);
            
            return (1);  // Arrêter la simulation
        }
    }
    return (0);  // Continuer la simulation
}
```

#### Threads Philosophes - Cycle de Vie

```c
void *philo_routine(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    
    // Synchronisation : les philosophes pairs attendent 1ms
    // Cela évite que tous essaient de prendre les fourchettes en même temps
    if (philo->id % 2 == 0)
        ft_usleep(1);
    
    // Boucle principale du philosophe
    while (!dead_loop(philo))  // Tant que personne n'est mort
    {
        // 1. PRENDRE LES FOURCHETTES
        if (!take_forks(philo))
            break;  // Simulation terminée
            
        // 2. MANGER
        eat(philo);
        
        // 3. DORMIR
        dream(philo);
        
        // 4. PENSER
        think(philo);
    }
    return (NULL);
}
```

### Mécanismes de Synchronisation Avancés

#### 1. Prévention des Deadlocks

**Problème :** Si tous les philosophes prennent leur fourchette de gauche simultanément, deadlock garanti.

**Solution :** Tous les philosophes prennent d'abord la fourchette droite, puis la gauche.

```c
static int take_forks(t_philo *philo)
{
    // TOUJOURS : droite puis gauche
    pthread_mutex_lock(philo->right_fork);
    print_status(philo, "has taken a fork");
    
    // Cas spécial : 1 seul philosophe
    if (philo->data->nb_philo == 1)
    {
        ft_usleep(philo->data->time_to_die);  // Attendre la mort
        pthread_mutex_unlock(philo->right_fork);
        return (0);
    }
    
    pthread_mutex_lock(philo->left_fork);
    print_status(philo, "has taken a fork");
    return (1);
}
```

#### 2. Protection du Flag "eating"

**Problème crucial :** Un philosophe peut être déclaré mort alors qu'il est en train de manger.

**Solution :** Le flag `eating` protège contre les fausses morts :

```c
static void eat(t_philo *philo)
{
    // ÉTAPE 1 : Marquer le début du repas
    philo->eating = 1;  // Protection contre la mort
    print_status(philo, "is eating");
    
    // ÉTAPE 2 : Mise à jour thread-safe des données
    pthread_mutex_lock(&philo->meal_mutex);
    philo->last_meal_time = get_time();  // Nouveau timestamp
    philo->meals_eaten++;                // Incrémenter le compteur
    pthread_mutex_unlock(&philo->meal_mutex);
    
    // ÉTAPE 3 : Simulation du temps de manger
    ft_usleep(philo->data->time_to_eat);
    
    // ÉTAPE 4 : Fin du repas
    philo->eating = 0;  // Plus protégé contre la mort
    
    // ÉTAPE 5 : Libération des fourchettes
    pthread_mutex_unlock(philo->left_fork);
    pthread_mutex_unlock(philo->right_fork);
}
```

#### 3. Vérification Thread-Safe de la Mort

```c
static int dead_loop(t_philo *philo)
{
    pthread_mutex_lock(&philo->data->dead_mutex);
    if (philo->data->dead == 1)
    {
        pthread_mutex_unlock(&philo->data->dead_mutex);
        return (1);  // Simulation terminée
    }
    pthread_mutex_unlock(&philo->data->dead_mutex);
    return (0);  // Continuer
}
```

### Diagramme de Séquence - Cycle Complet

```
Philosophe 1    Fourchette 1    Fourchette 2    Monitor         Console
    │               │               │              │               │
    │──lock()────────▶│               │              │               │
    │               │               │              │──"taken fork"─▶│
    │──lock()────────────────────────▶│              │               │
    │               │               │              │──"taken fork"─▶│
    │               │               │              │               │
    │ eating = 1    │               │              │               │
    │               │               │              │──"is eating"──▶│
    │ last_meal = now               │              │               │
    │ meals_eaten++                 │              │               │
    │               │               │              │               │
    │   sleep(time_to_eat)          │              │               │
    │               │               │              │               │
    │ eating = 0    │               │              │               │
    │──unlock()─────▶│               │              │               │
    │──unlock()─────────────────────▶│              │               │
    │               │               │              │               │
    │               │               │              │──"is sleeping"▶│
    │   sleep(time_to_sleep)        │              │               │
    │               │               │              │               │
    │               │               │              │──"is thinking"▶│
    │               │               │              │               │
    │               │               │              ▼               │
    │               │               │        check_death()         │
    │               │               │        check_meals()         │
```

### Optimisations et Bonnes Pratiques

#### 1. Timing Précis
```c
void ft_usleep(int ms)
{
    long long start = get_time();
    
    // Boucle active + micro-sleeps pour plus de précision
    while ((get_time() - start) < ms)
        usleep(500);  // 0.5ms micro-sleep
}
```

#### 2. Affichage Thread-Safe
```c
void print_status(t_philo *philo, char *status)
{
    pthread_mutex_lock(&philo->data->print_mutex);
    
    // Vérifier que la simulation n'est pas terminée
    if (!dead_loop(philo))
    {
        long long timestamp = get_time() - philo->data->start_time;
        printf("%lld %d %s\n", timestamp, philo->id, status);
    }
    
    pthread_mutex_unlock(&philo->data->print_mutex);
}
```

#### 3. Démarrage Synchronisé
```c
// Dans start_simulation()
data->start_time = get_time();

// Initialiser tous les last_meal_time au même moment
for (int i = 0; i < data->nb_philo; i++)
{
    data->philos[i].last_meal_time = data->start_time;
}

// Les philosophes pairs commencent avec un délai pour éviter la compétition
if (philo->id % 2 == 0)
    ft_usleep(1);
```

### Points Critiques de l'Implémentation

#### Race Conditions Évitées

1. **Lecture/Écriture de `dead`** → Protégée par `dead_mutex`
2. **Lecture/Écriture de `last_meal_time`** → Protégée par `meal_mutex`
3. **Affichage concurrent** → Protégé par `print_mutex`
4. **Accès aux fourchettes** → Chaque fourchette a son propre mutex

#### Deadlocks Évités

1. **Acquisition uniforme** → Tous prennent droite puis gauche
2. **Cas du philosophe seul** → Gestion spéciale avec timeout
3. **Évitement de la famine** → Synchronisation par délai initial

#### Performance

1. **Monitor léger** → Vérifications courtes avec micro-sleeps
2. **Mutex granulaires** → Chaque ressource a son propre mutex
3. **Pas de polling actif** → Utilisation de usleep() dans les boucles

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
- Validation des arguments de la ligne de commande (nombres positifs, limites)
- Initialisation des structures de données via `init_data()`
- Lancement de la simulation via `start_simulation()`
- Nettoyage des ressources : destruction des mutex, libération mémoire

### 2. utils.c - Fonctions Utilitaires Thread-Safe
```c
// Conversion string vers int avec gestion d'erreurs
int ft_atoi(const char *str);

// Obtient le temps actuel en millisecondes (gettimeofday)
long long get_time(void);

// Sleep précis en millisecondes avec boucle active
void ft_usleep(int ms);

// Affichage thread-safe des statuts avec mutex
void print_status(t_philo *philo, char *status);
```

**Points critiques :**
- `print_status()` utilise `print_mutex` pour éviter les messages mélangés
- `ft_usleep()` combine `usleep(500)` et vérification temporelle pour la précision
- `get_time()` utilise `gettimeofday()` pour un timing millisecondes précis

### 3. init.c - Initialisation Thread-Safe
```c
// Initialise les données principales et parse les arguments
int init_data(t_data *data, char **argv);

// Crée et initialise n mutex pour les fourchettes
int init_forks(t_data *data);

// Initialise n philosophes avec leurs mutex individuels
int init_philos(t_data *data);
```

**Processus d'initialisation critique :**
1. **Parse des arguments** avec validation des limites
2. **Création des mutex globaux** : `print_mutex`, `dead_mutex`
3. **Création des fourchettes** : `n` mutex, un par fourchette
4. **Initialisation des philosophes** :
   - Attribution des fourchettes (gauche = `i`, droite = `(i+1)%n`)
   - Création du `meal_mutex` individuel
   - Initialisation de `eating = 0`, `meals_eaten = 0`
5. **Synchronisation temporelle** : `start_time` commun à tous

### 4. philo.c - Gestion des Threads Philosophes

#### Fonctions Statiques (Thread-Safe)
```c
// Acquisition thread-safe des fourchettes avec protection deadlock
static int take_forks(t_philo *philo);

// Processus de manger avec flag eating et mise à jour thread-safe
static void eat(t_philo *philo);

// Processus de dormir avec affichage thread-safe
static void dream(t_philo *philo);

// Processus de penser avec affichage thread-safe
static void think(t_philo *philo);

// Vérification thread-safe du flag dead
static int dead_loop(t_philo *philo);
```

#### Thread Monitor
```c
// Thread de surveillance qui tourne en arrière-plan
static void *monitor(void *pointer);
```

**Responsabilités du monitor :**
- Appel continu de `check_death()` et `check_meals()`
- Pas de sleep entre les vérifications pour réactivité maximale
- Arrêt automatique dès qu'une condition de fin est détectée

#### Routine Principale des Philosophes
```c
void *philo_routine(void *arg);
```

**Cycle de vie d'un thread philosophe :**
1. **Synchronisation initiale** : Les pairs attendent 1ms
2. **Boucle principale** : `while (!dead_loop(philo))`
   - `take_forks()` → Acquisition avec protection deadlock
   - `eat()` → Marque `eating=1`, met à jour `last_meal_time`
   - `dream()` → Sleep pendant `time_to_sleep`
   - `think()` → Affichage uniquement
3. **Sortie propre** : Retour `NULL` quand la simulation se termine

#### Gestion de la Simulation
```c
int start_simulation(t_data *data);
```

**Séquence de démarrage :**
1. **Création du thread monitor** : `pthread_create(&observer, NULL, &monitor, data)`
2. **Création des threads philosophes** : Boucle de `pthread_create()`
3. **Attente du monitor** : `pthread_join(observer, NULL)` - bloque jusqu'à condition de fin
4. **Attente des philosophes** : `pthread_join()` sur tous les threads philosophes
5. **Retour** : Signal de fin propre de la simulation

### 5. monitor.c - Surveillance Thread-Safe

#### Vérification des Morts
```c
int check_death(t_data *data);
```

**Algorithme de détection :**
1. **Parcours de tous les philosophes** séquentiellement
2. **Lecture thread-safe** : `meal_mutex` pour `last_meal_time` et `eating`
3. **Calcul du temps écoulé** : `current_time - last_meal_time`
4. **Condition de mort** : `temps_écoulé >= time_to_die ET eating == 0`
5. **Si mort détectée** :
   - Marque `dead = 1` (avec `dead_mutex`)
   - Affiche le message de mort (avec `print_mutex`)
   - Retourne `1` pour arrêter la simulation

**Point crucial :** Le flag `eating` empêche qu'un philosophe soit déclaré mort pendant qu'il mange.

#### Vérification des Repas Terminés
```c
int check_meals(t_data *data);
```

**Algorithme de vérification :**
1. **Vérification du mode** : Si `nb_meals == -1`, simulation infinie
2. **Comptage thread-safe** : Lecture de `meals_eaten` avec `meal_mutex`
3. **Condition de fin** : Tous les philosophes ont `meals_eaten >= nb_meals`
4. **Si terminé** :
   - Marque `dead = 1` (avec `dead_mutex`) pour arrêter tous les threads
   - Retourne `1` pour arrêter la simulation

### Synchronisation Inter-Modules

#### Communication Thread Monitor ↔ Threads Philosophes
- **Flag partagé `dead`** : Monitor (écriture) → Philosophes (lecture)
- **Protection** : `dead_mutex` pour tous les accès
- **Vérification** : `dead_loop()` appelé à chaque itération du cycle

#### Communication Thread Philosophe ↔ Monitor
- **Données partagées** : `last_meal_time`, `meals_eaten`, `eating`
- **Protection** : `meal_mutex` individuel par philosophe
- **Fréquence** : Monitor lit en continu, Philosophe met à jour à chaque repas

#### Synchronisation des Fourchettes
- **Ressource partagée** : Chaque fourchette = 1 mutex
- **Accès concurrent** : Maximum 2 philosophes par fourchette (voisins)
- **Protection deadlock** : Ordre d'acquisition uniforme (droite puis gauche)

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