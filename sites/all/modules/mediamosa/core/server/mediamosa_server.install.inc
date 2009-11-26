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
 * Install for mediamosa_server module.
 */

require_once 'download/mediamosa_server_download.install.inc';
require_once 'still/mediamosa_server_still.install.inc';
require_once 'streaming/mediamosa_server_streaming.install.inc';
require_once 'transcoding/mediamosa_server_transcoding.install.inc';
require_once 'upload/mediamosa_server_upload.install.inc';

/**
 * Implements hook_install().
 */
function mediamosa_server_install() {

  // Call our child installers.

  // Download servers.
  mediamosa_server_download_install();

  // Still servers.
  mediamosa_server_still_install();

  // Streaming servers.
  mediamosa_server_streaming_install();

  // Transcoding servers.
  mediamosa_server_transcoding_install();

  // Upload servers.
  mediamosa_server_upload_install();
}

/**
 * Implements hook_uninstall().
 */
function mediamosa_server_uninstall() {

  // Upload servers.
  mediamosa_server_upload_uninstall();

  // Transcoding servers.
  mediamosa_server_transcoding_uninstall();

  // Streaming servers.
  mediamosa_server_streaming_uninstall();

  // Still servers.
  mediamosa_server_still_uninstall();

  // Download servers.
  mediamosa_server_download_uninstall();
}
