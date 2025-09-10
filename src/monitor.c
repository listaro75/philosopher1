#include "../include/philo.h"

/*
** Vérifie si un philosophe est mort
*/
int	check_death(t_data *data)
{
    int			i;
    long long	current_time;

    i = 0;
    while (i < data->nb_philo)
    {
        current_time = get_time();
        if (current_time - data->philos[i].last_meal_time > data->time_to_die)
        {
            pthread_mutex_lock(&data->dead_mutex);
            data->dead = 1;
            pthread_mutex_unlock(&data->dead_mutex);
            pthread_mutex_lock(&data->print_mutex);
            printf("%lld %d died\n", 
                current_time - data->start_time, data->philos[i].id);
            pthread_mutex_unlock(&data->print_mutex);
            return (1);
        }
        i++;
    }
    return (0);
}

/*
** Vérifie si tous les philosophes ont mangé le nombre requis de repas
*/
int	check_meals(t_data *data)
{
    int	i;
    int	finished;

    if (data->nb_meals == -1)
        return (0);
    finished = 0;
    i = 0;
    while (i < data->nb_philo)
    {
        if (data->philos[i].meals_eaten >= data->nb_meals)
            finished++;
        i++;
    }
    if (finished == data->nb_philo)
    {
        pthread_mutex_lock(&data->dead_mutex);
        data->dead = 1;
        pthread_mutex_unlock(&data->dead_mutex);
        return (1);
    }
    return (0);
}