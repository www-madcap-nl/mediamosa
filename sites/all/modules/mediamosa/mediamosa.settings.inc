<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

 /**
  * @file
  * Global settings file. (1.x replacement for shared_defines).
  */
class mediamosa_settings {

  // ------------------------------------------------------------------ Consts.
  const MINIMUM_PHP_VERSION = '5.1.0';

  // Maintenance/config page access
  const MEDIAMOSA_PERM_ACCESS_ADMINISTRATION = 'access mediamosa';
  const MEDIAMOSA_PERM_ADMINISTER = 'administer mediamosa';


  // Length of the unique ID generator, is always this length.
  const UUID_LENGTH = 24;

  // URL to asset.
  const URL_ASSET = 'mediamosa/browse/asset/@asset_id';

  // Path / SAN NAS Settings.
  const SAN_NAS_DATA_FOLDER = 'data';
  const SAN_NAS_STILLS_FOLDER = 'stills';
  const SAN_NAS_TRANSCODE_FOLDER = 'transcode';

  // Ticket locations.
  const PLAY_TICKET_LOCATION = 'links';
  const DOWNLOAD_TICKET_LOCATION = 'download_links';
  const STILL_TICKET_LOCATION = 'still_links';

  // Expiration settings, in seconds.
  const TICKET_EXPIRATION_PLAY = 600; // 10 minutes
  const TICKET_EXPIRATION_DOWNLOAD = 600; // 10 minutes
  const TICKET_EXPIRATION_STILL = 3600; // 60 minutes
  const TICKET_EXPIRATION_UPLOAD = 86400; // 24 hours

  // Max Limits.
  const LIMIT_DEFAULT = 10; // We return 10 items by default.
  const LIMIT_MAX = 200;
  const LIMIT_MAX_IDS_ONLY = 5000; // In case we only return IDs instead of complete array's.

  // Max. supplements per asset.
  const MAX_SUPPLEMENT_PER_ASSET = 16;

  // Still Settings.

  // Maximum number of still per video file
  // See jobserver_job table mediafile_dest field size
  // Notice: One still length is around 32 + 5 byte (because of serialization)
  const STILL_MAXIMUM = 100;
  // Still names extension on the case of multiple images
  // See: lua/vpx_transcode file too, it has the same constant!
  // If you want to change, you have to do it on both places
  const STILL_EXTENSION = '-%03d';

  // Maximum still file size in byte
  const STILL_FILE_MAXIMUM = 1000000;

  // Scene still: The still is THIS number of frames after the detected still change
  const STILL_SCENE_AFTER = 5;
  // Scene still: Minimal distance between two stills in frames
  const STILL_SCENE_DISTANCE = 25;

  // Time in seconds at which a still is made in a movie, if the movie is shorter
  // than 10 seconds, half the movie length is used.
  const STILL_DEFAULT_FRAME_TIME = 10;
  // Still default size (qcif = 176x144)
  const STILL_DEFAULT_SIZE = 'qcif';
  // Stills are generated with STILL_INTERVAL as interval in seconds
  const STILL_INTERVAL = 1;
  // If you wish to space the interval (STILL_INTERVAL) every still being
  // created (STILLS_AMOUNT). 1 for equal spaced intervals
  const STILL_INTERVAL_JITTER = 1;
  // Minimal amount of colors that need to be present in a still to be valid
  const STILL_MIN_COLORS = 8;
  // Amount of extra stills to create to get the best still
  const STILLS_AMOUNT = 4;
  // To check the stills the amount of colors is calculated, to improved speed
  // stills are resampled. Lower sizes is faster but less accurate
  const STILL_RESAMPLE_WIDTH = 150;
  const STILL_RESAMPLE_HEIGHT = 150;

  const STILL_STRING = "' -s %s -padtop %d -padbottom %d -padleft %d -padright %d -an -deinterlace -y -ss %s -t %s -r %s -vcodec mjpeg -f image2'";
  const STILL_SCENE_STRING = 'ffmpeg -i %s -s %s -padtop %d -padbottom %d -padleft %d -padright %d -an -deinterlace -y -scene %s %s -r 1 -f image2 %s';

  // Analyse settings.
  const ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION = '--always_hint_mp4';
  const ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION = '--always_insert_metadata';

  // Job timeout settings.
  const JOB_UPLOAD_TIMEOUT = 10800; // 3 hours, POST upload timeout is 2 hours.
  const JOB_JOB_TIMEOUT = 10800;
  const JOB_TRANSCODE_TIMEOUT = 7200;

  // Header item used in internal calls.
  const X_MEDIAMOSA_INTERNAL_PASS = 'X-Mediamosa-Internal-Pass';

  // Upload Settings.
  const UPLOAD_PUT_CHUNK_SIZE = 4096;
  const UPLOAD_PROGRESS_INTERVAL = 5;

  // ------------------------------------------------------------------ Functions.
  /**
   * returns the path where the lua scripts are.
   */
  static public function lua_base_get() {
    // LUA_BASE.
    $path = DRUPAL_ROOT . '/' . drupal_get_path('module', 'mediamosa');
    return $path . '/lib/';
  }

  /**
   * Return the filename of the transcode  lua file.
   */
  static public function ffmpeg_transcode_file() {
    // FFMPEG_TRANSCODE_FILE.
    return self::lua_base_get() . 'vpx-transcode';
  }

  /**
   * Return the filename of the windows transcode lha file.
   */
  static public function windows_transcode_file() {
    // WINDOWS_TRANSCODE_FILE.
    return 'start /min c:\lua\lua5.1 c:\lua\vpx-transcode-wmv.lua';
  }

  /**
   * Return the filename of the analyse lha file.
   */
  static public function analyse_file() {
    // ANALYSE_FILE.
    return self::lua_base_get() . 'vpx-analyse';
  }

  /**
   * Return the filename of the transcode lha file.
   */
  static public function still_file() {
    // STILL_FILE.
    return self::lua_base_get() . 'vpx-transcode';
  }
}
