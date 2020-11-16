#include "../include/ft_ping.h"

void	print_usege(void)
{
	fprintf(stderr,
			"usage: ping [-adfhv] [-c count] [-i wait] [-s packetsize]\n");
	fprintf(stderr, "            [-t ttl] [-w waittime]  destination\n");
	fprintf(stderr, "\t-a audible.\n");
	fprintf(stderr, "\t-c the number of packets to send.\n");
	fprintf(stderr, "\t-d use the SOCK_DGRAM socket type.\n");
	fprintf(stderr,
			"\t-f flood mode. Administration rights are required (sodo).\n");
	fprintf(stderr, "\t-h help.\n");
	fprintf(stderr, "\t-i wait wait seconds between sending each packet.\n");
	fprintf(stderr,
			"\t-s specify the number of data bytes to be sent. 0 - 32.\n");
	fprintf(stderr,
			"\t-t ttl. Set the IP Time To Live for outgoing packets.\n");
	fprintf(stderr,
			"\t-v verbose mode. Display additional information about ICMP.\n");
	fprintf(stderr,
			"\t-w Time in ms to wait for a reply for each packet sent.\n");
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
			fprintf(stderr,
					"ping: invalid count of packets to transmit: `%s'\n",
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
		fprintf(stderr,
				"ping: invalid count of packets to transmit: `%s'\n", av[*i]);
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
	else if (c == 'f')
		g_ping.fl_f = 1;	
	else if (c == 'i')
		g_ping.fl_i = check_number_param(c, i, av);	
	else if (c == 's')
		g_ping.fl_s = check_number_param(c, i, av);	
	else if (c == 't')
		g_ping.fl_t = check_number_param(c, i, av);	
	else if (c == 'v')
		g_ping.fl_v = 1;	
	else if (c == 'w')
		g_ping.fl_w = check_number_param(c, i, av);	
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
}

void	infill_destination(char *destination)
{
	if (g_ping.destination != NULL)
		print_usege();
	g_ping.destination = ft_strdup(destination);
}

void	check_parametes(int ac, char **av)
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
** Устанавливается таймер на получение ответного сообщения (SO_RCVTIMEO)
** После истечения таймера recvmsg возвращает ошибку.
** По умолчанию время ожидания 1 сек. В режиме flood mode 1 милисек.
** Утановка TTL. По умолчанию 64.
*/

void	set_flag_to_socket(const int fd_socket)
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

int		open_icmp_socket(struct addrinfo *hints)
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

int		check_recvmsg(unsigned char *buf, int len)
{
	struct icmp		recv_icmp;
	unsigned short	temp;
	
	if (len !=  sizeof(struct ip) + SIZE_PACKEGE)
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
	int				i;

	i = -1;
	c = src;
	while(++i < len)
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
		print_ip_packege(buffer, len);
		print_icmp_packege(buffer, len);
	}
	printf(ANSI_RESET);
}

void	infill_msg_iov(struct msghdr *msg, struct iovec *iov)
{
	ft_memset(msg, 0, sizeof(*msg));
	(*msg).msg_iov     = iov;
	(*msg).msg_iovlen  = 1;
}

/*
** Ожидание и получение входного сообщения.
** Если полученное сообщение содержит ошибку в checksum сообщение отбрасывается.
*/

int	recvest_message(void)
{
	unsigned char	buffer[sizeof(struct ip) + SIZE_PACKEGE];
	struct iovec	iov[1];
	struct msghdr	msg;

	ft_memset(iov, 0, sizeof(iov));
	iov[0].iov_base = &buffer;
	iov[0].iov_len  = sizeof(buffer);
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
			break;
	}
	g_ping.count_recv_packege++;
	print_packet(buffer, g_ping.count_recv_bits, ANSI_YELLOW);
	g_ping.count_recv_bits = g_ping.count_recv_bits - sizeof(struct ip);
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

void	preparation_to_send(void)
{
	int				temp;
	char			*ip_str;
	struct addrinfo	hints;

	infill_struct_hints(&hints);
	if ((temp = getaddrinfo(g_ping.destination, NULL, &hints, &g_ping.result)))
	{
		fprintf(stderr, "ping: cannot resolve %s: Unknown host\n",
				g_ping.destination);
		exit(-1);
	}
	g_ping.fd_socket = open_icmp_socket(g_ping.result);
	ip_str = get_ip_str();
	printf(RTT_HEAD_SRT, g_ping.destination, ip_str, SIZE_PACKEGE);
	free(ip_str);
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
** После приема сигнала SIGINT (ctrl+C) завершаем программу
** и выводим статистику.
*/ 

void	print_final_rtt(void)
{
	double stddev;
	double avr;
	double percent_lost;

	avr = get_average();
	stddev = get_stddev(avr);
	percent_lost = get_percont_lost();
	printf("--- %s ping statistics ---\n", g_ping.destination);
	printf("%d packets transmitted, %d packets received, %.1f%% packet loss",
			g_ping.count_send_packege,
			g_ping.count_recv_packege,
			percent_lost);
	if (g_ping.count_timeout_packege != 0)
		printf(", %lu packets out of wait time\n", g_ping.count_timeout_packege);
	else
		printf("\n");
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			g_ping.min_time_diff,
			avr,
			g_ping.max_time_diff,
			stddev);
	freeaddrinfo(g_ping.result);
	free(g_ping.destination);
	exit(0);
}

/*
** Обработка сигналов.
** SIGINT (ctrl+C)	- сигнал прерывания.
** SIGALRM			- сигнал испускаемый функцией alarm().
*/ 

void	working_signals(int sig)
{
	if (sig == SIGINT)
		print_final_rtt();		
	else if (sig == SIGALRM)
	{
		alarm(g_ping.fl_i);
		sendto_icmp();
	}
}

void	set_signals(void)
{
	signal(SIGALRM, working_signals);
	signal(SIGINT, working_signals);
}

/*
** Установка стандарных значений флагов.
** fl_i = 1		количество секунд между отправками пакетов.
** fl_t = 64	время жизни пакета.
** fl_s = 20	размер пакета.
*/

void	infill_default_flags(void)
{
	g_ping.fl_i = 1;
	g_ping.fl_t = 64;
	g_ping.fl_s = 20;
}

void	flood_mode(void)
{
	while (21)
		sendto_icmp();
}

int		main(int ac, char **av)
{
	int i;

	infill_default_flags();
	check_parametes(ac, av);
	set_signals();
	preparation_to_send();	
	if (g_ping.fl_f)
		flood_mode();
	sendto_icmp();
	alarm(g_ping.fl_i);
	while(21)
		i = 21;
	return (0);
}
