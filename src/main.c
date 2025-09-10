#include "../include/philo.h"

/**
 * @brief Vérifie si les arguments de la ligne de commande sont valides
 * 
 * @param argc Nombre d'arguments (doit être 5 ou 6)
 * @param argv Tableau des arguments sous forme de chaînes
 *             argv[1] = nombre de philosophes (> 0)
 *             argv[2] = temps avant la mort en ms (> 0)
 *             argv[3] = temps pour manger en ms (> 0)
 *             argv[4] = temps pour dormir en ms (> 0)
 *             argv[5] = nombre de repas par philosophe (optionnel, > 0)
 * @return int 1 si les arguments sont valides, 0 sinon
 * 
 * Vérifie que :
 * - Le nombre d'arguments est correct (4 ou 5 paramètres + nom du programme)
 * - Tous les arguments ne contiennent que des chiffres
 * - Tous les arguments sont des entiers positifs
 */
static int	check_args(int argc, char **argv)
{
    int	i;
    int	j;

    if (argc < 5 || argc > 6)
        return (0);
    i = 1;
    while (i < argc)
    {
        j = 0;
        while (argv[i][j])
        {
            if (argv[i][j] < '0' || argv[i][j] > '9')
                return (0);
            j++;
        }
        if (ft_atoi(argv[i]) <= 0)
            return (0);
        i++;
    }
    return (1);
}

/**
 * @brief Libère toutes les ressources allouées dynamiquement
 * 
 * @param data Pointeur vers la structure contenant toutes les données du programme
 * 
 * Cette fonction nettoie proprement :
 * - Détruit tous les mutex des fourchettes
 * - Libère la mémoire allouée pour le tableau des fourchettes
 * - Détruit les mutex d'affichage et de mort
 * - Libère la mémoire allouée pour le tableau des philosophes
 * 
 * Doit être appelée avant la fin du programme pour éviter les fuites mémoire
 */
void	cleanup(t_data *data)
{
    int	i;

    if (data->forks)
    {
        i = 0;
        while (i < data->nb_philo)
        {
            pthread_mutex_destroy(&data->forks[i]);
            i++;
        }
        free(data->forks);
    }
    pthread_mutex_destroy(&data->print_mutex);
    pthread_mutex_destroy(&data->dead_mutex);
    if (data->philos)
        free(data->philos);
}

/**
 * @brief Point d'entrée principal du programme
 * 
 * @param argc Nombre d'arguments de la ligne de commande
 * @param argv Tableau des arguments :
 *             ./philo [nb_philo] [time_to_die] [time_to_eat] [time_to_sleep] [nb_meals]
 * @return int Code de retour : 0 si succès, 1 si erreur
 * 
 * Séquence d'exécution :
 * 1. Valide les arguments d'entrée
 * 2. Initialise toutes les structures de données
 * 3. Lance la simulation des philosophes
 * 4. Nettoie les ressources avant de terminer
 * 
 * En cas d'erreur à n'importe quelle étape, affiche un message d'erreur
 * et retourne 1 après avoir nettoyé les ressources déjà allouées
 */
int	main(int argc, char **argv)
{
    t_data	data;

    if (!check_args(argc, argv))
    {
        printf("Error: Invalid arguments\n");
        return (1);
    }
    if (!init_data(&data, argv))
    {
        printf("Error: Initialization failed\n");
        return (1);
    }
    if (!start_simulation(&data))
    {
        printf("Error: Simulation failed\n");
        cleanup(&data);
        return (1);
    }
    cleanup(&data);
    return (0);
}