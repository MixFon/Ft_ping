/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_icmp_packege.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/17 08:13:46 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 08:31:15 by widraugr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

/*
** Отображает содержание icmp пакета, если поднят флаг -v
*/

void	print_icmp_packege(const unsigned char *buffer)
{
	struct icmp		icmp_heder;
	struct timeval	time_usec;

	if (!g_ping.fl_v)
		return ;
	ft_memcpy(&icmp_heder, buffer + sizeof(struct ip), sizeof(struct icmp));
	ft_memcpy(&time_usec, buffer + sizeof(struct ip) + sizeof(struct icmp),
			sizeof(struct timeval));
	printf("+---------------------------------------+\n");
	printf("|               ICMP HAEDER             |\n");
	printf("+---------+---------+-------------------+\n");
	printf("| Type %.2d | Code %.2x |   Checksum %.4x   |\n",
			icmp_heder.icmp_type,
			icmp_heder.icmp_code,
			icmp_heder.icmp_cksum);
	printf("+---------+---------+-------------------+\n");
	printf("|    PID %6d     |      SEQ %.4u     |\n",
			icmp_heder.icmp_hun.ih_idseq.icd_id,
			icmp_heder.icmp_hun.ih_idseq.icd_seq);
	printf("+-------------------+-------------------+\n");
	printf("|        Data   %20ld    |\n", time_usec.tv_sec);
	printf("+---------------------------------------+\n");
}

void	print_bits(void *src, const size_t len)
{
	unsigned char	*c;
	size_t			i;

	i = 0;
	c = src;
	while (++i <= len)
		printf("%x ", *c++);
	printf("\n");
}

void	print_packet(const unsigned char *buffer,
		const int len, const char *color)
{
	printf("%s", color);
	if (len != (sizeof(struct ip) + SIZE_PACKEGE))
		print_bits((void *)buffer, len);
	else
	{
		print_ip_packege(buffer);
		print_icmp_packege(buffer);
	}
	printf(ANSI_RESET);
}

void	infill_msg_iov(struct msghdr *msg, struct iovec *iov)
{
	ft_memset(msg, 0, sizeof(*msg));
	(*msg).msg_iov = iov;
	(*msg).msg_iovlen = 1;
}

/*
** Ожидание и получение входного сообщения.
** Если полученное сообщение содержит ошибку в checksum сообщение отбрасывается.
*/

int		recvest_message(void)
{
	unsigned char	buffer[sizeof(struct ip) + SIZE_PACKEGE];
	struct iovec	iov[1];
	struct msghdr	msg;

	ft_memset(iov, 0, sizeof(iov));
	iov[0].iov_base = &buffer;
	iov[0].iov_len = sizeof(buffer);
	infill_msg_iov(&msg, iov);
	while (21)
	{
		if ((g_ping.count_recv_bits = recvmsg(g_ping.fd_socket, &msg, 0)) < 0)
		{
			fprintf(stderr, "Request timeout for icmp_seq %d\n",
					g_ping.count_send_packege - 1);
			return (-1);
		}
		if (check_recvmsg(buffer, g_ping.count_recv_bits) == -1)
			print_packet(buffer, g_ping.count_recv_bits, ANSI_RED);
		else
			break ;
	}
	g_ping.count_recv_packege++;
	print_packet(buffer, g_ping.count_recv_bits, ANSI_YELLOW);
	g_ping.count_recv_bits = g_ping.count_recv_bits - sizeof(struct ip);
	return (1);
}
