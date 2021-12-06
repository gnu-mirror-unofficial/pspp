/* PSPP - a program for statistical analysis.
   Copyright (C) 1997-9, 2000, 2007, 2009, 2010, 2014 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef OUTPUT_DRIVER_H
#define OUTPUT_DRIVER_H 1

#include <stdbool.h>
#include <stddef.h>
#include "libpspp/compiler.h"

struct output_item;
struct page_setup;
struct string_set;
struct string_map;

void output_get_supported_formats (struct string_set *);

void output_engine_push (void);
void output_engine_pop (void);

void output_submit (struct output_item *);
void output_flush (void);

void output_log (const char *, ...) PRINTF_FORMAT (1, 2);
void output_log_nocopy (char *);

const char *output_get_title (void);
void output_set_title (const char *);
const char *output_get_subtitle (void);
void output_set_subtitle (const char *);
void output_set_filename (const char *);

const char *output_get_command_name (void);
char *output_get_uppercase_command_name (void);

size_t output_open_group (struct output_item *);
void output_close_groups (size_t nesting_level);

void output_driver_parse_option (const char *option,
                                 struct string_map *options);
struct output_driver *output_driver_create (struct string_map *options);
bool output_driver_is_registered (const struct output_driver *);

void output_driver_register (struct output_driver *);
void output_driver_unregister (struct output_driver *);

void output_set_page_setup (const struct page_setup *);

#endif /* output/driver.h */
