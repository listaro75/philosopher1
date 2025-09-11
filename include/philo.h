/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luda-cun <luda-cun@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 11:59:52 by luda-cun          #+#    #+#             */
/*   Updated: 2025/09/11 12:14:00 by luda-cun         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long long		last_meal_time;
	pthread_mutex_t	meal_mutex;
	pthread_t		thread;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	struct s_data	*data;
}					t_philo;

typedef struct s_data
{
	int				nb_philo;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				nb_meals;
	int				dead;
	long long		start_time;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	dead_mutex;
	t_philo			*philos;
}					t_data;

/* utils.c */
int					ft_atoi(const char *str);
long long			get_time(void);
void				ft_usleep(int ms);
void				print_status(t_philo *philo, char *status);

/* init.c */
int					init_data(t_data *data, char **argv);
int					init_philos(t_data *data);
int					init_forks(t_data *data);

/* philo.c */
void				*philo_routine(void *arg);
int					start_simulation(t_data *data);

/* monitor.c */
int					check_death(t_data *data);
int					check_meals(t_data *data);

#endif
