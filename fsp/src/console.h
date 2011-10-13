/*
 * console.h
 *
 *  Created on: Oct 13, 2011
 *      Author: nico
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_
#include "common/utils/commands.h"
void * console(void * t);
void fsinfo(void *context, t_array *args);
void finfo(void *context, t_array *args);

#endif /* CONSOLE_H_ */
