/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luda-cun <luda-cun@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 11:54:30 by luda-cun          #+#    #+#             */
/*   Updated: 2025/09/12 09:35:56 by luda-cun         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

/**
	* @brief Convertit une chaîne de caractères en entier
		(version simplifiée d'atoi)
 *
 * @param str Chaîne de caractères contenant uniquement des chiffres
 * @return int La valeur entière correspondante, 0 si la chaîne est vide
 *
 * Fonctionnement :
 * 1. Parcourt la chaîne caractère par caractère
 * 2. Pour chaque chiffre (0-9),
	multiplie le résultat par 10 et ajoute le chiffre
 * 3. S'arrête au premier caractère non-numérique ou à la fin de
  la chaîne
 *
 * Limitations :
 * - Ne gère PAS les nombres négatifs (pas de signe -)
 * - Ne gère PAS les espaces en début de chaîne
 * - Ne détecte PAS les dépassements d'entier (overflow)
 * - Version simplifiée adaptée au projet (arguments déjà validés)
 *
 * Exemple :
 * ft_atoi("123") → 123
 * ft_atoi("42abc") → 42
 * ft_atoi("") → 0
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

/**
 * @brief Obtient le temps actuel en millisecondes depuis l'époque Unix
 *
 * @return long long Timestamp en millisecondes
 *
 * Fonctionnement :
 * 1. Utilise gettimeofday() pour obtenir le temps avec précision microseconde
 * 2. Convertit tv_sec (secondes) en millisecondes (* 1000)
 * 3. Convertit tv_usec (microsecondes) en millisecondes (/ 1000)
 * 4. Additionne les deux pour obtenir le timestamp total
 *
 * Utilisation dans le projet :
 * - Mesure des temps d'attente (last_meal_time)
 * - Calcul des timestamps d'affichage relatifs au début
 * - Implémentation de ft_usleep() précis
 *
 * Précision : ±1 milliseconde
 *
 * Note : gettimeofday() peut échouer, mais on ignore l'erreur
 * car elle est très rare et non critique pour ce projet
 */
long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/**
 * @brief Sleep précis en millisecondes (alternative à usleep standard)
 *
 * @param ms Nombre de millisecondes à attendre
 *
 * Problème avec usleep() standard :
 * - Précision variable selon le système
 * - Peut dormir plus longtemps que demandé
 * - Imprécis pour des durées courtes
 *
 * Solution implémentée (busy-waiting hybride) :
 * 1. Enregistre le temps de début
 * 2. Boucle tant que le temps écoulé < temps demandé
 * 3. À chaque itération, dort 100 microsecondes (usleep(100))
 * 4. Vérifie le temps réel avec get_time()
 *
 * Avantages :
 * - Précision au milliseconde
 * - Temps d'attente respecté précisément
 * - Évite le busy-waiting pur (économie CPU)
 *
 * Inconvénients :
 * - Plus consommateur CPU qu'un sleep standard
 * - Acceptable pour ce projet (durées courtes)
 */
void	ft_usleep(int ms)
{
	long long	start;

	start = get_time();
	while (get_time() - start < ms)
		usleep(100);
}

/**
 * @brief Affiche le statut d'un philosophe de manière thread-safe
 *
 * @param philo Pointeur vers le philosophe dont on affiche le statut
 * @param status Chaîne décrivant l'action ("is eating", "is sleeping", etc.)
 *
 * Format de sortie :
 * [timestamp_relatif] [id_philosophe] [status]
 * Exemple : "150 3 is eating"
 *
 * Thread-safety :
 * 1. Verrouille print_mutex avant toute opération
 * 2. Vérifie que data->dead == 0 (pas d'affichage après la mort)
 * 3. Calcule le timestamp relatif (temps depuis le début)
 * 4. Affiche le message formaté
 * 5. Déverrouille print_mutex
 *
 * Protection contre les affichages parasites :
 * - Si un philosophe meurt, data->dead passe à 1
 * - Plus aucun message n'est affiché (évite les messages post-mortem)
 * - Le message de mort est géré spécialement dans check_death()
 *
 * Calcul du timestamp :
 * - current_time = temps_actuel - temps_début_simulation
 * - Donne un temps relatif depuis le démarrage
 * - Plus facile à interpréter que les timestamps Unix absolus
 */
void	print_status(t_philo *philo, char *status)
{
	long long	current_time;

	pthread_mutex_lock(&philo->data->print_mutex);
	pthread_mutex_lock(&philo->data->dead_mutex);
	if (!philo->data->dead)
	{
		current_time = get_time() - philo->data->start_time;
		printf("%lld %d %s\n", current_time, philo->id, status);
	}
	pthread_mutex_unlock(&philo->data->dead_mutex);
	pthread_mutex_unlock(&philo->data->print_mutex);
}
