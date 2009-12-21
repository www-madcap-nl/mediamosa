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
 * Installer Transcode.
 */
require_once 'profile/mediamosa_transcode_profile.install.inc';
require_once 'mapping/mediamosa_transcode_mapping.install.inc';

/**
 * Implement hook_install().
 */
function mediamosa_transcode_install() {
  // Create our table(s).
  mediamosa_transcode_profile_install();
  mediamosa_transcode_mapping_install();
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_transcode_uninstall() {
  // Drop our table(s).
  mediamosa_transcode_profile_uninstall();
  mediamosa_transcode_mapping_uninstall();
}
