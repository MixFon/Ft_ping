/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mixfon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/22 10:41:19 by mixfon            #+#    #+#             */
/*   Updated: 2020/11/17 09:06:01 by widraugr         ###   ########.fr       */
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
# include <unistd.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>

# include <netinet/ip.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <sys/time.h>

# define ANSI_RED     "\x1b[31m"
# define ANSI_GREEN   "\x1b[32m"
# define ANSI_YELLOW  "\x1b[33m"
# define ANSI_BLUE    "\x1b[34m"
# define ANSI_MAGENTA "\x1b[35m"
# define ANSI_CYAN    "\x1b[36m"
# define ANSI_RESET   "\x1b[0m"

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

/*
** File print_usege.c
*/
void				print_usege(void);
void				is_all_number(char *number);
int					check_number_param(char c, int *i, char **av);
void				check_char_flags(char c, int *i, char **av);
void				check_flags(int *i, char **av);
/*
** File infill_destination.c
*/
void				infill_destination(char *destination);
void				check_parametes(int ac, char **av);
unsigned short		checksum(void *p_buf, int len);
void				set_flag_to_socket(const int fd_socket);
int					open_icmp_socket(void);
/*
** File infill_struct_hints.c
*/
void				infill_struct_hints(struct addrinfo *hints);
void				infill_icmp_heder(struct icmp *icmp_heder);
int					check_recvmsg(unsigned char *buf, int len);
void				print_tail(struct ip *ip_heder);
void				print_ip_packege(const unsigned char *buffer);
/*
** File print_icmp_packege.c
*/
void				print_icmp_packege(const unsigned char *buffer);
void				print_bits(void *src, const size_t len);
void				print_packet(const unsigned char *buffer,
		const int len, const char *color);
void				infill_msg_iov(struct msghdr *msg, struct iovec *iov);
int					recvest_message(void);
/*
** File get_ip_str.c
*/
char				*get_ip_str(void);
int					get_time_diff(double *time_diff);
void				print_rtt(void);
void				realise_flags(const int sequence);
void				sendto_icmp(void);
/*
** File preparation_to_send.c
*/
void				preparation_to_send(void);
double				get_average(void);
double				get_stddev(double avr);
double				get_percont_lost(void);
void				print_final_rtt(void);
/*
** File working_signals.c
*/
void				working_signals(int sig);
void				set_signals(void);
void				infill_default_flags(void);
void				flood_mode(void);
int					main(int ac, char **av);
/*
** File library function.
*/
int					ft_atoi(const char *str);
int					ft_isdigit(int c);
size_t				ft_strlen(const char *s);
void				*ft_memset(void *b, int c, size_t len);
char				*ft_strnew(size_t size);
void				*ft_memcpy(void *dst, const void *src, size_t n);
char				*ft_strdup(const char *s1);
void				sys_err(char *err);
#endif
