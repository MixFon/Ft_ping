/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/17 08:19:33 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 09:15:52 by mixfon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

/*
** Обработка сигналов.
** SIGINT (ctrl+C)	- сигнал прерывания.
** SIGALRM			- сигнал испускаемый функцией alarm().
*/

void	working_signals(int sig)
{
	if (sig == SIGINT)
		print_final_rtt();
	else if (sig == SIGALRM)
	{
		alarm(g_ping.fl_i);
		sendto_icmp();
	}
}

void	set_signals(void)
{
	signal(SIGALRM, working_signals);
	signal(SIGINT, working_signals);
}

/*
** Установка стандарных значений флагов.
** fl_i = 1		количество секунд между отправками пакетов.
** fl_t = 64	время жизни пакета.
** fl_s = 20	размер пакета.
*/

void	infill_default_flags(void)
{
	g_ping.fl_i = 1;
	g_ping.fl_t = 64;
	g_ping.fl_s = 20;
}

void	flood_mode(void)
{
	while (21)
		sendto_icmp();
}

int		main(int ac, char **av)
{
	int i;

	i = 0;
	infill_default_flags();
	check_parametes(ac, av);
	set_signals();
	preparation_to_send();
	if (g_ping.fl_f)
		flood_mode();
	sendto_icmp();
	alarm(g_ping.fl_i);
	while (21)
		i++;
	return (0);
}
