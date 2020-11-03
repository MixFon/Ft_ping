#include "../include/ft_ping.h"

void	work();

void	check_parametes(int ac, char **av)
{
	// g_ping	
}

/*
** Создание checksum хедера icmp.
** Все сообщение делится на 16-ти битные слова. Все слова складываются.
** В конце все биты инвентируются (~).
*/
unsigned short checksum(void *pBuf, int nLen)
{
	unsigned int	sum;
	unsigned short	*buf;

	sum = 0;
	buf = pBuf;
	while (nLen > 1)
	{
		sum += *buf;
		buf++;
		nLen -= sizeof(unsigned short);
	}
	if (nLen == 1)
		sum += *(unsigned char *)buf;
	sum = (sum >> 16) + (sum &  0xffff);
	return (((unsigned short)~sum));
}

/*
** Открываем сокет:
** Сетейство	AF_INET			Домен Интернета IPv4
** Тип			SOCK_DGRAM		Не ориентирован на создание лог. соединения
** Протокол		IPPROTO_ICMP	Протокол управляющих сообщений Интернета
** Данный тип необходимо использовать для открытия сокета не для root.
** Устанавливается таймер на получение ответного сообщения (SO_RCVTIMEO)
*/

int	open_icmp_socket(struct addrinfo *hints)
{
	int	fd_socket;
	int temp;
	struct timeval timer = {1,0};

	temp = 1;
	if ((fd_socket = socket(
					AF_INET,
					//SOCK_RAW,
					SOCK_DGRAM,
					IPPROTO_ICMP)) == -1)
		sys_err("Error: open socket\n");
	ft_printf("Socket opened.\nfd_socket = %d\n\n", fd_socket);
	if (setsockopt(
				fd_socket,
				SOL_SOCKET,
				SO_RCVTIMEO,
				&timer,
				sizeof(timer)) == -1)
		sys_err("Error: setsockopt\n");
	return (fd_socket);
}	

/*
** Заполнение структуры hints для последуюшей отправки в getaddrinfo
*/

void	infill_struct_hints(struct addrinfo *hints)
{
	ft_memset(hints, 0, sizeof(struct addrinfo));
	hints->ai_family = AF_INET;
	//hints->ai_family = AF_UNSPEC;
	//hints->ai_flags = AI_V4MAPPED | AI_ALL;
	//hints->ai_socktype = SOCK_RAW;
	hints->ai_protocol = IPPROTO_ICMP;
}

/*
** Каноническое имя хоста google.com
*/
void	print_list_adrinfo(struct addrinfo *hints)
{
	struct				addrinfo *iter;	
	char				buf[100];
	struct sockaddr_in	*sinp;
	struct sockaddr_in6 *sinp6;
	int					i;

	iter = hints;
	i = -1;
	while (iter != NULL)
	{
		ft_printf("i = %d\n", ++i);
		ft_printf("ping->hints.ai_flags = %d\n", iter->ai_flags);
		ft_printf("ping->hints.ai_family = %d\n", iter->ai_family);
		ft_printf("ping->hints.ai_socktype = %d\n", iter->ai_socktype);
		ft_printf("ping->hints.ai_protocol = %d\n", iter->ai_protocol);
		ft_printf("ping->hints.ai_addrlen = %d\n", iter->ai_addrlen);
		ft_printf("\tping->hints.ai_addr.sa_family = %d\n",
				iter->ai_addr->sa_family);
		ft_printf("\tping->hints.ai_addr.sa_data = %#x\n",
				iter->ai_addr->sa_data);
		sinp = (struct sockaddr_in *)iter->ai_addr;
		sinp6 = (struct sockaddr_in6 *)iter->ai_addr;
		ft_printf("inet_ntop ipv4 = %s\n",
				inet_ntop(AF_INET, &sinp->sin_addr, buf, INET_ADDRSTRLEN));
		ft_printf("inet_ntop ipv6 = %s\n",
				inet_ntop(AF_INET6, &sinp6->sin6_addr, buf, INET6_ADDRSTRLEN));
		ft_printf("ping->hiets.ai_canonname = %s\n", iter->ai_canonname);
		//open_socket(iter);
		iter = iter->ai_next;
	}
}

/*
** Формируем icmp heder.
** Тип эхо-запроса	ICMP_ECHO	- 8
** Код эхо-запроса				- 0
** Хеш сумма эхо-запроса		- 0
** Id запущенного процесса		- getpid()
** Порядковый номер запроса 	- 0
*/

void	infill_icmp_heder(void)
{
	struct icmp icmp_heder;

	ft_memset(&g_ping.icmp_heder_send, 0, sizeof(struct icmp));
	g_ping.icmp_heder_send.icmp_type = ICMP_ECHO;
	g_ping.icmp_heder_send.icmp_code = 0;
	g_ping.icmp_heder_send.icmp_cksum = 0;
	g_ping.icmp_heder_send.icmp_hun.ih_idseq.icd_id = getpid();
	g_ping.icmp_heder_send.icmp_hun.ih_idseq.icd_seq = 0;
	//return (icmp_heder);
}

void	print_bits(void *src, size_t len)
{
	char *c;

	c = src;
	for (int i = 0; i < len; i++)
		ft_printf("%x ", *c++);
	ft_putendl("");
}

/*
** Проверка checksum входного icmp сообщения.
** Сохранение ip хедера.
** Созранение icmp хедера.
** Созранение временной точки отправления (struct timeval).
*/

int		check_recvmsg(unsigned char *buf, int len)
{
	struct icmp		recv_icmp;
	unsigned short	temp;
	
	if (len !=  sizeof(struct ip) + sizeof(struct icmp) + sizeof(struct timeval))	
		return (-1);
	ft_memcpy(&recv_icmp, buf + sizeof(struct ip), sizeof(struct icmp));
	g_ping.icmp_heder_recv = recv_icmp;
	if (recv_icmp.icmp_type != 0)
		return (-1);
	temp = recv_icmp.icmp_cksum;
	recv_icmp.icmp_cksum = 0;
	ft_memcpy(buf + sizeof(struct ip), &recv_icmp, sizeof(struct icmp)); 
	if (temp != checksum(buf + sizeof(struct ip),
				sizeof(struct icmp) + sizeof(struct timeval)))
		return (-1);
	ft_memcpy(&g_ping.ip_heder_recv, buf, sizeof(struct ip));
	ft_memcpy(&g_ping.time_recv, buf + sizeof(struct ip) + sizeof(struct icmp),
			sizeof(struct timeval));
	return (1);
}
/*
** Ожидание и получение входного сообщения.
*/
void	recvest_message(void)
{
	unsigned char	buffer[512];
	struct iovec	iov[1];
	struct msghdr	msg;

	ft_memset(&msg, 0, sizeof(msg));
	ft_memset(iov, 0, sizeof(iov));
	iov[0].iov_base = &buffer;
	iov[0].iov_len  = sizeof(buffer);
	msg.msg_iov     = iov;
	msg.msg_iovlen  = 1;
	//msg.msg_control    = (char *)&pass_sd;
	//msg.msg_controllen = sizeof(pass_sd);
	ft_printf("Waiting on recvmsg\n");
	g_ping.count_recv_bits = recvmsg(g_ping.fd_socket, &msg, 0);
	if (g_ping.count_recv_bits < 0)
	{
	  perror("recvmsg() failed");
	  //close(worker_sd);
	  //exit(-1);
	}
	ft_printf("recv_len = {%d}\n", g_ping.count_recv_bits);
	print_bits(buffer + 20, g_ping.count_recv_bits - 20);
	if (check_recvmsg(buffer, g_ping.count_recv_bits) == -1)
		sys_err("Error: recvest message.\n");
	print_bits(&g_ping.ip_heder_recv, sizeof(struct ip));
	print_bits(&g_ping.icmp_heder_recv, sizeof(struct icmp));
	print_bits(&g_ping.time_recv, sizeof(struct timeval));
	ft_printf("Hello!!\n\n");
}

void	sendto_icmp(void)
{
	int				sequence;
	struct timeval	time_send;
	int				count;
	unsigned char	data[sizeof(g_ping.icmp_heder_send) + sizeof(time_send)];

	sequence = 0;
	while(21)
	{
		ft_memset(&data, 0, sizeof(data));
		infill_icmp_heder();
		g_ping.icmp_heder_send.icmp_hun.ih_idseq.icd_seq = sequence++;
		ft_memcpy(data, &g_ping.icmp_heder_send, sizeof(g_ping.icmp_heder_send));
		if (gettimeofday(&time_send, NULL) == -1)
			sys_err("Error: gettimeofday.\n");
		ft_memcpy(data + sizeof(g_ping.icmp_heder_send), &time_send,
				sizeof(time_send));
		//ping->icmp_heder.icmp_cksum = 0;
		g_ping.icmp_heder_send.icmp_cksum = checksum(data,
				sizeof(g_ping.icmp_heder_send) + sizeof(time_send));
		ft_memcpy(data, &g_ping.icmp_heder_send, sizeof(g_ping.icmp_heder_send));
		if ((count = sendto(g_ping.fd_socket, data,
				sizeof(g_ping.icmp_heder_send) + sizeof(time_send),
				0, g_ping.result->ai_addr,
				sizeof(g_ping.result->ai_addr))) == -1)
			sys_err("Error: sendto.\n");
		ft_printf("Send {%d} octets.\n", count);
		print_bits(data, sizeof(g_ping.icmp_heder_send) + sizeof(time_send));
		recvest_message();
	}
}

void	work(void)
{
	int				temp;
	struct addrinfo	hints;

	infill_struct_hints(&hints);
	if ((temp = getaddrinfo("google.com", NULL, &hints, &g_ping.result)))
	{
		ft_printf("gai_strerror = %s\n", gai_strerror(temp));
		sys_err("Error. getaddrinfo\n");
	}
	g_ping.fd_socket = open_icmp_socket(g_ping.result);
	infill_icmp_heder();
	sendto_icmp();
}


int main(int ac, char **av)
{
	check_parametes(ac, av);
	work();	

	return (0);
}
