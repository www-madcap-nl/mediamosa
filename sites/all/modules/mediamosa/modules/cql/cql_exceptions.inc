<?php
// $Id$

/**
 * Copyright (c) 2008 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 *
 * Permission is granted for use, copying, modification, distribution,
 * and distribution of modified versions of this work as long as the
 * above copyright notice is included.
 */

/**
 * @file
 * Contains CQL exceptions defines
 */

define("CQL_EXCEPTION_SEVERITY_HIGH", 80);
define("CQL_EXCEPTION_SEVERITY_NORMAL", 50);
define("CQL_EXCEPTION_SEVERITY_LOW", 20);
define("CQL_EXCEPTION_SEVERITY_NONE", 0);

/**
 * Default cql exception
 */
class cql_exception extends Exception {

  public function __construct($s_message, $severity = CQL_EXCEPTION_SEVERITY_NORMAL) {
    if (function_exists('watchdog') && $severity > CQL_EXCEPTION_SEVERITY_NORMAL) {
      watchdog("cql_exception", t('@message (Line: @line, File: @file)', array('@message' => $s_message, '@line' => $this->getLine(), '@file' => $this->getFile()) ));
    }

    parent::__construct($s_message);
  }
}

/**
 * Thrown when the end of the CQL string is reached
 * Is not an error, used for catch purposes
 */
class cql_exception_eos extends cql_exception {

  public function __construct() {
    parent::__construct(t('CQL: End of string reached'), CQL_EXCEPTION_SEVERITY_NONE);
  }
}

/**
 * Thrown when ) was found.
 * (end of scope)
 */
class cql_exception_eo_scope extends cql_exception {

  public function __construct() {
    parent::__construct(t('CQL: End of scope reached'), CQL_EXCEPTION_SEVERITY_NONE);
  }
}

/**
 * Thrown when cql_exception_eo_scope was captured without
 * the matching ( to catch it.
 *
 */
class cql_exception_eo_scope_unmatched extends cql_exception {

  public function __construct() {
    parent::__construct(t('CQL: Unmatched \')\' encountered'), CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when not matching quotes was found.
 */
class cql_exception_eo_quote_unmatched extends cql_exception {

  public function __construct() {
    parent::__construct(t('CQL: Unmatched \'"\' encountered'), CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown if an illegal escape was done on a char
 *
 */
class cql_exception_invalid_escape_char extends cql_exception {

  public function __construct($char, $i_pos) {
    parent::__construct(t('CQL: Invalid escape char \'@char\' on position @pos.', array('@char' => $char, '@pos' => $i_pos)), CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when string that was provided was empty...
 *
 */
class cql_exception_string_is_empty extends cql_exception {

  public function __construct() {
    parent::__construct(t('CQL: An empty CQL string was provided'), CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when a character was expected
 *
 */
class cql_exception_expecting_char extends cql_exception {

  public function __construct($char, $i_pos) {
    parent::__construct(t('CQL: Syntax error, expecting @char on position @pos', array('@char' => $char, '@pos' => $i_pos)), CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when we were still expecting some cql...
 *
 */
class cql_exception_unexpected_eos extends cql_exception {

  public function __construct() {
    parent::__construct(t('Unexpected end of string'), CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * This exception is not used by CQL (yet), but top layers may use it to when detecting
 * wrong mixed booleans. Mixing booleans might not be logical, but is allowed by CQL.
 *
 */
class cql_exception_unlogic_boolean_usage extends cql_exception {

  public function __construct() {
    parent::__construct(t('You are mixing OR and AND booleans in the same scope, not allowed (e.g. bad; (expression AND expression OR expression), good; (expression AND (expression OR expression)) ).'), CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when syntax error was found
 *
 */
class cql_exception_syntax_error extends cql_exception {

  public function __construct($str_message, $i_pos) {
    parent::__construct(t('Syntax error at position @pos, @message', array('@pos' => $i_pos, '@message' => $str_message)), CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}
