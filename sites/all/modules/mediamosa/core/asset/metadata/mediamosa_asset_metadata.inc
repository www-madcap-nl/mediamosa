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
}