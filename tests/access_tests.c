/*
 * access_tests.c - tests for the access data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2007 Tyler Berry.
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

#include "check/check.h"
#include "newts/access.h"

int euid;
static struct access *access_entry;

void
setup_access (void)
{
  access_entry = access_alloc ();
}

void
teardown_access (void)
{
  access_free (access_entry);
}

START_TEST (test_name)
{
  char *string = "fred";

  access_set_name (access_entry, string);

  fail_if (access_name (access_entry) == NULL, NULL);
  fail_unless (strcmp (access_name (access_entry), string) == 0, NULL);
}
END_TEST

START_TEST (test_initial_permissions_are_clear)
{
  fail_unless (access_permissions (access_entry) == 0, NULL);
}
END_TEST

START_TEST (test_permissions)
{
  unsigned permissions = READ | WRITE;

  access_set_permissions (access_entry, permissions);

  fail_unless (access_permissions (access_entry) == permissions, NULL);
}
END_TEST

START_TEST (test_add_single_permission)
{
  unsigned permissions = READ;

  access_add_permissions (access_entry, permissions);

  fail_unless (access_has_permissions (access_entry, permissions), NULL);
  fail_if (access_has_permissions (access_entry, ~permissions), NULL);
  fail_unless (access_permissions (access_entry) == permissions, NULL);
}
END_TEST

START_TEST (test_add_multiple_permissions)
{
  unsigned permissions = READ | WRITE;

  access_add_permissions (access_entry, permissions);

  fail_unless (access_has_permissions (access_entry, permissions), NULL);
  fail_if (access_has_permissions (access_entry, ~permissions), NULL);
  fail_unless (access_permissions (access_entry) == permissions, NULL);
}
END_TEST

START_TEST (test_remove_single_permission)
{
  unsigned original_permissions = READ | WRITE | REPLY | DIRECTOR;
  unsigned target_permissions = READ | WRITE | REPLY;

  access_add_permissions (access_entry, original_permissions);

  fail_unless (access_has_permissions (access_entry, original_permissions),
               NULL);
  fail_if (access_has_permissions (access_entry, ~original_permissions),
           NULL);
  fail_unless (access_permissions (access_entry) == original_permissions,
               NULL);

  access_remove_permissions (access_entry, DIRECTOR);

  fail_unless (access_has_permissions (access_entry, target_permissions),
               NULL);
  fail_if (access_has_permissions (access_entry, ~target_permissions),
           NULL);
  fail_unless (access_permissions (access_entry) == target_permissions,
               NULL);
}
END_TEST

START_TEST (test_remove_multiple_permissions)
{
  unsigned original_permissions = READ | WRITE | REPLY | DIRECTOR;
  unsigned target_permissions = READ | REPLY;

  access_add_permissions (access_entry, original_permissions);

  fail_unless (access_has_permissions (access_entry, original_permissions),
               NULL);
  fail_if (access_has_permissions (access_entry, ~original_permissions),
           NULL);
  fail_unless (access_permissions (access_entry) == original_permissions,
               NULL);

  access_remove_permissions (access_entry, DIRECTOR | WRITE);

  fail_unless (access_has_permissions (access_entry, target_permissions),
               NULL);
  fail_if (access_has_permissions (access_entry, ~target_permissions),
           NULL);
  fail_unless (access_permissions (access_entry) == target_permissions,
               NULL);
}
END_TEST

START_TEST (test_clear_permissions)
{
  unsigned original_permissions = READ | WRITE;
  unsigned target_permissions = 0;

  access_add_permissions (access_entry, original_permissions);

  fail_unless (access_permissions (access_entry) == original_permissions,
               NULL);

  access_clear_permissions (access_entry);

  fail_unless (access_permissions (access_entry) == target_permissions, NULL);
}
END_TEST

START_TEST (test_scope)
{
  enum newts_access_scopes scope = SCOPE_GROUP;

  access_set_scope (access_entry, scope);

  fail_unless (access_scope (access_entry) == scope, NULL);
}
END_TEST

Suite *
access_suite (void)
{
  Suite *suite = suite_create ("access");
  TCase *accessors = tcase_create ("Accessors and Mutators");
  TCase *bitmap = tcase_create ("Bitmap Mutators");

  suite_add_tcase (suite, accessors);
  tcase_add_checked_fixture (accessors, setup_access, teardown_access);

  tcase_add_test (accessors, test_name);
  tcase_add_test (accessors, test_permissions);
  tcase_add_test (accessors, test_scope);

  suite_add_tcase (suite, bitmap);
  tcase_add_checked_fixture (bitmap, setup_access, teardown_access);

  tcase_add_test (bitmap, test_initial_permissions_are_clear);
  tcase_add_test (bitmap, test_add_single_permission);
  tcase_add_test (bitmap, test_add_multiple_permissions);
  tcase_add_test (bitmap, test_remove_single_permission);
  tcase_add_test (bitmap, test_remove_multiple_permissions);
  tcase_add_test (bitmap, test_clear_permissions);

  return suite;
}

int
main (void)
{
  int failures;
  Suite *suite = access_suite ();
  SRunner *srunner = srunner_create (suite);

  srunner_run_all (srunner, CK_ENV);
  failures = srunner_ntests_failed (srunner);
  srunner_free (srunner);

  return (failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
