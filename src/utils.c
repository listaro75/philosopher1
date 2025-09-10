#include "../include/philo.h"
/*
** Convertit une chaîne en entier
*/
int	ft_atoi(const char *str)
{
    int	result;
    int	i;

    result = 0;
    i = 0;
    while (str[i] && str[i] >= '0' && str[i] <= '9')
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return (result);
}

/*
** Retourne le temps actuel en millisecondes
*/
long long	get_time(void)
{
    struct timeval	tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
** Sleep précis en millisecondes
*/
void	ft_usleep(int ms)
{
    long long	start;

    start = get_time();
    while (get_time() - start < ms)
        usleep(100);
}

/*
** Affiche le statut d'un philosophe de manière thread-safe
*/
void	print_status(t_philo *philo, char *status)
{
    long long	current_time;

    pthread_mutex_lock(&philo->data->print_mutex);
    if (!philo->data->dead)
    {
        current_time = get_time() - philo->data->start_time;
        printf("%lld %d %s\n", current_time, philo->id, status);
    }
    pthread_mutex_unlock(&philo->data->print_mutex);
}