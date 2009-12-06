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
  * Collection search functions.
  */

class mediamosa_collection_search {

  // ------------------------------------------------------------------ Static Functions.
    /**
   * Helper function
   * @param unknown_type $column_name
   */
  static public function collection_get_join_name($column_name) {
    return 'c.' . $column_name;
  }

  // _media_management_search_collection

  static public function collection_search(
    array $a_app_id,
    string $cql = '',
    string $asset_id = '',
    string $fav_user_id = '',
    string $aut_user_id = '',

    bool $is_app_admin = FALSE,
    bool $is_public_list = FALSE,

    int $limit = 10,
    int $offset = 0) {

    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.*';
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_collection} AS c';

    $a_query[mediamosa_db_query::ALLOW_DISTINCT] = FALSE;
    $a_query[mediamosa_db_query::A_GROUP_BY][] = 'c.coll_id';

    // Moved here so numofvideos is always in the result.
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'COUNT(DISTINCT ac.asset_id) AS numofvideos';
    $a_query[mediamosa_db_query::A_JOIN]['asset_collection'][] = 'LEFT JOIN {mediamosa_asset_collection} AS ac USING(coll_id)';

    // Collect my slaves.
    $a_slaves = mediamosa_aut_app_master_slave::slave_get($a_app_id, mediamosa_aut::AUT_TYPE_COLLECTION);

    // Include possible asset restriction.
    if ($asset_id != '') {
      // Check if asset exists.
      mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

      $a_query[mediamosa_db_query::A_WHERE][VPX_DB_WHERE_AND]['ac'][] = sprintf("ac.asset_id = '%s'", mediamosa_db::escape_string($asset_id));
    }

    // isprivate / unappropiate test.
    // Is outside the ACL check, else we would have problems with 'granted'.
    if (!$is_app_admin) {
      if ($is_public_list && $aut_user_id) {
        $a_query[mediamosa_db_query::A_WHERE][VPX_DB_WHERE_AND]['collection']['access'][mediamosa_db_query::WHERE_AND][] = sprintf("(c.isprivate = 'FALSE' AND (c.is_unappropriate = 'FALSE' OR c.owner_id = '%s'))", mediamosa_db::escape_string($aut_user_id));
      }
      elseif ($is_public_list) {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection']['access'][mediamosa_db_query::WHERE_AND][] = "c.isprivate = 'FALSE' AND c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE
      }
      elseif ($aut_user_id) { // if provided, then we only have access to unappropate when owner.
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection']['access'][mediamosa_db_query::WHERE_AND][] = sprintf("(c.is_unappropriate = 'FALSE' OR c.owner_id = '%s')", mediamosa_db::escape_string($aut_user_id));
      }
      else {
        // No public list, no aut_user_id
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection']['access'][mediamosa_db_query::WHERE_AND][] = "c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE
      }
    }

    // Access selection is done here;
    mediamosa_aut::build_access_where($a_query, mediamosa_aut::AUT_TYPE_COLLECTION, NULL, $a_app_id, $aut_user_id, NULL, NULL, NULL, NULL, $is_app_admin);

    if ($cql != '') {
      $a_result_cql2sql = mediamosa_core_cql::parse_collection($cql, $a_app_id);

      if (isset($a_result_cql2sql['str_where']) && $a_result_cql2sql['str_where'] != "") {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][VPX_QUERY_DIM_NAME_SEARCH] = $a_result_cql2sql['str_where'];
      }

      if (isset($a_result_cql2sql['str_having']) && $a_result_cql2sql['str_having'] != "") {
        $a_query[mediamosa_db_query::A_HAVING][] = $a_result_cql2sql['str_having'];
      }

      if (isset($a_result_cql2sql['a_joins'])) {
        $a_query[mediamosa_db_query::A_JOIN]['cql'] = $a_result_cql2sql['a_joins'];
      }

      if (count($a_result_cql2sql['a_order_by']) > 1) {
        throw new mediamosa_exception_error_cql_error(array('@error' => 'you can not use \'sortBy\' on multiple columns, only specify one column'));
      }

      $a_order_by = reset($a_result_cql2sql['a_order_by']);

      $order_by = $a_order_by['column'];
      $order_direction = $a_order_by['direction'];
    }

    if ($order_by != '') {
      if ($order_by == 'numofvideos') {
        // Sort on numofvideos is a special case
        $a_query[mediamosa_db_query::A_ORDER_BY][] = 'numofvideos '. mediamosa_db::escape_string($order_direction);
      }
      else {
        $a_query[mediamosa_db_query::A_ORDER_BY][] = 'ISNULL(' . mediamosa_collection::collection_get_join_name($order_by) . ') ' . mediamosa_db::escape_string($order_direction);
        $a_query[mediamosa_db_query::A_ORDER_BY][] = mediamosa_collection::collection_get_join_name($order_by) . ' ' . mediamosa_db::escape_string($order_direction);
      }
    }

    if ($fav_user_id != '') {
      $a_query[mediamosa_db_query::A_JOIN]['user_favorite']['user_fav'] = "LEFT JOIN {mediamosa_user_favorite} AS user_fav ON user_fav.fav_type = '". mediamosa_user_favorite_db::FAV_TYPE_COLLECTION . "' AND user_fav.fav_id = c.coll_id\n";

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['user_fav'][] = sprintf("user_fav.name = '%s'", mediamosa_db::escape_string($fav_user_id));
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['user_fav'][] = sprintf("user_fav.app_id IN(%s)", mediamosa_db::escape_string($a_app_id));
    }

    $a_query[mediamosa_db_query::LIMIT] = $limit;
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Build the query.
    $query = mediamosa_db::db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

    // Do the query.
    $result = mediamosa_db::db_query();

    // Set the found_rows on the object.
    $result->found_rows = mediamosa_db_query::found_rows();

    // Return the result.
    return $result;
  }
}
