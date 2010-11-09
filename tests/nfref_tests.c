/*
 * nfref_tests.c - tests for the nfref data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005, 2006, 2007 Tyler Berry.
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
 * A
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
#include "newts/nfref.h"

int euid;
static newts_nfref *ref;

void
setup_nfref (void)
{
  ref = nfref_alloc ();
}

void
teardown_nfref (void)
{
  nfref_free (ref);
}

START_TEST (test_ncp_standard_port)
{
  /* Port 25 is the ICANN assigned port for SMTP, so this should always pass,
   * regardless of whether we change the value of NEWTS_NCP_STANDARD_PORT in
   * the future.  This test exists because the test suite assumes that 25 is
   * not NEWTS_NCP_STANDARD_PORT in other places.
   */
  fail_if (NEWTS_NCP_STANDARD_PORT == 25, NULL);
}
END_TEST

START_TEST (test_name)
{
  char *string = "zoom";

  nfref_set_name (ref, string);

  fail_if (nfref_name (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_name (ref), string) == 0, NULL);
}
END_TEST

START_TEST (test_owner)
{
  char *string = "newts";

  nfref_set_owner (ref, string);

  fail_if (nfref_owner (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_owner (ref), string) == 0, NULL);
}
END_TEST

START_TEST (test_port)
{
  unsigned short port = 19852;

  nfref_set_port (ref, port);

  fail_unless (nfref_port (ref) == port, NULL);
}
END_TEST

START_TEST (test_protocol)
{
  enum newts_protocols protocol = NEWTS_PROTOCOL_NCP;

  nfref_set_protocol (ref, protocol);

  fail_if (nfref_protocol (ref) == 25, NULL);
  fail_unless (nfref_protocol (ref) == protocol, NULL);
}
END_TEST

START_TEST (test_system)
{
  char *string = "newts";

  nfref_set_system (ref, string);

  fail_if (nfref_system (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_system (ref), string) == 0, NULL);
}
END_TEST

START_TEST (test_user)
{
  char *string = "newts";

  nfref_set_user (ref, string);

  fail_if (nfref_user (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_user (ref), string) == 0, NULL);
}
END_TEST

START_TEST (test_set_name_to_null)
{
  char *garbage = "garbage";

  nfref_set_name (ref, garbage);
  fail_if (nfref_name (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_name (ref), garbage) == 0, NULL);

  nfref_set_name (ref, NULL);
  fail_unless (nfref_name (ref) == NULL, NULL);
}
END_TEST

START_TEST (test_set_owner_to_null)
{
  char *garbage = "garbage";

  nfref_set_owner (ref, garbage);
  fail_if (nfref_owner (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_owner (ref), garbage) == 0, NULL);

  nfref_set_owner (ref, NULL);
  fail_unless (nfref_owner (ref) == NULL, NULL);
}
END_TEST

START_TEST (test_set_system_to_null)
{
  char *garbage = "garbage";

  nfref_set_system (ref, garbage);
  fail_if (nfref_system (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_system (ref), garbage) == 0, NULL);

  nfref_set_system (ref, NULL);
  fail_unless (nfref_system (ref) == NULL, NULL);
}
END_TEST

START_TEST (test_set_user_to_null)
{
  char *garbage = "garbage";

  nfref_set_user (ref, garbage);
  fail_if (nfref_user (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_user (ref), garbage) == 0, NULL);

  nfref_set_user (ref, NULL);
  fail_unless (nfref_user (ref) == NULL, NULL);
}
END_TEST

START_TEST (test_uninitialized_pretty_name)
{
  fail_unless (nfref_pretty_name (ref) == NULL, NULL);
}
END_TEST

START_TEST (test_local_pretty_name)
{
  char *expected = "=test";

  nfref_set_name (ref, "test");
  nfref_set_owner (ref, NULL);
  nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
  nfref_set_port (ref, NEWTS_NCP_STANDARD_PORT);
  nfref_set_system (ref, "localhost");
  nfref_set_user (ref, "tests");

  fail_unless (nfref_system_is_localhost (ref), NULL);

  fail_if (nfref_pretty_name (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_null_name_returns_null_from_pretty_name)
{
  nfref_set_name (ref, NULL);
  nfref_set_owner (ref, NULL);
  nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
  nfref_set_port (ref, NEWTS_NCP_STANDARD_PORT);
  nfref_set_system (ref, "localhost");
  nfref_set_user (ref, "tests");

  fail_unless (nfref_pretty_name (ref) == NULL, NULL);
}
END_TEST

START_TEST (test_null_system_means_localhost_to_pretty_name)
{
  char *expected = "=test";

  nfref_set_name (ref, "test");
  nfref_set_owner (ref, NULL);
  nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
  nfref_set_port (ref, NEWTS_NCP_STANDARD_PORT);
  nfref_set_system (ref, NULL);
  nfref_set_user (ref, "tests");

  fail_unless (nfref_system_is_localhost (ref), NULL);

  fail_if (nfref_pretty_name (ref) == NULL, NULL);
  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_personal_pretty_name)
{
  char *expected = "=george:test";

  nfref_set_name (ref, "test");
  nfref_set_owner (ref, "george");
  nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
  nfref_set_port (ref, NEWTS_NCP_STANDARD_PORT);
  nfref_set_system (ref, "localhost");
  nfref_set_user (ref, "tests");

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_nonlocal_pretty_name)
{
  char *expected = "=other.system/test";

  nfref_set_name (ref, "test");
  nfref_set_owner (ref, NULL);
  nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
  nfref_set_port (ref, NEWTS_NCP_STANDARD_PORT);
  nfref_set_system (ref, "other.system");
  nfref_set_user (ref, "tests");

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_nonlocal_personal_pretty_name)
{
  char *expected = "=other.system/george:test";

  nfref_set_name (ref, "test");
  nfref_set_owner (ref, "george");
  nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
  nfref_set_port (ref, NEWTS_NCP_STANDARD_PORT);
  nfref_set_system (ref, "other.system");
  nfref_set_user (ref, "tests");

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_nonstandard_port_pretty_name)
{
  char *expected = "=other.system:25/test";

  nfref_set_name (ref, "test");
  nfref_set_owner (ref, NULL);
  nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
  nfref_set_port (ref, 25);
  nfref_set_system (ref, "other.system");
  nfref_set_user (ref, "tests");

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_nonstandard_port_personal_pretty_name)
{
  char *expected = "=other.system:25/george:test";

  nfref_set_name (ref, "test");
  nfref_set_owner (ref, "george");
  nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
  nfref_set_port (ref, 25);
  nfref_set_system (ref, "other.system");
  nfref_set_user (ref, "tests");

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_local)
{
  char *expected = "=test";

  parse_single_nf ("=test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (nfref_owner (ref) == NULL, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == NEWTS_NCP_STANDARD_PORT, NULL);
  fail_unless (nfref_system_is_localhost (ref), NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_local_without_equals)
{
  char *expected = "=test";

  parse_single_nf ("test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (nfref_owner (ref) == NULL, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == NEWTS_NCP_STANDARD_PORT, NULL);
  fail_unless (nfref_system_is_localhost (ref), NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_local_personal)
{
  char *expected = "=george:test";

  parse_single_nf ("=george:test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (strcmp (nfref_owner (ref), "george") == 0, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == NEWTS_NCP_STANDARD_PORT, NULL);
  fail_unless (nfref_system_is_localhost (ref), NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_nonlocal)
{
  char *expected = "=other.system/test";

  parse_single_nf ("=other.system/test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (nfref_owner (ref) == NULL, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == NEWTS_NCP_STANDARD_PORT, NULL);
  fail_if (nfref_system_is_localhost (ref), NULL);
  fail_unless (strcmp (nfref_system (ref), "other.system") == 0, NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_nonlocal_personal)
{
  char *expected = "=other.system/george:test";

  parse_single_nf ("=other.system/george:test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (strcmp (nfref_owner (ref), "george") == 0, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == NEWTS_NCP_STANDARD_PORT, NULL);
  fail_if (nfref_system_is_localhost (ref), NULL);
  fail_unless (strcmp (nfref_system (ref), "other.system") == 0, NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_nonstandard_port)
{
  char *expected = "=other.system:2413/test";

  parse_single_nf ("=other.system:2413/test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (nfref_owner (ref) == NULL, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == 2413, NULL);
  fail_if (nfref_system_is_localhost (ref), NULL);
  fail_unless (strcmp (nfref_system (ref), "other.system") == 0, NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_user)
{
  char *expected = "=other.system/test";

  parse_single_nf ("=bob@other.system/test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (nfref_owner (ref) == NULL, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == NEWTS_NCP_STANDARD_PORT, NULL);
  fail_if (nfref_system_is_localhost (ref), NULL);
  fail_unless (strcmp (nfref_system (ref), "other.system") == 0, NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_complicated)
{
  char *expected = "=other.system:2413/george:test";

  parse_single_nf ("=bob@other.system:2413/george:test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (strcmp (nfref_owner (ref), "george") == 0, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == 2413, NULL);
  fail_if (nfref_system_is_localhost (ref), NULL);
  fail_unless (strcmp (nfref_system (ref), "other.system") == 0, NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

START_TEST (test_parse_protocol)
{
  char *expected = "=test";

  parse_single_nf ("newts:///test", ref);

  fail_unless (strcmp (nfref_name (ref), "test") == 0, NULL);
  fail_unless (nfref_owner (ref) == NULL, NULL);
  fail_unless (nfref_protocol (ref) == NEWTS_PROTOCOL_NCP, NULL);
  fail_unless (nfref_port (ref) == NEWTS_NCP_STANDARD_PORT, NULL);
  fail_unless (nfref_system_is_localhost (ref), NULL);

  fail_unless (strcmp (nfref_pretty_name (ref), expected) == 0,
               "got '%s' instead of '%s'",
               nfref_pretty_name (ref), expected);
}
END_TEST

Suite *
nfref_suite (void)
{
  Suite *suite = suite_create ("nfref");
  TCase *assumptions = tcase_create ("Test Assumptions");
  TCase *accessors = tcase_create ("Accessors and Mutators");
  TCase *pretty = tcase_create ("Pretty Names");
  TCase *parsing = tcase_create ("Parsing");

  suite_add_tcase (suite, assumptions);

  tcase_add_test (assumptions, test_ncp_standard_port);

  suite_add_tcase (suite, accessors);
  tcase_add_checked_fixture (accessors, setup_nfref, teardown_nfref);

  tcase_add_test (accessors, test_name);
  tcase_add_test (accessors, test_owner);
  tcase_add_test (accessors, test_port);
  tcase_add_test (accessors, test_protocol);
  tcase_add_test (accessors, test_system);
  tcase_add_test (accessors, test_user);
  tcase_add_test (accessors, test_set_name_to_null);
  tcase_add_test (accessors, test_set_owner_to_null);
  tcase_add_test (accessors, test_set_system_to_null);
  tcase_add_test (accessors, test_set_user_to_null);

  suite_add_tcase (suite, pretty);
  tcase_add_checked_fixture (pretty, setup_nfref, teardown_nfref);

  tcase_add_test (pretty, test_uninitialized_pretty_name);
  tcase_add_test (pretty, test_local_pretty_name);
  tcase_add_test (pretty, test_null_name_returns_null_from_pretty_name);
  tcase_add_test (pretty, test_null_system_means_localhost_to_pretty_name);
  tcase_add_test (pretty, test_personal_pretty_name);
  tcase_add_test (pretty, test_nonlocal_pretty_name);
  tcase_add_test (pretty, test_nonlocal_personal_pretty_name);
  tcase_add_test (pretty, test_nonstandard_port_pretty_name);
  tcase_add_test (pretty, test_nonstandard_port_personal_pretty_name);

  suite_add_tcase (suite, parsing);
  tcase_add_checked_fixture (parsing, setup_nfref, teardown_nfref);

  tcase_add_test (parsing, test_parse_local);
  tcase_add_test (parsing, test_parse_local_without_equals);
  tcase_add_test (parsing, test_parse_local_personal);
  tcase_add_test (parsing, test_parse_nonlocal);
  tcase_add_test (parsing, test_parse_nonlocal_personal);
  tcase_add_test (parsing, test_parse_nonstandard_port);
  tcase_add_test (parsing, test_parse_user);
  tcase_add_test (parsing, test_parse_complicated);
  tcase_add_test (parsing, test_parse_protocol);

  return suite;
}

int
main (void)
{
  int failures;
  Suite *suite = nfref_suite ();
  SRunner *srunner = srunner_create (suite);

  srunner_run_all (srunner, CK_ENV);
  failures = srunner_ntests_failed (srunner);
  srunner_free (srunner);

  return (failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
