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
 * Contains cql_context class
 */

/**
 * Includes we need
 */
require_once('cql_class.inc');

/**
 * Default CQL Context Class
 *
 */
abstract class cql_context extends cql {

  // You must define these when extending with this class
  abstract protected function cql_context_prefix_assignment_get();
  abstract protected function cql_context_is_named_comparitor($str);
  abstract public function cql_context_index2column(cql_part $o_cql_part);

  // Store the last exception here.
  private $m_last_exception = FALSE;

  /**
   * Parse the given CQL string
   *
   * @param string $str_cql
   * @param array $a_settings
   * @return array/boolean
   */
  final protected function cql_parse($str_cql, $a_settings = array()) {

    try {
      parent::cql_parse($str_cql, $a_settings);
    }
    catch (cql_exception $e) {
      $this->m_last_exception = $e;
      return FALSE;
    }

    // If we get here to this point, there were no errors.
    return TRUE;
  }

  /**
   * Verify the given CQL string
   *
   * @param string $str_cql
   * @return boolean
   */
  final public function cql_verify($str_cql) {
    try {
      parent::cql_verify($str_cql);
    }
    catch (cql_exception $e) {
      $this->m_last_exception = $e;
      return FALSE;
    }

    // If we get here to this point, there were no errors.
    return TRUE;
  }

  /**
   * Return the last fatal exception that cause cql to stop
   *
   * @return object
   */
  final public function cql_last_exception_get() {
    return $this->m_last_exception;
  }

  /**
   * Check if given string is a known relation syntax
   *
   * @param string $str
   * @return boolean
   */
  final public function cql_context_is_relation($str) {
    return ($this->cql_context_is_comparitor_symbol($str) || $this->cql_context_is_named_comparitor($str));
  }

  final public function cql_context_is_comparitor_symbol($str) {
    $a_comparitor_symbol = array('=', '>', '<', '>=', '<=', '<>', '==');
    return (array_search($str, $a_comparitor_symbol) === FALSE ? FALSE : TRUE);
  }

  public function cql_context_is_boolean($str) {
    $a_booleans = array('and', 'or', 'not', 'prox');
    return (array_search(drupal_strtolower($str), $a_booleans) === FALSE) ? FALSE : TRUE;
  }

  public function cql_context_is_boolean_not($str) {
    return (drupal_strtolower($str) == 'not' ? TRUE : FALSE);
  }
}
