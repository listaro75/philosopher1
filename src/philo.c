/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luda-cun <luda-cun@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 11:54:23 by luda-cun          #+#    #+#             */
/*   Updated: 2025/09/11 15:25:52 by luda-cun         ###   ########.fr       */
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
	// Vérifier si la simulation est arrêtée
	pthread_mutex_lock(&philo->data->dead_mutex);
	if (philo->data->dead)
	{
		pthread_mutex_unlock(&philo->data->dead_mutex);
		return (0);
	}
	pthread_mutex_unlock(&philo->data->dead_mutex);
		
	if (philo->id % 2 == 0)
	{
		pthread_mutex_lock(philo->right_fork);
		// Vérifier à nouveau si la simulation est arrêtée
		pthread_mutex_lock(&philo->data->dead_mutex);
		if (philo->data->dead)
		{
			pthread_mutex_unlock(&philo->data->dead_mutex);
			pthread_mutex_unlock(philo->right_fork);
			return (0);
		}
		pthread_mutex_unlock(&philo->data->dead_mutex);
		print_status(philo, "has taken a fork");
		pthread_mutex_lock(philo->left_fork);
		print_status(philo, "has taken a fork");
	}
	else
	{
		pthread_mutex_lock(philo->left_fork);
		// Vérifier à nouveau si la simulation est arrêtée
		pthread_mutex_lock(&philo->data->dead_mutex);
		if (philo->data->dead)
		{
			pthread_mutex_unlock(&philo->data->dead_mutex);
			pthread_mutex_unlock(philo->left_fork);
			return (0);
		}
		pthread_mutex_unlock(&philo->data->dead_mutex);
		print_status(philo, "has taken a fork");
		pthread_mutex_lock(philo->right_fork);
		print_status(philo, "has taken a fork");
	}
	return (1);
}



/**
 * @brief Calcule et exécute un temps de réflexion intelligent
 *
 * @param philo Pointeur vers le philosophe qui va penser
 * @param silent Si true, pas d'affichage du statut "thinking"
 *
 * Cette fonction calcule un temps de réflexion optimal basé sur :
 * - Le temps écoulé depuis le dernier repas
 * - Le temps de mort et le temps de manger
 * Cela aide à éviter la famine en étalant les cycles des philosophes
 */
static void	think_routine(t_philo *philo, int silent)
{
	long long	time_to_think;
	long long	current_time;
	long long	last_meal;

	current_time = get_time();
	pthread_mutex_lock(&philo->meal_mutex);
	last_meal = philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_mutex);

	time_to_think = (philo->data->time_to_die
			- (current_time - last_meal)
			- philo->data->time_to_eat) / 2;
	
	if (time_to_think < 0)
		time_to_think = 0;
	if (time_to_think == 0 && silent == 1)
		time_to_think = 1;
	if (time_to_think > 600)
		time_to_think = 200;
	
	if (silent == 0)
		print_status(philo, "is thinking");
	ft_usleep(time_to_think);
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
	
	// Initialiser last_meal_time au temps de démarrage
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = philo->data->start_time;
	pthread_mutex_unlock(&philo->meal_mutex);
	
	// Délai synchronisé pour tous les threads
	while (get_time() < philo->data->start_time)
		continue;
	
	// Les philosophes impairs commencent par penser (évite les deadlocks)
	if (philo->id % 2 == 1)
		think_routine(philo, 1);
	
	while (1)
	{
		int	current_meals;
		int	is_dead;
		
		pthread_mutex_lock(&philo->data->dead_mutex);
		is_dead = philo->data->dead;
		pthread_mutex_unlock(&philo->data->dead_mutex);
		
		if (is_dead)
			break;
		
		// Manger et dormir
		if (!take_forks(philo))
			break;
		print_status(philo, "is eating");
		pthread_mutex_lock(&philo->meal_mutex);
		philo->last_meal_time = get_time();
		philo->meals_eaten++;
		pthread_mutex_unlock(&philo->meal_mutex);
		ft_usleep(philo->data->time_to_eat);
		pthread_mutex_unlock(philo->left_fork);
		pthread_mutex_unlock(philo->right_fork);
		
		// Vérifier si assez mangé
		if (philo->data->nb_meals > 0)
		{
			pthread_mutex_lock(&philo->meal_mutex);
			current_meals = philo->meals_eaten;
			pthread_mutex_unlock(&philo->meal_mutex);
			if (current_meals >= philo->data->nb_meals)
				break;
		}
		
		// Dormir
		print_status(philo, "is sleeping");
		ft_usleep(philo->data->time_to_sleep);
		
		// Penser de manière intelligente
		think_routine(philo, 0);
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
		usleep(10);
	i = 0;
	while (i < data->nb_philo)
	{
		pthread_join(data->philos[i].thread, NULL);
		i++;
	}
	return (1);
}
