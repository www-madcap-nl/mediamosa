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
 * Contains cql_part class
 */

/**
 * Includes we need
 */
require_once('cql_part_group_class.inc');

/**
 * Default CQL part Class
 * A part class contains [prefix assignment] [index[modifier] relation[modifier]] searchTerm
 *
 */
class cql_part extends cql_part_group {

  private $m_a_cql_part_group = array();

  private $m_cql_part_parent = FALSE;

  // Parts
  private $m_a_prefix_assignments = array();  // optional
  private $m_o_cql_part_index = FALSE;        // optional
  private $m_o_cql_part_relation = FALSE;     // optional
  private $m_o_cql_part_search_term = FALSE;  // required
  private $m_o_cql_part_boolean = FALSE;      // optional
  private $m_b_not_boolean = FALSE;           // optional (has NOT parameter in front)


  /**
   * Constructor
   */
  final public function __construct($o_cql_part_parent, $o_prefix_assignment) {
    $this->m_cql_part_parent = $o_cql_part_parent;
    $this->cql_part_prefix_assignment_add($o_prefix_assignment);
  }

  /**
   * Go back recursive, and check if we are 'notted'
   *
   */
  final public function cql_part_has_not() {
    if ($this->m_b_not_boolean) {
      return TRUE;
    }

    if ($this->m_cql_part_parent) {
      return $this->m_cql_part_parent->cql_part_has_not();
    }

    return FALSE;
  }

  /**
   * Add a prefix assignment (is object, might have modifiers)
   *
   * @param object $o_prefix_assignment
   */
  final public function cql_part_prefix_assignment_add($o_prefix_assignment) {
    $this->m_a_prefix_assignments[] = $o_prefix_assignment;
  }

  /**
   * Get the prefix assignments
   *
   * @param object
   */
  final public function cql_part_prefix_assignment_get() {
    return $this->m_a_prefix_assignments;
  }

  /**
   * Set a index (is object, might have modifiers)
   *
   * @param object $o_cql_part_index
   */
  final public function cql_part_index_set($o_cql_part_index) {
    $this->m_o_cql_part_index = $o_cql_part_index;
  }

  /**
   * Get the index object
   *
   * @return object
   */
  final public function cql_part_index_get() {
    return $this->m_o_cql_part_index;
  }

  /**
   * Set a relation (is object, might have modifiers)
   *
   * @param object $o_cql_part_relation
   */
  final public function cql_part_relation_set($o_cql_part_relation) {
    $this->m_o_cql_part_relation = $o_cql_part_relation;
  }

  /**
   * Get the relation object
   *
   * @return object $o_cql_part_relation
   */
  final public function cql_part_relation_get() {
    return $this->m_o_cql_part_relation;
  }

  /**
   * Set a search term (has no modifiers)
   *
   * @param object $o_cql_part_index
   */
  final public function cql_part_search_term_set($o_cql_part_search_term) {
    $this->m_o_cql_part_search_term = $o_cql_part_search_term;
  }

  /**
   * Get the search term object
   *
   * @return object $o_cql_part_index
   */
  final public function cql_part_search_term_get() {
    return $this->m_o_cql_part_search_term;
  }

  /**
   * Set a boolean (is object, might have modifiers)
   *
   * @param object $o_cql_part_relation
   */
  final public function cql_part_boolean_set($o_cql_part_boolean) {
    $this->m_o_cql_part_boolean = $o_cql_part_boolean;
  }

  /**
   * Get the boolean object (if any)
   *
   * @return object $o_cql_part_relation
   */
  final public function cql_part_boolean_get() {
    return $this->m_o_cql_part_boolean;
  }

  /**
   * checks if any exception was thrown
   *
   * @param cql_context $o_cql_context
   * @param integer $i_pos
   */
  final public function cql_part_verify_index(cql_context $o_cql_context, $i_pos) {
    if ($o_cql_context->cql_context_index2column($this) === FALSE) {
     $o_cql_part_index = $this->cql_part_index_get();
     $str_index = $o_cql_part_index->cql_part_str_get();
     throw new cql_exception_syntax_error(sprintf('unknown index; %s', $str_index), $i_pos - drupal_strlen($str_index));
    }
  }

  /**
   * Set if NOT is present in front of
   *
   * @param boolean $b_set
   */
  final public function cql_part_has_boolean_not_set($b_set) {
    $this->m_b_not_boolean = $b_set ? TRUE : FALSE;
  }
}

/**
 * Index is part of cql_part
 *
 */
class cql_part_index extends cql_modifiers {

  private $m_str_index = "";

  final public function __construct($str_index) {
    $this->m_str_index = $str_index;
  }

  protected function cql_modifier_allowed_modifier($str) {
    return in_array($str, array()) ? TRUE : FALSE;
  }

  /**
   * return the contents of the relation
   *
   * @return string
   */
  final public function cql_part_str_get() {
    return $this->m_str_index;
  }
}

/**
 * Relation is part of cql_part
 *
 */
class cql_part_relation extends cql_modifiers {

  private $m_str_relation = "";

  final public function __construct($str_relation) {
    $this->m_str_relation = $str_relation;
  }

  protected function cql_modifier_allowed_modifier($str) {
    return in_array($str, array('ignorecase', 'respectcase')) ? TRUE : FALSE;
  }

  /**
   * return the contents of the relation
   *
   * @return string
   */
  final public function cql_part_str_get() {
    return $this->m_str_relation;
  }
}

/**
 * Search term is part of cql_part
 *
 */
class cql_part_search_term {

  private $m_str_search_term = "";

  final public function __construct($str_search_term) {
    assert($str_search_term != "");

    // Remove any trailing "
    if ($str_search_term == '""') {
      $str_search_term = '';
    }
    elseif ($str_search_term{0} == '"' && drupal_substr($str_search_term, -1) == '"') {
      $str_search_term = drupal_substr($str_search_term, 1, -1);
    }

    $this->m_str_search_term = $str_search_term;
  }

  /**
   * return the contents of the search term
   *
   * @return string
   */
  final public function cql_part_str_get() {
    return $this->m_str_search_term;
  }

  /**
   * Check a escaped cql search term if it contains any wildcards
   *
   * @param string $str
   * @return boolean
   */
  final public function cql_contains_wildcards($str = FALSE) {
    $str = ($str === FALSE ? $this->cql_part_str_get() : $str);
    $str = str_replace(array('\\', '\*', '\?'), array(''), $str);
    return (mb_strpos($str, '?') === FALSE && mb_strpos($str, '*') === FALSE ? FALSE : TRUE);
  }
}

/**
 * boolean is part of cql_part
 *
 */
class cql_part_boolean extends cql_modifiers {

  private $m_type = FALSE;

  final public function __construct($str_boolean) {
    $a_booleans = array('or' => CQL_BOOLEAN_OR, 'and' => CQL_BOOLEAN_AND, 'not' => CQL_BOOLEAN_NOT, 'prox' => CQL_BOOLEAN_PROX);

    assert(isset($a_booleans[drupal_strtolower($str_boolean)]));
    if (!isset($a_booleans[drupal_strtolower($str_boolean)])) {
      throw new cql_exception(sprintf('Program error; %s is not an valid boolean', $str_boolean), CQL_EXCEPTION_SEVERITY_HIGH);
    }

    $this->m_type = $a_booleans[drupal_strtolower($str_boolean)];
  }

  protected function cql_modifier_allowed_modifier($str) {
    return in_array($str, array()) ? TRUE : FALSE;
  }

  /**
   * Returns the boolean type
   *
   * @return type
   */
  final public function cql_part_str_get() {
    $a_booleans = array(CQL_BOOLEAN_OR => 'OR', CQL_BOOLEAN_AND => 'AND', CQL_BOOLEAN_NOT => 'NOT', CQL_BOOLEAN_PROX => 'PROX');
    return $a_booleans[$this->m_type];
  }

  /**
   * Returns the boolean type
   *
   * @return type
   */
  final public function cql_part_type_get() {
    return $this->m_type;
  }
}
