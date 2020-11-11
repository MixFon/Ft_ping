#include "../include/ft_ping.h"

void	work();

void	print_usege(void)
{
	fprintf(stderr, "usage: ping [-ahv] [-c count] [-i wait] [-t ttl] destination\n");
	fprintf(stderr, "\t-a audible.\n");
	fprintf(stderr, "\t-d use the SOCK_DGRAM socket type.\n");
	fprintf(stderr, "\t-c the number of packets to send.\n");
	fprintf(stderr, "\t-h help.\n");
	fprintf(stderr, "\t-i wait wait seconds between sending each packet..\n");
	fprintf(stderr, "\t-v verbose mode. Display additional information about ICMP.\n");
	fprintf(stderr, "\t-t ttl. Set the IP Time To Live for outgoing packets.\n");
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
			fprintf(stderr, "ping: invalid count of packets to transmit: `%s'\n",
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
		fprintf(stderr, "ping: invalid count of packets to transmit: `%s'\n", av[*i]);
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
	else if (c == 'i')
		g_ping.fl_i = check_number_param(c, i, av);	
	else if (c == 't')
		g_ping.fl_t = check_number_param(c, i, av);	
	else if (c == 'v')
		g_ping.fl_v = 1;	
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
	//ft_printf("Flags\n");
	//exit(-1);
}

void	infill_destination(char *destination)
{
	if (g_ping.destination != NULL)
		print_usege();
	g_ping.destination = ft_strdup(destination);
}

void	print_flags(void)
{
	ft_printf("-a ={%d}\n", g_ping.fl_a);
	ft_printf("-d ={%d}\n", g_ping.fl_d);
	ft_printf("-c ={%d}\n", g_ping.fl_c);
	ft_printf("-i ={%d}\n", g_ping.fl_i);
	ft_printf("-t ={%d}\n", g_ping.fl_t);
	ft_printf("-v ={%d}\n", g_ping.fl_v);
	ft_printf("destination = {%s}\n", g_ping.destination);
}

void	check_parametes(int ac, char **av)
{
	int i;

	i = 0;
	if (ac == 1)
		print_usege();
	if (av[1][0] == '\0')
		print_usege();
	ft_printf("ac = {%d} *av {%s}\n", ac, *av);
	while (++i < ac)
	{
		ft_printf("av[%d] = {%s}\n",i ,av[i]);
		if (av[i][0] == '-')
			check_flags(&i, av);
		else
			infill_destination(av[i]);
	}
	if (g_ping.destination == NULL)
		print_usege();
	print_flags();
	//exit(-1);
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
** Тип			SOCK_RAW		Сырой сокет. Требует права администратора.
** Протокол		IPPROTO_ICMP	Протокол управляющих сообщений Интернета
** Тип SOCK_DGRAM необходимо использовать для открытия сокета не для root.
** Устанавливается таймер на получение ответного сообщения (SO_RCVTIMEO)
*/

int	open_icmp_socket(struct addrinfo *hints)
{
	int				fd_socket;
	int				socket_type;
	struct timeval	timer = {1,0};

	if (g_ping.fl_d)
		socket_type = SOCK_DGRAM;
	else
		socket_type = SOCK_RAW;
	if ((fd_socket = socket(
					AF_INET,
					socket_type,
					IPPROTO_ICMP)) == -1)
		sys_err("ping: error open socket. Use sudo or -d frag.\n");
	ft_printf("Socket opened.\nfd_socket = %d\n\n", fd_socket);
	if (setsockopt(
				fd_socket,
				SOL_SOCKET,
				SO_RCVTIMEO,
				&timer,
				sizeof(timer)) == -1)
		sys_err("Error: setsockopt\n");
	if (setsockopt(
				fd_socket,
				IPPROTO_IP,
				IP_TTL,
				&g_ping.fl_t,
				sizeof(g_ping.fl_t)) == -1)
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

void	infill_icmp_heder(struct icmp *icmp_heder)
{
	ft_memset(icmp_heder, 0, sizeof(struct icmp));
	icmp_heder->icmp_type = ICMP_ECHO;
	icmp_heder->icmp_code = 0;
	icmp_heder->icmp_cksum = 0;
	icmp_heder->icmp_hun.ih_idseq.icd_id = getpid();
	icmp_heder->icmp_hun.ih_idseq.icd_seq = 0;
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
** Сохранение icmp хедера.
** Сохранение временной точки отправления сообщения (struct timeval).
*/

int		check_recvmsg(unsigned char *buf, int len)
{
	struct icmp		recv_icmp;
	unsigned short	temp;
	
	if (len !=  sizeof(struct ip) + sizeof(struct icmp) + sizeof(struct timeval))	
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
	if (temp != checksum(buf + sizeof(struct ip),
				sizeof(struct icmp) + sizeof(struct timeval)))
		return (-1);
	ft_memcpy(&g_ping.ip_heder_recv, buf, sizeof(struct ip));
	ft_memcpy(&g_ping.time_recv, buf + sizeof(struct ip) + sizeof(struct icmp),
			sizeof(struct timeval));
	return (1);
}

/*
** Строчки сверху вниз. 
** Версия ip пакета, размер ip (x4), тип обслуживания, размер пакета.
** Идентификатор, флаги/смещение фрагмента.
** Время жизни, протокол, котрольная сумма заголовка
** Адрес источника, адрес назначения
*/

void	print_ip_packege(const unsigned char *buffer, const int len)
{
	struct ip			ip_heder;
	struct sockaddr_in	*sinp;
	char				*buf;

	if (!g_ping.fl_v)
		return;
	buf = ft_strnew(INET_ADDRSTRLEN);
	ft_memcpy(&ip_heder, buffer, sizeof(struct ip));
	sinp = (struct sockaddr_in *)&ip_heder.ip_src;
	inet_ntop(AF_INET, &sinp->sin_addr, buf, INET_ADDRSTRLEN);
	ft_printf("+---------------------------------------+\n");
	ft_printf("|                IP HAEDER              |\n");
	ft_printf("+----+----+---------+-------------------+\n");
	ft_printf("| %.2d | %.2d | Type %.2x |    Length %.4d    |\n",
			ip_heder.ip_v,
			ip_heder.ip_hl,
			ip_heder.ip_tos,
			ip_heder.ip_len);
	ft_printf("+---------+---------+-------------------+\n");
	ft_printf("|    IP ID %.4x     |    Offset %.4x    |\n",
			ip_heder.ip_id,
			ip_heder.ip_off);
	ft_printf("+---------+---------+-------------------+\n");
	ft_printf("| TTL %.3d | Prot %.2x |   Checksum %.4x   |\n",
			ip_heder.ip_ttl,
			ip_heder.ip_p,
			ip_heder.ip_sum);
	ft_printf("+---------+---------+-------------------+\n");
	ft_printf("|     Source            %-14s  |\n", buf);
	sinp = (struct sockaddr_in *)&ip_heder.ip_dst;
	inet_ntop(AF_INET, &sinp->sin_addr, buf, INET_ADDRSTRLEN);
	ft_printf("+---------------------------------------+\n");
	ft_printf("|     Dertanation       %-14s  |\n", buf);
	free(buf);
}

/*
** Отображает содержание icmp пакета, если поднят флаг -v
*/ 

void	print_icmp_packege(const unsigned char *buffer, const int len)
{
	struct icmp		icmp_heder;
	struct timeval	time_usec;

	if (!g_ping.fl_v)
		return;
	ft_memcpy(&icmp_heder, buffer + sizeof(struct ip), sizeof(struct icmp));
	ft_memcpy(&time_usec, buffer + sizeof(struct ip) + sizeof(struct icmp),
			sizeof(struct timeval));
	ft_printf("+---------------------------------------+\n");
	ft_printf("|               ICMP HAEDER             |\n");
	ft_printf("+---------+---------+-------------------+\n");
	ft_printf("| Type %.2d | Code %.2x |   Checksum %.4x   |\n",
			icmp_heder.icmp_type,
			icmp_heder.icmp_code,
			icmp_heder.icmp_cksum);
	ft_printf("+---------+---------+-------------------+\n");
	ft_printf("|      PID %.4x     |      SEQ %.4u     |\n",
			icmp_heder.icmp_hun.ih_idseq.icd_id,
			icmp_heder.icmp_hun.ih_idseq.icd_seq);
	ft_printf("+-------------------+-------------------+\n");
	ft_printf("|       Data   % .15ld         |\n",
			time_usec.tv_sec);
	ft_printf("+---------------------------------------+\n");
}

void	print_packet(const unsigned char *buffer, const int len, const char *color)
{

	ft_printf(color);
	print_ip_packege(buffer, g_ping.count_recv_bits);
	print_icmp_packege(buffer, g_ping.count_recv_bits);
	ft_printf(ANSI_RESET);
}

/*
** Ожидание и получение входного сообщения.
** Если полученное сообщение содержит ошибку в checksum сообщение отбрасывается.
*/

int	recvest_message(void)
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
	while (21)
	{
		g_ping.count_recv_bits = recvmsg(g_ping.fd_socket, &msg, 0);
		if (g_ping.count_recv_bits < 0)
		{
			fprintf(stderr, "Request timeout for icmp_seq %d\n",
					g_ping.count_send_packege - 1);
			return (-1);
		}
		ft_printf("recv_len = {%d}\n", g_ping.count_recv_bits);
		print_bits(buffer + 20, g_ping.count_recv_bits - 20);
		if (check_recvmsg(buffer, g_ping.count_recv_bits) == -1)
			print_packet(buffer, g_ping.count_recv_bits, ANSI_RED);
		else
			break;
			//sys_err("error: recvest message. will delete.\n");
	}
	g_ping.count_recv_packege++;
	print_packet(buffer, g_ping.count_recv_bits, ANSI_YELLOW);
	print_bits(&g_ping.ip_heder_recv, sizeof(struct ip));
	print_bits(&g_ping.icmp_heder_recv, sizeof(struct icmp));
	print_bits(&g_ping.time_recv, sizeof(struct timeval));
	ft_printf("End!!\n\n");
	return (1);
}

char	*get_ip_str(void)
{
	struct sockaddr_in	*sinp;
	char				*buf;

	buf = ft_strnew(INET_ADDRSTRLEN);
	sinp = (struct sockaddr_in *)g_ping.result->ai_addr;
	inet_ntop(AF_INET, &sinp->sin_addr, buf, INET_ADDRSTRLEN);
	return (buf);
}

double	get_time_diff(void)
{
	struct timeval	time_now;
	double			time_diff;

	if (gettimeofday(&time_now, NULL) == -1)
		sys_err("Error: gettimeofday.\n");
	time_diff = (time_now.tv_usec - g_ping.time_recv.tv_usec) / 1000.0;
	if (time_diff < 0)
		time_diff = g_ping.min_time_diff;
	if (g_ping.max_time_diff < time_diff || g_ping.max_time_diff == 0)
		g_ping.max_time_diff = time_diff;
	if (g_ping.min_time_diff > time_diff || g_ping.min_time_diff == 0)
		g_ping.min_time_diff = time_diff;
	g_ping.sum_time_diff += time_diff;
	g_ping.sum_sq_time_diff += (time_diff * time_diff);
	return (time_diff);
}

void	print_rtt(void)
{
	char	*ip_str;
	double	time_diff;
	
	ip_str = get_ip_str();
	time_diff = get_time_diff();	
	ft_printf("ip_str = {%s}\n", ip_str);
	ft_printf("time_diff = {%.3f}\n", time_diff);
	ft_printf("max_time_diff = {%.3f}\n", (double)g_ping.max_time_diff);
	ft_printf("min_time_diff = {%.3f}\n", (double)g_ping.min_time_diff);
	ft_printf("avr_time_diff = {%.3f}\n", ((double)g_ping.sum_time_diff /
			(double)g_ping.icmp_heder_recv.icmp_hun.ih_idseq.icd_seq));
	ft_printf(RTT_SRT, g_ping.count_recv_bits, ip_str,
			g_ping.icmp_heder_recv.icmp_hun.ih_idseq.icd_seq,
			g_ping.ip_heder_recv.ip_ttl,
			time_diff);
	free(ip_str);
	//sleep(1);
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
	if (sequence >= g_ping.fl_c && g_ping.fl_c != 0)
		print_final_rtt();
	if (g_ping.fl_a)
		ft_printf("%c", 0x07);
}

void	sendto_icmp(void)
{
	static int		sequence = 0;
	struct timeval	time_send;
	//int				count;
	unsigned char	data[sizeof(struct icmp) + sizeof(struct timeval)];
	struct icmp		icmp_heder;

	ft_memset(&data, 0, sizeof(data));
	infill_icmp_heder(&icmp_heder);
	icmp_heder.icmp_hun.ih_idseq.icd_seq = sequence++;
	ft_memcpy(data, &icmp_heder, sizeof(struct icmp));
	if (gettimeofday(&time_send, NULL) == -1)
		sys_err("Error: gettimeofday.\n");
	ft_memcpy(data + sizeof(struct icmp), &time_send,
			sizeof(struct timeval));
	//ping->icmp_heder.icmp_cksum = 0;
	icmp_heder.icmp_cksum = checksum(data,
			sizeof(struct icmp) + sizeof(struct timeval));
	ft_memcpy(data, &icmp_heder, sizeof(struct icmp));
	if ((g_ping.count_send_bits = sendto(g_ping.fd_socket, data,
			sizeof(struct icmp) + sizeof(struct timeval),
			0, g_ping.result->ai_addr,
			sizeof(g_ping.result->ai_addr))) == -1)
		fprintf(stderr, "ping: sendto: Host is down.\n");
	ft_printf("Send {%d} octets.\n", g_ping.count_send_bits);
	ft_printf("icmp + timeval {%d} octets.\n",
			sizeof(struct icmp) + sizeof(struct timeval));
	print_bits(data, sizeof(struct icmp) + sizeof(struct timeval));
	g_ping.count_send_packege++;
	if (recvest_message() != -1)
		print_rtt();
	realise_flags(sequence);
}

void	preparation_to_send(void)
{
	int				temp;
	char			*ip_str;
	struct addrinfo	hints;
	size_t			size;

	infill_struct_hints(&hints);
	size = sizeof(struct ip) + sizeof(struct icmp) + sizeof(struct timeval);
	if ((temp = getaddrinfo(g_ping.destination, NULL, &hints, &g_ping.result)))
	{
		ft_printf("gai_strerror = %s\n", gai_strerror(temp));
		sys_err("Error. getaddrinfo\n");
	}
	g_ping.fd_socket = open_icmp_socket(g_ping.result);
	ip_str = get_ip_str();
	ft_printf(RTT_HEAD_SRT, g_ping.destination, ip_str, size);
	free(ip_str);
	//sendto_icmp();
}

double	get_average(void)
{
	return(g_ping.sum_time_diff / g_ping.count_recv_packege);
}

double	get_stddev(double avr)
{
	double res;

	res = ABS(((g_ping.sum_sq_time_diff - 2 * avr * g_ping.sum_time_diff) +
		avr * avr * g_ping.count_recv_packege) / g_ping.count_recv_packege);
	ft_printf("res = {%f}\n", res);
	return (sqrt(res));
}

/*
** Определение процента потерянных пакетов.
*/ 

double	get_percont_lost(void)
{
	double lost;

	if (g_ping.count_recv_packege >= g_ping.count_send_packege)
		return (0.0);	
	lost = g_ping.count_send_packege - g_ping.count_recv_packege;
	return (lost / (double)g_ping.count_send_packege * 100.0);
}

/*
** После приема сигнала SIGINT (ctrl+C) завершаем программу и выводим статистику..
*/ 

void	print_final_rtt(void)
{
	double stddev;
	double avr;
	double percent_lost;

	avr = get_average();
	stddev = get_stddev(avr);
	percent_lost = get_percont_lost();
	ft_printf("--- %s ping statistics ---\n", g_ping.destination);
	ft_printf("%d packets transmitted, %d packets received, %.1f%% packet loss\n",
			g_ping.count_send_packege,
			g_ping.count_recv_packege,
			percent_lost);
	ft_printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			g_ping.min_time_diff,
			avr,
			g_ping.max_time_diff,
			stddev);
	exit(0);
}

/*
** Обработка сигналов.
** SIGINT (ctrl+C)	- сигнал прерывания.
** SIGALRM			- сигнал испускаемый функцией alarm().
*/ 

void	working_signals(int sig)
{
	ft_printf("sig = {%d}!!\n", sig);
	if (sig == SIGINT)
		print_final_rtt();		
	else if (sig == SIGALRM)
	{
		sendto_icmp();
		alarm(g_ping.fl_i);
	}
}

void	set_signals(void)
{
	signal(SIGALRM, working_signals);
	signal(SIGINT, working_signals);
}

/*
** Установка стандарных значений флагов.
** fl_i = 1 количество секунд между отправками пакетов.
*/

void	infill_default_flags(void)
{
	g_ping.fl_i = 1;
	g_ping.fl_t = 64;
}

int		main(int ac, char **av)
{
	int i;

	infill_default_flags();
	check_parametes(ac, av);
	set_signals();
	preparation_to_send();	
	sendto_icmp();
	//ft_printf("alarm %d\n", alarm(2));
	alarm(g_ping.fl_i);
	while(21)
		i = 21;
	return (0);
}
