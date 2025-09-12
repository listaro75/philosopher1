/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luda-cun <luda-cun@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 11:53:58 by luda-cun          #+#    #+#             */
/*   Updated: 2025/09/12 09:29:38 by luda-cun         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

/**
 * @brief Initialise les mutex meal_mutex pour tous les philosophes
 *
 * @param data Pointeur vers la structure de données contenant les philosophes
 * @return int 1 si l'initialisation réussit, 0 en cas d'erreur
 *
 * Cette fonction initialise les mutex meal_mutex qui protègent
 * l'accès à last_meal_time de chaque philosophe
 */
static int	init_philos_meal_mutex(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_philo)
	{
		if (pthread_mutex_init(&data->philos[i].meal_mutex, NULL))
			return (0);
		i++;
	}
	return (1);
}

/**
 * @brief Initialise les données principales du programme à partir des arguments
 *
 * @param data Pointeur vers la structure de données principale à initialiser
 * @param argv Tableau des arguments de la ligne de commande :
 *             argv[1] = nombre de philosophes
 *             argv[2] = temps avant la mort (ms)
 *             argv[3] = temps pour manger (ms)
 *             argv[4] = temps pour dormir (ms)
 *             argv[5] = nombre de repas (optionnel)
 * @return int 1 si l'initialisation réussit, 0 en cas d'erreur
 *
 * Cette fonction :
 * - Parse et stocke tous les paramètres de simulation
 * - Initialise les flags de contrôle (dead = 0)
 * - Enregistre le timestamp de début de simulation
 * - Crée les mutex de synchronisation (print_mutex, dead_mutex)
 * - Appelle les fonctions d'initialisation des fourchettes et philosophes
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

/**
 * @brief Initialise le tableau des fourchettes (mutex)
 *

 * @param data Pointeur vers la structure de données contenant le
	nombre de philosophes
 * @return int 1 si l'initialisation réussit,
	0 en cas d'erreur d'allocation ou de mutex
 *
 * Processus d'initialisation :
 * 1. Alloue dynamiquement un tableau de nb_philo mutex
 * 2. Initialise chaque mutex représentant une fourchette
 * 3. En cas d'échec de pthread_mutex_init, retourne 0
 *
 * Chaque fourchette est partagée entre deux philosophes adjacents :
 * - Fourchette i est utilisée par philosophe i et philosophe (i+1)%nb_philo
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

/**
 * @brief Initialise le tableau des philosophes avec leurs propriétés
 *
 * @param data Pointeur vers la structure de données principale
 * @return int 1 si l'initialisation réussit, 0 en cas d'erreur d'allocation
 *
 * Pour chaque philosophe, initialise :
 * - id : numéro unique du philosophe (1 à nb_philo)
 * - meals_eaten : compteur de repas (initialisé à 0)
 * - last_meal_time : timestamp du dernier repas (début de simulation)
 * - data : référence vers les données partagées
 * - left_fork : pointeur vers la fourchette de gauche (mutex i)
 * - right_fork : pointeur vers la fourchette de droite (mutex (i+1)%nb_philo)
 *
 * Attribution des fourchettes :
 * - Philosophe 0 : fourchettes 0 et 1
 * - Philosophe 1 : fourchettes 1 et 2
 * - Philosophe n-1 : fourchettes n-1 et 0 (bouclage)
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
		data->philos[i].eating = 0;
		data->philos[i].last_meal_time = data->start_time;
		data->philos[i].data = data;
		data->philos[i].left_fork = &data->forks[i];
		data->philos[i].right_fork = &data->forks[(i + 1) % data->nb_philo];
		if (pthread_mutex_init(&data->philos[i].meal_mutex, NULL))
			return (0);
		i++;
	}
	if (!init_philos_meal_mutex(data))
		return (0);
	return (1);
}
