/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_usege.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/17 08:15:33 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 08:16:38 by widraugr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

void	print_usege(void)
{
	fprintf(stderr,
			"usage: ping [-adfhv] [-c count] [-i wait] [-s packetsize]\n");
	fprintf(stderr, "            [-t ttl] [-w waittime]  destination\n");
	fprintf(stderr, "\t-a audible.\n");
	fprintf(stderr, "\t-c the number of packets to send.\n");
	fprintf(stderr, "\t-d use the SOCK_DGRAM socket type.\n");
	fprintf(stderr,
			"\t-f flood mode. Administration rights are required (sodo).\n");
	fprintf(stderr, "\t-h help.\n");
	fprintf(stderr, "\t-i wait wait seconds between sending each packet.\n");
	fprintf(stderr,
			"\t-s specify the number of data bytes to be sent. 0 - 32.\n");
	fprintf(stderr,
			"\t-t ttl. Set the IP Time To Live for outgoing packets.\n");
	fprintf(stderr,
			"\t-v verbose mode. Display additional information about ICMP.\n");
	fprintf(stderr,
			"\t-w Time in ms to wait for a reply for each packet sent.\n");
	exit(-1);
}

void	is_all_number(char *number)
{
	char *iter;

	iter = number;
	while (*iter != '\0')
	{
		if (!ft_isdigit(*iter))
		{
			fprintf(stderr,
					"ping: invalid count of packets to transmit: `%s'\n",
					number);
			exit(-1);
		}
		iter++;
	}
}

/*
** Проверяет, что следующий аргумент является числом.
** Если аргумент число - возвращает это число.
** Иначе выводит ошибку.
*/

int		check_number_param(char c, int *i, char **av)
{
	int temp;

	if (av[*i + 1] == NULL)
	{
		fprintf(stderr, "ping: option requires an argument -- %c\n", c);
		print_usege();
	}
	is_all_number(av[*i + 1]);
	(*i)++;
	temp = ft_atoi(av[*i]);
	if (temp <= 0)
	{
		fprintf(stderr,
				"ping: invalid count of packets to transmit: `%s'\n", av[*i]);
		exit(-1);
	}
	return (ft_atoi(av[*i]));
}

void	check_char_flags(char c, int *i, char **av)
{
	if (c == 'h')
		print_usege();
	if (c == 'a')
		g_ping.fl_a = 1;
	else if (c == 'c')
		g_ping.fl_c = check_number_param(c, i, av);
	else if (c == 'd')
		g_ping.fl_d = 1;
	else if (c == 'f')
		g_ping.fl_f = 1;
	else if (c == 'i')
		g_ping.fl_i = check_number_param(c, i, av);
	else if (c == 's')
		g_ping.fl_s = check_number_param(c, i, av);
	else if (c == 't')
		g_ping.fl_t = check_number_param(c, i, av);
	else if (c == 'v')
		g_ping.fl_v = 1;
	else if (c == 'w')
		g_ping.fl_w = check_number_param(c, i, av);
	else
	{
		fprintf(stderr, "ping: -%c flag: Operation not permitted.\n", c);
		exit(-1);
	}
}

void	check_flags(int *i, char **av)
{
	char *flags;

	flags = av[*i];
	if (ft_strlen(flags) <= 1)
		sys_err("ping: unrecognized option \'-\'\n");
	while (*(++flags) != '\0')
		check_char_flags(*flags, i, av);
}
