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
 * Unicode library is our layer for supporting charsets. MediaMosa only supports UTF8 for now.
 */


class mediamosa_unicode {

  /**
   * Translate special chars to simple version
   * Šíleně žluťoučký will translate to Silene zlutoucky
   * While it will leave untranslatable utf8 chars as original.
   *
   * @param string $str
   */
  static public function clear($str) {
    $result = '';

    // Set local to utf8, but make sure we switch back to original, no idea
    // what effect this might have.
    $old_locale = setlocale(LC_ALL, 0);
    setlocale(LC_ALL, array('en_GB.utf8', 'en_US.utf8'));
    $ascii = iconv('UTF-8', 'ASCII//TRANSLIT', (string)$str);
    setlocale(LC_ALL, $old_locale); // Back to orginal.

    for ($pos = 0; $pos < strlen($ascii); $pos++) {
      $result .= $ascii[$pos] == '?' ? mb_substr($str, $pos, 1) : $ascii[$pos];
    }

    return $result;
  }

  /**
   * Returns the correct size of an string
   */
  static public function strlen($str) {
    return mb_strlen($str);
  }

  /**
   * UTF8 version of substr
   * Using drupal function
   */
  static public function substr($str, $start, $length = NULL) {
    return is_null($length) ? mb_substr($str, $start) : mb_substr($str, $start, $length);
  }

  /**
   * mb_strrev doesn't exist, so need to make one.
   */
  static public function strrev($str) {
    $str_rev = '';
    for ($pos = mb_strlen($str); $pos >= 0; $pos--) {
      $str_rev .= mb_substr($str, $pos, 1);
    }

    return $str_rev;
  }

  /**
   * To lower case
   */
  static public function strtolower($str) {
    return mb_strtolower($str);
  }

  /**
   * To upper case
   */
  static public function strtoupper($str) {
    return mb_strtoupper($str);
  }

  /**
   * Returns pos of found needle
   */
  static public function strpos($str, $needle, $offset = 0) {
    return mb_strpos($str, $needle, $offset);
  }

  /**
   * Returns pos of found needle
   */
  static public function stripos($str, $needle, $offset = 0) {
    return mb_strpos(self::strtolower($str), self::strtolower($needle), $offset);
  }

  static public function printable($str) {
    for ($pos = 0; $pos < mb_strlen($str); $pos++) {
      if (in_array(mb_substr($str, $pos, 1), array("\n", "\t", "\0", "\r"))) {
        return FALSE;
      }
    }

    return TRUE;
  }
}
