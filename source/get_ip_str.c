/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_ip_str.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/17 08:00:57 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 08:57:43 by widraugr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

char	*get_ip_str(void)
{
	struct sockaddr_in	*sinp;
	char				*buf;

	buf = ft_strnew(INET_ADDRSTRLEN);
	sinp = (struct sockaddr_in *)g_ping.result->ai_addr;
	inet_ntop(AF_INET, &sinp->sin_addr, buf, INET_ADDRSTRLEN);
	return (buf);
}

int		get_time_diff(double *time_diff)
{
	struct timeval	time_now;

	if (gettimeofday(&time_now, NULL) == -1)
		sys_err("Error: gettimeofday.\n");
	*time_diff = (time_now.tv_usec - g_ping.time_recv.tv_usec) / 1000.0;
	if (*time_diff < 0)
		*time_diff = g_ping.min_time_diff;
	if (g_ping.max_time_diff < *time_diff || g_ping.max_time_diff == 0)
		g_ping.max_time_diff = *time_diff;
	if (g_ping.min_time_diff > *time_diff || g_ping.min_time_diff == 0)
		g_ping.min_time_diff = *time_diff;
	g_ping.sum_time_diff += *time_diff;
	g_ping.sum_sq_time_diff += (*time_diff * *time_diff);
	if (*time_diff > g_ping.fl_w && g_ping.fl_w != 0)
	{
		g_ping.count_timeout_packege++;
		return (0);
	}
	return (1);
}

void	print_rtt(void)
{
	char	*ip_str;
	double	time_diff;

	ip_str = get_ip_str();
	if (get_time_diff(&time_diff))
		printf(RTT_SRT, g_ping.count_recv_bits, ip_str,
			g_ping.icmp_heder_recv.icmp_hun.ih_idseq.icd_seq,
			g_ping.ip_heder_recv.ip_ttl,
			time_diff);
	free(ip_str);
}

/*
** Отправка icmp пакета.
** ip header предоставляется и заполняется  системой.
** Сомостятельно заполняем icmp header:
** Инициализируем начальными значениями, в поле "данные" записываем
** время отправления пакета (struct timeval). Checksum захватывает
** весь header и все данные. Само поле checksum должно быть равно 0.
** В конце испускаем сигнал SIGALRM (alarm()) через 1 сек. для пов-
** торного вызова функции.
*/

void	realise_flags(const int sequence)
{
	if (g_ping.fl_a)
		printf("%c", 0x07);
	if (sequence >= g_ping.fl_c && g_ping.fl_c != 0)
		print_final_rtt();
}

void	sendto_icmp(void)
{
	static int		sequence = 0;
	struct timeval	time_send;
	unsigned char	data[SIZE_PACKEGE];
	struct icmp		icmp_heder;

	ft_memset(&data, 0, sizeof(data));
	infill_icmp_heder(&icmp_heder);
	icmp_heder.icmp_seq = sequence++;
	ft_memcpy(data, &icmp_heder, sizeof(struct icmp));
	if (gettimeofday(&time_send, NULL) == -1)
		sys_err("Error: gettimeofday.\n");
	ft_memcpy(data + sizeof(struct icmp), &time_send,
			sizeof(struct timeval));
	icmp_heder.icmp_cksum = checksum(data, sizeof(data));
	ft_memcpy(data, &icmp_heder, sizeof(struct icmp));
	if ((g_ping.count_send_bits = sendto(g_ping.fd_socket, data,
			sizeof(data),
			0, g_ping.result->ai_addr,
			sizeof(g_ping.result->ai_addr))) == -1)
		fprintf(stderr, "ping: sendto: Host is down.\n");
	g_ping.count_send_packege++;
	if (recvest_message() != -1)
		print_rtt();
	realise_flags(sequence);
}
