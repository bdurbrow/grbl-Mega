/*
 * Copyright (c) 2019 by Tim hawkins <tim.thawkins@gmail.com>
 *
 * Tachometer support for grbl-Mega.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef tachometer_h
#define tachometer_h


extern void setup_tachometer(void);
extern unsigned int poll_tachometer(void);
extern void pin_itr_tachometer(void);

#endif

