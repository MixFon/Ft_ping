/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnew.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: widraugr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/12/03 10:05:56 by widraugr          #+#    #+#             */
/*   Updated: 2020/11/17 08:58:47 by widraugr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

char	*ft_strnew(size_t size)
{
	char *str;

	if (size == (size_t)(-1))
		return (NULL);
	if (!(str = (char *)malloc(sizeof(char) * (size + 1))))
		return (NULL);
	str = (char *)ft_memset(str, '\0', size + 1);
	return (str);
}
