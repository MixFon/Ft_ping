/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   infill_struct_hints.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/17 08:08:59 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 09:17:01 by mixfon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

/*
** Заполнение структуры hints для последуюшей отправки в getaddrinfo.
** Устанавливается семейство адресов AI_INER (IPv4).
** Устанавливается протокол IPPROTO_ICMP.
** Остальные значения устанавливаются в ноль.
*/

void	infill_struct_hints(struct addrinfo *hints)
{
	ft_memset(hints, 0, sizeof(struct addrinfo));
	hints->ai_family = AF_INET;
	hints->ai_protocol = IPPROTO_ICMP;
}

/*
** Формируем icmp heder.
** Тип эхо-запроса	ICMP_ECHO	- 8
** Код эхо-запроса				- 0
** Хеш сумма эхо-запроса		- 0
** Id запущенного процесса		- getpid()
** Порядковый номер запроса 	- 0
*/

void	infill_icmp_heder(struct icmp *icmp_heder)
{
	ft_memset(icmp_heder, 0, sizeof(struct icmp));
	icmp_heder->icmp_type = ICMP_ECHO;
	icmp_heder->icmp_code = 0;
	icmp_heder->icmp_cksum = 0;
	icmp_heder->icmp_id = getpid();
	icmp_heder->icmp_seq = 0;
}

/*
** Проверка checksum входного icmp сообщения.
** Сохранение ip хедера.
** Сохранение icmp хедера.
** Сохранение временной точки отправления сообщения (struct timeval).
*/

int		check_recvmsg(unsigned char *buf, size_t len)
{
	struct icmp		recv_icmp;
	unsigned short	temp;

	if (len != sizeof(struct ip) + SIZE_PACKEGE)
		return (-1);
	ft_memcpy(&recv_icmp, buf + sizeof(struct ip), sizeof(struct icmp));
	g_ping.icmp_heder_recv = recv_icmp;
	if (recv_icmp.icmp_hun.ih_idseq.icd_id != getpid())
		return (-1);
	if (recv_icmp.icmp_type != 0)
		return (-1);
	temp = recv_icmp.icmp_cksum;
	recv_icmp.icmp_cksum = 0;
	ft_memcpy(buf + sizeof(struct ip), &recv_icmp, sizeof(struct icmp));
	if (temp != checksum(buf + sizeof(struct ip), SIZE_PACKEGE))
		return (-1);
	ft_memcpy(&g_ping.ip_heder_recv, buf, sizeof(struct ip));
	ft_memcpy(&g_ping.time_recv, buf + sizeof(struct ip) + sizeof(struct icmp),
			sizeof(struct timeval));
	return (1);
}

void	print_tail(struct ip *ip_heder)
{
	char				*buf;
	struct sockaddr_in	*sinp;

	buf = ft_strnew(INET_ADDRSTRLEN);
	printf("+---------+---------+-------------------+\n");
	printf("| TTL %.3d | Prot %.2x |   Checksum %.4x   |\n",
			ip_heder->ip_ttl,
			ip_heder->ip_p,
			ip_heder->ip_sum);
	printf("+----------+---------+-------------------+\n");
	printf("|     Source            %-14s  |\n", buf);
	sinp = (struct sockaddr_in *)&ip_heder->ip_dst;
	inet_ntop(AF_INET, &sinp->sin_addr, buf, INET_ADDRSTRLEN);
	printf("+---------------------------------------+\n");
	printf("|     Dertanation       %-14s  |\n", buf);
	free(buf);
}

/*
** Строчки сверху вниз.
** Версия ip пакета, размер ip (x4), тип обслуживания, размер пакета.
** Идентификатор, флаги/смещение фрагмента.
** Время жизни, протокол, котрольная сумма заголовка
** Адрес источника, адрес назначения
*/

void	print_ip_packege(const unsigned char *buffer)
{
	struct ip			ip_heder;
	struct sockaddr_in	*sinp;
	char				*buf;

	if (!g_ping.fl_v)
		return ;
	buf = ft_strnew(INET_ADDRSTRLEN);
	ft_memcpy(&ip_heder, buffer, sizeof(struct ip));
	sinp = (struct sockaddr_in *)&ip_heder.ip_src;
	inet_ntop(AF_INET, &sinp->sin_addr, buf, INET_ADDRSTRLEN);
	printf("+---------------------------------------+\n");
	printf("|                IP HAEDER              |\n");
	printf("+----+----+---------+-------------------+\n");
	printf("| %.2d | %.2d | Type %.2x |    Length %.4d    |\n",
			ip_heder.ip_v,
			ip_heder.ip_hl,
			ip_heder.ip_tos,
			ip_heder.ip_len);
	printf("+---------+---------+-------------------+\n");
	printf("|    IP ID %.4x     |    Offset %.4x    |\n",
			ip_heder.ip_id,
			ip_heder.ip_off);
	free(buf);
	print_tail(&ip_heder);
}
