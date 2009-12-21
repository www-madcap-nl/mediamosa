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

/**
 * The mediamosa INSTALL ID is used for multiple installations of the jobserver.
 */
if (is_null(@constant('MEDIAMOSA_INSTALL_ID'))) {
  define('MEDIAMOSA_INSTALL_ID', 'default'); // Not longer than 10 chars.
}

class mediamosa_settings {

  // ------------------------------------------------------------------ Consts.
  const MINIMUM_PHP_VERSION = '5.1.0';

  // Path / SAN NAS Settings.
  const SAN_NAS_DATA_FOLDER = 'data';
  const SAN_NAS_STILLS_FOLDER = 'stills';
  const SAN_NAS_TRANCODE_FOLDER = 'transcode';

  // Ticket locations.
  const PLAY_TICKET_LOCATION = 'links';
  const DOWNLOAD_TICKET_LOCATION = 'download_links';
  const STILL_TICKET_LOCATION = 'still_links';

  // Max Limits.
  const LIMIT_DEFAULT = 10; // We return 10 items by default.
  const LIMIT_MAX = 200;
  const LIMIT_MAX_IDS_ONLY = 5000; // In case we only return IDs instead of complete array's.

  // Max. supplements per asset.
  const MAX_SUPPLEMENT_PER_ASSET = 16;
}
