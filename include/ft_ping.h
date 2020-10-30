/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mixfon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/22 10:41:19 by mixfon            #+#    #+#             */
/*   Updated: 2020/10/30 21:25:03 by mixfon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include "../libft/libft.h"

# include <netinet/ip.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <sys/time.h>

/*
typedef struct		s_icmp_head
{
	unsigned char	by_type;
	unsigned char	by_code;
	unsigned short	n_checksum;
	unsigned short	n_id;
	unsigned short	n_sequence;
}					t_icmp_head;

typedef struct		s_ip_head
{
	unsigned char	by_verlen;
	unsigned char	by_tos;
	unsigned short	n_to_tal_length;
	unsigned short	n_id;
	unsigned short	n_offset;
	unsigned char	by_ttl;
	unsigned char	by_protocol;
	unsigned short	n_checksum;
	unsigned int	n_src_addr;
	unsigned int	n_dest_addr;
}					t_ip_head;
*/

typedef struct		s_ping
{
	struct addrinfo	*result;
	struct icmp		icmp_heder;
	int				fd_socket;
}					t_ping;

#endif
