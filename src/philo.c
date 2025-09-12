/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luda-cun <luda-cun@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 11:54:23 by luda-cun          #+#    #+#             */
/*   Updated: 2025/09/12 14:35:46 by luda-cun         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

/**
 * @brief Gère l'acquisition des fourchettes par un philosophe
 *
 * @param philo Pointeur vers le philosophe qui veut prendre les fourchettes
 *
 * Stratégie anti-deadlock :
 * - Philosophes PAIRS (id
	% 2 == 0) : prennent d'abord la fourchette DROITE puis GAUCHE
 * - Philosophes IMPAIRS (id
	% 2 == 1) : prennent d'abord la fourchette GAUCHE puis DROITE
 *
 * Cette différenciation d'ordre évite les interblocages (deadlocks) car :
 * - Tous les philosophes ne peuvent pas prendre la même première fourchette
 * - Au moins un philosophe peut toujours progresser dans le cycle
 *
 * Actions effectuées :
 * 1. Verrouille le premier mutex selon la stratégie
 * 2. Affiche "has taken a fork"
 * 3. Verrouille le second mutex
 * 4. Affiche "has taken a fork" à nouveau
 *
 * Note : Cette fonction ne libère PAS les fourchettes (fait dans eat())
 */
static int	take_forks(t_philo *philo)
{
	pthread_mutex_lock(philo->right_fork);
	print_status(philo, "has taken a fork");
	if (philo->data->nb_philo == 1)
	{
		ft_usleep(philo->data->time_to_die);
		pthread_mutex_unlock(philo->right_fork);
		return (0);
	}
	pthread_mutex_lock(philo->left_fork);
	print_status(philo, "has taken a fork");
	return (1);
}

static void	eat(t_philo *philo)
{
	philo->eating = 1;
	print_status(philo, "is eating");
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_mutex);
	ft_usleep(philo->data->time_to_eat);
	philo->eating = 0;
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
}

static void	dream(t_philo *philo)
{
	print_status(philo, "is sleeping");
	ft_usleep(philo->data->time_to_sleep);
}

static void	think(t_philo *philo)
{
	print_status(philo, "is thinking");
}

static int	dead_loop(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->dead_mutex);
	if (philo->data->dead == 1)
	{
		pthread_mutex_unlock(&philo->data->dead_mutex);
		return (1);
	}
	pthread_mutex_unlock(&philo->data->dead_mutex);
	return (0);
}

void	*philo_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	if (philo->id % 2 == 0)
		ft_usleep(1);
	while (!dead_loop(philo))
	{
		if (!take_forks(philo))
			break ;
		eat(philo);
		dream(philo);
		think(philo);
	}
	return (NULL);
}

static void	*monitor(void *pointer)
{
	t_data	*data;

	data = (t_data *)pointer;
	while (1)
		if (check_death(data) == 1 || check_meals(data) == 1)
			break ;
	return (NULL);
}

void	start_simulation2(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_philo)
	{
		pthread_mutex_lock(&data->philos[i].meal_mutex);
		data->philos[i].last_meal_time = data->start_time;
		pthread_mutex_unlock(&data->philos[i].meal_mutex);
		i++;
	}
}

int	start_simulation(t_data *data)
{
	pthread_t	observer;
	int			i;

	data->start_time = get_time();
	start_simulation2(data);
	if (pthread_create(&observer, NULL, &monitor, data) != 0)
		return (0);
	i = 0;
	while (i < data->nb_philo)
	{
		if (pthread_create(&data->philos[i].thread, NULL, philo_routine,
				&data->philos[i]) != 0)
			return (0);
		i++;
	}
	pthread_join(observer, NULL);
	i = 0;
	while (i < data->nb_philo)
	{
		pthread_join(data->philos[i].thread, NULL);
		i++;
	}
	return (1);
}
