/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   infill_destination.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/17 08:05:28 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 08:23:23 by widraugr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

void			infill_destination(char *destination)
{
	if (g_ping.destination != NULL)
		print_usege();
	g_ping.destination = ft_strdup(destination);
}

void			check_parametes(int ac, char **av)
{
	int i;

	i = 0;
	if (ac == 1)
		print_usege();
	if (av[1][0] == '\0')
		print_usege();
	while (++i < ac)
	{
		if (av[i][0] == '-')
			check_flags(&i, av);
		else
			infill_destination(av[i]);
	}
	if (g_ping.destination == NULL)
		print_usege();
	if (g_ping.fl_s > 32)
	{
		fprintf(stderr, "ping: packet size too large: %d > 32\n", g_ping.fl_s);
		exit(-1);
	}
}

/*
** Создание checksum хедера icmp.
** Все сообщение делится на 16-ти битные слова. Все слова складываются.
** В конце все биты инвентируются (~).
*/

unsigned short	checksum(void *p_buf, int len)
{
	unsigned int	sum;
	unsigned short	*buf;

	sum = 0;
	buf = p_buf;
	while (len > 1)
	{
		sum += *buf;
		buf++;
		len -= sizeof(unsigned short);
	}
	if (len == 1)
		sum += *(unsigned char *)buf;
	sum = (sum >> 16) + (sum & 0xffff);
	return (((unsigned short)~sum));
}

/*
** Устанавливается таймер на получение ответного сообщения (SO_RCVTIMEO)
** После истечения таймера recvmsg возвращает ошибку.
** По умолчанию время ожидания 1 сек. В режиме flood mode 1 милисек.
** Утановка TTL. По умолчанию 64.
*/

void			set_flag_to_socket(const int fd_socket)
{
	struct timeval	timer;

	ft_memset(&timer, 0, sizeof(struct timeval));
	if (g_ping.fl_f)
		timer.tv_usec = 1;
	else
		timer.tv_sec = 1;
	if (setsockopt(
				fd_socket,
				SOL_SOCKET,
				SO_RCVTIMEO,
				&timer,
				sizeof(timer)) == -1)
		sys_err("ping: error  setsockopt\n");
	if (setsockopt(
				fd_socket,
				IPPROTO_IP,
				IP_TTL,
				&g_ping.fl_t,
				sizeof(g_ping.fl_t)) == -1)
		sys_err("ping: error  setsockopt\n");
}

/*
** Открываем сокет:
** Сетейство	AF_INET			Домен Интернета IPv4
** Тип			SOCK_DGRAM		Не ориентирован на создание лог. соединения
** Тип			SOCK_RAW		Сырой сокет. Требует права администратора.
** Протокол		IPPROTO_ICMP	Протокол управляющих сообщений Интернета
** Тип SOCK_DGRAM необходимо использовать для открытия сокета не для root.
*/

int				open_icmp_socket(void)
{
	int	fd_socket;
	int	socket_type;

	if (g_ping.fl_d)
		socket_type = SOCK_DGRAM;
	else
		socket_type = SOCK_RAW;
	if ((fd_socket = socket(
					AF_INET,
					socket_type,
					IPPROTO_ICMP)) == -1)
		sys_err("ping: error open socket. Use sudo or -d frag.\n");
	set_flag_to_socket(fd_socket);
	return (fd_socket);
}
