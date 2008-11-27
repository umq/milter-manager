/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008  Kouhei Sutou <kou@cozmixng.org>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gcutter.h>

#define shutdown inet_shutdown
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>

#include <milter/server.h>
#include <milter/core.h>
#undef shutdown
#include "milter-test-utils.h"

void test_check_connect (void);
void test_check_helo (void);
void test_check_envelope_from (void);
void test_check_envelope_recipient (void);
void test_check_header (void);
void test_check_end_of_header (void);
void test_check_body (void);
void data_check_end_of_message (void);
void test_check_end_of_message (gconstpointer data);

static MilterServerContext *context;

static GIOChannel *channel;
static MilterReader *reader;
static MilterWriter *writer;

static gchar *actual_header_name;
static gchar *actual_header_value;
static gchar *actual_envelope_from;
static gchar *actual_envelope_recipient;

static gchar *connect_host_name;
static struct sockaddr *connect_address;
static socklen_t connect_address_size;
static gchar *helo_fqdn;
static gchar *body_chunk;
static gsize body_chunk_size;
static gchar *end_of_message_chunk;
static gsize end_of_message_chunk_size;

static guint n_accept;
static guint n_check_connect;
static guint n_check_helo;
static guint n_check_envelope_from;
static guint n_check_envelope_recipient;
static guint n_check_header;
static guint n_check_end_of_header;
static guint n_check_body;
static guint n_check_end_of_message;

static void
cb_accept (MilterServerContext *context)
{
    n_accept++;
}

static gboolean
cb_check_connect (MilterServerContext *context,
                  const gchar *host_name,
                  const struct sockaddr *address,
                  socklen_t address_size)
{
    connect_host_name = g_strdup(host_name);
    connect_address = malloc(address_size);
    memcpy(connect_address, address, address_size);
    connect_address_size = address_size;

    n_check_connect++;

    return TRUE;
}

static gboolean
cb_check_helo (MilterServerContext *context, const gchar *fqdn)
{
    helo_fqdn = g_strdup(fqdn);

    n_check_helo++;
    return TRUE;
}

static gboolean
cb_check_envelope_from (MilterServerContext *context, const gchar *from)
{
    actual_envelope_from = g_strdup(from);

    n_check_envelope_from++;
    return TRUE;
}

static gboolean
cb_check_envelope_recipient (MilterServerContext *context,
                             const gchar *recipient)
{
    actual_envelope_recipient = g_strdup(recipient);

    n_check_envelope_recipient++;
    return TRUE;
}

static gboolean
cb_check_header (MilterServerContext *context,
                  const gchar *name, const gchar *value)
{
    actual_header_name = g_strdup(name);
    actual_header_value = g_strdup(value);

    n_check_header++;
    return TRUE;
}

static gboolean
cb_check_end_of_header (MilterServerContext *context)
{
    n_check_end_of_header++;
    return TRUE;
}

static gboolean
cb_check_body (MilterServerContext *context, const gchar *chunk, gsize size,
               gpointer user_data)
{
    body_chunk = g_memdup(chunk, size);
    body_chunk_size = size;

    n_check_body++;
    return TRUE;
}

static gboolean
cb_check_end_of_message (MilterServerContext *context,
                         const gchar *chunk, gsize size,
                         gpointer user_data)
{
    end_of_message_chunk = g_memdup(chunk, size);
    end_of_message_chunk_size = size;

    n_check_end_of_message++;
    return TRUE;
}

static void
setup_signals (MilterServerContext *context)
{
#define CONNECT(name)                                                   \
    g_signal_connect(context, #name, G_CALLBACK(cb_ ## name), NULL)

    CONNECT(check_connect);
    CONNECT(check_helo);
    CONNECT(check_envelope_from);
    CONNECT(check_envelope_recipient);
    CONNECT(check_header);
    CONNECT(check_end_of_header);
    CONNECT(check_body);
    CONNECT(check_end_of_message);

    CONNECT(accept);
#undef CONNECT
}

void
setup (void)
{
    context = milter_server_context_new();
    setup_signals(context);

    channel = gcut_string_io_channel_new(NULL);
    g_io_channel_set_encoding(channel, NULL, NULL);
    reader = milter_reader_io_channel_new(channel);
    g_object_set(context, "milter-reader", reader, NULL);

    writer = milter_writer_io_channel_new(channel);
    milter_agent_set_writer(MILTER_AGENT(context), writer);

    actual_header_name = NULL;
    actual_header_value = NULL;
    actual_envelope_from = NULL;
    actual_envelope_recipient = NULL;
    connect_host_name = NULL;
    connect_address = NULL;
    connect_address_size = 0;
    helo_fqdn = NULL;
    body_chunk = NULL;
    body_chunk_size = 0;
    end_of_message_chunk = NULL;
    end_of_message_chunk_size = 0;

    n_accept = 0;
    n_check_connect = 0;
    n_check_helo = 0;
    n_check_envelope_from = 0;
    n_check_envelope_recipient = 0;
    n_check_header = 0;
    n_check_end_of_header = 0;
    n_check_body = 0;
    n_check_end_of_message = 0;
}

void
teardown (void)
{
    if (context)
        g_object_unref(context);

    if (reader)
        g_object_unref(reader);

    if (writer)
        g_object_unref(writer);

    if (channel) {
        gcut_string_io_channel_set_limit(channel, 0);
        g_io_channel_unref(channel);
    }

    if (actual_header_name)
        g_free(actual_header_name);
    if (actual_header_value)
        g_free(actual_header_value);
    if (actual_envelope_from)
        g_free(actual_envelope_from);
    if (actual_envelope_recipient)
        g_free(actual_envelope_recipient);
    if (connect_host_name)
        g_free(connect_host_name);
    if (connect_address)
        g_free(connect_address);
    if (helo_fqdn)
        g_free(helo_fqdn);
    if (body_chunk)
        g_free(body_chunk);
    if (end_of_message_chunk)
        g_free(end_of_message_chunk);
}

void
test_check_connect (void)
{
    struct sockaddr_in address;
    const gchar host_name[] = "mx.local.net";
    const gchar ip_address[] = "192.168.123.123";
    guint16 port;

    port = g_htons(50443);
    address.sin_family = AF_INET;
    address.sin_port = port;
    inet_aton(ip_address, &(address.sin_addr));

    cut_assert_true(milter_server_context_connect(context,
                                                  host_name,
                                                  (struct sockaddr *)&address,
                                                  sizeof(address)));
    cut_assert_equal_uint(1, n_check_connect);

    cut_assert_equal_string(host_name, connect_host_name);
    cut_assert_equal_int(sizeof(struct sockaddr_in), connect_address_size);

    cut_assert_equal_uint(1, n_accept);
}

void
test_check_helo (void)
{
    const gchar fqdn[] = "delian";

    cut_assert_true(milter_server_context_helo(context, fqdn));

    cut_assert_equal_uint(1, n_check_helo);
    cut_assert_equal_string(fqdn, helo_fqdn);

    cut_assert_equal_uint(1, n_accept);
}

void
test_check_envelope_from (void)
{
    const gchar from[] = "example@example.com";

    cut_assert_true(milter_server_context_envelope_from(context, from));

    cut_assert_equal_uint(1, n_check_envelope_from);
    cut_assert_equal_string(from, actual_envelope_from);

    cut_assert_equal_uint(1, n_accept);
}

void
test_check_envelope_recipient (void)
{
    const gchar recipient[] = "example@example.com";

    cut_assert_true(milter_server_context_envelope_recipient(context,
                                                             recipient));

    cut_assert_equal_uint(1, n_check_envelope_recipient);
    cut_assert_equal_string(recipient, actual_envelope_recipient);

    cut_assert_equal_uint(1, n_accept);
}

void
test_check_header (void)
{
    const gchar name[] = "X-Test-Header";
    const gchar value[] = "Test Value";

    cut_assert_true(milter_server_context_header(context, name, value));

    cut_assert_equal_uint(1, n_check_header);
    cut_assert_equal_string(name, actual_header_name);
    cut_assert_equal_string(value, actual_header_value);

    cut_assert_equal_uint(1, n_accept);
}

void
test_check_end_of_header (void)
{
    cut_assert_true(milter_server_context_end_of_header(context));
    cut_assert_equal_uint(1, n_check_end_of_header);
    cut_assert_equal_uint(1, n_accept);
}

void
test_check_body (void)
{
    const gchar body[] = "abcdefg";

    cut_assert_true(milter_server_context_body(context,
                                               body, sizeof(body)));


    cut_assert_equal_uint(1, n_check_body);
    cut_assert_equal_memory(body, sizeof(body), body_chunk, body_chunk_size);

    cut_assert_equal_uint(1, n_accept);
}

void
data_check_end_of_message (void)
{
    cut_add_data("no data", NULL, NULL,
                 "with data", g_strdup("abcdefg"), NULL);
}

void
test_check_end_of_message (gconstpointer data)
{
    const gchar *chunk = data;
    gsize size = 0;

    if (chunk)
        size = strlen(chunk);

    cut_assert_true(milter_server_context_end_of_message(context, chunk, size));

    cut_assert_equal_uint(1, n_check_end_of_message);
    cut_assert_equal_memory(chunk, size,
                            end_of_message_chunk, end_of_message_chunk_size);

    cut_assert_equal_uint(1, n_accept);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
