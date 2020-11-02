#include "../include/ft_ping.h"

void	work(t_ping *ping);

void	check_parametes(t_ping *ping, int ac, char **av)
{
	
}

unsigned short CalcChecksum (unsigned char *pBuffer, int nLen)
{
	//Checksum for ICMP is calculated in the same way as for
	//IP header

	//This code was taken from: http://www.netfor2.com/ipsum.htm

	unsigned short nWord;
	unsigned int nSum = 0;
	int i;

	//Make 16 bit words out of every two adjacent 8 bit words in the packet
	//and add them up
	for (i = 0; i < nLen; i = i + 2)
	{
		nWord =((pBuffer [i] << 8)& 0xFF00) + (pBuffer [i + 1] & 0xFF);
		nSum = nSum + (unsigned int)nWord;
	}

	//Take only 16 bits out of the 32 bit sum and add up the carries
	while (nSum >> 16)
	{
		nSum = (nSum & 0xFFFF) + (nSum >> 16);
	}

	//One's complement the result
	nSum = ~nSum;

	return ((unsigned short) nSum);
}

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
*/

int	open_icmp_socket(struct addrinfo *hints)
{
	int	fd_socket;
	int temp;
	struct timeval timer = {1,0};

	temp = 1;
	if ((fd_socket = socket(
					AF_INET,
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
** Id процесса					- getpid()
** Порядковый номер запроса 	- 0
*/

struct icmp	infill_icmp_heder(void)
{
	struct icmp icmp_heder;

	ft_memset(&icmp_heder, 0, sizeof(struct icmp));
	icmp_heder.icmp_type = ICMP_ECHO;
	icmp_heder.icmp_code = 0;
	icmp_heder.icmp_cksum = 0;
	icmp_heder.icmp_hun.ih_idseq.icd_id = getpid();
	icmp_heder.icmp_hun.ih_idseq.icd_seq = 0;
	return (icmp_heder);
}

void	print_bits(void *src, size_t len)
{
	char *c;

	c = src;
	for (int i = 0; i < len; i++)
		ft_printf("%x ", *c++);
	ft_putendl("");
}

void	recvest_message(t_ping *ping)
{
	int    rc, len;
	int    worker_sd, pass_sd;
	unsigned char   buffer[512];
	struct iovec   iov[1];
	struct msghdr  msg;

	worker_sd = ping->fd_socket;
	//worker_sd = 0;
	memset(&msg,   0, sizeof(msg));
	memset(iov,    0, sizeof(iov));
	iov[0].iov_base = &buffer;
	iov[0].iov_len  = sizeof(buffer);
	msg.msg_iov     = iov;
	msg.msg_iovlen  = 1;
	msg.msg_control    = (char *)&pass_sd;
	msg.msg_controllen = sizeof(pass_sd);
	ft_printf("Waiting on recvmsg\n");
	rc = recvmsg(worker_sd, &msg, 0);
	if (rc < 0)
	{
	  perror("recvmsg() failed");
	  //close(worker_sd);
	  //exit(-1);
	}
	ft_printf("rc = {%d}\n", rc);
	//ft_printf("msg_data = {%s}\n", NLMSG_DATA(buffer));
	//ft_printf("buffer = {%s}\n", buffer);
	//print_bits(buffer, rc);
	//close(ping->fd_socket);
	//work(ping);
	//sleep(5);
}

void	sendto_icmp(t_ping *ping)
{
	int				sequence;
	struct timeval	time_send;
	int				count;
	unsigned char	data[sizeof(ping->icmp_heder) + sizeof(time_send)];

	sequence = 0;
	while(21)
	{
		ft_memset(&data, 0, sizeof(data));
		ping->icmp_heder = infill_icmp_heder();
		ping->icmp_heder.icmp_hun.ih_idseq.icd_seq = sequence++;
		ft_memcpy(data, &ping->icmp_heder, sizeof(ping->icmp_heder));
		if (gettimeofday(&time_send, NULL) == -1)
			sys_err("Error: gettimeofday.\n");
		ft_memcpy(data + sizeof(ping->icmp_heder), &time_send,
				sizeof(time_send));
		//ping->icmp_heder.icmp_cksum = 0;
		ping->icmp_heder.icmp_cksum = checksum(data,
				sizeof(ping->icmp_heder) + sizeof(time_send));
		ft_memcpy(data, &ping->icmp_heder, sizeof(ping->icmp_heder));
		if ((count = sendto(ping->fd_socket, data,
					sizeof(ping->icmp_heder) + sizeof(time_send), 0,
					ping->result->ai_addr, sizeof(ping->result->ai_addr))) == -1)
			sys_err("Error: sendto.\n");
		ft_printf("Send {%d} octets.\n", count);
		print_bits(data, sizeof(ping->icmp_heder) + sizeof(time_send));
		recvest_message(ping);
	}
}

void	work(t_ping *ping)
{
	int				temp;
	struct addrinfo	hints;

	infill_struct_hints(&hints);
	if ((temp = getaddrinfo("google.com", NULL, &hints, &ping->result)))
	{
		ft_printf("gai_strerror = %s\n", gai_strerror(temp));
		sys_err("Error. getaddrinfo\n");
	}
	//print_list_adrinfo(ping->result);
	ping->fd_socket = open_icmp_socket(ping->result);
	ping->icmp_heder = infill_icmp_heder();
	sendto_icmp(ping);
}


int main(int ac, char **av)
{
	t_ping ping;	

	check_parametes(&ping, ac, av);
	work(&ping);	

	return (0);
}
/*
void	ft_print_adr(char **argv)
{
	struct addrinfo *ailist, *aip;
	struct addrinfo hint;
	struct sockaddr_in *sinp;
	const char *addr = NULL;
	int err;
	//char abuf[INET_ADDRSTRLEN];
	hint.ai_flags = AI_CANONNAME;
	hint.ai_family = 0;
	hint.ai_socktype = 0;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	if ((err = getaddrinfo(argv[1], argv[2], &hint, &ailist)) != 0)
		sys_err("ошибка вызова функции getaddrinfo:");
	ft_printf("one %s, %s\n", argv[1], argv[2]);
	for (aip = ailist; aip != NULL; aip = aip->ai_next)
	{
		printf("\n\tхост %s", aip->ai_canonname ? aip->ai_canonname: "-");
		if (aip->ai_family == AF_INET)
		{
			sinp = (struct sockaddr_in *)aip->ai_addr;
			printf(" адрес %s", addr?addr:"не известен");
			printf(" порт %d", ntohs(sinp->sin_port));
		}
		printf("\n");
	}
	exit(0);

}
*/

