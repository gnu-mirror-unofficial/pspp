/* PSPP - a program for statistical analysis.
   Copyright (C) 2020 Free Software Foundation, Inc.

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

#ifndef OUTPUT_PAGE_EJECT_ITEM_H
#define OUTPUT_PAGE_EJECT_ITEM_H 1

/* Page eject items.

   This ejects the page (for output devices that have pages). */

#include <stdbool.h>
#include "output/output-item.h"

/* A page eject item. */
struct page_eject_item
  {
    struct output_item output_item;
  };

struct page_eject_item *page_eject_item_create (void);

/* This boilerplate for page_eject_item, a subclass of output_item, was
   autogenerated by mk-class-boilerplate. */

#include <assert.h>
#include "libpspp/cast.h"

extern const struct output_item_class page_eject_item_class;

/* Returns true if SUPER is a page_eject_item, otherwise false. */
static inline bool
is_page_eject_item (const struct output_item *super)
{
  return super->class == &page_eject_item_class;
}

/* Returns SUPER converted to page_eject_item.  SUPER must be a page_eject_item, as
   reported by is_page_eject_item. */
static inline struct page_eject_item *
to_page_eject_item (const struct output_item *super)
{
  assert (is_page_eject_item (super));
  return UP_CAST (super, struct page_eject_item, output_item);
}

/* Returns INSTANCE converted to output_item. */
static inline struct output_item *
page_eject_item_super (const struct page_eject_item *instance)
{
  return CONST_CAST (struct output_item *, &instance->output_item);
}

/* Increments INSTANCE's reference count and returns INSTANCE. */
static inline struct page_eject_item *
page_eject_item_ref (const struct page_eject_item *instance)
{
  return to_page_eject_item (output_item_ref (&instance->output_item));
}

/* Decrements INSTANCE's reference count, then destroys INSTANCE if
   the reference count is now zero. */
static inline void
page_eject_item_unref (struct page_eject_item *instance)
{
  output_item_unref (&instance->output_item);
}

/* Returns true if INSTANCE's reference count is greater than 1,
   false otherwise. */
static inline bool
page_eject_item_is_shared (const struct page_eject_item *instance)
{
  return output_item_is_shared (&instance->output_item);
}

void page_eject_item_submit (struct page_eject_item *);

#endif /* output/page-eject-item.h */