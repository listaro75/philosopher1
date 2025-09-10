#include "../include/philo.h"
/*
** Fonction pour que le philosophe prenne les fourchettes
*/
static void	take_forks(t_philo *philo)
{
    if (philo->id % 2 == 0)
    {
        pthread_mutex_lock(philo->right_fork);
        print_status(philo, "has taken a fork");
        pthread_mutex_lock(philo->left_fork);
        print_status(philo, "has taken a fork");
    }
    else
    {
        pthread_mutex_lock(philo->left_fork);
        print_status(philo, "has taken a fork");
        pthread_mutex_lock(philo->right_fork);
        print_status(philo, "has taken a fork");
    }
}

/*
** Fonction pour que le philosophe mange
*/
static void	eat(t_philo *philo)
{
    take_forks(philo);
    print_status(philo, "is eating");
    philo->last_meal_time = get_time();
    philo->meals_eaten++;
    ft_usleep(philo->data->time_to_eat);
    pthread_mutex_unlock(philo->left_fork);
    pthread_mutex_unlock(philo->right_fork);
}

/*
** Fonction pour que le philosophe dorme
*/
static void	sleep_and_think(t_philo *philo)
{
    print_status(philo, "is sleeping");
    ft_usleep(philo->data->time_to_sleep);
    print_status(philo, "is thinking");
}

/*
** Routine principale de chaque philosophe
*/
void	*philo_routine(void *arg)
{
    t_philo	*philo;

    philo = (t_philo *)arg;
    if (philo->id % 2 == 0)
        usleep(1000);
    while (!philo->data->dead)
    {
        eat(philo);
        if (philo->data->nb_meals > 0 
            && philo->meals_eaten >= philo->data->nb_meals)
            break ;
        sleep_and_think(philo);
    }
    return (NULL);
}

/*
** Lance la simulation avec tous les threads
*/
int	start_simulation(t_data *data)
{
    int	i;

    i = 0;
    while (i < data->nb_philo)
    {
        if (pthread_create(&data->philos[i].thread, NULL, 
            philo_routine, &data->philos[i]))
            return (0);
        i++;
    }
    while (!check_death(data) && !check_meals(data))
        usleep(1000);
    i = 0;
    while (i < data->nb_philo)
    {
        pthread_join(data->philos[i].thread, NULL);
        i++;
    }
    return (1);
}