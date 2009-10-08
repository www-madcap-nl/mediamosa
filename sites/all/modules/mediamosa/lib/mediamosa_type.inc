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
 * The mediamosa TYPE class.
 */

class mediamosa_type {

  // ------------------------------------------------------------------ Consts
  // Do not make the ID value longer than 32 characters.
  //    12345678901234567890123456789012    12345678901234567890123456789012
  const TYPE_APP_ID                      = 'APP_ID'; // 1 - 9999
  const TYPE_INT                         = 'INT';
  const TYPE_UINT                        = 'UINT';
  const TYPE_SERIAL                      = 'SERIAL'; // Like autoinc. in db
  const TYPE_ALPHA                       = 'ALPHA'; // [A-Za-z]
  const TYPE_PRINTABLE                   = 'STRING_PRINTABLE';// only printable chars

  const TYPE_ALPHA_NUM                   = 'ALPHA_NUM'; // [A-Za-z0-9]
  const TYPE_ALPHA_NUM_UNDERSCORE        = 'ALPHA_NUM_UNDERSCORE'; // [A-Za-z0-9_]
  const TYPE_STRING                      = 'STRING';
  const TYPE_STRING_CQL                  = 'STRING_CQL';
  const TYPE_DATETIME                    = 'DATETIME';
  const TYPE_DATE                        = 'DATETIME';//@todo: Extend the DATETIME check and throw errors when time values exist.
  const TYPE_TIME                        = 'DATETIME';//@todo: Extend the DATETIME check and throw errors when date values exist.
  const TYPE_BOOL                        = 'BOOL';
  const TYPE_MIMETYPE                    = 'MIMETYPE';
  const TYPE_XML                         = 'STRING';
  const TYPE_XML_VALIDATED               = 'XML'; // XML with syntax validation via simplexml_load_string()
  const TYPE_EOR                         = 'EOR'; // Exclusive OR type (like 1 2 4 8, or 1|4 = 5), always UINT

  const TYPE_ORDER_DIRECTION             = 'ORDER_DIRECTION';

  // Store NOW() time as date
  const FUNC_DATETIME_NOW                = 'NOW()';

  // Default fixed values.
  const ORDER_DIRECTION_ASC = 'asc';
  const ORDER_DIRECTION_DESC = 'desc';



  // ------------------------------------------------------------------ Functions (statics)
  /**
   * Check the input based on type.
   * Will throw exception if not met with conditions.
   * Note: You must accept the value returned as the new value for this param.
   *
   * @param string $param
   *  name of param.
   * @param string $param_type
   *  Type of the param, see TYPE_* list.
   * @param mixed $mixed_param_value
   *  Either string or array (with strings)
   * @param boolean $is_required
   *  Supplied value must be provided
   * @param boolean $toplevel
   *  Internal boolean for array loops, leave as is
   * @return new param value
   */
  static public function check($param, $param_type, $mixed_param_value, $is_required, $range_start = NULL, $range_end = NULL, $allowed_values = NULL, $toplevel = TRUE) {

    if (is_array($mixed_param_value)) {
      // If the top array is empty and required, then throw an error.
      // Any child array is allowed to be empty.
      if (empty($mixed_param_value) && $is_required && $toplevel) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array('@param' => $param, '@type' => $param_type));
      }

      foreach ($mixed_param_value as $key => $value) {
        $mixed_param_value[$key] = self::check($param, $param_type, $value, $is_required, $range_start, $range_end, $allowed_values, false);
      }

      return $mixed_param_value; // done.
    }

    // @TODO add range for integers

    // When required and empty string, then in any type its an error
    if ($is_required && trim($mixed_param_value) == '') {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array('@param' => $param, '@type' => $param_type));
    }


    // Set to null so we know its been set, if not, it will throw exception.
    $checked_param_value = NULL;

    switch ($param_type) {
      case self::TYPE_APP_ID:
        if ( (!is_int($mixed_param_value) && !ctype_digit($mixed_param_value)) || $mixed_param_value < 1 || $mixed_param_value > 9999) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_APP_ID, array('@app_id' => $mixed_param_value));
        }

      case self::TYPE_SERIAL:
        // Same as UINT, but when required the value must be > 0. When not required, the value must be >= 0.
        if (!is_numeric($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (int)$mixed_param_value;

        // If required, then value must be > 0, if not required, then we allow >= 0.
        if ( ($is_required && $checked_param_value < 1) || (!$is_required && $checked_param_value < 0) ) { // unsigned only
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
        break;

      case self::TYPE_UINT:
      case self::TYPE_EOR:
        if (!is_numeric($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
        $checked_param_value = (int)$mixed_param_value;
        if ($checked_param_value < 0) { // unsigned only
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
        break;

      case self::TYPE_INT:
        if (!is_numeric($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (int)$mixed_param_value;
        break;

      case self::TYPE_ALPHA:
        if (!ctype_alpha($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
        $checked_param_value = (string)$mixed_param_value;
        break;

      case self::TYPE_ALPHA_NUM:
        if (!ctype_alnum($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case self::TYPE_PRINTABLE:
        // Must contain printable chars
        if (!mediamosa_unicode::printable($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case self::TYPE_ALPHA_NUM_UNDERSCORE:
        if (!ctype_alnum(str_replace('_', '', $mixed_param_value)) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case self::TYPE_STRING_CQL:
      case self::TYPE_STRING:
        $checked_param_value = (string)$mixed_param_value;
        break;

      case self::TYPE_XML_VALIDATED:
        // Validate xml if value is required or not empty.
        if ($is_required || $mixed_param_value != '') {
          try {
            if(!simplexml_load_string($mixed_param_value)) {
              throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
            }
          }
          catch (Exception $e) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
          }
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case self::TYPE_MIMETYPE:
        // @todo: some check here.
        $checked_param_value = (string)$mixed_param_value;
        break;

      case self::TYPE_BOOL:
        if (!is_bool($mixed_param_value)) {
          if (!in_array(mediamosa_unicode::strtolower($mixed_param_value), array('true', 'false'))) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
          }

          $mixed_param_value = (mediamosa_unicode::strtolower($mixed_param_value) == 'true') ? true : false;
        }

        $checked_param_value = (bool)$mixed_param_value;
        break;

      case self::TYPE_DATETIME:
        if ($mixed_param_value == '') {
          return '';
        }

        // If we supply NOW(), we fill in the time as UTC.
        if (mediamosa_unicode::strtoupper($mixed_param_value) == self::FUNC_DATETIME_NOW) {
          $mixed_param_value = mediamosa::utc_current_timestamp_now();
        }

        $iso8601 = '#^(\d{4}((-)?(0[1-9]|1[0-2])((-)?(0[1-9]|[1-2][0-9]|3[0-1])((T| )(24:00(:00(\.[0]+)?)?|(([0-1][0-9]|2[0-3])(:)[0-5][0-9])((:)[0-5][0-9](\.[\d]+)?)?)((\+|-)(14:00|(0[0-9]|1[0-3])(:)[0-5][0-9])|(|Z)))?)?)?)$#';
        if (!preg_match($iso8601, $mixed_param_value)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        // @todo fix it, strtotime is not 1901 lower compliant.
        $checked_param_value = date('Y-m-d H:i:s', strtotime($mixed_param_value));
        break;

      case self::TYPE_ORDER_DIRECTION:
        if (!in_array(mediamosa_unicode::strtolower($mixed_param_value), array(self::ORDER_DIRECTION_ASC, self::ORDER_DIRECTION_DESC))) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = mediamosa_unicode::strtolower($mixed_param_value);
        break;

      default:
        throw new mediamosa_exception_program_error(strtr('Using unspecified type (@type) in type lib used for param @param.', array('@param' => $param, '@type' => $param_type)));
    }

    if (is_bool($checked_param_value)) {
      assert(!isset($allowed_values)); // do not specify when bool.
      return $checked_param_value;
    }

    if (!is_string($checked_param_value) && !is_int($checked_param_value)) {
        throw new mediamosa_exception_program_error('Value type param @param (@type) is not integer or string value.', array('@param' => $param, '@type' => $param_type));
    }

    // Check if value is allowed as value (if specified and if required var).
    if ($is_required && isset($allowed_values) && !in_array($checked_param_value, $allowed_values)) {
       throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_VALUE_NOT_ALLOWED, array('@param' => $param, '@type' => $param_type, '@allowed_values' => implode("', '", $allowed_values)));
    }

    // Now do a range check.
    return is_string($checked_param_value) ? self::string_range($param, $checked_param_value, $range_start, $range_end) : self::int_range($param, $checked_param_value, $range_start, $range_end);
  }

  /**
   * Make sure the integer is in the valid range.
   *
   * @param string $param
   *  The Param name.
   * @param integer $int_val
   *  The value.
   * @param integer $min_value
   *  The minimum value allowed.
   * @param unknown_type $max_value
   *  The maximum value allowed.
   * @return
   *  Returns the same value, use it for future changes.
   *
   * Considered success when returning.
   * Returns the same value, use it for future changes.
   */
  static private function int_range($param, $int_val, $min_value, $max_value) {
    // Must be int type.
    assert(is_int($int_val));

    if (is_null($min_value) && is_null($max_value)) {
      return $int_val;
    }

    // To small?
    if (!is_null($min_value) && $int_val < $min_value) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_INT_TO_SMALL, array('@param' => $param, '@min_value' => $min_value));
    }

    // To big?
    if (!is_null($max_value) && $int_val > $max_value) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_INT_TO_BIG, array('@param' => $param, '@max_value' => $max_value));
    }

    return $int_val;
  }

  /**
   * Make sure that the string is nog to long or to short.
   *
   * @param string $param
   *  The Param name.
   * @param string $string_val
   *  The string value.
   * @param integer $min_length
   *  The minimum length.
   * @param integer $max_length
   *  The maximum length.
   * @return
   *  Returns the same value, use it for future changes.
   */
  static private function string_range($param, $string_val, $min_length, $max_length) {
    // Must be string type.
    assert(is_string($string_val));

    if (is_null($min_length) && is_null($max_length)) {
      return $string_val;
    }

    // To short?
    if (!is_null($min_length) && mediamosa_unicode::strlen($string_val) < $min_length) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_STRING_TO_SHORT, array('@param' => $param, '@min_value' => $min_length));
    }

    // To long?
    if (!is_null($max_length) && mediamosa_unicode::strlen($string_val) > $max_length) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_STRING_TO_LONG, array('@param' => $param, '@max_length' => $max_length));
    }

    return $string_val;
  }
}
