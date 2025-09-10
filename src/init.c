#include "../include/philo.h"

/*
** Initialise les données principales du programme
*/
int	init_data(t_data *data, char **argv)
{
    data->nb_philo = ft_atoi(argv[1]);
    data->time_to_die = ft_atoi(argv[2]);
    data->time_to_eat = ft_atoi(argv[3]);
    data->time_to_sleep = ft_atoi(argv[4]);
    if (argv[5])
        data->nb_meals = ft_atoi(argv[5]);
    else
        data->nb_meals = -1;
    data->dead = 0;
    data->start_time = get_time();
    if (pthread_mutex_init(&data->print_mutex, NULL))
        return (0);
    if (pthread_mutex_init(&data->dead_mutex, NULL))
        return (0);
    if (!init_forks(data))
        return (0);
    if (!init_philos(data))
        return (0);
    return (1);
}

/*
** Initialise les fourchettes (mutex)
*/
int	init_forks(t_data *data)
{
    int	i;

    data->forks = malloc(sizeof(pthread_mutex_t) * data->nb_philo);
    if (!data->forks)
        return (0);
    i = 0;
    while (i < data->nb_philo)
    {
        if (pthread_mutex_init(&data->forks[i], NULL))
            return (0);
        i++;
    }
    return (1);
}

/*
** Initialise les philosophes
*/
int	init_philos(t_data *data)
{
    int	i;

    data->philos = malloc(sizeof(t_philo) * data->nb_philo);
    if (!data->philos)
        return (0);
    i = 0;
    while (i < data->nb_philo)
    {
        data->philos[i].id = i + 1;
        data->philos[i].meals_eaten = 0;
        data->philos[i].last_meal_time = data->start_time;
        data->philos[i].data = data;
        data->philos[i].left_fork = &data->forks[i];
        data->philos[i].right_fork = &data->forks[(i + 1) % data->nb_philo];
        i++;
    }
    return (1);
}