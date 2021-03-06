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
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __MILTER_MANAGER_CONTROL_PROTOCOL_H__
#define __MILTER_MANAGER_CONTROL_PROTOCOL_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define MILTER_MANAGER_CONTROL_COMMAND_SET_CONFIGURATION "set-configuration"
#define MILTER_MANAGER_CONTROL_COMMAND_GET_CONFIGURATION "get-configuration"
#define MILTER_MANAGER_CONTROL_COMMAND_RELOAD "reload"
#define MILTER_MANAGER_CONTROL_COMMAND_STOP_CHILD "stop-child"
#define MILTER_MANAGER_CONTROL_COMMAND_GET_STATUS "get-status"

#define MILTER_MANAGER_CONTROL_REPLY_SUCCESS "success"
#define MILTER_MANAGER_CONTROL_REPLY_FAILURE "failure"
#define MILTER_MANAGER_CONTROL_REPLY_ERROR "error"
#define MILTER_MANAGER_CONTROL_REPLY_CONFIGURATION "configuration"
#define MILTER_MANAGER_CONTROL_REPLY_STATUS "status"

G_END_DECLS

#endif /* __MILTER_MANAGER_CONTROL_PROTOCOL_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
