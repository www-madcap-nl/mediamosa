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
 * Common function for sannas.
 */
/* Data locations
 */

class mediamosa_configuration_storage {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the sannas setting for linux.
   *
   * @return string
   */
  static public function linux_get() {
    return mediamosa::variable_get('mediamosa_current_mount_point', '');
  }

  /**
   * Get the sannas setting for windows.
   *
   * @return string
   */
  static public function windows_get() {
    return mediamosa::variable_get('mediamosa_current_mount_point_windows', '');
  }

  /**
   * Set the sannas setting for linux.
   *
   * @param string $value
   */
  static public function linux_set($value) {
    mediamosa::variable_set('mediamosa_current_mount_point', $value);
  }

  /**
   * Set the sannas setting for windows.
   *
   * @param string $value
   */
  static public function windows_set($value) {
    mediamosa::variable_set('mediamosa_current_mount_point_windows', $value);
  }

  /**
   * Return the data location folder.
   * Based on 1.x define DATA_LOCATION.
   *
   * @return string
   */
  static public function data_location_get() {
    return mediamosa_settings::SAN_NAS_DATA_LOCATION;
  }

  /**
   * Return the still data location folder.
   * Based on the 1.x define STILL_LOCATION.
   *
   * @return string
   */
  static public function still_location_get() {
    return self::data_location_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_STILLS_FOLDER;
  }

	/**
	 * Return file location of asx file.
	 * The ASX file is stored in still directory?
	 *
	 * @return string
	 *  path to ticket location
	 */
	function asx_file_location($ticket) {
	  return self::still_location_get() . DIRECTORY_SEPARATOR . $ticket . '.asx';
	}


  /**
   * Return the base path based on OS.
   *
   * @return string
   */
  static public function base_path_get() { // vpx_get_san_nas_base_path mount_point_get
    if (substr(PHP_OS, 0, 3) == "WIN") {
      return self::windows_get();
    }

    // By default we return linux path.
    return self::linux_get();
  }


  /**
   * Get the Mediafile SAN/NAS path.
   *
   * @param array $a_mediafile
   *  The mediafile array.
   */
  static public function mediafile_filename_get($a_mediafile) {
    if (!isset($a_mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]) || empty($a_mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT])) {
      $a_mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT] = self::base_path_get();
    }

    return $a_mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT] . DIRECTORY_SEPARATOR . self::data_location_get() . DIRECTORY_SEPARATOR . $a_mediafile[mediamosa_asset_mediafile_db::ID]{0} . DIRECTORY_SEPARATOR . $a_mediafile[mediamosa_asset_mediafile_db::ID];
  }

  /**
   * Get the Mediafile still path.
   * @param string $mediafile_id
   *  The mediafile ID.
   */
  static public function mediafile_still_filename_get($mediafile_id) {
    return self::base_path_get() . DIRECTORY_SEPARATOR . self::still_location_get() . DIRECTORY_SEPARATOR . $mediafile_id{0} . DIRECTORY_SEPARATOR . $mediafile_id;
  }
}
