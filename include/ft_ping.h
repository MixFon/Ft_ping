/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mixfon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/22 10:41:19 by mixfon            #+#    #+#             */
/*   Updated: 2020/11/17 07:40:09 by mixfon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# define ABS(x) ((x)<0?-(x):(x))

# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <math.h>
# include <arpa/inet.h>
# include "../libft/libft.h"

# include <netinet/ip.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <sys/time.h>

#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"

# define RTT_SRT "%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n"
# define RTT_HEAD_SRT "PING %s (%s): %ld data bytes\n"

# define SIZE_PACKEGE sizeof(struct icmp) + sizeof(struct timeval) + g_ping.fl_s

typedef struct		s_ping
{
	struct addrinfo	*result;
	struct ip		ip_heder_recv;
	struct icmp		icmp_heder_send;
	struct icmp		icmp_heder_recv;
	struct timeval	time_recv;
	int				fd_socket;
	int				count_recv_bits;
	int				count_send_bits;
	int				count_recv_packege;
	int				count_send_packege;
	size_t			count_timeout_packege;
	char			*destination;
	double			max_time_diff;
	double			min_time_diff;
	double			sum_time_diff;
	double			sum_sq_time_diff;
	int				fl_a : 2;
	int				fl_d : 2;
	int				fl_c;
	int				fl_f : 2;
	int				fl_i;
	int				fl_s;
	socklen_t		fl_t;
	int				fl_v : 2;
	int				fl_w;
}					t_ping;

/*
** Разрешенная глобальная переменная.
*/

t_ping g_ping;


void	print_final_rtt(void);

#endif
