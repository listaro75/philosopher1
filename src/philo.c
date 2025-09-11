/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luda-cun <luda-cun@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 11:54:23 by luda-cun          #+#    #+#             */
/*   Updated: 2025/09/11 12:15:17 by luda-cun         ###   ########.fr       */
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

/**
 * @brief Gère le processus complet de manger d'un philosophe
 *
 * @param philo Pointeur vers le philosophe qui va manger
 *
 * Séquence d'actions :
 * 1. Appelle take_forks() pour acquérir les deux fourchettes
 * 2. Affiche "is eating" avec timestamp
 * 3. Met à jour last_meal_time avec l'heure actuelle (crucial pour check_death)
 * 4. Incrémente le compteur meals_eaten
 * 5. Dort pendant time_to_eat millisecondes (simulation du temps de manger)
 * 6. Libère les deux fourchettes dans l'ordre (gauche puis droite)
 *
 * Thread-safety :
 * - Les fourchettes (mutex) sont correctement acquises et libérées
 * - last_meal_time est mis à jour de façon thread-safe
 * - meals_eaten est incrémenté (pas de race condition car accès séquentiel)
 */
static void	eat(t_philo *philo)
{
	take_forks(philo);
	print_status(philo, "is eating");
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_mutex);
	philo->meals_eaten++;
	ft_usleep(philo->data->time_to_eat);
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
}

/**
 * @brief Gère les phases de sommeil et de réflexion du philosophe
 *
 * @param philo Pointeur vers le philosophe qui va dormir puis penser
 *
 * Séquence d'actions :
 * 1. Affiche "is sleeping" avec timestamp
 * 2. Dort pendant time_to_sleep millisecondes
 * 3. Affiche "is thinking" avec timestamp
 * 4. Continue immédiatement (pas de temps dédié à la réflexion)
 *
 * Note sur la réflexion :
 * - Le temps de "thinking" n'est pas simulé par un sleep
 * - Le philosophe retourne immédiatement au cycle pour essayer de manger
 * - Ceci optimise les performances et réduit les risques de famine
 */
static void	sleep_and_think(t_philo *philo)
{
	print_status(philo, "is sleeping");
	ft_usleep(philo->data->time_to_sleep);
	print_status(philo, "is thinking");
}

/**
 * @brief Routine principale exécutée par chaque thread de philosophe
 *
 * @param arg Pointeur void* vers la structure t_philo du philosophe
 * @return void* NULL à la fin de l'exécution
 *
 * Cas spécial - Philosophe seul :
 * - Avec 1 philosophe, impossible de manger (besoin de 2 fourchettes)
 * - Prend une fourchette et attend la mort (comportement attendu)
 */
void	*philo_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	if (philo->data->nb_philo == 1)
	{
		pthread_mutex_lock(philo->left_fork);
		print_status(philo, "has taken a fork");
		ft_usleep(philo->data->time_to_die + 1);
		pthread_mutex_unlock(philo->left_fork);
		return (NULL);
	}
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

/**
 * @brief Lance et gère la simulation complète des philosophes
 *

	* @param data Pointeur vers la structure contenant tous
	les paramètres et philosophes
 * @return int 1 si la simulation s'est déroulée avec succès,
  0 en cas d'erreur
 *
 * Phase de lancement :
 * 1. Crée un thread pthread pour chaque philosophe
 * 2. Passe la structure t_philo correspondante à chaque thread
 * 3. Si pthread_create échoue, retourne 0 immédiatement
 *
 * Phase de surveillance :
 * - Boucle de monitoring principale du thread principal
 * - Appelle check_death() pour vérifier les morts de faim
 * - Appelle check_meals() pour vérifier la fin des quotas
 *
	- Sleep de 1ms entre chaque vérification (évite la consommation
	CPU excessive)
 * - Sort quand check_death() OU check_meals() retourne 1
 *
 * Phase de nettoyage :
 * - Attend la terminaison de tous les threads avec pthread_join()
 * - Garantit une fin propre de tous les threads avant de continuer
 * - Retourne 1 pour indiquer le succès
 *
 * Gestion d'erreur :
 * - Si échec de création d'un thread, retourne 0
 * - Les threads déjà créés continuent (comportement indéfini)
 * - Dans un vrai projet, il faudrait nettoyer les threads créés
 */
int	start_simulation(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_philo)
	{
		if (pthread_create(&data->philos[i].thread, NULL, philo_routine,
				&data->philos[i]))
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
