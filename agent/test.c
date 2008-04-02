/*
 * This file is part of the Nice GLib ICE library.
 *
 * (C) 2006, 2007 Collabora Ltd.
 *  Contact: Dafydd Harries
 * (C) 2006, 2007 Nokia Corporation. All rights reserved.
 *  Contact: Kai Vehmanen
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Nice GLib ICE library.
 *
 * The Initial Developers of the Original Code are Collabora Ltd and Nokia
 * Corporation. All Rights Reserved.
 *
 * Contributors:
 *   Dafydd Harries, Collabora Ltd.
 *
 * Alternatively, the contents of this file may be used under the terms of the
 * the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
 * case the provisions of LGPL are applicable instead of those above. If you
 * wish to allow use of your version of this file only under the terms of the
 * LGPL and not to allow others to use your version of this file under the
 * MPL, indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL. If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the MPL or the LGPL.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include "udp-fake.h"
#include "agent.h"
#include "agent-priv.h"

gint
main (void)
{
  NiceAgent *agent;
  NiceAddress addr_local, addr_remote;
  NiceCandidate *candidate;
  NiceUDPSocketFactory factory;
  GSList *candidates;

  nice_address_init (&addr_local);
  nice_address_init (&addr_remote);
  g_type_init ();
  g_thread_init (NULL);

  nice_udp_fake_socket_factory_init (&factory);

  g_assert (nice_address_set_from_string (&addr_local, "192.168.0.1"));
  g_assert (nice_address_set_from_string (&addr_remote, "192.168.0.2"));
  nice_address_set_port (&addr_remote, 2345);

  agent = nice_agent_new (&factory);

  g_assert (agent->local_addresses == NULL);

  /* add one local address */
  nice_agent_add_local_address (agent, &addr_local);

  g_assert (agent->local_addresses != NULL);
  g_assert (g_slist_length (agent->local_addresses) == 1);
  g_assert (nice_address_equal (agent->local_addresses->data, &addr_local));

  /* add a stream */
  nice_agent_add_stream (agent, 1);

  /* adding a stream should cause host candidates to be generated */
  candidates = nice_agent_get_local_candidates (agent, 1, 1);
  g_assert (g_slist_length (candidates) == 1);
  candidate = candidates->data;
  /* fake socket manager uses incremental port numbers starting at 1 */
  nice_address_set_port (&addr_local, 1);
  g_assert (nice_address_equal (&(candidate->addr), &addr_local));
  g_assert (strncmp (candidate->foundation, "1", 1) == 0);
  g_slist_free (candidates);

  /* add remote candidate */
  nice_agent_add_remote_candidate (agent, 1, 1, NICE_CANDIDATE_TYPE_HOST,
      &addr_remote, "username", "password");
  candidates = nice_agent_get_remote_candidates (agent, 1, 1);
  g_assert (candidates != NULL);
  g_assert (g_slist_length (candidates) == 1);
  candidate = candidates->data;
  g_assert (nice_address_equal (&(candidate->addr), &addr_remote));
  g_assert (candidate->stream_id == 1);
  g_assert (candidate->component_id == 1);
  g_assert (candidate->type == NICE_CANDIDATE_TYPE_HOST);
  g_assert (0 == strcmp (candidate->username, "username"));
  g_assert (0 == strcmp (candidate->password, "password"));
  g_slist_free (candidates);

  /* clean up */
  g_object_unref (agent);
  return 0;
}

