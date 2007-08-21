/*
 *  OpenMoko Media Player
 *   http://openmoko.org/
 *
 *  Copyright (C) 2007 by the OpenMoko team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @file persistent.c
 * Manages application configuration and session data
 */

#include <glib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "persistent.h"

/// The default configuration
struct _omp_config omp_default_config =
{
	FALSE,
	TRUE,
	OMP_REPEAT_OFF,
//	FALSE,
	FALSE,
	"%f",
	0.0,
	{0,0,0,0,0,0,0,0,0,0,0},
	TRUE
};

struct _omp_config *omp_config = NULL;			///< Global and persistent configuration data
struct _omp_session *omp_session = NULL;		///< Global and persistent session data



/**
 * Load application configuration data
 */
void
omp_config_init()
{
	#ifdef DEBUG
		g_print("Loading application configuration\n");
	#endif

	// This mustn't be called more than once
	g_assert(omp_config == NULL);

	// Set default config
	omp_config = g_new(struct _omp_config, 1);
	g_memmove(omp_config, &omp_default_config, sizeof(struct _omp_config));

	/// @todo GConf
}

/**
 * Releases resources allocated for configuration data
 */
void
omp_config_free()
{
	g_free(omp_config);
}

/**
 * Saves the current application configuration data to persistent storate
 */
void
omp_config_save()
{
}

/**
 * Fills the session data with sane default values
 */
void
omp_session_reset()
{
	memset(omp_session, 0, sizeof(struct _omp_session));

	omp_session->volume = 100;
	omp_session->fade_speed = 5000;
}

/**
 * Restores program state from last session
 */
void
omp_session_restore_state()
{
	#ifdef DEBUG
		g_print("Loading session data\n");
	#endif

	// This mustn't be called more than once
	g_assert(omp_session == NULL);

	omp_session = g_new0(struct _omp_session, 1);

	// Load config
	omp_session_load();

	omp_playback_set_volume(omp_session->volume);

	if (omp_session->playlist_file[0])
	{
		// Don't reset playlist state on load or else we'll alter the session
		// data in unwanted ways since the new session state would be saved
		omp_playlist_load(omp_session->playlist_file, FALSE);
	}

	// Check whether playlist_position is valid
	if (!omp_playlist_set_current_track(omp_session->playlist_position))
	{
		// Reset playlist state as playlist must have been modified since it was last loaded
		omp_session->playlist_position = 0;
		omp_session->track_position = 0;
		omp_playlist_set_current_track(0);
	}

	// Try to load the track, set the playback position and resume playback if needed
	if (omp_playlist_load_current_track())
	{
		if (omp_session->was_playing)
		{
			omp_playback_fade_volume();
			omp_playback_play();
		}

		omp_playback_set_track_position(omp_session->track_position);
	}
}

/**
 * Free resources used for session data
 */
void
omp_session_free()
{
	g_free(omp_session);
}

/**
 * Save session data to persistent storage
 */
void
omp_session_save()
{
	gint session_file, result;
	gchar *file_name;

	// SESSION_FILE_NAME is relative to user's home dir
	file_name = g_build_filename(g_get_home_dir(), SESSION_FILE_NAME, NULL);

	// Permissions for created session file are 0600
	session_file = creat(file_name, S_IRUSR | S_IWUSR);
	if (session_file == -1) goto io_error;

	result = write(session_file, omp_session, sizeof(struct _omp_session));
	if (result != sizeof(struct _omp_session)) goto io_error;

	result = close(session_file);
	if (result == -1) goto io_error;

	g_free(file_name);
	return;

	// We use a label to avoid duplicating code and improve readability
io_error:
	#ifdef DEBUG
		g_printerr("Failed trying to save session data to %s: %s\n", file_name, strerror(errno));
	#endif

	g_free(file_name);
}

/**
 * Load session data from persistent storage
 */
void
omp_session_load()
{
	gint session_file, result;
	gchar *file_name;

	// SESSION_FILE_NAME is relative to user's home dir
	file_name = g_build_filename(g_get_home_dir(), SESSION_FILE_NAME, NULL);

	session_file = open(file_name, O_RDONLY);
	if (session_file == -1) goto io_error;

	result = read(session_file, omp_session, sizeof(struct _omp_session));
	if (result != sizeof(struct _omp_session)) goto io_error;

	result = close(session_file);
	if (result == -1) goto io_error;

	g_free(file_name);
	return;

	// We use a label to avoid duplicating code and improve readability
io_error:
	#ifdef DEBUG
		g_printerr("Failed trying to load session data from %s: %s\n", file_name, strerror(errno));
		g_printerr("Resetting session data\n");
	#endif

	g_free(file_name);

	// Reset session data on error - just to be safe
	omp_session_reset();
}

/**
 * Saves values the playback engine needs to resume operation next time the player is started
 */
void
omp_session_set_playback_state(glong track_position, gboolean is_playing)
{
	omp_session->track_position = track_position;
	omp_session->was_playing = is_playing;

	omp_session_save();
}

/**
 * Set playlist to be loaded next session
 */
void
omp_session_set_playlist(gchar *playlist_file)
{
	g_snprintf(omp_session->playlist_file, sizeof(omp_session->playlist_file), "%s", playlist_file);
	omp_session_save();
}

/**
 * Set track to be loaded next session
 */
void
omp_session_set_track_id(guint track_id)
{
	omp_session->playlist_position = track_id;
	omp_session_save();
}

/**
 * Set volume to be set next session
 */
void
omp_session_set_volume(guint volume)
{
	omp_session->volume = volume;
	omp_session_save();
}
