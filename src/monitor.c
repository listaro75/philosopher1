/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luda-cun <luda-cun@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 11:54:16 by luda-cun          #+#    #+#             */
/*   Updated: 2025/09/11 12:15:50 by luda-cun         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

/**
 * @brief Vérifie si un philosophe est mort de faim
 *

	* @param data Pointeur vers la structure contenant tous les 
	philosophes et paramètres
 * @return int 1 si un philosophe est mort, 0 si tous sont encore vivants
 *
 * Processus de vérification :
 * 1. Parcourt tous les philosophes séquentiellement
 * 2. Pour chaque philosophe, calcule le temps écoulé depuis son dernier repas
 * 3. Si ce temps dépasse time_to_die, le philosophe meurt :
 *    - Active le flag global 'dead' (protégé par mutex)
 *    - Affiche le message de mort avec timestamp
 *    - Retourne 1 pour arrêter la simulation
 *
 * Thread-safety :
 * - Utilise dead_mutex pour protéger l'accès au flag 'dead'
 * - Utilise print_mutex pour garantir l'intégrité du message de mort
 *
 * Note : Cette fonction est appelée en continu par le thread principal
 */
int    check_death(t_data *data)
{
    int         i;
    long long   current_time;
    long long   last_meal;

    i = 0;
    while (i < data->nb_philo)
    {
        current_time = get_time();
        
        pthread_mutex_lock(&data->philos[i].meal_mutex);
        last_meal = data->philos[i].last_meal_time;
        pthread_mutex_unlock(&data->philos[i].meal_mutex);
        
        if (current_time - last_meal > data->time_to_die)
        {
            pthread_mutex_lock(&data->dead_mutex);
            data->dead = 1;
            pthread_mutex_unlock(&data->dead_mutex);
            pthread_mutex_lock(&data->print_mutex);
            printf("%lld %d died\n", current_time - data->start_time,
                data->philos[i].id);
            pthread_mutex_unlock(&data->print_mutex);
            return (1);
        }
        i++;
    }
    return (0);
}

/**
 * @brief Vérifie si tous les philosophes ont terminé de manger
 *
 * @param data Pointeur vers la structure contenant les philosophes et nb_meals
 * @return int 1 si tous ont fini leurs repas, 0 sinon
 *
 * Logique de vérification :
 * 1. Si nb_meals == -1 (simulation infinie), retourne toujours 0
 * 2. Compte le nombre de philosophes ayant atteint leur quota de repas
 * 3. Si tous les philosophes ont mangé nb_meals fois ou plus :
 *    - Active le flag 'dead' pour arrêter la simulation
 *    - Retourne 1 pour signaler la fin de la simulation
 *
 * Cas d'usage :
 * - Utilisé quand un nombre spécifique de repas est demandé (5ème argument)
 * - Permet une fin propre de la simulation sans mort
 * - Alternative à l'arrêt par timeout (check_death)
 *
 * Thread-safety :
 * - Utilise dead_mutex pour protéger l'accès au flag 'dead'
 * - Lecture thread-safe de meals_eaten (entier atomique)
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
