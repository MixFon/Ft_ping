/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   preparation_to_send.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/17 08:11:02 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 08:25:28 by widraugr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

void	preparation_to_send(void)
{
	int				temp;
	char			*ip_str;
	struct addrinfo	hints;

	infill_struct_hints(&hints);
	if ((temp = getaddrinfo(g_ping.destination, NULL, &hints, &g_ping.result)))
	{
		fprintf(stderr, "ping: cannot resolve %s: Unknown host\n",
				g_ping.destination);
		exit(-1);
	}
	g_ping.fd_socket = open_icmp_socket();
	ip_str = get_ip_str();
	printf(RTT_HEAD_SRT, g_ping.destination, ip_str, SIZE_PACKEGE);
	free(ip_str);
}

double	get_average(void)
{
	return (g_ping.sum_time_diff / g_ping.count_recv_packege);
}

double	get_stddev(double avr)
{
	double res;

	res = ABS(((g_ping.sum_sq_time_diff - 2 * avr * g_ping.sum_time_diff) +
		avr * avr * g_ping.count_recv_packege) / g_ping.count_recv_packege);
	return (sqrt(res));
}

/*
** Определение процента потерянных пакетов.
*/

double	get_percont_lost(void)
{
	double lost;

	if (g_ping.count_recv_packege >= g_ping.count_send_packege)
		return (0.0);
	lost = g_ping.count_send_packege - g_ping.count_recv_packege;
	return (lost / (double)g_ping.count_send_packege * 100.0);
}

/*
** После приема сигнала SIGINT (ctrl+C) завершаем программу
** и выводим статистику.
*/

void	print_final_rtt(void)
{
	double stddev;
	double avr;
	double percent_lost;

	avr = get_average();
	stddev = get_stddev(avr);
	percent_lost = get_percont_lost();
	printf("--- %s ping statistics ---\n", g_ping.destination);
	printf("%d packets transmitted, %d packets received, %.1f%% packet loss",
			g_ping.count_send_packege,
			g_ping.count_recv_packege,
			percent_lost);
	if (g_ping.count_timeout_packege != 0)
		printf(", %lu packets out of wait time\n",
				g_ping.count_timeout_packege);
	else
		printf("\n");
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			g_ping.min_time_diff,
			avr,
			g_ping.max_time_diff,
			stddev);
	freeaddrinfo(g_ping.result);
	free(g_ping.destination);
	exit(0);
}
