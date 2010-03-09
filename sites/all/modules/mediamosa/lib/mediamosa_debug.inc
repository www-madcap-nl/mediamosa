<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
  * Debug class.
  */
class mediamosa_debug {

  /**
   * Default debug logger, logs only in debug mode (level 5, normal).
   *
   * @param string $message
   * @param array $variables
   * @param string $type
   */
  static public function log_debug($message, array $variables = array(), $type = NULL) {
    if (self::is_debug()) {
      mediamosa::log($message, $variables, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * High debug logger, logs only in high debug mode (level 10, high).
   *
   * @param string $message
   * @param array $variables
   * @param string $type
   */
  static public function log_debug_high($message, array $variables = array(), $type = NULL) {
    if (self::is_debug_high()) {
      mediamosa::log($message, $variables, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * Log without logging to the database, go to php.log instead.
   */
  static public function error_log($message, $filename = NULL, $line = NULL) {
    error_log(strtr('In @filename at line @line; @message.', array('@filename' => ($filename ? $filename : 'unknown'), '@line' => (isset($line) ? $line : 'unknown'), '@message' => $message)));
  }

  /**
   * Returns TRUE / FALSE to indicate debug mode ON / OFF.
   */
  static public function is_debug() {
    return (variable_get('mediamosa_debug_level', 0) > 0);
  }

  /**
   * Returns TRUE / FALSE to indicate debug mode for logging.
   */
  static public function is_debug_high() {
    return (variable_get('mediamosa_debug_level', 0) >= 10);
  }
}