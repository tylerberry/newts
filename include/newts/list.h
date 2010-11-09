/*
 * list.h - interface for linked list datatype
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Newts is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Newts is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Newts; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef NEWTS_LIST_H
#define NEWTS_LIST_H

/* This file defines the public interface to a linked list datatype. */

typedef struct list
{
  struct listnode *head;
  struct listnode *tail;
  int size;
  void * (*alloc_item) (void);
  void (*free_item) (void *data);
  int (*compare_items) (const void *one, const void *two);
} List;

typedef struct listnode
{
  struct listnode *next;
  void *data;
} ListNode;

#ifdef __cplusplus
extern "C" {
#endif

extern void list_init (List *list,
                       void * (*alloc_item) (void),
                       void (*free_item) (void *data),
                       int (*compare_items) (const void *one,
                                             const void *two));
extern void list_destroy (List *list);
extern int list_insert_next (List *list, ListNode *node, void *data);
extern int list_insert_sorted (List *list, void *data);
extern int list_remove_next (List *list, ListNode *node, void **data);
extern int list_remove_match (List *list, void *data);
extern void list_merge_sort (List *list);
/* extern void list_natural_merge_sort (List *list); */

#ifdef __cplusplus
}
#endif

#define list_alloc_item(list)            ((list)->alloc_item ())
#define list_compare_items(list,one,two) ((list)->compare_items (one, two))
#define list_data(node)                  ((node)->data)
#define list_free_item(list,data)        ((list)->free_item (data))
#define list_head(list)                  ((list)->head)
#define list_is_head(list, node)         ((node) == (list)->head ? 1 : 0)
#define list_is_tail(list, node)         ((node) == (list)->tail ? 1 : 0)
#define list_next(node)                  ((node)->next)
#define list_size(list)                  ((list)->size)
#define list_tail(list)                  ((list)->tail)

#endif /* not NEWTS_LIST_H */
