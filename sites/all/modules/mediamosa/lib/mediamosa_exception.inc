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
  * MediaMosa Exception classes.
  */

define('MEDIAMOSA_EXCEPTION_SEVERITY_HIGH', 80);
define('MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL', 50);
define('MEDIAMOSA_EXCEPTION_SEVERITY_LOW', 20);

/**
 * The basic omp_exception class
 *
 */
class mediamosa_exception extends Exception {

  const MEDIAMOSA_EXCEPTION_SEVERITY_HIGH = 80;
  const MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL = 50;
  const MEDIAMOSA_EXCEPTION_SEVERITY_LOW = 20;

  public function __construct($i_errorcode, $msg = '', $severity = self::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL) {
    $msg = ($msg == '' ? sprintf("MediaMosa exception was thrown: %d", $i_errorcode) : $msg);

    if ($severity >= MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL) {
      $log_serverity = WATCHDOG_NOTICE;

      if ($severity >= MEDIAMOSA_EXCEPTION_SEVERITY_HIGH) {
        $log_serverity = WATCHDOG_CRITICAL;
      }

      try {
        $backtrace = $this->getTrace();
      }
      catch (Exception $e) {
        assert($e);
        $backtrace = FALSE;
      }

      mediamosa::log(
        '@message (Exception thrown on line @line, in @file)<br />Partial backtrace: <pre>@backtrace</pre>',
        array(
          '@message' => $msg,
          '@line' => $this->getLine(),
          '@file' => $this->getFile(),
          '@backtrace' => $backtrace === FALSE ? 'not available' : var_export(array_slice($backtrace, 0, 3), TRUE),
        ),
        $log_serverity
      );
    }

    parent::__construct($msg, $i_errorcode);
  }

  public function mediamosa_exception_rest_response() {
    mediamosa::get()->set_result(mediamosa_response::ERROR, $this->getCode(), $this->getMessage());
  }
}

class mediamosa_exception_no_rest_interface extends mediamosa_exception {

  public function __construct() {
    parent::__construct(0, t('No REST interface available... Please enable ENABLE_* settings in your Apache 2 ini file.'));
  }
}

/**
 * Wrapper class for vp_error
 */
class mediamosa_exception_error extends mediamosa_exception {
  private $a_error;

  public function __construct($mediamosa_errorcode, $a_tags = array(), $severity = MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL) {
    if (!is_numeric($mediamosa_errorcode) && !is_null(constant($mediamosa_errorcode))) {
      $mediamosa_errorcode = constant($mediamosa_errorcode);
    }

    $this->a_error = mediamosa_error::error_code_find($mediamosa_errorcode, $a_tags);
    parent::__construct($this->a_error['code'], $this->a_error['message'], $severity);
  }

  public function mediamosa_exception_error_array_get() {
    return $this->a_error;
  }
}

/**
 * Thrown when access was denied, like not being owner.
 */
/*
class mediamosa_exception_error_access_denied extends mediamosa_exception_error {
  public function __construct($a_tags = array()) {
    parent::__construct(vp_error::ERRORCODE_NOT_AUTHORIZED, $a_tags);
  }
}
*/
/**
 * Low severity exception (is not logged).
 */
/*
class mediamosa_exception_error_low extends mediamosa_exception_error {
  public function __construct($mediamosa_errorcode, $a_tags = array()) {
    parent::__construct($mediamosa_errorcode, $a_tags, VP_EXCEPTION_SEVERITY_LOW);
  }
}
*/
/**
 * Exceptions extended on mediamosa_exception.
 */

class mediamosa_exception_program_error extends mediamosa_exception {
  public function __construct($msg, $a_tags = array()) {
    $msg = strtr($msg, $a_tags);
    parent::__construct(mediamosa_error::HTTP_INTERNAL_SERVER_ERROR, $msg, MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
  }
}

/**
 * Exceptions extended on mediamosa_exception.
 */
/*
class mediamosa_exception_empty_result extends mediamosa_exception_error {
  public function __construct() {
    parent::__construct(vp_error::ERRORCODE_EMPTY_RESULT);
  }
}
*/

/**
 * Exceptions extended on mediamosa_exception_program_error.
 */

/**
 * Not implemented exception handler.1
 */
/*
class mediamosa_exception_not_implemented_error extends mediamosa_exception_program_error {
  public function __construct($msg = '', $a_tags = array()) {
    $bt = $this->getTrace();

    $msg = sprintf(
      'Method %s::%s() (%s:%d) is not yet implemented! ' . $msg,
      $bt[0]['class'],
      $bt[0]['function'],
      basename($bt[0]['file']),
      $bt[0]['line']
    );
    parent::__construct($msg);
  }
}
*/
