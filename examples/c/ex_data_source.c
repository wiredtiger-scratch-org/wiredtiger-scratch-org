/*-
 * Public Domain 2008-2013 WiredTiger, Inc.
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * ex_data_source.c
 * 	demonstrates how to create and access a data source
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wiredtiger.h>

/*! [WT_EXTENSION_API declaration] */
#include <wiredtiger_ext.h>

static WT_EXTENSION_API *wt_api;

void
my_data_source_init()
{
	wiredtiger_extension_api(&wt_api);
}
/*! [WT_EXTENSION_API declaration] */

/*! [WT_DATA_SOURCE create] */
static int
my_create(WT_DATA_SOURCE *dsrc, WT_SESSION *session,
    const char *uri, int exclusive, void *config)
/*! [WT_DATA_SOURCE create] */
{
	/* Unused parameters */
	(void)dsrc;
	(void)session;
	(void)uri;
	(void)exclusive;
	(void)config;

	return (0);
}

/*! [WT_DATA_SOURCE compact] */
static int
my_compact(WT_DATA_SOURCE *dsrc, WT_SESSION *session,
    const char *uri, void *config)
/*! [WT_DATA_SOURCE compact] */
{
	/* Unused parameters */
	(void)dsrc;
	(void)session;
	(void)uri;
	(void)config;

	return (0);
}

/*! [WT_DATA_SOURCE drop] */
static int
my_drop(WT_DATA_SOURCE *dsrc, WT_SESSION *session,
    const char *uri, void *config)
/*! [WT_DATA_SOURCE drop] */
{
	/* Unused parameters */
	(void)dsrc;
	(void)session;
	(void)uri;
	(void)config;

	return (0);
}

int
data_source_cursor(void)
{
	return (0);
}
const char *
data_source_error(int v)
{
	return (v == 0 ? "one" : "two");
}

/*! [WT_DATA_SOURCE open_cursor] */
static int
my_open_cursor(WT_DATA_SOURCE *dsrc, WT_SESSION *session,
    const char *uri, void *config, WT_CURSOR **new_cursor)
/*! [WT_DATA_SOURCE open_cursor] */
{
	int ret;

	/* Unused parameters */
	(void)dsrc;
	(void)session;
	(void)uri;
	(void)config;
	(void)new_cursor;

	{
	/*! [WT_EXTENSION_CONFIG boolean] */
	WT_EXTENSION_CONFIG value;
	int my_data_source_overwrite;

	if ((ret =
	    wt_api->config(session, "overwrite", config, &value)) != 0) {
		(void)wt_api->err_printf(session,
		    "overwrite configuration: %s", wiredtiger_strerror(ret));
		return (ret);
	}
	my_data_source_overwrite = value.value != 0;
	/*! [WT_EXTENSION_CONFIG boolean] */
	}

	{
	/*! [WT_EXTENSION_CONFIG integer] */
	WT_EXTENSION_CONFIG value;
	int64_t my_data_source_page_size;

	if ((ret =
	    wt_api->config(session, "page_size", config, &value)) != 0) {
		(void)wt_api->err_printf(session,
		    "page_size configuration: %s", wiredtiger_strerror(ret));
		return (ret);
	}
	my_data_source_page_size = value.value;
	/*! [WT_EXTENSION_CONFIG integer] */
	}

	{
	/*! [WT_EXTENSION_CONFIG string] */
	WT_EXTENSION_CONFIG value;
	const char *my_data_source_key;

	if ((ret =
	    wt_api->config(session, "key_format", config, &value)) != 0) {
		(void)wt_api->err_printf(session,
		    "key_format configuration: %s", wiredtiger_strerror(ret));
		return (ret);
	}

	/*
	 * Strings returned from WT_EXTENSION_API::config are not necessarily
	 * nul-terminated; the associated length must be used instead.
	 */
	if (value.len == 1 && value.str[0] == 'r')
		my_data_source_key = "recno";
	else
		my_data_source_key = "bytestring";
	/*! [WT_EXTENSION_CONFIG string] */
	}

	{
	/*! [WT_EXTENSION_CONFIG list] */
	WT_EXTENSION_CONFIG value;
	size_t cnt;
	char **ap, **argv, *copy, *p, *t;

	if ((ret =
	    wt_api->config(session, "paths", config, &value)) != 0) {
		(void)wt_api->err_printf(session,
		    "paths configuration: %s", wiredtiger_strerror(ret));
		return (ret);
	}

	/*
	 * Strings returned from WT_EXTENSION_API::config are not necessarily
	 * nul-terminated; the associated length must be used instead.
	 *
	 * The strsep function requries nul-termination of the list.  Allocate
	 * memory and copy the list so it's nul-terminated.   Parse the list,
	 * counting entries.
	 *
	 * Allocate an array of pointers large enough to hold a reference to
	 * each list element, then re-parse the list, this time filling in the
	 * array.   Strsep nul-terminates each element in the list, so there's
	 * no addition work required.
	 *
	 * Finally, display the list.
	 */
	copy = calloc(1, value.len + 1);
	memcpy(copy, value.str, value.len);
	for (p = copy, cnt = 0; (t = strsep(&p, ",\t ")) != NULL;)
		if (*t != '\0')
			++cnt;

	memcpy(copy, value.str, value.len);
	argv = calloc(sizeof(char *), cnt + 1);
	for (ap = argv, p = copy; (*ap = strsep(&p, ",\t ")) != NULL;)
		if (**ap != '\0')
			++ap;

	for (ap = argv; *ap != NULL; ++ap)
		printf("%s\n", *ap);
	/*! [WT_EXTENSION_CONFIG list] */
	}

	/*! [WT_DATA_SOURCE error message] */
	/*
	 * If an underlying function fails, log the error and then return an
	 * error within WiredTiger's name space.
	 */
	if ((ret = data_source_cursor()) != 0) {
		(void)wt_api->err_printf(
		    session, "my_open_cursor: %s", data_source_error(ret));
		return (WT_ERROR);
	}
	/*! [WT_DATA_SOURCE error message] */

	return (0);
}

/*! [WT_DATA_SOURCE rename] */
static int
my_rename(WT_DATA_SOURCE *dsrc, WT_SESSION *session,
    const char *uri, const char *newname, void *config)
/*! [WT_DATA_SOURCE rename] */
{
	/* Unused parameters */
	(void)dsrc;
	(void)session;
	(void)uri;
	(void)newname;
	(void)config;

	return (0);
}

/*! [WT_DATA_SOURCE salvage] */
static int
my_salvage(WT_DATA_SOURCE *dsrc, WT_SESSION *session,
    const char *uri, void *config)
/*! [WT_DATA_SOURCE salvage] */
{
	/* Unused parameters */
	(void)dsrc;
	(void)session;
	(void)uri;
	(void)config;

	return (0);
}

/*! [WT_DATA_SOURCE truncate] */
static int
my_truncate(
    WT_DATA_SOURCE *dsrc, WT_SESSION *session, const char *uri, void *config)
/*! [WT_DATA_SOURCE truncate] */
{
	/* Unused parameters */
	(void)dsrc;
	(void)session;
	(void)uri;
	(void)config;

	return (0);
}

/*! [WT_DATA_SOURCE verify] */
static int
my_verify(
    WT_DATA_SOURCE *dsrc, WT_SESSION *session, const char *uri, void *config)
/*! [WT_DATA_SOURCE verify] */
{
	/* Unused parameters */
	(void)dsrc;
	(void)session;
	(void)uri;
	(void)config;

	return (0);
}

int
main(void)
{
	WT_CONNECTION *conn;
	int ret;

	ret = wiredtiger_open(NULL, NULL, "create", &conn);

	/*! [WT_DATA_SOURCE register] */
	static WT_DATA_SOURCE my_dsrc = {
		my_create,
		my_compact,
		my_drop,
		my_open_cursor,
		my_rename,
		my_salvage,
		my_truncate,
		my_verify
	};
	ret = conn->add_data_source(conn, "dsrc:", &my_dsrc, NULL);
	/*! [WT_DATA_SOURCE register] */

	/*! [WT_DATA_SOURCE configure boolean] */
	/* my_boolean defaults to true. */
	ret = conn->configure_method(conn,
	    "session.open_cursor", NULL, "my_boolean=true", "boolean", NULL);
	/*! [WT_DATA_SOURCE configure boolean] */

	/*! [WT_DATA_SOURCE configure integer] */
	/* my_integer defaults to 5. */
	ret = conn->configure_method(conn,
	    "session.open_cursor", NULL, "my_integer=5", "int", NULL);
	/*! [WT_DATA_SOURCE configure integer] */

	/*! [WT_DATA_SOURCE configure string] */
	/* my_string defaults to "name". */
	ret = conn->configure_method(conn,
	    "session.open_cursor", NULL, "my_string=name", "string", NULL);
	/*! [WT_DATA_SOURCE configure string] */

	/*! [WT_DATA_SOURCE configure list] */
	/* my_list defaults to "first" and "second". */
	ret = conn->configure_method(conn,
	    "session.open_cursor",
	    NULL, "my_list=[first, second]", "list", NULL);
	/*! [WT_DATA_SOURCE configure list] */

	/*! [WT_DATA_SOURCE configure integer with checking] */
	/*
	 * Limit the number of devices to between 1 and 30; the default is 5.
	 */
	ret = conn->configure_method(conn,
	    "session.open_cursor", NULL, "devices=5", "int", "min=1, max=30");
	/*! [WT_DATA_SOURCE configure integer with checking] */

	/*! [WT_DATA_SOURCE configure string with checking] */
	/*
	 * Limit target to one of /device, /home or /target; default to /home.
	 */
	ret = conn->configure_method(conn,
	    "session.open_cursor", NULL,
	    "target=/home", "string", "choices=[/device, /home, /target]");
	/*! [WT_DATA_SOURCE configure string with checking] */

	/*! [WT_DATA_SOURCE configure list with checking] */
	/*
	 * The paths are optional, so there's no default; limit paths to one or
	 * more of /device, /home, /mnt or /target.
	 */
	ret = conn->configure_method(conn,
	    "session.open_cursor", NULL,
	    "paths", "list", "choices=[/device, /home, /mnt, /target]");
	/*! [WT_DATA_SOURCE configure list with checking] */

	(void)conn->close(conn, NULL);

	return (ret);
}
