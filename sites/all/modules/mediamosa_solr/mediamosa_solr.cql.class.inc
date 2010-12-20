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
 * Extended on the existing mediamosa_core_cql class.
 */

class mediamosa_solr_asset_cql_context extends mediamosa_asset_cql_context {
  /**
   * returns the array containing mapping voor cql2sql
   *
   * @return array
   */
  protected function mediamosa_core_cql_context_array_get() {
    static $mediamosa_context = NULL;

    if (!isset($mediamosa_context)) {
      $mediamosa_context = $this->m_a_mediamosa_context;

      foreach ($mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES] as $context_set => &$lvl1) {
        foreach ($lvl1[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS] as $column_name => $column) {
          if ($column_name == 'mime_type') {
            $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['column'] = 'mime_type';
          }

          $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['table_for_sort'] = $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['column'];
        }
      }

      $a_defs = mediamosa_asset_metadata_property::get_metadata_properties_full($this->app_ids);

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
            $column = $name . '_vi';
            $sort_column = $name . '_svc';
            $type = mediamosa_sdk::TYPE_INT;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            $column = $name . '_vd';
            $sort_column = $name . '_svc';
            $type = mediamosa_sdk::TYPE_DATETIME;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
          default:
            assert(0);
          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            $column = $name . '_vc';
            $sort_column = $name . '_svc';
            $type = mediamosa_sdk::TYPE_STRING;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
        }

        $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$name] = array(
          MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => $column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => $sort_column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'unused',
          MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => $type,
          MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID => $a_def['propdef_id'], // indication its a asset metadata field.
//          MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => sprintf('LEFT JOIN {mediamosa_asset_metadata} AS mediamosa_asset_metadata_%s ON mediamosa_asset_metadata_%s.asset_id = a.asset_id AND mediamosa_asset_metadata_%s.prop_id = %d', $name, $name, $name, $a_def['propdef_id']),
        );
      }
    }

    return $mediamosa_context;
  }

  /**
   * Finalize the relation, index, search term
   *
   * @param array $a_parts
   * @param cql_part $cql_part
   * @param cql_part_relation $o_part_relation
   * @param array $a_result
   * @param string $str_index
   *
   * @return array
   */
  public function mediamosa_core_cql_context_index2column_finalize(mediamosa_cql_part $cql_part, mediamosa_cql_part_relation $cql_part_relation, array &$a_result, $str_index, $do_having) {

    $cql_part_search_term = $cql_part->mediamosa_cql_part_search_term_get();
    $cql_part_boolean = $cql_part->mediamosa_cql_part_boolean_get();

    $pos = strrpos($str_index, '.');
    if ($pos !== FALSE) {
      $str_index = mediamosa_unicode::substr($str_index, $pos + 1); // default context set id
    }

    $a_relation_modifiers = $cql_part_relation->mediamosa_cql_modifier_get_array();

    $index2column = $this->mediamosa_core_cql_context_index2column($str_index);
    assert($index2column);// use verify 1st so this will not happen

    if ($index2column === FALSE) {
      throw new mediamosa_cql_exception('index not found, use verify before you parse the cql string.', CQL_EXCEPTION_SEVERITY_HIGH);
    }

    if (isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING]) && $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING] && !$do_having) {
      return '';
    }

    if (!isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING])) {
      $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING] = FALSE;
    }

    if (isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING]) && !$index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING] && $do_having) {
      return '';
    }

    $str_table_column = $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN];

    if (isset($index2column['select_expr'])) {
      $a_result['a_select_expr'][$str_index] = $index2column['select_expr'];
    }

    $str_search_term = $cql_part_search_term->mediamosa_cql_part_str_get();

// Export data.
mediamosa_debug::log_export($index2column);

    $parts_or = $parts = $sub_parts = array();

    // Default is OR.
    $str_sub_parts_glue = ' OR ';
    $str_relation = $cql_part_relation->mediamosa_cql_part_str_get();
    $has_not = $cql_part->mediamosa_cql_part_has_not();
mediamosa_debug::log($str_search_term);
mediamosa_debug::log($str_relation);
mediamosa_debug::log($has_not ? "has not" : 'no not');

    switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
      case mediamosa_sdk::TYPE_INT:
        switch ($str_relation) {
          case '>=':
            $str_search_term -= 1;
            break;

          case '<=':
            $str_search_term += 1;
            break;
        }
        break;

      case mediamosa_sdk::TYPE_DATETIME:
        $str = '0000-01-01T00:00:00Z';
        $str_search_term = $str_search_term . mediamosa_unicode::substr($str, mediamosa_unicode::strlen($str_search_term));

        switch ($str_relation) {
          case '>=':
            $str_search_term .= '-1MINUTES';
            break;

          case '<=':
            $str_search_term .= '+1MINUTES';
            break;
        }
        break;

      case mediamosa_sdk::TYPE_DOMAIN:
      case mediamosa_sdk::TYPE_REALM:
      case mediamosa_sdk::TYPE_STRING:
        break;

      case mediamosa_sdk::TYPE_BOOL:
        break;

      case mediamosa_sdk::TYPE_BOOL_IS_SLAVED:
        break;

      case mediamosa_sdk::TYPE_JOB_PROGRESS:
        throw new mediamosa_solr_exception('Type TYPE_JOB_PROGRESS is not supported by asset SOLR, job context.');
    }


    switch ($str_relation) {
      case 'all':
        $str_sub_parts_glue = ' AND ';

      case '=':
      case 'any':
        $words = explode(' ', $str_search_term);
        foreach ($words as $word) {
          if (trim($word) == '') {
            continue;
          }

          $sub_parts[] = $word;
        }

        $sub_parts = array($str_table_column . ':' . mediamosa_solr_apache_solr_service::phrase(implode(' ', $sub_parts)) );
        break;

      case '==':
      case 'adj':
        $sub_parts = array($str_table_column . ':' . mediamosa_solr_apache_solr_service::phrase($str_search_term));
        break;

      case '<>':
        $has_not = !$has_not;
        $sub_parts = array($str_table_column . ':"' . $str_search_term . '"');
        break;

      case '>':
        $sub_parts = array(strtr('@field:[@value TO *]', array('@field' => $str_table_column, '@value' => mediamosa_solr_apache_solr_service::escape($str_search_term))));
        break;
      case '>=':
        $sub_parts = array(strtr('@field:[@value TO *]', array('@field' => $str_table_column, '@value' => mediamosa_solr_apache_solr_service::escape($str_search_term))));
        break;
      case '<':
        $sub_parts = array(strtr('@field:[* TO @value]', array('@field' => $str_table_column, '@value' => mediamosa_solr_apache_solr_service::escape($str_search_term))));
        break;
      case '<=':
        $sub_parts = array(strtr('@field:[* TO @value]', array('@field' => $str_table_column, '@value' => mediamosa_solr_apache_solr_service::escape($str_search_term))));
        break;
      case 'within':
        // Split up.
        $values = explode(' ', $str_search_term);

        $sub_parts = array(strtr('@field:[@value_1 TO @value_2]', array('@field' => $str_table_column, '@value_1' => mediamosa_solr_apache_solr_service::escape($values[0]), '@value_2' => mediamosa_solr_apache_solr_service::escape($values[1]))));
        break;

    }
mediamosa_debug::log_export($sub_parts);
    // Into one.
    $parts[] = (count($sub_parts) < 2 ? '' : '(') . implode($str_sub_parts_glue, $sub_parts) . (count($sub_parts) < 2 ? '' : ')');
mediamosa_debug::log_export($parts);

    return ($has_not ? '-' : '') . (count($parts) < 2 ? '' : '(') . implode (' OR ', $parts) . (count($parts) < 2 ? '' : ')');

  //  if ($cql_part_boolean) {
 //     $str_boolean = $cql_part_boolean->mediamosa_cql_part_str_get();
 //     $a_parts[] = ($str_boolean == 'NOT' ? 'AND ' : '') . $str_boolean;
  //  }

    //$a_parts_or[] = (($has_not && count($a_parts) ? 'NOT ' : '') . implode(' ', $a_parts));

    //if ( ($do_null || $has_not) && $has_null) {
    //  $a_parts_or[] = sprintf('%s IS NULL', $str_table_column);
    //}

    return count($a_parts_or) > 1 ? '(' . implode(' OR ', $a_parts_or) . ')' : reset($a_parts_or);


    switch ($str_relation) {
      case 'all':
        $str_sub_parts_glue = ' AND ';

      case '=':
      case 'any':
        $a_words = explode(' ', $str_search_term);
        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_INT:
          case mediamosa_sdk::TYPE_UINT:
            // Rewrite to IN statement: $str_table_column IN ($a_sub_parts).
            $tmp = array();
            foreach ($a_words as $str_word) {
              $tmp[] = sprintf('%d', (int)$str_word);
            }

            if (count($tmp)  == 1) {
              $a_sub_parts[] = sprintf('%s = %s', $str_table_column, implode(',', $tmp));
            }
            else {
              $a_sub_parts[] = sprintf('%s IN(%s)', $str_table_column, implode(',', $tmp));
            }
            break;
          default:
            foreach ($a_words as $str_word) {
              if (trim($str_word) == '') {
                continue;
              }

              if (mediamosa_unicode::substr($str_word, 0, 1) == '^' && mediamosa_unicode::substr($str_word, -1) == '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_word), -2) != '\^') {
                $str_search_term = mediamosa_unicode::substr($str_search_term, 1, -1);
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

        // Fixing search on empty string.
        // TODO: needs testing.
        /*
        if (trim($str_search_term) == '') {
          continue;
        }
         */

        if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^') {
          $a_sub_parts[] = sprintf("%s%s %s %s%s%%%s", $str_binary, $str_table_column, $str_like, $str_quotes, $this->mediamosa_cql_context_sql_escape_like(mediamosa_unicode::substr($str_search_term, 1)), $str_quotes);
        }
        elseif (mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_search_term), -2) != '\^') {
          $a_sub_parts[] = sprintf("%s%s %s %s%%%s%s", $str_binary, $str_table_column, $str_like, $str_quotes, $this->mediamosa_cql_context_sql_escape_like(mediamosa_unicode::substr($str_search_term, 0, -1)), $str_quotes);
        }
        else {
          if ($cql_part_search_term->mediamosa_cql_contains_wildcards($str_search_term)) {
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

        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_INT:
            $a_sub_parts[] = sprintf("%s >= %s", $str_table_column, intval($a_within[0]));
            $a_sub_parts[] = sprintf("%s <= %s", $str_table_column, intval($a_within[1]));
            break;
          case mediamosa_sdk::TYPE_DATETIME:
            $a_sub_parts[] = sprintf("%s >= '%s'", $str_table_column, $this->mediamosa_cql_context_sql_escape($this->mediamosa_cql_str_date_2_db_date($a_within[0])));
            $a_sub_parts[] = sprintf("%s <= '%s'", $str_table_column, $this->mediamosa_cql_context_sql_escape($this->mediamosa_cql_str_date_2_db_date($a_within[1])));
            break;
          default:
            $a_sub_parts[] = sprintf("%s%s >= '%s'", $str_binary, $str_table_column, $this->mediamosa_cql_context_sql_escape($a_within[0]));
            $a_sub_parts[] = sprintf("%s%s <= '%s'", $str_binary, $str_table_column, $this->mediamosa_cql_context_sql_escape($a_within[1]));
            break;
        }
    }


    return;
/*
    $a_parts_or = $a_parts = $a_sub_parts = array();
    $str_sub_parts_glue = ' OR ';
    $str_like = 'LIKE';
    $str_compare = '=';
    $str_binary = '';
    $has_not = $cql_part->mediamosa_cql_part_has_not();

    $str_relation = $cql_part_relation->mediamosa_cql_part_str_get();
    $has_null = isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL]) ? $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL] : FALSE;
    $do_null = FALSE;

    switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
      case mediamosa_sdk::TYPE_INT:
        $str_quotes = '';
        if ($str_relation != 'within') {
          $str_search_term = (int)$str_search_term; // make sure its an integer.... :)
        }
        break;
      case mediamosa_sdk::TYPE_DATETIME:
        if ($str_relation != 'within') {
          // Now into DB
          $str_search_term = $this->mediamosa_cql_str_date_2_db_date($str_search_term);

          if ($str_relation == '=' || $str_relation == 'any') {
            $str_relation = '==';
          }
          $str_quotes = '\'';
        }
        break;
      case mediamosa_sdk::TYPE_DOMAIN:
      case mediamosa_sdk::TYPE_REALM:
      case mediamosa_sdk::TYPE_STRING:
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
      case mediamosa_sdk::TYPE_BOOL:
        $str_search_term = (mediamosa_unicode::strtolower($str_search_term) == "true" ? "TRUE" : "FALSE");
        $str_quotes = '\'';
        $str_like = '=';
        $str_relation = '==';
        break;

      case mediamosa_sdk::TYPE_BOOL_IS_SLAVED:
        $is_slaved = mediamosa_unicode::strtolower($str_search_term) == "true";
        $is_slaved = $str_relation == '<>' ? !$is_slaved : $is_slaved;

        $str_search_term = reset($this->app_ids);
        $str_quotes = '';
        $str_relation = $str_like = $is_slaved ? '=' : '<>';
        // Change type to int.
        $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE] = mediamosa_sdk::TYPE_UINT;
        break;

      case mediamosa_sdk::TYPE_JOB_PROGRESS:
        $str_quotes = '\'';
        $str_like = '=';
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
        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_INT:
          case mediamosa_sdk::TYPE_UINT:
            // Rewrite to IN statement: $str_table_column IN ($a_sub_parts).
            $tmp = array();
            foreach ($a_words as $str_word) {
              $tmp[] = sprintf('%d', (int)$str_word);
            }

            if (count($tmp)  == 1) {
              $a_sub_parts[] = sprintf('%s = %s', $str_table_column, implode(',', $tmp));
            }
            else {
              $a_sub_parts[] = sprintf('%s IN(%s)', $str_table_column, implode(',', $tmp));
            }
            break;
          default:
            foreach ($a_words as $str_word) {
              if (trim($str_word) == '') {
                continue;
              }

              if (mediamosa_unicode::substr($str_word, 0, 1) == '^' && mediamosa_unicode::substr($str_word, -1) == '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_word), -2) != '\^') {
                $str_search_term = mediamosa_unicode::substr($str_search_term, 1, -1);
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

        if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^') {
          $a_sub_parts[] = sprintf("%s%s %s %s%s%%%s", $str_binary, $str_table_column, $str_like, $str_quotes, $this->mediamosa_cql_context_sql_escape_like(mediamosa_unicode::substr($str_search_term, 1)), $str_quotes);
        }
        elseif (mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\\\', '', $str_search_term), -2) != '\^') {
          $a_sub_parts[] = sprintf("%s%s %s %s%%%s%s", $str_binary, $str_table_column, $str_like, $str_quotes, $this->mediamosa_cql_context_sql_escape_like(mediamosa_unicode::substr($str_search_term, 0, -1)), $str_quotes);
        }
        else {
          if ($cql_part_search_term->mediamosa_cql_contains_wildcards($str_search_term)) {
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

        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_INT:
            $a_sub_parts[] = sprintf("%s >= %s", $str_table_column, intval($a_within[0]));
            $a_sub_parts[] = sprintf("%s <= %s", $str_table_column, intval($a_within[1]));
            break;
          case mediamosa_sdk::TYPE_DATETIME:
            $a_sub_parts[] = sprintf("%s >= '%s'", $str_table_column, $this->mediamosa_cql_context_sql_escape($this->mediamosa_cql_str_date_2_db_date($a_within[0])));
            $a_sub_parts[] = sprintf("%s <= '%s'", $str_table_column, $this->mediamosa_cql_context_sql_escape($this->mediamosa_cql_str_date_2_db_date($a_within[1])));
            break;
          default:
            $a_sub_parts[] = sprintf("%s%s >= '%s'", $str_binary, $str_table_column, $this->mediamosa_cql_context_sql_escape($a_within[0]));
            $a_sub_parts[] = sprintf("%s%s <= '%s'", $str_binary, $str_table_column, $this->mediamosa_cql_context_sql_escape($a_within[1]));
            break;
        }
    }

    $a_parts[] = ( (isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_PREFIX]) ? $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_PREFIX] : '') . (count($a_sub_parts) ? '(' : '') . implode($str_sub_parts_glue, $a_sub_parts) . (count($a_sub_parts) ? ')' : '') . (isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_SUFFIX]) ? $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_SUFFIX] : ''));

    if ($cql_part_boolean) {
      $str_boolean = $cql_part_boolean->mediamosa_cql_part_str_get();
      $a_parts[] = ($str_boolean == 'NOT' ? 'AND ' : '') . $str_boolean;
    }

    $a_parts_or[] = (($has_not && count($a_parts) ? 'NOT ' : '') . implode(' ', $a_parts));

    if ( ($do_null || $has_not) && $has_null) {
      $a_parts_or[] = sprintf('%s IS NULL', $str_table_column);
    }

    return count($a_parts_or) > 1 ? '(' . implode(' OR ', $a_parts_or) . ')' : reset($a_parts_or);
    */
  }
}

class mediamosa_solr_cql extends mediamosa_core_cql {

  /**
   * Start parsing the CQL for asset search.
   *
   * @param string $str_cql
   * @param array $app_ids
   */
  static public function parse_asset($str_cql, array $app_ids) {

    // Create the CQL object.
    $mediamosa_cql = new mediamosa_solr_asset_cql_context($app_ids);
    return $mediamosa_cql->mediamosa_core_parse_cql($str_cql);
  }
}

