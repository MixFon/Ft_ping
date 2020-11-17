/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/27 16:48:08 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 08:59:56 by widraugr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

int	ft_atoi(const char *str)
{
	unsigned int		i;
	int					n;
	long long int		res;

	i = 0;
	n = 1;
	res = 0;
	while ((str[i] <= 13 && str[i] >= 9) || str[i] == 32)
		i++;
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			n = -1;
		i++;
	}
	while (str[i] >= 48 && str[i] <= 57)
	{
		res = res * 10 + (str[i] - '0');
		if (i >= 19 && n == 1)
			return (-1);
		else if (i > 19 && n == -1)
			return (0);
		i++;
	}
	return (res * n);
}
