#include "../include/philo.h"

/*
** Vérifie si les arguments sont valides
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

/*
** Libère toutes les ressources allouées
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