/*
 *  moko-listener; An interface for listening to libgsmd events
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef MOKO_LISTENER_H
#define MOKO_LISTENER_H

#include <glib-object.h>
#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>
#include <libgsmd/misc.h>
#include <libgsmd/sms.h>
#include <libgsmd/voicecall.h>
#include <libgsmd/phonebook.h>
#include <gsmd/usock.h>

#define MOKO_TYPE_LISTENER		(moko_listener_get_type ())
#define MOKO_LISTENER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj),\
				 MOKO_TYPE_LISTENER, MokoListener))
#define MOKO_IS_LISTENER(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
				 MOKO_TYPE_LISTENER))
#define MOKO_LISTENER_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst),\
					 MOKO_TYPE_LISTENER, MokoListenerInterface))

typedef struct _MokoListener MokoListener; /* Dummy object */
typedef struct _MokoListenerInterface MokoListenerInterface;

struct _MokoListenerInterface {
	GTypeInterface parent;

  void  (*on_network_registered) (MokoListener *listener,
                                  struct lgsm_handle *handle, int type,
                                  int lac, int cell);
  void  (*on_pin_requested)      (MokoListener *listener,
                                  struct lgsm_handle *handle, int type);
  void  (*on_network_name)       (MokoListener *listener,
                                  struct lgsm_handle *handle,
                                  const gchar *name);
  void  (*on_network_list)       (MokoListener *listener,
                                  struct lgsm_handle *handle,
                                  const struct gsmd_msg_oper *opers);
  void  (*on_imsi)               (MokoListener *listener,
                                  struct lgsm_handle *handle,
                                  const gchar *imsi);
  void  (*on_subscriber_number)  (MokoListener *listener,
                                  struct lgsm_handle *handle,
                                  const gchar *number);
  void  (*on_incoming_call)      (MokoListener *listener,
                                  struct lgsm_handle *handle, int type);
  void  (*on_incoming_clip)      (MokoListener *listener,
                                  struct lgsm_handle *handle,
                                  const gchar *number);
  void  (*on_call_progress)      (MokoListener *listener,
                                  struct lgsm_handle *handle, int type);
  void  (*on_incoming_sms)       (MokoListener *listener,
                                  struct lgsm_handle *handle,
                                  const struct gsmd_sms_list *sms);
  void  (*on_incoming_ds)        (MokoListener *listener,
                                  struct lgsm_handle *handle,
                                  const struct gsmd_sms_list *sms);
  void  (*on_send_sms)           (MokoListener *listener,
                                  struct lgsm_handle *handle,
                                  int result);
};

GType moko_listener_get_type (void);

void  moko_listener_on_network_registered (MokoListener *listener,
                                struct lgsm_handle *handle, int type,
                                int lac, int cell);
void  moko_listener_on_pin_requested      (MokoListener *listener,
                                struct lgsm_handle *handle, int type);
void  moko_listener_on_network_name       (MokoListener *listener,
                                struct lgsm_handle *handle,
                                const gchar *name);
void  moko_listener_on_network_list       (MokoListener *listener,
                                struct lgsm_handle *handle,
                                const struct gsmd_msg_oper *opers);
void  moko_listener_on_imsi               (MokoListener *listener,
                                struct lgsm_handle *handle,
                                const gchar *imsi);
void  moko_listener_on_subscriber_number  (MokoListener *listener,
                                struct lgsm_handle *handle,
                                const gchar *number);
void  moko_listener_on_incoming_call      (MokoListener *listener,
                                struct lgsm_handle *handle, int type);
void  moko_listener_on_incoming_clip      (MokoListener *listener,
                                struct lgsm_handle *handle,
                                const gchar *number);
void  moko_listener_on_call_progress      (MokoListener *listener,
                                struct lgsm_handle *handle, int type);
void  moko_listener_on_incoming_sms       (MokoListener *listener,
                                struct lgsm_handle *handle,
                                const struct gsmd_sms_list *sms);
void  moko_listener_on_incoming_ds        (MokoListener *listener,
                                struct lgsm_handle *handle,
                                const struct gsmd_sms_list *sms);
void  moko_listener_on_send_sms           (MokoListener *listener,
                                struct lgsm_handle *handle,
                                int result);

#endif /* MOKO_LISTENER_H */

