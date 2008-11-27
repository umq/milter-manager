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
#include <milter-test-utils.h>
#include <milter/core/milter-writer.h>
#undef shutdown
#include <errno.h>

void test_writer (void);
void test_writer_huge_data (void);
void test_writer_error (void);

static MilterWriter *writer;

static GIOChannel *channel;

static GError *expected_error;
static GError *actual_error;

void
setup (void)
{
    writer = NULL;

    channel = gcut_string_io_channel_new(NULL);
    g_io_channel_set_encoding(channel, NULL, NULL);

    expected_error = NULL;
    actual_error = NULL;
}

void
teardown (void)
{
    if (channel)
        g_io_channel_unref(channel);

    if (writer)
        g_object_unref(writer);

    if (expected_error)
        g_error_free(expected_error);
    if (actual_error)
        g_error_free(actual_error);
}

void
test_writer (void)
{
    const gchar first_chunk[] = "first\n";
    const gchar second_chunk[] = "sec\0ond\n";
    const gchar third_chunk[] = "third\n";
    gsize written_size;
    GString *actual_data;

    writer = milter_writer_io_channel_new(channel);

    milter_writer_write(writer, first_chunk, sizeof(first_chunk) - 1,
                        &written_size, &actual_error);
    gcut_assert_error(actual_error);
    cut_assert_equal_uint(sizeof(first_chunk) - 1, written_size);

    actual_data = gcut_string_io_channel_get_string(channel);
    cut_assert_equal_memory("first\n", sizeof(first_chunk) - 1,
                            actual_data->str, actual_data->len);
    gcut_string_io_channel_clear(channel);

    actual_error = NULL;
    milter_writer_write(writer, second_chunk, sizeof(second_chunk) - 1,
                        &written_size, &actual_error);
    gcut_assert_error(actual_error);
    cut_assert_equal_uint(sizeof(second_chunk) - 1, written_size);

    actual_data = gcut_string_io_channel_get_string(channel);
    cut_assert_equal_memory("sec\0ond\n", (sizeof(second_chunk) - 1),
                            actual_data->str, actual_data->len);
    gcut_string_io_channel_clear(channel);

    milter_writer_write(writer, third_chunk, sizeof(third_chunk) - 1,
                        &written_size, &actual_error);
    gcut_assert_error(actual_error);
    cut_assert_equal_uint(sizeof(third_chunk) - 1, written_size);

    actual_data = gcut_string_io_channel_get_string(channel);
    cut_assert_equal_memory("third\n", (sizeof(third_chunk) - 1),
                            actual_data->str, actual_data->len);
}

void
test_writer_huge_data (void)
{
    gchar *binary_data;
    gsize data_size;
    gsize written_size;
    GString *actual_data;

    writer = milter_writer_io_channel_new(channel);

    data_size = 192 * 8192;
    binary_data = g_new(gchar, data_size);
    cut_take_memory(binary_data);
    memset(binary_data, '\0', data_size);

    milter_writer_write(writer, binary_data, data_size,
                        &written_size, &actual_error);
    gcut_assert_error(actual_error);

    cut_assert_equal_uint(data_size, written_size);

    actual_data = gcut_string_io_channel_get_string(channel);
    cut_assert_equal_memory(binary_data, data_size,
                            actual_data->str, actual_data->len);
}

void
test_writer_error (void)
{
    gsize written_size;
    GError *error = NULL;

    g_io_channel_set_buffered(channel, FALSE);
    g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, &error);
    gcut_assert_error(error);
    gcut_string_io_channel_set_limit(channel, 1);
    writer = milter_writer_io_channel_new(channel);

    expected_error = g_error_new(G_IO_CHANNEL_ERROR,
                                 G_IO_CHANNEL_ERROR_NOSPC,
                                 "%s", g_strerror(ENOSPC));
    milter_writer_write(writer, "test-data", strlen("test-data"),
                        &written_size, &actual_error);
    gcut_assert_equal_error(expected_error, actual_error);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
