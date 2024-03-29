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

// Change build number and version here.
define('MEDIAMOSA_BUILD', 1792);
define('MEDIAMOSA_VERSION', '3.0.8');

// Optional changes:
define('MEDIAMOSA_VERSION_SHORT', MEDIAMOSA_VERSION . '.' . MEDIAMOSA_BUILD);
define('MEDIAMOSA_VERSION_FULL', MEDIAMOSA_VERSION . '.' . MEDIAMOSA_BUILD);
//define('MEDIAMOSA_VERSION_SHORT', MEDIAMOSA_VERSION . '.' . MEDIAMOSA_BUILD . '-RC2');
//define('MEDIAMOSA_VERSION_FULL', MEDIAMOSA_VERSION . '.' . MEDIAMOSA_BUILD . '-Release Candidate 2');

/**
 * @file
 * Current version description.
 */
class mediamosa_version {

  // ------------------------------------------------------------------ Constants.
  const MEDIAMOSA_BUILD = MEDIAMOSA_BUILD;
  const MEDIAMOSA_VERSION = MEDIAMOSA_VERSION_SHORT;
  const MEDIAMOSA_VERSION_FULL = MEDIAMOSA_VERSION_FULL;

  // Official versions, only in define when actually used in REST calls.
  const MEDIAMOSA_VERSION_1_1_0 = '1.1.0';
  const MEDIAMOSA_VERSION_1_1_5 = '1.1.5';
  const MEDIAMOSA_VERSION_1_1_7 = '1.1.7';
  const MEDIAMOSA_VERSION_1_2_0 = '1.2.0';
  const MEDIAMOSA_VERSION_1_3_0 = '1.3.0';
  const MEDIAMOSA_VERSION_1_4_0 = '1.4.0';
  const MEDIAMOSA_VERSION_1_5_0 = '1.5.0';
  const MEDIAMOSA_VERSION_1_6_0 = '1.6.0';
  const MEDIAMOSA_VERSION_1_6_1 = '1.6.1';
  const MEDIAMOSA_VERSION_1_7_0 = '1.7.0';
  const MEDIAMOSA_VERSION_2_0_0 = '2.0.0';
  const MEDIAMOSA_VERSION_2_1_0 = '2.1.0';
  const MEDIAMOSA_VERSION_2_1_1 = '2.1.1';
  const MEDIAMOSA_VERSION_2_2_0 = '2.2.0';
  const MEDIAMOSA_VERSION_2_3_0 = '2.3.0';
  const MEDIAMOSA_VERSION_3_0_0 = '3.0.0';

  // Used for APPs.
  const LATEST = 'LATEST';

  const MAJOR = 'major';
  const MINOR = 'minor';
  const RELEASE = 'release';
  const BUILD = 'build';
  const INFO = 'info';

  // ------------------------------------------------------------------ Static Function..
  /**
   * Return the array of an version.
   *
   * @param string $version
   * @return array
   */
  static public function get_version($version) {
    list($major, $minor, $release) = preg_split("/[.:]+/", $version, 3) + array(0 => 1, 1 => 0, 2 => 0);
    return array(
      self::MAJOR => (int) $major,
      self::MINOR => (int) $minor,
      self::RELEASE => (int) $release,
      self::BUILD => NULL,
    );
  }

  /**
   * Return the current version of MediaMosa.
   *
   * @return array
   */
  static public function get_current_version($return_full = FALSE) {
    list($major, $minor, $release, $build, $info) = preg_split("/[.:-]+/", ($return_full? self::MEDIAMOSA_VERSION_FULL : self::MEDIAMOSA_VERSION), 5) + array(0 => 1, 1 => 0, 2 => 0, 3 => 1, 4 => '');
    return array(
      self::MAJOR => $major,
      self::MINOR => $minor,
      self::RELEASE => $release,
      self::BUILD => $build,
      self::INFO => $info,
    );
  }

  /**
   * Return the current version of MediaMosa as string.
   *
   * @param boolean $return_full
   *  Include the build and changeset.
   * @return string
   */
  static public function get_current_version_str($return_full = FALSE) {
    $version = self::get_current_version($return_full);
    return $version[self::MAJOR] . '.' . $version[self::MINOR] . '.' . $version[self::RELEASE] . ($return_full ? ' (build ' . $version[self::BUILD] . ') ' . $version[self::INFO] : '');
  }
}
