<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Contains all roles for Drupal / admin.
 */

/**
 * The MediaMosa permission work from right to left. Having a part of the role
 * name, will grant you access splitted on the '|';
 *
 * mediamosa|subrole will match mediamosa.
 *
 *
 */
class mediamosa_permission {

  // Name of the function to use for access ('access callback').
  const CALLBACK_USER_ACCESS = '_mediamosa_user_access';

  // Super user.
  const ADMIN_MEDIAMOSA = 'mediamosa';

  // Status page.
  const ACCESS_STATUS = 'mediamosa|status';
  const ACCESS_STATUS_FRONT = 'mediamosa|status|front';
  const ACCESS_STATUS_SERVERS = 'mediamosa|status|servers';
  const ACCESS_STATUS_JOB = 'mediamosa|status|job';

  // Browse page.
  const ACCESS_BROWSE = 'mediamosa|browse';
  const ACCESS_BROWSE_ASSETS = 'mediamosa|browse|assets';
  const ACCESS_BROWSE_COLLECTIONS = 'mediamosa|browse|collections';
  const ACCESS_BROWSE_RESTCALLS = 'mediamosa|browse|restcalls';
  const ACCESS_BROWSE_REVISIONS = 'mediamosa|browse|revisions';
  const ACCESS_BROWSE_LOG = 'mediamosa|browse|log';
  const ACCESS_BROWSE_ERRORCODES = 'mediamosa|browse|errorcodes';
  const ACCESS_BROWSE_TESTING = 'mediamosa|browse|testing';
  const ACCESS_BROWSE_BATCH = 'mediamosa|browse|batch';
  const ACCESS_BROWSE_FILE_INTEGRITY = 'mediamosa|browse|file_integrity';

  // Config.
  const ACCESS_CONFIG = 'mediamosa|config';
  const ACCESS_CONFIG_SERVERS = 'mediamosa|config|servers';
  const ACCESS_CONFIG_APPS = 'mediamosa|config|apps';
  const ACCESS_CONFIG_GLOBAL = 'mediamosa|config|global';
  const ACCESS_CONFIG_DEVELOPMENT = 'mediamosa|config|development';
  const ACCESS_CONFIG_TOOLPARAMS = 'mediamosa|config|toolparams';
  const ACCESS_CONFIG_TRANSCODE_PROFILES = 'mediamosa|config|transcode_profiles';
  const ACCESS_CONFIG_WEBSERVICES = 'mediamosa|config|webservices';

  // hmmm.
  const ACCESS_DRUPAL = 'mediamosa|drupal page';

  // Statistics.
  const ACCESS_STATS = 'mediamosa|stats';
  const ACCESS_STATS_DISKSPACE_USED = 'mediamosa|stats|diskspace used';
  const ACCESS_STATS_LATEST_COLLECTIONS = 'mediamosa|stats|latest collection';
  const ACCESS_STATS_LATEST_MEDIAFILES = 'mediamosa|stats|latest mediafile';
  const ACCESS_STATS_SEARCH_WORDS = 'mediamosa|stats|search words';
  const ACCESS_STATS_STREAM_REQUEST = 'mediamosa|stats|stream request';

  // Mediafile permissions.
  const MEDIAFILE_DELETE = 'mediamosa|mediafiles|delete';
  const MEDIAFILE_ANALYSE = 'mediamosa|mediafiles|analyse';
  const MEDIAFILE_DOWNLOAD = 'mediamosa|mediafiles|download';

  // Mediafiles job permissions.
  const JOB_DELETE = 'mediamosa|mediafiles|jobs|delete';
}
