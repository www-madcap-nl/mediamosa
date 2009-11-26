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
  * Default collection include file.
  */

/**
 * Core Mediamosa core library.
 */
class mediamosa_lib {

  // ------------------------------------------------------------------ Consts.
  const MIME_TYPE_APPLICATION_X_EMPTY = 'application/x-empty';


  // ------------------------------------------------------------------ Static Functions.
  /**
   * Converts a string boolean ('TRUE', 'true', 'FALSE', 'false')
   * to a normal boolean value TRUE/FALSE
   *
   * @param string $str
   * @return boolean
   */
  static public function boolstr2bool($str) {
    if (is_bool($str)) {
      return $str;
    }
    $str = trim(mediamosa_unicode::strtolower($str));
    assert($str == 'true' || $str == 'false' || $str == '');

    return $str == 'true';
  }

  /**
   * Converts a boolean to a string variant  ('TRUE', 'FALSE')
   *
   * @param boolean $boolean
   * @return string
   */
  static public function bool2boolstr($boolean) {
    return $boolean ? 'TRUE' : 'FALSE';
  }
}
