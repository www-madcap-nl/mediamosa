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
  * The parent main class for mediamosa CQL.
  */

define('MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING', 'use_having');
define('MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN', 'column');
define('MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT', 'table_for_sort');
define('MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS', 'table_alias');
define('MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID', 'prop_id');
define('MEDIAMOSA_CQL_CONTEXT_KEY_TYPE', 'type');
define('MEDIAMOSA_CQL_CONTEXT_KEY_JOIN', 'join');
define('MEDIAMOSA_CQL_CONTEXT_KEY_PREFIX', 'prefix');
define('MEDIAMOSA_CQL_CONTEXT_KEY_SUFFIX', 'suffix');
define('MEDIAMOSA_CQL_CONTEXT_KEY_NULL', 'null');// Indicate that value null is possible on this field
define('MEDIAMOSA_CQL_CONTEXT_KEY_DIRECTION', 'direction');


class mediamosa_core_cql {
  // ------------------------------------------------------------------ Static Functions.
  /**
   * Convert old 1.x search param to cql.
   *
   * @param string $name
   *  Name of variable.
   * @param string $match
   *  Match type.
   * @param array $a_value
   *  Value to compare.
   * @param unknown_type $type
   *  Type of param.
   * @param array $a_translate_to
   *  Extra array to translate name.
   */

  static public function convert_searchparam_to_cql($name, $match, array $a_value, $type, array $a_translate_to = array()) {
    $a_or = array();

    if (isset($a_translate_to[$name])) {
     $name = $a_translate_to[$name];
    }

    $a_or = array();

    switch ($type) {
      case mediamosa_type::TYPE_SEARCH_STRING:
        switch ($match) {
          case 'exact':
            foreach ($a_value as $value) {
              $a_or[] = $name . ' == "^' . addslashes($value) . '^"';
            }
            break;

          case 'begin':
            foreach ($a_value as $value) {
              $a_or[] = $name . ' == "^' . addslashes($value) . '"';
            }
            break;

          case 'contains':
            foreach ($a_value as $value) {
              $a_or[] = $name . ' = "' . addslashes($value) . '"';
            }
            break;
        }
        break;

      case mediamosa_type::TYPE_SEARCH_INT:
        foreach ($a_value as $value) {
          $a_or[] = $name . ' == ' . intval($value);
        }
        break;

      case mediamosa_type::TYPE_SEARCH_BOOL:
        foreach ($a_value as $value) {
          $a_or[] = $name . ' == "^' . addslashes($value) . '^"';
        }
        break;

      case mediamosa_type::TYPE_SEARCH_DATETIME:
        foreach ($a_value as $value) {
          if (strpos($value, '|') === FALSE) {
            $value = mediamosa_type::cast_timestamp($value, $name, mediamosa_type::TYPE_SEARCH_DATETIME);
            $a_or[] = $name . ' == "' . addslashes($value) . '"';
          }
          else {
           list($v1, $v2) = explode('|', $value, 2);
           $v1 = mediamosa_type::cast_timestamp($v1, $name, mediamosa_type::TYPE_SEARCH_DATETIME);
           $v2 = mediamosa_type::cast_timestamp($v2, $name, mediamosa_type::TYPE_SEARCH_DATETIME);
           $a_or[] = $name . ' within "' . addslashes($v1) . ' ' . addslashes($v2) . '"';
          }
        }
        break;

      default:
        assert(0);
        break;
    }

    return (count($a_or) > 1 ? '(' : '') . implode(' OR ', $a_or) . (count($a_or) > 1 ? ')' : '');
  }

  /**
   * Start parsing the CQL for asset search.
   *
   * @param string $str_cql
   * @param array $a_app_ids
   */
  static public function parse_asset($str_cql, array $a_app_ids) {

    // Create the CQL object.
    $mediamosa_cql = new mediamosa_asset_cql_context($a_app_ids);
    return $mediamosa_cql->mediamosa_core_parse_cql($str_cql);
  }

  /**
   * Format check if CQL has been correctly formated
   * Will throw exception when verify has failed.
   *
   * @param string $str_cql
   * @return boolean
   */
  static public function verify_asset($str_cql, array $a_app_ids, &$error_text) {

    // Create the CQL object.
    $mediamosa_cql = new mediamosa_asset_cql_context($a_app_ids);
    $result = $mediamosa_cql->mediamosa_cql_verify($str_cql);
    if (!$result) {
      $error_text = $mediamosa_cql->mediamosa_cql_last_exception_get()->getMessage();
    }
    return $result;
  }

  /**
   * Start parsing the CQL for collection search.
   *
   * @param string $str_cql
   * @param array $a_app_ids
   */
  static public function parse_collection($str_cql, array $a_app_ids) {

    // Create the CQL object.
    $mediamosa_cql = new mediamosa_collection_cql_context($a_app_ids);
    return $mediamosa_cql->mediamosa_core_parse_cql($str_cql);
  }

  /**
   * Format check if CQL has been correctly formated
   * Will throw exception when verify has failed.
   *
   * @param string $str_cql
   * @return boolean
   */
  static public function verify_collection($str_cql, $a_app_ids, &$error_text) {

    // Create the CQL object.
    $mediamosa_cql = new mediamosa_collection_cql_context($a_app_ids);
    $result = $mediamosa_cql->mediamosa_cql_verify($str_cql);
    if (!$result) {
      $error_text = $mediamosa_cql->mediamosa_cql_last_exception_get()->getMessage();
    }

    return $result;
  }

  /**
   * Add search parameters to var setup based on old param conf.

   * @param array $a_var_setup
   * @param array $a_search
   */
  static public function get_var_setup_search(array $a_var_setup, array $a_search) {

    foreach ($a_search as $param => $type) {
      $a_var_setup[mediamosa_rest_call::VARS][$param] = array(
        mediamosa_rest_call::VAR_TYPE => $type,
        mediamosa_rest_call::VAR_DESCRIPTION => 'Search parameter, try to use CQL instead.',
        mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      );

      $a_var_setup[mediamosa_rest_call::VARS][$param . '_match'] = array(
        mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_SEARCH_MATCH,
        mediamosa_rest_call::VAR_DESCRIPTION => 'Type matching for search parameter, try to use CQL instead.',
        mediamosa_rest_call::VAR_DEFAULT_VALUE => 'contains',
        mediamosa_rest_call::VAR_ALLOWED_VALUES => array('contains', 'exact', 'begin'),
      );
    }

    return $a_var_setup;
  }

  /**
   * Fill the a_var_setup with allowed parameters for search.
   * @param array $a_var_setup
   */
  static public function get_var_setup_search_asset(array $a_app_ids, array $a_var_setup, array $a_search) {

    // Enrich with normal search.
    $a_var_setup = self::get_var_setup_search($a_var_setup, $a_search);

    // Now add custom allowed metadata and own.
    $a_metadata_properties = mediamosa_asset_metadata_property::get_metadata_properties_full($a_app_ids);

    foreach ($a_metadata_properties as $param => $a_metadata_property) {
      switch ($a_metadata_property['propdef_type']) {
        case mediamosa_asset_metadata_property_db::TYPE_CHAR:
          $type = mediamosa_type::TYPE_SEARCH_STRING;
          break;

        case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
          $type = mediamosa_type::TYPE_SEARCH_DATETIME;
          break;

        case mediamosa_asset_metadata_property_db::TYPE_INT:
          $type = mediamosa_type::TYPE_SEARCH_INT;
          break;
      }

      $a_var_setup[mediamosa_rest_call::VARS][$param] = array(
        mediamosa_rest_call::VAR_TYPE => $type,
        mediamosa_rest_call::VAR_DESCRIPTION => sprintf('Metadata (%s) search parameter, try to use CQL instead.', $a_metadata_property['propgroup_name']),
        mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      );

      $a_var_setup[mediamosa_rest_call::VARS][$param . '_match'] = array(
        mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_SEARCH_MATCH,
        mediamosa_rest_call::VAR_DESCRIPTION => 'Type matching for search parameter, try to use CQL instead.',
        mediamosa_rest_call::VAR_DEFAULT_VALUE => 'contains',
        mediamosa_rest_call::VAR_ALLOWED_VALUES => array('contains', 'exact', 'begin'),
      );
    }

    return $a_var_setup;
  }
}

/**
 * @file
 * This file contains non-drupal functions and classes for VPX-CQL
 */

abstract class mediamosa_core_cql_context extends mediamosa_cql_context {

  abstract protected function mediamosa_core_cql_context_array_get();

  protected $m_a_app_ids;
  public static $search_keywords;

  /**
   * Constructor
   */
  final public function __construct($a_app_ids) {
    $this->m_a_app_ids = $a_app_ids;
    $this->m_o_prefix_assignment = new mediamosa_cql_prefix_assignment('mediamosa', 'info:srw/cql-context-set/1/,mediamosa-v1.1');
    $this->search_keywords = array();
  }

  final public function mediamosa_core_parse_cql($str_cql) {

    if ($this->mediamosa_cql_parse($str_cql) === FALSE) {
      $o_cql_exception = $this->mediamosa_cql_last_exception_get();
      throw new mediamosa_exception_error_cql_error(array('@error' => $o_cql_exception->getMessage()));
    }

    return $this->mediamosa_core_cql_context_cql2sql();
  }

  // Override... do not support prox.
  public function mediamosa_cql_context_is_boolean($str) {
    $a_booleans = array('and', 'or', 'not');
    return (array_search(strtolower($str), $a_booleans) === FALSE) ? FALSE : TRUE;
  }

  /**
   * return the context prefix assignment object
   *
   * @return prefix assignment object
   */
  final public function mediamosa_cql_context_prefix_assignment_get() {
    return $this->m_o_prefix_assignment;
  }

  /**
   * Create new index object.
   *
   * @param string $str_index
   * @return object
   */
  protected function mediamosa_cql_new_part_index_obj($str_index) {
    return new mediamosa_core_cql_part_index($str_index);
  }

  /**
   * Create new relation object.
   *
   * @param string $str_relation
   * @return object
   */
  protected function mediamosa_cql_new_part_relation_obj($str_relation) {
    return new mediamosa_core_cql_part_relation($str_relation);
  }

  /**
   * Check if given parameter is one of our named comparitor (a relation like 'adj', 'all' etc)
   *
   */
  protected function mediamosa_cql_context_is_named_comparitor($str) {
    $a_comparitors = array('adj', 'all', 'any', 'within');
    return (in_array($str, $a_comparitors) ? TRUE : FALSE);
  }

  /**
   * DB Escape a like string correctly
   *
   * @param string $str
   * @return string
   *
   * Escaped CQL to Escaped SQL;
   *
   * '\\' = '\' = '\\'
   * '\%' = '%' = '\%'
   * '\_' = '_' = '\_'
   * '\'' = ''' = '\''
   * '\"' = '"' = '\"'
   * '\' + any other char = any other char
   * '*' = '%' = '%'
   * '?' = '_' = '_'
   * '%' = '\%' = '\%'
   * '_' = '\_' = '\_'
   * any char = any char
   */
  public function mediamosa_cql_context_sql_escape_like($str) {
    $str = (string)$str;// Fix when passing integer.

    $a_result = array();
    for ($x = 0; $x < mediamosa_unicode::strlen($str); $x++) {
      $char = mediamosa_unicode::substr($str, $x, 1);

      // Is next char escaped?
      if ($char == '\\' && ($x + 1) < mediamosa_unicode::strlen($str)) {
        $char = drupal_substr($str, ++$x, 1);

        switch ($char) {
          case '\\':
          case '%':
          case '_':
          case '\'':
            $a_result[] = '\\' . $char;
            break;
          default:
            $a_result[] = mediamosa_db::escape_string($char);
            break;
        }
      }
      else {
        // Not escaped.
        $a_result[] = str_replace(array('*', '?'), array('%', '_'), mediamosa_db::escape_string_like($char));
      }
    }

    return implode('', $a_result);
  }

  /**
   * DB escape string
   *
   * @param string $str
   * @return string
   */
  protected function mediamosa_cql_context_sql_escape($str) {
    return mediamosa_db::escape_string($str);
  }

  /**
   * Save search keywords
   *
   * @param array $keywords
   */
  private function mediamosa_save_search($keywords) {
    // @todo:
    //_vpx_statistics_log_cql_search_query($keywords, $this->m_a_app_ids);
  }

  /**
   * converts the cql array into sql
   *
   */
  protected function mediamosa_core_cql_context_cql2sql() {

    $o_part_group = $this->mediamosa_cql_part_group_get();
    if ($o_part_group === FALSE) {
      return;
    }

    $a_result_parse = $a_result_cql2sql = array();

    $a_where = $o_part_group->mediamosa_cql_part_parse($this, 'mediamosa_core_cql_hook_cql2sql_where', $a_result_parse);
    $str_sql_where = $this->mediamosa_cql_parseresult_2_sql($a_where);
    $a_having = $o_part_group->mediamosa_cql_part_parse($this, 'mediamosa_core_cql_hook_cql2sql_having', $a_result_parse);
    $str_sql_having = $this->mediamosa_cql_parseresult_2_sql($a_having);

    // Get the keywords fron CQL and put them in a seperate array
    $_keywords = $this->search_keywords;
    $keywords = array();
    foreach ($_keywords as $key => $value) {
      $keywords[] = $_keywords[$key]->mediamosa_cql_part_str_get();
    }

    $this->mediamosa_save_search($keywords);

    if (drupal_strlen($str_sql_where) > 1) {
      // The where
      $a_result_cql2sql['str_where'] = $str_sql_where;
    }

    if (drupal_strlen($str_sql_having) > 1) {
      // The having
      $a_result_cql2sql['str_having'] = $str_sql_having;
    }

    if (drupal_strlen($str_sql_where) > 1 || drupal_strlen($str_sql_having) > 1) {
      // The joins
      $a_result_cql2sql['a_joins'] = (isset($a_result_parse['a_joins']) ? $a_result_parse['a_joins'] : array());

      // Extra select expr.
      $a_result_cql2sql['a_select_expr'] = (isset($a_result_parse['a_select_expr']) ? $a_result_parse['a_select_expr'] : array());
    }

    $o_sortby = $this->mediamosa_cql_sortby_get();
    $a_result_cql2sql['a_order_by'] = array();

    if ($o_sortby) {
      $a_result_cql2sql['a_order_by'] = $o_sortby->mediamosa_cql_sub_sortby_parse($this, 'mediamosa_core_cql_hook_cql2sql_orderby');
    }

    return $a_result_cql2sql;
  }

  /**
   * Having hook
   *
   * @param cql_part $o_cql_part
   * @param array $a_result
   * @return unknown
   */
  final public function mediamosa_core_cql_hook_cql2sql_having(mediamosa_cql_part $o_cql_part, array &$a_result, $is_append) {
    return $this->mediamosa_core_cql_hook_cql2sql($o_cql_part, $a_result, $is_append, TRUE);
  }

  /**
   * Where hook
   *
   * @param cql_part $o_cql_part
   * @param array $a_result
   * @return unknown
   */
  final public function mediamosa_core_cql_hook_cql2sql_where(mediamosa_cql_part $o_cql_part, array &$a_result, $is_append) {
    return $this->mediamosa_core_cql_hook_cql2sql($o_cql_part, $a_result, $is_append, FALSE);
  }

  private function mediamosa_core_cql_add_keyword($keyword) {

    if (!in_array($keyword, $this->search_keywords) && $keyword !== FALSE) {
      $this->search_keywords[] = $keyword;
    }
  }
  /**
   * Hook function that translates cql to sql (where/having part)
   *
   * @param cql_part $o_cql_part
   * @return string to use as sql
   */
  final public function mediamosa_core_cql_hook_cql2sql(mediamosa_cql_part $o_cql_part, array &$a_result, $is_append, $do_having) {
    $o_cql_part_search_term = $o_cql_part->mediamosa_cql_part_search_term_get();

    $this->mediamosa_core_cql_add_keyword($o_cql_part_search_term);

    $o_cql_part_boolean = $o_cql_part->mediamosa_cql_part_boolean_get();
    $a_mediamosa_core_cql_context = $this->mediamosa_core_cql_context_array_get();

    if ($o_cql_part_search_term === FALSE) {

      if ($o_cql_part_boolean) {
        if (!$is_append && $o_cql_part_boolean->mediamosa_cql_part_type_get() == MEDIAMOSA_CQL_BOOLEAN_NOT) {
          return $o_cql_part_boolean->mediamosa_cql_part_str_get();
        }

        if ($is_append && $o_cql_part_boolean->mediamosa_cql_part_type_get() != MEDIAMOSA_CQL_BOOLEAN_NOT) {
          return $o_cql_part_boolean->mediamosa_cql_part_str_get();
        }
      }

      return ''; // if its FALSE, its an empty container
    }

    if (!$is_append) {
      return ''; // only when appending...
    }

    $o_cql_part_relation = $o_cql_part->mediamosa_cql_part_relation_get();
    $a_parts = array();

    if (!$o_cql_part_relation) {
      assert(isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_RELATION]));
      assert(isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_SEARCH_TERMS]));

      $o_part_relation = new mediamosa_cql_part_relation($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_RELATION]);

      foreach ($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_SEARCH_TERMS] as $str_index) {
        $str_part = $this->mediamosa_core_cql_context_index2column_finalize($o_cql_part, $o_part_relation, $a_result, $str_index, $do_having);
        if (trim($str_part) != '') {
           $a_parts[] = $str_part;
        }
      }

      return implode(' OR ', $a_parts);
    }

    $o_cql_part_index = $o_cql_part->mediamosa_cql_part_index_get();
    $str_index = $o_cql_part_index->mediamosa_cql_part_str_get();
    return $this->mediamosa_core_cql_context_index2column_finalize($o_cql_part, $o_cql_part_relation, $a_result, $str_index, $do_having);
  }

  /**
   * Finalize the relation, index, search term
   *
   * @param array $a_parts
   * @param cql_part $o_cql_part
   * @param cql_part_relation $o_part_relation
   * @param array $a_result
   * @param string $str_index
   * @return array
   */
  final public function mediamosa_core_cql_context_index2column_finalize(mediamosa_cql_part $o_cql_part, mediamosa_cql_part_relation $o_cql_part_relation, array &$a_result, $str_index, $do_having) {
    $o_cql_part_search_term = $o_cql_part->mediamosa_cql_part_search_term_get();
    $o_cql_part_boolean = $o_cql_part->mediamosa_cql_part_boolean_get();

    $pos = strrpos($str_index, '.');
    if ($pos !== FALSE) {
      $str_index = mediamosa_unicode::substr($str_index, $pos + 1); // default context set id
    }

    $a_relation_modifiers = $o_cql_part_relation->mediamosa_cql_modifier_get_array();

    $a_index2column = $this->mediamosa_core_cql_context_index2column($str_index);
    assert($a_index2column);// use verify 1st so this will not happen

    if ($a_index2column === FALSE) {
      throw new mediamosa_cql_exception('index not found, use verify before you parse the cql string.', CQL_EXCEPTION_SEVERITY_HIGH);
    }

    if (isset($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING]) && $a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING] && !$do_having) {
      return '';
    }

    if (!isset($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING])) {
      $a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING] = FALSE;
    }

    if (isset($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING]) && !$a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING] && $do_having) {
      return '';
    }

    $str_table_column = ($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS] != '' ? $a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS] . '.' : '') . $a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN];

    if (isset($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN])) {
      $a_result['a_joins'][$str_index] = $a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN];
    }

    if (isset($a_index2column['select_expr'])) {
      $a_result['a_select_expr'][$str_index] = $a_index2column['select_expr'];
    }

    $str_search_term = $o_cql_part_search_term->mediamosa_cql_part_str_get();

    $a_parts_or = $a_parts = $a_sub_parts = array();
    $str_sub_parts_glue = ' OR ';
    $str_like = 'LIKE';
    $str_compare = '=';
    $str_binary = '';
    $has_not = $o_cql_part->mediamosa_cql_part_has_not();

    $str_relation = $o_cql_part_relation->mediamosa_cql_part_str_get();
    $has_null = isset($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL]) ? $a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL] : FALSE;
    $do_null = FALSE;

    switch ($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
      case mediamosa_type::TYPE_INT:
        $str_quotes = '';
        if ($str_relation != 'within') {
          $str_search_term = (int)$str_search_term; // make sure its an integer.... :)
        }
        break;
      case mediamosa_type::TYPE_DATETIME:
        if ($str_relation != 'within') {
          // Now into DB
          $str_search_term = $this->mediamosa_cql_str_date_2_db_date($str_search_term);

          if ($str_relation == '=' || $str_relation == 'any') {
            $str_relation = '==';
          }
          $str_quotes = '\'';
        }
        break;
      case mediamosa_type::TYPE_DOMAIN:
      case mediamosa_type::TYPE_REALM:
      case mediamosa_type::TYPE_STRING:
        if (in_array('respectcase', $a_relation_modifiers)) {
          $str_binary = 'BINARY ';
        }
        $str_quotes = '\'';
        if (mediamosa_unicode::substr($str_search_term, 0, 1) !== '*' && mediamosa_unicode::substr($str_search_term, 0, 1) !== '^') {
          $str_search_term = '*' . $str_search_term;
        }

        if (drupal_strlen($str_search_term) > 1) {
          if (mediamosa_unicode::substr($str_search_term, -1) != '*' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_search_term), -1) != '\*' && mediamosa_unicode::substr($str_search_term, -1) != '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_search_term), -2) != '\^') {
            $str_search_term .= '*';
          }
        }

        break;
      case mediamosa_type::TYPE_BOOL:
        $str_search_term = (mediamosa_unicode::strtolower($str_search_term) == "true" ? "TRUE" : "FALSE");
        $str_quotes = '\'';
        $str_like = '=';
        $str_relation = '==';
        break;
      default:
        $str_quotes = '\'';
        break;
    }

    $ancor_right = $ancor_left = '';

    switch ($str_relation) {
      case 'all':
        $str_sub_parts_glue = ' AND ';
      case '=':
      case 'any':
        $a_words = explode(' ', $str_search_term);

        foreach ($a_words as $str_word) {
          if (mediamosa_unicode::substr($str_word, 0, 1) == '^' && mediamosa_unicode::substr($str_word, -1) == '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_word), -2) != '\^') {
            $str_search_term = mediamosa_unicode::substr($str_search_term, 1, -1);
          }

          if (trim($str_word) == '') {
            continue;
          }

          $ancor_right = $ancor_left = '%';

          if (mediamosa_unicode::substr($str_word, 0, 1) == '^') {
            $str_word = mediamosa_unicode::substr($str_word, 1);
            $ancor_left = '';
          }

          if (mediamosa_unicode::substr($str_word, -1) == '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_word), -2) != '\^') {
            $str_word = mediamosa_unicode::substr($str_word, 0, -1);
            $ancor_right = '';
          }

          $a_sub_parts[] = sprintf("%s%s LIKE %s%s%s%s%s", $str_binary, $str_table_column, $str_quotes, $ancor_left, $this->mediamosa_cql_context_sql_escape_like($str_word), $ancor_right, $str_quotes);
        }
        break;
      case '<>':
        $str_like = 'NOT LIKE';
        $str_compare = '<>';
        $do_null = TRUE;
      case '==':
      case 'adj':
        if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^' && mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_search_term), -2) != '\^') {
          $str_search_term = mediamosa_unicode::substr($str_search_term, 1, -1);
        }

        if (trim($str_search_term) == '') {
          continue;
        }

        if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^') {
          $a_sub_parts[] = sprintf("%s%s %s %s%s%%%s", $str_binary, $str_table_column, $str_like, $str_quotes, $this->mediamosa_cql_context_sql_escape_like(mediamosa_unicode::substr($str_search_term, 1)), $str_quotes);
        }
        elseif (mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_search_term), -2) != '\^') {
          $a_sub_parts[] = sprintf("%s%s %s %s%%%s%s", $str_binary, $str_table_column, $str_like, $str_quotes, $this->mediamosa_cql_context_sql_escape_like(mediamosa_unicode::substr($str_search_term, 0, -1)), $str_quotes);
        }
        else {
          if ($o_cql_part_search_term->mediamosa_cql_contains_wildcards($str_search_term)) {
            $ancor_right = $ancor_left = '%';
            $str_compare = $str_like;
          }

          $a_sub_parts[] = sprintf("%s%s %s %s%s%s%s%s", $str_binary, $str_table_column, $str_compare, $str_quotes, $ancor_left, $this->mediamosa_cql_context_sql_escape_like($str_search_term), $ancor_right, $str_quotes);
        }
        break;
      case '>':
        $a_sub_parts[] = sprintf("%s%s > %s%s%s", $str_binary, $str_table_column, $str_quotes, $this->mediamosa_cql_context_sql_escape_like($str_search_term), $str_quotes);
        break;
      case '>=':
        $a_sub_parts[] = sprintf("%s%s >= %s%s%s", $str_binary, $str_table_column, $str_quotes, $this->mediamosa_cql_context_sql_escape_like($str_search_term), $str_quotes);
        break;
      case '<':
        $a_sub_parts[] = sprintf("%s%s < %s%s%s", $str_binary, $str_table_column, $str_quotes, $this->mediamosa_cql_context_sql_escape_like($str_search_term), $str_quotes);
        break;
      case '<=':
        $a_sub_parts[] = sprintf("%s%s <= %s%s%s", $str_binary, $str_table_column, $str_quotes, $this->mediamosa_cql_context_sql_escape_like($str_search_term), $str_quotes);
        break;
      case 'within':
        $a_within = explode(' ', $str_search_term);
        $str_sub_parts_glue = ' AND ';

        switch ($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_type::TYPE_INT:
            $a_sub_parts[] = sprintf("%s >= %s", $str_table_column, intval($a_within[0]));
            $a_sub_parts[] = sprintf("%s <= %s", $str_table_column, intval($a_within[1]));
            break;
          case mediamosa_type::TYPE_DATETIME:
            $a_sub_parts[] = sprintf("%s >= '%s'", $str_table_column, $this->mediamosa_cql_context_sql_escape($this->mediamosa_cql_str_date_2_db_date($a_within[0])));
            $a_sub_parts[] = sprintf("%s <= '%s'", $str_table_column, $this->mediamosa_cql_context_sql_escape($this->mediamosa_cql_str_date_2_db_date($a_within[1])));
            break;
          default:
            $a_sub_parts[] = sprintf("%s%s >= '%s'", $str_binary, $str_table_column, $this->mediamosa_cql_context_sql_escape($a_within[0]));
            $a_sub_parts[] = sprintf("%s%s <= '%s'", $str_binary, $str_table_column, $this->mediamosa_cql_context_sql_escape($a_within[1]));
            break;
        }
    }

    $a_parts[] = ( (isset($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_PREFIX]) ? $a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_PREFIX] : '') . (count($a_sub_parts) ? '(' : '') . implode($str_sub_parts_glue, $a_sub_parts) . (count($a_sub_parts) ? ')' : '') . (isset($a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_SUFFIX]) ? $a_index2column[MEDIAMOSA_CQL_CONTEXT_KEY_SUFFIX] : ''));

    if ($o_cql_part_boolean) {
      $str_boolean = $o_cql_part_boolean->mediamosa_cql_part_str_get();
      $a_parts[] = ($str_boolean == 'NOT' ? 'AND ' : '') . $str_boolean;
    }

    $a_parts_or[] = (($has_not && count($a_parts) ? 'NOT ' : '') . implode(' ', $a_parts));

    if ( ($do_null || $has_not) && $has_null) {
      $a_parts_or[] = sprintf('%s IS NULL', $str_table_column);
    }

    return count($a_parts_or) > 1 ? '(' . implode(' OR ', $a_parts_or) . ')' : reset($a_parts_or);
  }

  /**
   * Translate the index text into a valid column.
   * Is used lower to check valid index
   *
   * @param cql_part $o_cql_part
   * @return array
   */
  final public function mediamosa_cql_context_index2column(mediamosa_cql_part $o_cql_part) {
    $o_cql_part_index = $o_cql_part->mediamosa_cql_part_index_get();
    $str_index = $o_cql_part_index->mediamosa_cql_part_str_get();
    return $this->mediamosa_core_cql_context_index2column($str_index);
  }

  /**
   * Translate the index text into a valid column.
   *
   * @param cql_part $o_cql_part
   * @return array
   */
  final public function mediamosa_core_cql_context_index2column($str_index) {
    assert($str_index != '');
    $a_mediamosa_core_cql_context = $this->mediamosa_core_cql_context_array_get();

    if (strpos($str_index, '.') === FALSE) {
      $str_index = 'mediamosa.' . $str_index; // default context set id
    }

    list($context_set, $str_index) = explode('.', $str_index, 2);
    return $this->mediamosa_core_cql_context_index2column_wrapper($a_mediamosa_core_cql_context, $context_set, $str_index);
  }

  /**
   * Simple wrapper for recursive search in the context array
   *
   * @param array $a_mediamosa_core_cql_context
   * @param string $context_set
   * @param string $str_index
   * @return array
   */
  final protected function mediamosa_core_cql_context_index2column_wrapper($a_mediamosa_core_cql_context, $context_set, $str_index) {

    // Check of een index reference bestaat (alleen in root toegestaan)
    if (isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEX_REFERENCES][$context_set])) {
      $context_set_reference = $a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEX_REFERENCES][$context_set];
      assert(!isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEX_REFERENCES][$context_set_reference]));
      if (isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEX_REFERENCES][$context_set_reference])) {
        throw new mediamosa_cql_exception('program error; index reference loop!', CQL_EXCEPTION_SEVERITY_HIGH);
      }

      return $this->mediamosa_core_cql_context_index2column_wrapper($a_mediamosa_core_cql_context, $context_set_reference, $str_index);
    }

    // Vanwege alleen in root mag voorkomen van CQL_CONTEXT_KEY_INDEX_REFERENCES....
    return $this->mediamosa_core_cql_context_index2column_wrapper_2($a_mediamosa_core_cql_context, $context_set, $str_index);
  }

  /**
   * Minor helper for mediamosa_core_cql_context_index2column_wrapper
   *
   * @param array $a_mediamosa_core_cql_context
   * @param string $context_set
   * @param string $str_index
   * @return array
   */
  final private function mediamosa_core_cql_context_index2column_wrapper_2($a_mediamosa_core_cql_context, $context_set, $str_index) {

    // Check voor context set
    if (isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set])) {
      // Context set gevonden.

      // Nu in deze boom moet de index -> column array gevonden worden, anders FALSE
      return $this->mediamosa_core_cql_context_index2column_find_column($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set], $str_index);
    }

    if (isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES])) {

      foreach ($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES] as $a_mediamosa_core_cql_context_2) {
        return $this->mediamosa_core_cql_context_index2column_wrapper($a_mediamosa_core_cql_context_2, $context_set, $str_index);
      }
    }

    return FALSE;
  }

  /**
   * Walks through the final array to find the index
   *
   * @param array $a_mediamosa_core_cql_context
   * @param string $str_index
   * @return array
   */
  final protected function mediamosa_core_cql_context_index2column_find_column($a_mediamosa_core_cql_context, $str_index) {

    // Find column def.
    if (isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS]) && isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$str_index])) {
      return $a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$str_index];
    }

    if (isset($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES])) {
      foreach ($a_mediamosa_core_cql_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES] as $sub_context_set => $a_mediamosa_core_cql_context_2) {
        $mixed_result = $this->mediamosa_core_cql_context_index2column_find_column($a_mediamosa_core_cql_context_2, $str_index);
        if ($mixed_result !== FALSE) {
          return $mixed_result;
        }
      }
    }

    return FALSE;
  }

  /**
   * Hook function that translates cql to sql (order by part)
   *
   * @param cql_part $o_cql_part
   * @return string to use as sql
   */
  final public function mediamosa_core_cql_hook_cql2sql_orderby(mediamosa_cql_sub_sortby $o_cql_sub_sortby) {

    $str_direction = 'ASC';

    $a_modifiers = $o_cql_sub_sortby->mediamosa_cql_modifier_get_array();
    foreach ($a_modifiers as $str_modifier) {
      switch ($str_modifier) {
        case 'sort.descending':
        case 'descending':
          $str_direction = 'DESC';
          break;
        case 'sort.ascending':
        case 'ascending':
        default:
          $str_direction = 'ASC';
          break;
      }
    }

    // Get the order by column.
    $order_by = $o_cql_sub_sortby->mediamosa_cql_sort_by_get();

    // Translate to cql column.
    $a_index = $this->mediamosa_core_cql_context_index2column($order_by);

    // Test we can sort on this column.
    if (!isset($a_index[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT])) {
      throw new mediamosa_exception_error_sort_field_error(array('@field' => $order_by));
    }

    return array(
      MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => $a_index['column'] == 'val_char' ? 'val_char_lft' : $a_index['column'],
      MEDIAMOSA_CQL_CONTEXT_KEY_DIRECTION => $str_direction,
      MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => $a_index[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT],
      MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID => isset($a_index[MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID]) ? $a_index[MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID] : 0,
    );
  }
}
/**
 * Asset CQL Context class
 *
 */
class mediamosa_asset_cql_context extends mediamosa_core_cql_context {

  /**
   * returns the array containing mapping voor cql2sql
   *
   * @return array
   */
  protected function mediamosa_core_cql_context_array_get() {
    static $a_mediamosa_context = NULL;

    if (!isset($a_mediamosa_context)) {
      $a_mediamosa_context = $this->m_a_mediamosa_context;

      $a_defs = mediamosa_asset_metadata_property::get_metadata_properties_full($this->m_a_app_ids);

      foreach ($a_defs as $name => $a_def) {
        switch ($a_def['propgroup_name']) {
          case 'qualified_dublin_core':
            $context_set = 'mediamosa:qdc';
            break;
          case 'dublin_core':
            $context_set = 'mediamosa:dc';
            break;
          case 'czp':
            $context_set = 'mediamosa:czp';
            break;
          case 'acl':
          case 'aut':
            $context_set = 'mediamosa:aut';
            break;
          default:
            $context_set = 'mediamosa:' . $a_def['propgroup_name'];
            break;
        }

        switch ($a_def['propdef_type']) {
          case mediamosa_asset_metadata_property_db::TYPE_INT:
            $column = 'val_int';
            $type = mediamosa_type::TYPE_INT;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            $column = 'val_datetime';
            $type = mediamosa_type::TYPE_DATETIME;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
          default:
            assert(0);
          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            $column = 'val_char';
            $type = mediamosa_type::TYPE_STRING;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
        }

        $a_mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$name] = array(
          MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => $column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'mediamosa_asset_metadata_' . $name,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => $table,
          MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => $type,
          MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID => $a_def['propdef_id'],
          MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => sprintf('LEFT JOIN {mediamosa_asset_metadata} AS mediamosa_asset_metadata_%s ON mediamosa_asset_metadata_%s.prop_id = %d AND mediamosa_asset_metadata_%s.asset_id = a.asset_id', $name, $name, $a_def['propdef_id'], $name),
//          MEDIAMOSA_CQL_CONTEXT_KEY_PREFIX => sprintf('(v_asset_property_%s.prop_id = %d AND ', $name, $a_def['propdef_id']),
//          MEDIAMOSA_CQL_CONTEXT_KEY_SUFFIX => ')',
        );
      }
    }

    return $a_mediamosa_context;
  }

  // The CQL_CONTEXT_KEY_* keys are default and must be used,
  // however all other keys are open for own class specification.
  private $m_a_mediamosa_context = array(

    MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_RELATION => '=',
    MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_SEARCH_TERMS => array('mediamosa.title'),

    MEDIAMOSA_CQL_CONTEXT_KEY_INDEX_REFERENCES => array(
     'dc' => 'mediamosa:dc', // Reference
     'qdc' => 'mediamosa:qdc', // Reference
     'czp' => 'mediamosa:czp', // Reference
     'acl' => 'mediamosa:acl', // Reference (1.x)
     'aut' => 'mediamosa:aut', // Reference (1.x)
     'vpx:acl' => 'mediamosa:acl', // Reference (1.x)
     'vpx:aut' => 'mediamosa:aut', // Reference (1.x)
     'vpx:dc' => 'mediamosa:dc', // Reference (1.x)
     'vpx:qdc' => 'mediamosa:qdc', // Reference (1.x)
     'vpx:czp' => 'mediamosa:czp', // Reference (1.x)
     'vpx' => 'mediamosa', // Reference (1.x)
    ),

    MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES => array(
      'mediamosa' => array(
        MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES => array(
          'mediamosa:asset' => array(
            MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS=> array(
              'asset_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'asset_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'group_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'group_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'owner_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'owner_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'batch_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'batch_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_ftp_batch_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'ba',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'FTP_BATCH',
              ),
              'provider_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'provider_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'reference_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'reference_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'mediafile_duration' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'mediafile_duration',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'mediafile_container_type' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'mediafile_container_type',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'videotimestamp' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'videotimestamp',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_DATETIME,
              ),
              'videotimestampmodified' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'videotimestampmodified',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_DATETIME,
              ),
              'mime_type' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'mime_type',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'mm_mime_type',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'MEDIAFILE_METADATA_MIMETYPE',// Trick to include the join of this table.
              ),
              'filename' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'filename',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_mediafile_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'mf',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'MEDIAFILE_FILENAME',// Trick to include the join of this table.
              ),
              'mediafile_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'mediafile_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_mediafile_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'mf',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'MEDIAFILE_FILENAME',// Trick to include the join of this table.
              ),
              'numofviews' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'viewed',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_INT,
              ),
              'is_unappropriate' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'is_unappropriate',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_BOOL,
              ),
              'uri' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'uri',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_mediafile_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'mf',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'MEDIAFILE_FILENAME',// Trick to include the join of this table.
              ),
              'numofplays' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'played',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_INT,
              ),
              'app_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'app_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_INT,
              ),
              mediamosa_asset_db::IS_EMPTY_ASSET => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => mediamosa_asset_db::IS_EMPTY_ASSET,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'a',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_BOOL,
              ),
              mediamosa_asset_collection_db::COLL_ID => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => mediamosa_asset_collection_db::COLL_ID,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_asset_mediafile_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'asset_coll',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'ASSET_COLLECTION',// Trick to include the join of this table.
              ),
            ),
          ),
          'mediamosa:aut' => array(
            MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS=> array(
              'aut_app' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'app_id_master',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'aut_ms',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_INT,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'AUT_APP_MASTER_SLAVE',// Trick to include the join of this table.
              ),
              'is_protected' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'is_protected',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'mf',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_BOOL,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'MEDIAFILE_FILENAME',// Trick to include the join of this table.
              ),
              'aut_user' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'aut_name',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'aut_u',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'AUT_NAME_USER',// Trick to include the join of this table.
                MEDIAMOSA_CQL_CONTEXT_KEY_NULL => TRUE,
              ),
              'aut_group' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'aut_name',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'aut_ug',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'AUT_NAME_USER_GROUP',// Trick to include the join of this table.
                MEDIAMOSA_CQL_CONTEXT_KEY_NULL => TRUE,
              ),
              'aut_domain' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'aut_name',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'aut_gd',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_DOMAIN,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'AUT_GROUP_DOMAIN',// Trick to include the join of this table.
                MEDIAMOSA_CQL_CONTEXT_KEY_NULL => TRUE,
              ),
              'aut_realm' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'aut_name',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'aut_gr',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_REALM,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'AUT_GROUP_REALM',// Trick to include the join of this table.
                MEDIAMOSA_CQL_CONTEXT_KEY_NULL => TRUE,
              ),
              'aut_realm_prefix' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'aut_prefix',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'aut_gr',
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
                MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => 'AUT_GROUP_REALM',// Trick to include the join of this table.
                MEDIAMOSA_CQL_CONTEXT_KEY_NULL => TRUE,
              ),
            ),
          ),
        ),
      ),
    ),
  );
}

/**
 * Collection CQL Context class
 *
 */
class mediamosa_collection_cql_context extends mediamosa_core_cql_context {

  // The CQL_CONTEXT_KEY_* keys are default and must be used,
  // however all other keys are open for own class specification.
  private $m_a_mediamosa_context = array(

    MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_RELATION => '=',
    MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_SEARCH_TERMS => array('mediamosa.title'),

    MEDIAMOSA_CQL_CONTEXT_KEY_INDEX_REFERENCES => array(
     'vpx' => 'mediamosa', // Reference (1.x)
     'vpx:collection' => 'mediamosa:collection', // Reference (1.x)
    ),

    MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES => array(
      'mediamosa' => array(
        MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES => array(
          'mediamosa:collection' => array(
            MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS=> array(
              'title' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'title',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'description' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'description',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'owner_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'owner_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'group_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'group_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'created' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'created',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_DATETIME,
              ),
              'changed' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'changed',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_DATETIME,
              ),
              'private' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'private',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_BOOL,
              ),
              'is_unappropriate' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'is_unappropriate',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_BOOL,
              ),
              'public' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'public',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_BOOL,
              ),
              'category' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'category',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_STRING,
              ),
              'numofvideos' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'numofvideos',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => '',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => FALSE,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_INT,
                MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING => TRUE,// use having instead of where
              ),
              'app_id_search' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'app_id_search',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_INT,
              ),
              'app_id' => array(
                MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => 'app_id',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'c',
                MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => mediamosa_collection_db::TABLE_NAME,
                MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => mediamosa_type::TYPE_INT,
              ),
            ),
          ),
        ),
      ),
    ),
  );

  protected function mediamosa_core_cql_context_array_get() {
    return $this->m_a_mediamosa_context;
  }
}

/**
 * Extending the relation part.
 *
 */
class mediamosa_core_cql_part_relation extends mediamosa_cql_part_relation {

  protected function cql_modifier_allowed_modifier($str) {
    return in_array($str, array()) ? TRUE : mediamosa_cql_part_relation::cql_modifier_allowed_modifier($str);
  }
}

/**
 * Extending for future
 *
 */
class mediamosa_core_cql_part_index extends mediamosa_cql_part_index {

  protected function cql_modifier_allowed_modifier($str) {
    return in_array($str, array()) ? TRUE : mediamosa_cql_part_index::cql_modifier_allowed_modifier($str);
  }
}
