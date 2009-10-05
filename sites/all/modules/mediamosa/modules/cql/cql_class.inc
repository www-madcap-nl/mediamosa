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
 * cql               ::= cqlQuery ['sortby' sortSpec]
 * cqlQuery          ::= prefixAssignment cqlQuery | scopedClause
 * prefixAssignment  ::= '>' prefix '=' uri | '>' uri
 * scopedClause  	   ::= scopedClause booleanGroup searchClause | searchClause
 * booleanGroup  	   ::= boolean [modifierList]
 * boolean           ::= 'and' | 'or' | 'not' | 'prox'
 * searchClause   	 ::= '(' cqlQuery ')' | index relation searchTerm | searchTerm
 * relation          ::= comparitor [modifierList]
 * comparitor     	 ::= comparitorSymbol | namedComparitor
 * comparitorSymbol  ::= '=' | '>' | '<' | '>=' | '<=' | '<>' | '=='
 * namedComparitor   ::= identifier
 * modifierList      ::= modifierList modifier | modifier
 * modifier          ::= '/' modifierName [comparitorSymbol modifierValue]
 * sortSpec  	       ::= sortSpec singleSpec | singleSpec
 * singleSpec  	     ::= index [modifierList]
 * prefix, uri
 * modifierName,
 * modifierValue,
 * searchTerm, index ::= term
 * term              ::= identifier | 'and' | 'or' | 'not' | 'prox' | 'sortby'
 * identifier        ::= charString1 | charString2
 * charString1  	    := Any sequence of characters that does not include any of the following:
 *                       whitespace
 *                       ( (open parenthesis )
 *                       ) (close parenthesis)
 *                       =
 *                       <
 *                       >
 *                       '"' (double quote)
 *                       /
 * If the final sequence is a reserved word, that token is returned instead. Note that '.' (period)
 * may be included, and a sequence of digits is also permitted. Reserved words are 'and', 'or',
 * 'not', and 'prox' (case insensitive). When a reserved word is used in a search term,
 * case is preserved.
 *
 * charString2        := Souble quotes enclosing a sequence of any characters except double quote
 *                       (unless preceded by backslash (\)). Backslash escapes the character
 *                       following it. The resultant value includes all backslash characters except
 *                       those releasing a double quote (this allows other systems to interpret the
 *                       backslash character). The surrounding double quotes are not included.
 *
 *
 *
 * [[index] relation[/modifier]] [term] [sortBy index[/modifier]]
 */

/**
 * @file
 * Main CQL class
 */

/**
 * Settings and defines
 */
define('CQL_KEY_SETTING_VERIFY', 'cql_key_setting_verify');

// CQL part keys
define("CQL_PART_ID", "cql_part_id");
define("CQL_PART_STRING", "cql_part_string");


/**
 * Includes we need
 */
require_once('cql_exceptions.inc');

/**
 * CQL Class
 *
 * Contains the basic function for parsing the CQL string
 * Use cql_context instead
 */
abstract class cql {

  private $m_str_cql = FALSE;
  private $m_o_cql_part_group = FALSE;
  private $m_o_cql_sortby = FALSE;

  /**
   * Parse the CQL string
   * Will throw exceptions when errors are found in syntax
   *
   * @param string $str_cql
   * @param boolean $b_verify
   */
  protected function cql_parse($str_cql, $a_settings = array()) {
    assert(is_string($str_cql));
    assert(is_array($a_settings));

    // Copy it for later usage
    $this->m_str_cql = $str_cql;

    // Clear our collection of possible previous parse
    $this->m_o_cql_part_group = new cql_part_group();

    // Make sure these settings are set
    if (!isset($a_settings[CQL_KEY_SETTING_VERIFY])) {
      $a_settings[CQL_KEY_SETTING_VERIFY] = FALSE;
    }

    if ($str_cql == '') {
      throw new cql_exception_string_is_empty();
    }

    $i_pos = 0;

    try {
      while (1) {
        $this->cql_next_part($str_cql, $i_pos, $this->m_o_cql_part_group);
      }
    }
    catch (cql_exception_eos $e) {
      // no error when we catch this here
    }
    catch (cql_exception_eo_scope $e) {
      throw new cql_exception_eo_scope_unmatched();
    }
  }

  /**
   * Create new relation object.
   *
   * @param string $str_relation
   * @return object
   */
  protected function cql_new_part_relation_obj($str_relation) {
    return new cql_part_relation($str_relation);
  }

  /**
   * Create new index object.
   *
   * @param string $str_index
   * @return object
   */
  protected function cql_new_part_index_obj($str_index) {
    return new cql_part_index($str_index);
  }

  /**
   * Returns the main group object
   *
   * @return o_cql_part_group
   */
  protected function cql_part_group_get() {
    return $this->m_o_cql_part_group;
  }

  /**
   * Returns the sortby group
   *
   * @return o_cql_sortby
   */
  protected function cql_sortby_get() {
    return $this->m_o_cql_sortby;
  }

  /**
   * Verify the CQL string
   * Will throw exceptions when parsing failed
   *
   * @param string $str_cql
   * @return boolean
   */
  protected function cql_verify($str_cql) {
    cql::cql_parse($str_cql, array(CQL_KEY_SETTING_VERIFY => TRUE));
  }

  /**
   * Next position in the string..
   * throws cql_exception_eos when it reached the end
   *
   * @param string $str_cql
   * @param integer $i_pos
   * @return boolean true; char is escaped
   */
  private function cql_next_pos($str_cql, &$i_pos, $is_quoted) {

    $i_pos++;

    if (drupal_strlen($str_cql) <= $i_pos) {
      throw new cql_exception_eos(); // end of string
    }

    // If next char is escape '\', then skip it and test if the next char is allowed to escaped
    if (drupal_substr($str_cql, $i_pos, 1) == '\\') {

      if (!$is_quoted) {
        throw new cql_exception_syntax_error('character \ can only be used inside "quoted" strings');
      }

      $i_pos++;

      if (drupal_strlen($str_cql) <= $i_pos) {
        throw new cql_exception_unexpected_eos(); // end of string
      }

      // Removed code to prevent escape all chars, its now allowed on all chars.

      return TRUE;
    }

    return FALSE;
  }

  /**
   * Skip Whitespaces
   *
   * @param string $str_cql
   * @param integer $i_pos
   * @return integer
   */
  private function cql_skip_ws($str_cql, &$i_pos) {
    if (drupal_strlen($str_cql) <= $i_pos) {
      throw new cql_exception_eos(); // end of string
    }

    $is_quoted = FALSE;
    while (drupal_substr($str_cql, $i_pos, 1) == ' ') {
      $is_quoted = $this->cql_next_pos($str_cql, $i_pos, $is_quoted);
    }

    return $is_quoted;
  }

  /**
   * Check if anything is left in the string
   * will throw if end is reached, so only call
   * if you really need something to process or
   * expect it.
   *
   * @param string $str_cql
   * @param integer $i_pos
   */
  private function cql_has_next($str_cql, $i_pos) {
    return $this->cql_skip_ws($str_cql, $i_pos);
  }

  /**
   * Get next cql (sub)part
   *
   * @param string $str_cql
   * @param integer $i_pos
   * @return string
   */
  private function cql_next($str_cql, &$i_pos, $is_quoted = FALSE, $is_escaped = FALSE, $array_end_chars = array(' ', '/', '(', ')', '"'), $array_end_words = array('<>', '<=', '>=', '==', '=', '>', '<')) {
    $str_result = '';

    $a_symbol_end_keys = array('<', '>', '=');

    try {
      // 1st skip any whitechars before we capture
      if (!$is_quoted) {
        $this->cql_skip_ws($str_cql, $i_pos);
      }

      // Get 1st char
      $str_result = ($is_escaped ? '\\' : '') . drupal_substr($str_cql, $i_pos, 1);

      if ($str_result == ')') {
        return $str_result;
      }

      // If it starts with '"', then we must end with one
      if ($str_result == '"') {
        if ($is_quoted) {
          return '';
        }

        return $this->cql_next_quoted($str_cql, $i_pos, $is_escaped);
      }

      // If it starts with '(', then we are opening a new dimension
      // If it starts with ')', then we are closing a new dimension
      if (!$is_quoted && $str_result == '(') {
        $i_pos++;
        return $str_result;
      }

      // Next char
      $is_escaped = $this->cql_next_pos($str_cql, $i_pos, $is_quoted);

      // If a '/', we can expect whitespaces, so skip them
      if (!$is_quoted && $str_result == '/') {
        $this->cql_skip_ws($str_cql, $i_pos);
      }

      $b_include_end_keys_alhpa = false;
      if (in_array($str_result, $a_symbol_end_keys)) {
        $b_include_end_keys_alhpa = true;
      }

      // Now get the next part until any of the end chars or eos
      while (1) {
        if (!$is_escaped) {
          // End chars will contain '"', when we are inside quoted
          if (array_search(drupal_substr($str_cql, $i_pos, 1), $array_end_chars) !== FALSE) {
            break;
          }

          if (!$is_quoted) {
            if ($b_include_end_keys_alhpa) {

              if (ord(drupal_substr($str_cql, $i_pos, 1)) >= ord('A') && ord(drupal_substr($str_cql, $i_pos, 1)) <= ord('Z')) {
                break;
              }

              if (ord(drupal_substr($str_cql, $i_pos, 1)) >= ord('a') && ord(drupal_substr($str_cql, $i_pos, 1)) <= ord('z')) {
                break;
              }

              if (ord(drupal_substr($str_cql, $i_pos, 1)) >= ord('0') && ord(drupal_substr($str_cql, $i_pos, 1)) <= ord('9')) {
                break;
              }
            }
            elseif (array_search(drupal_substr($str_cql, $i_pos, 1), $a_symbol_end_keys) !== FALSE) {
              break;
            }
          }
        }

        if (!$is_quoted) {
          foreach ($array_end_words as $end_word) {
            if (drupal_strlen($str_cql) >= drupal_strlen($end_word) + $i_pos) {
              if (drupal_substr($str_cql, 0, drupal_strlen($end_word)) == $end_word) {
                break;
              }
            }
          }
        }

        // Copy next char
        $str_result .= ($is_escaped ? '\\' : '') . drupal_substr($str_cql, $i_pos, 1);

        // Next position...
        $is_escaped = $this->cql_next_pos($str_cql, $i_pos, $is_quoted);
      }
    }
    catch (cql_exception_eos $e) {
      // No problem if we catch this
      if ($str_result == '' || $str_result == '/') {
        throw $e; // rethrow, there was nothing left
      }

      assert($str_result != '"'); // removed code here that handled this, but $str_result can never be '"'...
    }

    return $str_result;
  }

  /**
   * Same as cql_next, except we are inside quoted string
   *
   * @param string $str_cql
   * @param integer $i_pos
   * @return string
   */
  private function cql_next_quoted($str_cql, &$i_pos) {
    $is_escaped = $this->cql_next_pos($str_cql, $i_pos, TRUE);

    $str = '"' . $this->cql_next($str_cql, $i_pos, TRUE, $is_escaped, array('"')) . '"';

    // If the string didn't end with an ", the cql is not valid.
    if (drupal_strlen($str_cql) <= $i_pos || drupal_substr($str_cql, $i_pos, 1) != '"') {
      throw new cql_exception_eo_quote_unmatched();
    }

    $i_pos++;
    return $str;
  }

  /**
   * Trim " from $str
   *
   * @param string $str
   * @return string
   */
  private function cql_trim_quotes($str) {
    if (drupal_strlen($str) < 2) {
      return $str;
    }

    if (drupal_substr($str, 0, 1) == '"' && drupal_substr($str, -1, -1) == '"') {
      return drupal_substr($str, 1, -1);
    }

    return $str;
  }

  /**
   * Get the next complete part
   *
   * @param string $str_cql
   * @param integer $i_pos
   */
  private function cql_next_part($str_cql, &$i_pos, &$o_cql_part_group) {

    // Get the first item from current pos
    $str_1 = $this->cql_next($str_cql, $i_pos);

    // Create a new cql part object
    $o_cql_part = new cql_part($o_cql_part_group, $this->cql_context_prefix_assignment_get());

    try {
      // Prefix assignment?
      while (drupal_substr($str_1, 0, 1) == '>') {

        // Get the next part
        $str_2 = $this->cql_next($str_cql, $i_pos);

        // Check if the next char is a '='
        if ($this->cql_next_is($str_cql, $i_pos, '=')) {
          // Get the next part
          $str_3 = $this->cql_next($str_cql, $i_pos);

          if ($str_3 != '=') {
            throw new cql_exception_expecting_char('=', $i_pos);
          }

          // Get the next part
          $str_4 = $this->cql_next($str_cql, $i_pos);

          $o_prefix_assignment = new cql_prefix_assignment($str_2, cql_trim_quotes($str_4));
          $o_cql_part->cql_part_prefix_assignment_add($o_prefix_assignment);
        }
        else {
          $o_prefix_assignment = new cql_prefix_assignment('', cql_trim_quotes($str_2));
          $o_cql_part->cql_part_prefix_assignment_add($o_prefix_assignment);
        }

        $str_1 = $this->cql_next($str_cql, $i_pos);
      }

      // May start with the word 'NOT'
      if ($this->cql_context_is_boolean_not($str_1)) {
        $o_cql_part->cql_part_has_boolean_not_set(TRUE);
        $str_1 = $this->cql_next($str_cql, $i_pos);
      }

      // '(' cqlQuery ')' | index relation searchTerm | searchTerm
      if ($str_1 == '(') {

        try {
          while(1) {
            $this->cql_next_part($str_cql, $i_pos, $o_cql_part);
          }
        }
        catch (cql_exception_eo_scope $e) {
          // Matching with this one, ignore
          // Expecting now boolean, sortby or eos
        }
      }
      elseif ($str_1 == ')') {
        $i_pos++;
        throw new cql_exception_eo_scope();
      }
      elseif (drupal_strtolower($str_1) != 'sortby')  {
        // str_1 possible index or searchTerm. We know when next is an relation or not

        // Check if this char_string_1 or _2 (both cases must be charString valid)
        $this->cql_must_be_charstring($str_1, $i_pos);

        // In case its a index...
        $o_cql_part_index = $this->cql_new_part_index_obj($str_1);
        assert($o_cql_part_index);

        // do modifiers
        $this->cql_modifiers_do($str_cql, $i_pos, $o_cql_part_index);

        // Get the next part
        $i_pos_old = $i_pos; // Incase its a single search term.

        try {
          $str_2 = $this->cql_next($str_cql, $i_pos);
        }
        catch (cql_exception_eos $e) {
          $str_2 = '';
        }

        // Next is either relation or search term
        if ($this->cql_context_is_relation($str_2)) {

          // If this an relation, then check if we created the index part

          // Add it to the main part
          $o_cql_part->cql_part_index_set($o_cql_part_index);

          // Check the index
          $o_cql_part->cql_part_verify_index($this, $i_pos - drupal_strlen($str_2));

          // Create relation part object
          $o_cql_part_relation = $this->cql_new_part_relation_obj($str_2);

          // do modifiers (if any)
          $this->cql_modifiers_do($str_cql, $i_pos, $o_cql_part_relation);

          // Add it to our object
          $o_cql_part->cql_part_relation_set($o_cql_part_relation);

          // Next is search term
          $str_1 = $this->cql_next($str_cql, $i_pos);

          // Check if this char_string_1 or _2
          $this->cql_must_be_charstring($str_1, $i_pos);
        }
        else {
          // At this point $str_1 must be a single search term, so modifiers on this one is not allowed
          // On the optional index object, there should be no modifiers.
          if ($o_cql_part_index->cql_modifier_has()) {
            throw new cql_exception_syntax_error('modifiers not allowed here', $i_pos);
          }

          // Ok one single search term, change back our pos.
          $i_pos = $i_pos_old;
        }

        // Search term (required)
        $o_cql_part->cql_part_search_term_set(new cql_part_search_term($str_1));
      }

      if (drupal_strtolower($str_1) != 'sortby') {
        $i_pos_old = $i_pos;
        try {
          $str_1 = $this->cql_next($str_cql, $i_pos);
        }
        catch (cql_exception_eos $e) {
          // we are done, nothing left and we may end here too
          $o_cql_part_group->cql_part_add($o_cql_part);
          return;
        }
      }

      if (drupal_strtolower($str_1) == 'sortby') { // Sortby
        $this->m_o_cql_sortby = new cql_sortby();

        // Get the next part
        $str_1 = $this->cql_next($str_cql, $i_pos);

        // Check if this char_string_1 or _2
        $this->cql_must_be_charstring($str_1, $i_pos);

        // Add index for sortby object
        $o_cql_sub_sortby = new cql_sub_sortby($str_1);

        // do modifiers (if any)
        $this->cql_modifiers_do($str_cql, $i_pos, $o_cql_sub_sortby);

        // Ok add it
        $this->m_o_cql_sortby->cql_sub_sortby_add($o_cql_sub_sortby);

        try {
          // Get the next part, if we end here its ok.
          while (1) {
            $str_1 = $this->cql_next($str_cql, $i_pos);

            // Make sure its valid
            $this->cql_must_be_charstring($str_1, $i_pos);

            $o_cql_sub_sortby = new cql_sub_sortby($str_1);

            // do modifiers (if any)
            $this->cql_modifiers_do($str_cql, $i_pos, $o_cql_sub_sortby);

            // Add it to our object
            $this->m_o_cql_sortby->cql_sub_sortby_add($o_cql_sub_sortby);
          }
        }
        catch (cql_exception_eos $e) {
          // this was expected here and ok
        }
      }
      elseif ($this->cql_context_is_boolean($str_1)) {
        $o_cql_part_boolean = new cql_part_boolean($str_1);

        // do modifiers (if any)
        $this->cql_modifiers_do($str_cql, $i_pos, $o_cql_part_boolean);

        // Set the boolean
        $o_cql_part->cql_part_boolean_set($o_cql_part_boolean);

        // If we end with a boolean, we must 'something' after it.
        $this->cql_has_next($str_cql, $i_pos);
      }
      elseif ($str_1 == ')') {
        // ignore, we handle this in next iteration
      }
      else {
        throw new cql_exception_syntax_error('expecting boolean, operator or sortby', $i_pos_old);
      }
    }
    catch (cql_exception_eos $e) {
      // unexpected!
      throw new cql_exception_unexpected_eos();
    }

    $o_cql_part_group->cql_part_add($o_cql_part);
  }

  /**
   * Check if next char matches with given char
   *
   * @param string $str_cql
   * @param integer $i_pos
   * @return boolean
   */
  final private function cql_next_is($str_cql, $i_pos, $char) {

    try {
      $this->cql_skip_ws($str_cql, $i_pos);
      return (drupal_substr($str_cql, $i_pos, 1) == $char ? TRUE : FALSE);
    }
    catch (cql_exception_eos $e) {
      // ignore
    }

    return FALSE;
  }

  /**
   * Check for modifiers and collect them when present
   *
   * @param string $str_cql
   * @param integer $i_pos
   * @param object/array $mixed_item
   */
  final private function cql_modifiers_do($str_cql, &$i_pos, $mixed_item) {

    try {
      while ($this->cql_next_is($str_cql, $i_pos, '/')) {
        $str_1 = drupal_substr($this->cql_next($str_cql, $i_pos), 1);

        // Check for valid string
        $this->cql_must_be_charstring($str_1, $i_pos);

        if (is_object($mixed_item)) {
          $mixed_item->cql_modifier_add($str_1, $i_pos);
        }
        elseif (is_array($mixed_item)) {
          $mixed_item[] = $str_1;
        }
        else {
          assert(0);
          throw new cql_exception('program error in cql_modifiers_do', $i_pos);
        }
      }
    }
    catch (cql_exception_eos $e) {
      // ignore
    }
  }

  /**
   * Test if its a valid 'string', will throw exception if not
   *
   * @param string $str_1
   */
  final private function cql_must_be_charstring($str_1, $i_pos) {
    if (!$this->cql_is_charstring($str_1)) {
      throw new cql_exception_syntax_error(sprintf('%s is not valid char string', $str_1), $i_pos);
    }
  }

  /**
   * Test if its a valid 'string'
   *
   * @param string $str
   * @return string
   */
  final function cql_is_charstring($str) {
    return ($this->cql_is_charstring_2($str) || $this->cql_is_charstring_1($str));
  }

  final function cql_is_charstring_1($str) {
    if ($str == '') {
      return false;
    }

    $a_invalidchars = array('(', ')', '=', '<', '>', '"', '/', ' ');
    for ($x = 0; $x < count($str); $x++) {
      if (drupal_substr($str, $x, 1) == '\\') {
        $x++ ;;// 2x
        continue;
      }

      if (in_array(drupal_substr($str, $x, 1), $a_invalidchars)) {
        return false;
      }
    }
    return true;
  }

  final private function cql_is_charstring_2($str) {
    return ($str != '' && drupal_substr($str, 0, 1) == '"' && drupal_substr($str, -1) == '"' && $str != '""');
  }

  /**
   * Converts a any date into a db mysql date (YYYY-MM-DD HH:MM:SS)
   *
   * @param string $str
   */
  final protected function cql_str_date_2_db_date($str) {
    $str_default = '0000-01-01T00:00:00';
    $str = $str . drupal_substr($str_default, drupal_strlen($str));

    $tz_old = date_default_timezone_get();
    date_default_timezone_set('UTC');
    $str_result = date('Y-m-d H:i:s', strtotime($str));
    date_default_timezone_set($tz_old);
    return $str_result;
  }

  /**
   * Build SQL string using array
   */
  protected function cql_parseresult_2_sql($a_results, $a_booleans = array('OR', 'AND')) {
    $a_return = array();
    $do_boolean = false;

    foreach ($a_results as $mixed_result) {
      if (is_array($mixed_result)) {
        $mixed_result = $this->cql_parseresult_2_sql($mixed_result, $a_booleans);
      }

      if (in_array($mixed_result, $a_booleans) && !$do_boolean) {
        return '';
      }

      $do_boolean = false;
      if ($mixed_result != '') {
        $a_return[] = $mixed_result;
        $do_boolean = !in_array($mixed_result, $a_booleans);
      }
    }

    // The last one can not end with an boolean, so make sure its removed
    if (count($a_return)) {
      $str_pop = array_pop($a_return);

      foreach ($a_booleans as $str_bool) {
        $str_bool = ' ' . $str_bool;
        while (drupal_strlen($str_pop) >= drupal_strlen($str_bool)) {
          if (drupal_substr($str_pop, -drupal_strlen($str_bool)) == $str_bool) {
            $str_pop = drupal_substr($str_pop, 0, drupal_strlen($str_pop) - drupal_strlen($str_bool));
           }
          else {
            break;
          }
        }
      }

      $a_return[] = $str_pop;
    }

    return (count($a_return) > 1 ? "(" . implode(" ", $a_return) . ")" : implode(" ", $a_return));
  }
}

/**
 * Class modifiers is used a extension on cql_parts and on sortby object
 *
 */
abstract class cql_modifiers {

  abstract protected function cql_modifier_allowed_modifier($str);

  protected $m_a_str_modifiers = array();

  /**
   * Add Modifier to the array
   *
   * @param string $str_modifier
   */
  public function cql_modifier_add($str_modifier, $i_pos) {
    assert(drupal_substr($str_modifier, 0, 1) != '/'); // provide without '/'

    if (!$this->cql_modifier_allowed_modifier(drupal_strtolower($str_modifier))) {
      throw new cql_exception_syntax_error(sprintf('%s is not an allowed modifier', $str_modifier), $i_pos - drupal_strlen($str_modifier));
    }

    $this->m_a_str_modifiers[] = drupal_strtolower($str_modifier);
  }

  /**
   * Test if there are any modifiers
   *
   * @return boolean
   */
  public function cql_modifier_has() {
     return (count($this->m_a_str_modifiers) ? TRUE : FALSE);
  }

  /**
   * retrieve modifier array
   *
   * @return array
   */
  public function cql_modifier_get_array() {
    return $this->m_a_str_modifiers;
  }
}

/**
 * Class sortby handles the sorting parameters in the cql string
 *
 */
class cql_sortby {
  private $m_a_sub_sortby = array();

  final public function cql_sub_sortby_add($str) {
    $this->m_a_sub_sortby[] = $str;
  }

  final public function cql_sub_sortby_parse($o_obj, $function_name, $a_prefix_assignments = array(), $array_settings = array()) {
    $a_results = array();

    // Walk through my sortby's
    foreach ($this->m_a_sub_sortby as $o_sub_sortby) {
      $str_result = $o_obj->$function_name($o_sub_sortby);

      assert($str_result != "");

      $a_results[] = $str_result;
    }

    return $a_results;
  }
}

/**
 * Sub class for multi sort bys
 *
 */
class cql_sub_sortby extends cql_modifiers {

  private $m_str_sort_by = "";

  final public function __construct($str_sort_by) {
    $this->m_str_sort_by = $str_sort_by;
  }

  /**
   * return the modifier string
   *
   * @return string
   */
  final public function cql_sort_by_get() {
    return $this->m_str_sort_by;
  }

  protected function cql_modifier_allowed_modifier($str) {
    return in_array($str, array('sort.descending', 'sort.ascending', 'descending', 'ascending')) ? TRUE : FALSE;
  }
}
