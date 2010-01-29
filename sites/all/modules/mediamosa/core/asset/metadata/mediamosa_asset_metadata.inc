<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
  * The asset metadata module.
  */

class mediamosa_asset_metadata {

  // ------------------------------------------------------------------ Consts.
  const MATCH_TYPE_CONTAINS = 'contains';
  const MATCH_TYPE_EXACT = 'exact';
  const MATCH_TYPE_BEGIN = 'begin';
  const MATCH_TYPE_BOOLEAN = 'boolean';
  const MATCH_TYPE_PERIOD = 'period';
  const MATCH_TYPE_PERIOD_FROM = 'period_from';
  const MATCH_TYPE_PERIOD_TO = 'period_to';
  const MATCH_TYPE_RANGE = 'range';
  const MATCH_TYPE_RANGE_FROM = 'range_from';
  const MATCH_TYPE_RANGE_TO = 'range_to';

  // ------------------------------------------------------------------ Static Function.
  /**
   * Build the expression for SQL
   * FIXME seems to be no longer used(!)
   *
   * @param array $a_search
   * @param string $s_tablename_alias_full
   * @return string
   */
  static public function search_get_part_where($a_search, $tablename_alias_full) { // _media_management_search_get_part_where

    if (!is_array($a_search['s_value']) && $a_search['s_value'] == "") {
      return sprintf("(%s = '' || %s IS NULL)", $tablename_alias_full, $tablename_alias_full);
    }

    switch ($a_search['s_type']) {
      case self::MATCH_TYPE_BOOLEAN:
        $a_search['s_value'] = (drupal_strtolower($a_search['s_value']) == "true" ? "TRUE" : "FALSE");

      case self::MATCH_TYPE_EXACT:
        return sprintf("(%s = '%s')", $tablename_alias_full, mediamosa_db::escape_string($a_search['s_value']));

      case self::MATCH_TYPE_CONTAINS:
        return sprintf("(%s LIKE '%%%s%%')", $tablename_alias_full, mediamosa_db::escape_string_like($a_search['s_value']));

      case self::MATCH_TYPE_BEGIN:
        return sprintf("(%s LIKE '%s%%')", $tablename_alias_full, mediamosa_db::escape_string_like($a_search['s_value']));

      case self::MATCH_TYPE_PERIOD:
        assert(isset($a_search['s_value'][self::MATCH_TYPE_PERIOD_FROM]) && isset($a_search['s_value'][self::MATCH_TYPE_PERIOD_TO]));
        return sprintf("(%s >= '%s' AND %s < '%s')", $tablename_alias_full, mediamosa_db::escape_string($a_search['s_value'][self::MATCH_TYPE_PERIOD_FROM]['s_value']), $tablename_alias_full, mediamosa_db::escape_string($a_search['s_value'][self::MATCH_TYPE_PERIOD_TO]['s_value']));

      case self::MATCH_TYPE_RANGE:
        assert(isset($a_search['s_value'][self::MATCH_TYPE_RANGE_FROM]) && isset($a_search['s_value'][self::MATCH_TYPE_RANGE_TO]));
        return sprintf("(%s >= %d AND %s < %d)", $tablename_alias_full, mediamosa_db::escape_string($a_search['s_value'][self::MATCH_TYPE_RANGE_FROM]['s_value']), $tablename_alias_full, mediamosa_db::escape_string($a_search['s_value'][self::MATCH_TYPE_RANGE_TO]['s_value']));
    }

    // We should not get here...
    throw new mediamosa_exception_error_unexpected_error();
  }

  /**
   * Return a list of property values and their count per given property.
   *
   * @param integer $app_id
   *  The application ID.
   * @param integer $prop_id
   *  The property ID.
   * @param integer $offset
   *  The offset in the result set.
   * @param integer $limit
   *  Maximum number of rows in result.
   */
  static public function tagcount($app_id, $prop_id, $offset, $limit) {

    // FIXME test if prop_id exists.

    return mediamosa_db::db_query_range(
      'SELECT COUNT(#val_char) AS tagcount, #val_char FROM {#mediamosa_asset} AS a
       INNER JOIN {#mediamosa_asset_metadata} AS ap ON a.#asset_id = ap.#asset_id
       WHERE ap.#prop_id = :prop_id AND a.#app_id = :app_id
       GROUP BY #val_char
       ORDER BY cnt DESC',
      $offset,
      $limit,
      array(
        '#val_char' => mediamosa_asset_metadata_db::VAL_CHAR,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_db::ID,
        '#prop_id' => mediamosa_asset_metadata_db::PROP_ID,
        ':prop_id' => $prop_id,
        '#app_id' => mediamosa_asset_db::APP_ID,
        ':app_id' => $app_id,
      )
    );
  }

  /**
   * Create metadata for asset.
   *
   * @param $asset_id
   *  The asset ID.
   * @param $a_metadata_definitions_full
   *  The allowed metadata properties.
   * @param $a_params
   *  The values for metadata array(name => value)
   * @param $action
   *  The action; append, replace, update.
   */
  static public function metadata_create($asset_id, array $a_metadata_definitions_full, array $a_params, $action) {
    $a_value_set = array();

    foreach ($a_metadata_definitions_full as $name => $a_definition) {
      // All off.
      $do_delete = FALSE;

      // lets not use the alias.
      $prop_id = $a_definition['propdef_id'];

      // Based on action, set it.
      switch ($action) {
        case 'replace':
          $do_delete = TRUE;
        case 'append':
          break;
        case 'update':
          if (isset($a_params[$name])) {
            $do_delete = TRUE;
          }
          break;
      }

      if ($do_delete) {
        mediamosa_db::db_query(
          'DELETE FROM {#mediamosa_asset_metadata} WHERE #asset_id = :asset_id AND #prop_id = :prop_id',
          array(
            '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
            '#asset_id' => mediamosa_asset_metadata_db::ASSET_ID,
            ':asset_id' => $asset_id,
            '#prop_id' => mediamosa_asset_metadata_db::PROP_ID,
            ':prop_id' => $prop_id
          )
        );
      }

      if (!isset($a_params[$name])) {
        continue;
      }

      // Get the value.
      $a_values = $a_params[$name];
      foreach ($a_values as $value) {

        // We do not store empty values.
        if (trim($value) == '') {
          continue;
        }

        $query = mediamosa_db::db_insert(mediamosa_asset_metadata_db::TABLE_NAME);
        // FIXME: changed date will never be right if we just delete, and then replace.
        //        really should rewrite this properly with updating the metadata instead of deleting it.
        $a_fields = array(
          mediamosa_asset_metadata_db::ASSET_ID => $asset_id,
          mediamosa_asset_metadata_db::PROP_ID => $prop_id,
        );

        // Add created value.
        $a_fields = mediamosa_db::db_insert_enrich($a_fields);

        // Change from 1.x, we don't store all values in every val_[char/int/datetime] type. No more int value also in val_char etc.
        switch ($a_definition['propdef_type']) {
          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            $a_fields[mediamosa_asset_metadata_db::VAL_DATETIME] = $value;
            break;
          case mediamosa_asset_metadata_property_db::TYPE_INT:
            $a_fields[mediamosa_asset_metadata_db::VAL_INT] = $value;
            break;
          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            $a_fields[mediamosa_asset_metadata_db::VAL_CHAR] = $value;
            $a_fields[mediamosa_asset_metadata_db::VAL_CHAR_LFT] = mediamosa_unicode::substr($value, 0, mediamosa_asset_metadata_db::VAL_CHAR_LFT_LENGTH);
            $a_fields[mediamosa_asset_metadata_db::VAL_CHAR_RGHT] = mediamosa_unicode::strrev(mediamosa_unicode::substr($value, 0, mediamosa_asset_metadata_db::VAL_CHAR_RGHT_LENGTH));
            break;
          default:
            assert(0);
            throw new mediamosa_exception_program_error('unknown metadata type');
            break;
        }

        // Insert it.
        $query->fields($a_fields)->execute();

        $a_value_set[$name] = $value;
      }
    }

    // update de timestamps van de asset
    mediamosa_asset::update_asset_timestamps($asset_id);

    // Return whats been changed.
    return $a_value_set;
  }

  /**
   * Collect the metadata of the asset.
   * In array -> [groupname][propertyname] = array(values).
   *
   * @param $asset_id
   */
  function metadata_get($asset_id) {
    $result = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_asset_metadata} AS am
       JOIN {#mediamosa_asset_metadata_property} AS amp ON amp.#prop_id = am.#prop_id
       JOIN {#mediamosa_asset_metadata_property_group} AS ampg ON ampg.#propgroup_id = amp.#propgroup_id
       WHERE am.#asset_id = :asset_id',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property' => mediamosa_asset_metadata_property_db::TABLE_NAME,
        '#prop_id' => mediamosa_asset_metadata_property_db::ID,
        '#mediamosa_asset_metadata_property_group' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        '#propgroup_id' => mediamosa_asset_metadata_property_group_db::ID,
        '#asset_id' => mediamosa_asset_metadata_db::ASSET_ID,
        ':asset_id' => $asset_id
      )
     );

    $a_result = array();
    foreach ($result as $a_metadata) {
      $a_result[$a_metadata[mediamosa_asset_metadata_property_group_db::NAME]][$a_metadata[mediamosa_asset_metadata_property_db::NAME]][] = $a_metadata[mediamosa_asset_metadata_db::VAL_CHAR];
    }

    return $a_result;
  }
}
