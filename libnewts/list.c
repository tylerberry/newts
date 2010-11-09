/*
 * list.c - routines for linked list datatype
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003 Tyler Berry
 *
 * Newts is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Newts is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Newts; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if STDC_HEADERS
# include <stdlib.h>
#endif

#if STDC_HEADERS || HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif

#include "newts/list.h"

static int
list_rtrue (const void *one, const void *two)
{
  return 1;
}

/* list_init - initialize the linked list LIST.
 *
 * ALLOC_ITEM is a pointer to a function which will allocate space for one data
 * item to store in the nodes of the list.  The list code itself never calls
 * this function.
 *
 * FREE_ITEM is a pointer to a function which will free the data you're storing
 * in the nodes of the list; if you're storing static data, FREE_ITEM should be
 * NULL.
 *
 * COMPARE_ITEMS is a pointer to a function which that should tell you the relative
 * order of ONE and TWO; it should return a number greater than, equal to, or
 * less than 0 if ONE is greater than, equal to, or less than TWO,
 * respectively.
 */

void
list_init (List *list,
           void * (*alloc_item) (void),
           void (*free_item) (void *data),
           int (*compare_items) (const void *one, const void *two))
{
  list->size = 0;
  list->head = NULL;
  list->tail = NULL;
  list->alloc_item = alloc_item;
  list->free_item = free_item;
  if (compare_items != NULL)
    list->compare_items = compare_items;
  else
    list->compare_items = list_rtrue;

  return;
}

/* list_destroy - delete all nodes from and clear the linked list LIST. */

void
list_destroy (List *list)
{
  void **data = NULL;

  while (list->size > 0)
    {
      list_remove_next (list, NULL, data);
      if (list->free_item != NULL && data != NULL)
        list->free_item (*data);
    }

  memset (list, 0, sizeof (List));

  return;
}

/* list_insert_next - insert a new node after NODE. If LIST currently contains
 * no nodes, the new node becomes the first and only node in LIST. If NODE is
 * NULL, insert at the head of the list.
 *
 * Returns: 0 if successful, -1 on error.
 */

int
list_insert_next (List *list, ListNode *node, void *data)
{
  ListNode *new_node;

  if (list == NULL)
    return -1;

  new_node = (ListNode *) malloc (sizeof (ListNode));
  if (new_node == NULL)
    return -1;

  if (node == NULL)
    {
      new_node->next = list->head;
      list->head = new_node;
    }
  else
    {
      new_node->next = node->next;
      node->next = new_node;
    }

  if (new_node->next == NULL)
    list->tail = new_node;

  new_node->data = data;

  list->size++;

  return 0;
}

/* list_insert_sorted - insert a new node containing DATA into LIST in order.
 * This function assumes that LIST is already sorted.  It operates using a
 * single round of insertion sort - O(n).
 *
 * Returns: 0 if successful, -1 on error.
 */

int
list_insert_sorted (List *list, void *data)
{
  ListNode *new_node, *prev, *current;
  int i;

  if (list == NULL || data == NULL)
    return -1;

  new_node = (ListNode *) malloc (sizeof (ListNode));
  if (new_node == NULL)
    return -1;

  prev = NULL;
  current = list_head (list);

  for (i=0; i < list_size (list); i++)
    {
      if (list->compare_items (data, list_data (current)) < 0)
        break;
      prev = current;
      current = list_next (prev);
    }

  if (prev == NULL)
    {
      new_node->next = list->head;
      list->head = new_node;
    }
  else
    {
      new_node->next = prev->next;
      prev->next = new_node;
    }

  if (new_node->next == NULL)
    list->tail = new_node;

  new_node->data = data;

  list->size++;

  return 0;
}

/* list_remove_next - Remove the node following NODE from LIST, saving the data
 * stored in NODE into the memory pointed to by DATA if DATA is not NULL. If
 * NODE is NULL, remove from the head of the list.
 *
 * Returns: 0 if successful, -1 on error.
 */

int
list_remove_next (List *list, ListNode *node, void **data)
{
  ListNode *removed;

  if (list == NULL || list->size == 0)
    return -1;

  if (node == NULL)
    {
      removed = list->head;
      list->head = removed->next;
      if (list->head == NULL)
        list->tail = NULL;
    }
  else
    {
      removed = node->next;
      node->next = removed->next;
      if (node->next == NULL)
        list->tail = node;
    }

  if (data != NULL)
    *data = removed->data;

  free (removed);

  list->size--;

  return 0;
}

/* list_remove_match - Iterate through LIST; if we find an object which matches
 * DATA, delete it.
 *
 * Returns: -1 on error or if item wasn't found; 0 if successful.
 */

int
list_remove_match (List *list, void *data)
{
  ListNode *prev, *current;

  if (list == NULL || data == NULL)
    return -1;

  prev = list_head (list);
  if (list->compare_items (data, list_data (prev)) == 0)
    {
      list_remove_next (list, NULL, NULL);
      return 0;
    }
  else
    {
      current = list_next (prev);
      while (current != NULL)
        {
          if (list->compare_items (data, list_data (current)) == 0)
            {
              list_remove_next (list, prev, NULL);
              return 0;
            }
          prev = current;
          current = list_next (prev);
        }
    }

  return -1;
}

/* list_merge_sort - sort LIST using merge sort.
 *
 * list_merge_sort is adapted from a C merge sort algorithm by Simon Tatham.
 * <http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.c>
 *
 * Copyright (C) 2001 Simon Tatham.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software (the merge sort algorithm), to deal in the software without
 * restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicence, and/or sell copies of the software,
 * and to permit persons to whom the software is furnished to do so, subject to
 * the following conditions: The above copyright notice and this permission
 * notice shall be included in all copies or substantial portions of the
 * software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN
 * NO EVENT SHALL SIMON TATHAM BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

void
list_merge_sort (List *list)
{
  ListNode *p, *q, *e, *tail;
  int insize, nmerges, psize, qsize, i;

  if (list == NULL)
    return;
  insize = 1;
  while (1)
    {
      p = list->head;
      list->head = NULL;
      tail = NULL;
      nmerges = 0;
      while (p)
        {
          nmerges++;
          q = p;
          psize = 0;
          for (i = 0; i < insize; i++)
            {
              psize++;
              q = q->next;
              if (q == NULL)
                break;
            }
          qsize = insize;
          while (psize > 0 || (qsize > 0 && q != NULL))
            {
              if (psize == 0)
                {
                  e = q;
                  q = q->next;
                  qsize--;
                }
              else if (qsize == 0 || q == NULL)
                {
                  e = p;
                  p = p->next;
                  psize--;
                }
              else if (list->compare_items (list_data (p), list_data (q)) <= 0)
                {
                  e = p;
                  p = p->next;
                  psize--;
                }
              else
                {
                  e = q;
                  q = q->next;
                  qsize--;
                }
              if (tail)
                tail->next = e;
              else
                list->head = e;
              tail = e;
            }
          p = q;
        }
      tail->next = NULL;
      if (nmerges <= 1)
        return;
      insize *= 2;
    }
}

/* list_natural_merge_sort - sort LIST using natural merge sort.
 *
 * list_natural_merge_sort is adapted from a C++ natural merge sort algorithm
 * by Daher Fawares. <http://daher.openglforums.com/downloads/nmsort.h>
 *
 * As best as I can tell, this algorithm doesn't work yet.
 */

void
list_natural_merge_sort (List *list)
{
  int i, n;
  ListNode top;
  ListNode *a, *b, *c, *head, *todo, *t;

  head = &top;
  head->next = list->head;
  a = list->tail;
  for (n=1; a != head->next; n=n+n)
    {
      todo = head->next;
      list->head = head;
      while (todo != list->tail)
        {
          t = todo;
          a = t;
          for (i=1; i<n; i++)
            t = t->next;
          b = t->next;
          t->next = list->tail;
          t = b;
          for (i=1; i<n; i++)
            t = t->next;
          todo = t->next;
          t->next = list->tail;
          c = list->tail;
          do
            {
              if (list->compare_items (list_data (a), list_data (b)) >= 0)
                {
                  c->next = a;
                  c = a;
                  a = a->next;
                }
              else
                {
                  c->next = b;
                  c = b;
                  b = b->next;
                }
            }
          while (c != list->tail);
          c = list->tail->next;
          list->tail->next = list->tail;
          list->head->next = c;
          for (i=1; i<n; i++)
            list->head = list->head->next;
        }
    }
  list->head = head->next;
}
