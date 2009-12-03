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
  * The asset search class contains the search functions that are moved here
  * to keep code more clean from the normal asset code.
  */

class mediamosa_asset_search {
  // ------------------------------------------------------------------ Static Function.
	// array name for search in where
  const SEARCH = 'search';
  const APPS = 'apps';
  const ORDER_BY = 'order_by';
  const ASSET_COLLECTION = 'asset_coll';
  const COLLECTION = 'collection';
  const FTP_BATCH = 'ftp_batch';

  // ------------------------------------------------------------------ Static Function.

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
    }

    return (count($a_or) > 1 ? '(' : '') . implode(' OR ', $a_or) . (count($a_or) > 1 ? ')' : '');
  }

  /**
   * Search for assets with given parameters.
   * 1.x _media_management_search_asset
   *
   * @param mediamosa_response $o_mediamosa
   *  Mediamosa response object.
   * @param array $a_app_ids
   *  Application IDs.
   * @param string $cql
   *  The CQL search string.
   * @param string $aut_user_id
   *  The authentication user.
   * @param array $a_aut_group_id
   *  The authentication group.
   * @param string $aut_domain
   *  The authentication domain.
   * @param string $aut_realm
   *  The authentication realm.
   * @param string $batch_id
   *  The batch ID to search through.
   * @param string $coll_id
   *  The Collection to search through.
   * @param string $fav_user_id
   *  The user ID.
   * @param boolean $granted
   *  When true, assets are include on which we dont have access
   *  on. The granted parameter with the asset indicates if we have
   *  access.
   * @param boolean $is_public_list
   *  Used to include or exclude unappropriate assets.
   * @param boolean $is_app_admin
   *  Used for unappropriate.
   * @param boolean $hide_empty_assets
   *  Don't include empty assets.
   * @param boolean $show_deleted
   *  Include deleted.
   * @param boolean $return_asset_ids
   *  Return asset_ids only.
   * @param boolean $show_stills
   *  Include stills.
   * @param boolean $show_collections
   *  Include collections.
   * @param integer $limit
   * @param integer $offset
   * @param string $order_by (CQL?)
   * @param string $order_direction (CQL?)
   */
  static public function asset_search(
    mediamosa_response $o_mediamosa,
    array $a_app_ids,
    $cql = '',

    $aut_user_id = '',
    array $a_aut_group_id = array(),
    $aut_domain = '',
    $aut_realm = '',

    $batch_id = 0,
    $coll_id = 0,

    $fav_user_id = '',

    $granted = false,
    $is_public_list = false,
    $is_app_admin = false,
    $hide_empty_assets = false,
    $show_deleted = false,
    $return_asset_ids = false,
    $show_stills = false,
    $show_collections = false,

    $limit = 10,
    $offset = 0

   // $order_by = '',
   // $order_direction = mediamosa_db::ORDER_DIRECTION_ASC

  ) {
	  // Do master slave depends on granted.
	  $do_master_slave = $granted;

	  $a_query = array();
	  $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.asset_id';
	  $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_asset} AS a';

	  $a_query[mediamosa_db_query::ALLOW_DISTINCT] = TRUE;
	  $a_query[mediamosa_db_query::A_GROUP_BY][] = 'a.asset_id';

	  // Get the slaves based on the app ids.
	  $a_slaves = mediamosa_aut_app_master_slave::slave_get($a_app_ids, mediamosa_aut::AUT_TYPE_MEDIAFILE);

	  // Merge the slave app ids with my app ids.
	  $a_app_ids_tmp = array_unique(array_merge($a_app_ids, array_keys($a_slaves)));
	  sort($a_app_ids_tmp);

	  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf('a.app_id IN(%s)', implode(',', $a_app_ids_tmp));
	  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = 'a.parent_id IS NULL';

	  // isprivate / unappropiate test.
	  // Is outside the ACL check, else we would have problems with 'granted'.
	  if (!$is_app_admin) {
	    if ($is_public_list && $aut_user_id) {
	      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = sprintf("(a.is_private = 'FALSE' AND (a.is_unappropriate = 'FALSE' OR a.owner_id = '%s'))", mediamosa_db::escape_string($aut_user_id));
	    }
	    elseif ($is_public_list) {
	      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = "(a.is_private = 'FALSE' AND a.is_unappropriate = 'FALSE')"; // Must both be FALSE
	    }
	    elseif ($aut_user_id) { // if provided, then we only have access to unappropate when owner.
	      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = sprintf("(a.is_private = 'FALSE' OR a.is_private = 'TRUE') AND (a.is_unappropriate = 'FALSE' OR a.owner_id = '%s')", mediamosa_db::escape_string($aut_user_id));
	    }
	    else {
	      // No public list, no aut_user_id
	      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = "((a.is_private = 'FALSE' OR a.is_private = 'TRUE') AND a.is_unappropriate = 'FALSE')"; // Ignore isprivate, is_unappropriate must be TRUE
	    }
	  }
	  else {
	    // just to match index for speed
	    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = "(a.is_private = 'FALSE' OR a.is_unappropriate = 'FALSE' OR a.is_unappropriate = 'TRUE')";// only 3, will match always
	  }

	  // Switch to hide assets that have no mediafiles
	  if ($hide_empty_assets) {
	    // exclude empty assets
	    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = "a.is_empty_asset = 'FALSE'";
	  }
	  else {
	    // just to match index for speed
	    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = 'a.is_empty_asset IS NOT NULL'; // Never NULL, will always match index
	  }

	  if ($cql != '') {
	    // Create the seach CQL.
	    $a_result_cql2sql = mediamosa_core_cql::parse_asset($cql, $a_app_ids);

	    if ($a_result_cql2sql['str_where'] != '') {
	      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::SEARCH] = $a_result_cql2sql['str_where'];
	    }

	    if (isset($a_result_cql2sql['str_having']) && $a_result_cql2sql['str_having'] != '') {
	      $a_query[mediamosa_db_query::A_HAVING][] = $a_result_cql2sql['str_having'];
	      $a_query[VPX_DB_QUERYB_ALLOW_DISTINCT] = FALSE;
	    }

	    // Quick hack, or else i really need to extend the VPX_CQL class
	    if (isset($a_result_cql2sql['a_joins'])) {
	      $key = array_search('MEDIAFILE_METADATA_MIMETYPE', $a_result_cql2sql['a_joins']);
	      if ($key) {
	        unset($a_result_cql2sql['a_joins'][$key]);

	        $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
	        $a_query[mediamosa_db_query::A_JOIN]['mediafile_metadata']['mm_mime_type'] = sprintf('LEFT JOIN {mediafile_metadata} AS mm_mime_type ON mf.mediafile_id=mm_mime_type.mediafile_id');
	      }

	      $key = array_search('MEDIAFILE_FILENAME', $a_result_cql2sql['a_joins']);
	      if ($key) {
	        unset($a_result_cql2sql['a_joins'][$key]);

	        $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
	      }

	      $key = array_search('AUT_NAME_USER', $a_result_cql2sql['a_joins']);
	      if ($key) {
	        unset($a_result_cql2sql['a_joins'][$key]);

	        $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
	        $a_query[mediamosa_db_query::A_JOIN]['aut_object'] = mediamosa_aut_object::join_aut_object_get(mediamosa_aut::AUT_TYPE_MEDIAFILE);
	        $a_query[mediamosa_db_query::A_JOIN]['aut_name_user'] = sprintf("LEFT JOIN {aut_name} AS aut_u ON aut_u.app_id IN (%s) AND aut_u.aut_type = 'USER' AND aut_u.aut_name_id=aut_obj.aut_id", implode(',', $a_app_ids));
	        $do_master_slave = TRUE;
	        unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
	      }

	      $key = array_search('AUT_NAME_USER_GROUP', $a_result_cql2sql['a_joins']);
	      if ($key) {
	        unset($a_result_cql2sql['a_joins'][$key]);

	        $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
	        $a_query[mediamosa_db_query::A_JOIN]['aut_object'] = mediamosa_aut_object::join_aut_object_get(mediamosa_aut::AUT_TYPE_MEDIAFILE);
	        $a_query[mediamosa_db_query::A_JOIN]['aut_name_group'] = sprintf("LEFT JOIN {aut_name} AS aut_ug ON aut_ug.app_id IN (%s) AND aut_ug.aut_type = 'USER_GROUP' AND aut_ug.aut_name_id=aut_obj.aut_id", implode(',', $a_app_ids));
	        $do_master_slave = TRUE;
	        unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
	      }

	      $key = array_search('AUT_GROUP_DOMAIN', $a_result_cql2sql['a_joins']);
	      if ($key) {
	        unset($a_result_cql2sql['a_joins'][$key]);

	        $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
	        $a_query[mediamosa_db_query::A_JOIN]['aut_object'] = mediamosa_aut_object::join_aut_object_get(mediamosa_aut::AUT_TYPE_MEDIAFILE);
	        $a_query[mediamosa_db_query::A_JOIN]['aut_group_domain'] = sprintf("LEFT JOIN {aut_name} AS aut_gd ON aut_gd.app_id IN (%s) AND aut_gd.aut_type = 'DOMAIN' AND aut_gd.aut_name_id=aut_obj.aut_id", implode(',', $a_app_ids));
	        $do_master_slave = TRUE;
	        unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
	      }

	      $key = array_search('AUT_GROUP_REALM', $a_result_cql2sql['a_joins']);
	      if ($key) {
	        do {
	          unset($a_result_cql2sql['a_joins'][$key]);
	          $key = array_search('AUT_GROUP_REALM', $a_result_cql2sql['a_joins']);
	        }
	        while ($key !== FALSE);

	        $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
	        $a_query[mediamosa_db_query::A_JOIN]['aut_object'] = mediamosa_aut_object::join_aut_object_get(mediamosa_aut::AUT_TYPE_MEDIAFILE);
	        $a_query[mediamosa_db_query::A_JOIN]['aut_group_realm'] = sprintf("LEFT JOIN {aut_name} AS aut_gr ON aut_gr.app_id IN (%s) AND aut_gr.aut_type = 'REALM' AND aut_gr.aut_name_id=aut_obj.aut_id", implode(',', $a_app_ids));
	        $do_master_slave = TRUE;
	        unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
	      }

	      $key = array_search('AUT_APP_MASTER_SLAVE', $a_result_cql2sql['a_joins']);
	      if ($key) {
	        do {
	          unset($a_result_cql2sql['a_joins'][$key]);
	          $key = array_search('AUT_APP_MASTER_SLAVE', $a_result_cql2sql['a_joins']);
	        }
	        while ($key !== FALSE);

	        $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
	        $a_query[mediamosa_db_query::A_JOIN]['aut_app_master_slave_2'] = sprintf("JOIN {aut_app_master_slave} AS aut_ms ON aut_ms.aut_object_type = 'MEDIAFILE' AND aut_ms.aut_object_id=mf.mediafile_id AND aut_ms.app_id_slave IN (%s)", implode(',', $a_app_ids));
	        $do_master_slave = TRUE;
	        unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
	        unset($a_query[mediamosa_db_query::A_JOIN]['aut_app_master_slave']);
	      }

	      $a_query[mediamosa_db_query::A_JOIN]['cql'] = $a_result_cql2sql['a_joins'];
	    }

	    if (isset($a_result_cql2sql['a_select_expr'])) {
	      $a_query[mediamosa_db_query::A_SELECT_EXPR] = array_merge($a_query[mediamosa_db_query::A_SELECT_EXPR], $a_result_cql2sql['a_select_expr']);
	    }

	    if (count($a_result_cql2sql['a_order_by']) > 1) {
	      throw new mediamosa_exception_error_cql_error(array('@error' => 'you can not use \'sortBy\' on multiple columns, only specify one column'));
	    }

	    $a_order_by = reset($a_result_cql2sql['a_order_by']);
	    $order_by = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN];
	    $order_direction = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_DIRECTION];
	    $order_table = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT];
	    $order_prop_id = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID];
	  }


    if ($order_by != '') {
      if ($order_prop_id) {
        $join = strtr(
          'JOIN {#table} AS sort ON sort.asset_id = a.asset_id AND sort.prop_id = #prop_id',
          array(
            '#table' => $order_table,
            '#prop_id' => intval($order_prop_id),
          )
        );
      }
      else {
        $join = strtr(
      	 'JOIN {#table} AS sort ON sort.asset_id = a.asset_id',
          array(
            '#table' => $order_table,
          )
        );
      }

      // 000_ will make me top join.
      $a_query[mediamosa_db_query::A_JOIN]['000_sort'] = $join;
      sort($a_query[mediamosa_db_query::A_JOIN]);
      $a_query[mediamosa_db_query::A_ORDER_BY][] = 'sort.' . $order_by . ' ' . $order_direction;
    }
	  /*
	 * @todo:
	  if ($order_by != '') {
	    $a_query[mediamosa_db_query::A_ORDER_BY][] = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by) .' '. mediamosa_db::escape_string($order_direction);

	    $a_asset_property = _media_management_searchsort_is_type($a_funcparam, $order_by, 'asset_property');

	    $order_by_alias = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by, TRUE);
	    $order_by_alias_full = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by);
	    if ($a_asset_property) {

	      // Force it to be first in this array...
	      $a_query[mediamosa_db_query::A_JOIN] = array('asset_property' => array($order_by_alias => sprintf('JOIN {asset_property} AS %s ON %s.prop_id = %d AND %s.asset_id = a.asset_id', $order_by_alias, $order_by_alias, $a_asset_property['prop_id'], $order_by_alias))) + (isset($a_query[mediamosa_db_query::A_JOIN]) ? $a_query[mediamosa_db_query::A_JOIN] : array());

	      if ($a_asset_property['type'] == VPX_PROP_DEF_TYPE_CHAR) {
	        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY][] = $order_by_alias_full .'<>''';
	      }
	    }
	    else {
	      //$a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY][] = $order_by_alias_full .' IS NOT NULL';

	      if ($a_funcparam['a_sort'][self::ORDER_BY]['type'] == VPX_TYPE_ALPHANUM) {
	        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY][] = $order_by_alias_full .'<>''';
	      }
	    }
	  }
	*/
	  // If coll_id is given, then search within the given collection(s)
	  if ($coll_id) {
	    $a_query[mediamosa_db_query::A_JOIN]['asset_collection'][self::ASSET_COLLECTION] = 'JOIN {asset_collection} AS asset_coll USING(asset_id)';

	    // is_unappropriate
	    if (!$is_app_admin) {
	      // Default.
	    	$tmp = "c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE

	      if ($is_public_list && $aut_user_id) {
	        $tmp = sprintf("(c.isprivate = 'FALSE' AND (c.is_unappropriate = 'FALSE' OR c.owner_id = '%s'))", mediamosa_db::escape_string($aut_user_id));
	      }
	      elseif ($is_public_list) {
	        $tmp = "c.isprivate = 'FALSE' AND c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE
	      }
	      elseif ($aut_user_id) { // if provided, then we only have access to unappropate when owner.
	        $tmp = sprintf("(c.is_unappropriate = 'FALSE' OR c.owner_id = '%s')", mediamosa_db::escape_string($aut_user_id));
	      }

	      $a_query[mediamosa_db_query::A_JOIN]['collection'][self::COLLECTION] = 'INNER JOIN {collection} AS c ON asset_coll.coll_id = c.coll_id AND '. $tmp;
	    }

	    $a_coll_ids = is_array($coll_id) ? $coll_id : array($coll_id);
	    sort($a_coll_ids);
	    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ASSET_COLLECTION][mediamosa_db_query::WHERE_OR][] = sprintf("asset_coll.coll_id IN ('%s')", mediamosa_db::escape_string($a_coll_ids, "','"));
	  }

	  // FTP Batch.
	  if ($batch_id) {
	    // Check if batch exists
	    mediamosa_db::db_must_exists(mediamosa_ftp_batch_db::TABLE_NAME, array(mediamosa_ftp_batch_db::ID => $batch_id));

	    $a_query[mediamosa_db_query::A_JOIN]['ftp_batch_asset'][self::FTP_BATCH] = 'JOIN {ftp_batch_asset} AS ftp_ba USING(asset_id)';

	    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::FTP_BATCH][] = sprintf("ftp_ba.batch_id='%s'", mediamosa_db::escape_string($batch_id));
	  }

	  // When fav_user_id provided, we search for this user as owner.
	  if ($fav_user_id != '') {
	    $a_query[mediamosa_db_query::A_JOIN]['user_favorites']['user_fav'] = "LEFT JOIN {mediamosa_user_favorites} AS user_fav ON user_fav.fav_type='" . mediamosa_user_favorite_db::FAV_TYPE_ASSET . "' AND user_fav.fav_id = a.asset_id\n";

	    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['user_fav'][] = sprintf("user_fav.name = '%s'", mediamosa_db::escape_string($fav_user_id));
	    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['user_fav'][] = sprintf("user_fav.app_id IN('%s')", mediamosa_db::escape_string($a_app_ids, "','"));
	  }

	  // Authentication check
	  // All authentication is done as last part in the where to make the selection to authenticate as small as possible
	  // Also we only do authication here when we need to filter out the results (granted == FALSE)
	  // Except we need to do master/slave check, so we set switch only to include master/slave...
	  // Add to the where the access check for our original app_id
	  mediamosa_aut::build_access_where($a_query, mediamosa_aut::AUT_TYPE_MEDIAFILE, NULL, $a_app_ids, $aut_user_id, $a_aut_group_id, $aut_domain, $aut_realm, $a_slaves, FALSE, $do_master_slave);

	  // Limit, offset stuff
	  $a_query[mediamosa_db_query::LIMIT] = $limit;
	  $a_query[mediamosa_db_query::OFFSET] = $offset;

	  // HACK: Unset the aut_app_master_slave when aut_app_master_slave_2 is set
	  if (isset($a_query[mediamosa_db_query::A_JOIN]['aut_app_master_slave_2'])) {
	    unset($a_query[mediamosa_db_query::A_JOIN]['aut_app_master_slave']);
	  }

	  // Deleted assets
	  if (!$show_deleted || $coll_id) {
	    // Normal behaviour
	    // Do the query

	  	// @todo: remove hack (skipping SQL_CALC_FOUND_ROWS).
	    $s_query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_QUERY::SQL_CALC_FOUND_ROWS => ($a_query[mediamosa_db_query::LIMIT] < 7 ? FALSE : TRUE)));

	    global $db_url;
	    /*
	     * @todo: fix hack here.
	     *
	    if (array_key_exists('data2', $db_url)) {
	      db_set_active('data2');
	    }
	    else {
	      db_set_active('data');
	    }
	    */

	    // Do the query.
	    $db_result = mediamosa_db::db_query($s_query);

	    // Get the number of found rows.
	    $i_found_rows = mediamosa_db::db_found_rows();

	    $a_asset_ids = array();
	    foreach ($db_result as $a_row) {
	      $a_asset_ids[] = $a_row[mediamosa_asset_db::ID];
	    }
	  }
	  else {
	    // Oai behaviour with deleted assets

	  	// RBL: not touching this code to much, really needs a rewrite without the temp. table.

// How it is working:
//CREATE TEMPORARY TABLE assets_all (asset_id VARCHAR(32) NOT NULL, videotimestampmodified TIMESTAMP NULL DEFAULT NULL, PRIMARY KEY  (asset_id)) ENGINE=MEMORY;
//
//INSERT INTO assets_all SELECT DISTINCT a.asset_id, a.videotimestampmodified FROM asset AS a LEFT JOIN mediafile AS mf ON a.asset_id=mf.asset_id_root
//WHERE a.app_id IN(14) AND
//a.parent_id IS NULL AND
//(((a.is_private = 'FALSE' OR a.is_private = 'TRUE') AND a.is_unappropriate = 'FALSE') AND
//a.is_empty_asset IS NOT NULL) AND
//((a.videotimestampmodified >= '2009-06-12 00:00:00' AND a.videotimestampmodified < '2009-08-15 00:00:00')) AND
//(mf.asset_id IS NULL OR (mf.is_original_file='TRUE' AND
//mf.app_id IN(14)));
//
//INSERT INTO assets_all SELECT a.asset_id, a.videotimestampmodified FROM asset_delete AS a WHERE a.app_id IN(14) AND
//((a.videotimestampmodified >= '2009-06-12 00:00:00' AND a.videotimestampmodified < '2009-08-15 00:00:00'));
//
//SELECT SQL_CALC_FOUND_ROWS DISTINCT a.asset_id FROM assets_all AS a
//ORDER BY a.videotimestampmodified ASC LIMIT 20;
//
//DROP TABLE assets_all;

	  	// Copy so we can change it back.
	    $tmp_a_query = $a_query;

	    // Collecting the data
	    $d_ordby = $a_query[mediamosa_db_query::A_ORDER_BY][0];
	    $d_limit = $a_query[mediamosa_db_query::LIMIT];
	    $d_offset = $a_query[mediamosa_db_query::OFFSET];
	    $d_search = isset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::SEARCH]) ? $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::SEARCH] : FALSE;
	    $d_where = sprintf('a.app_id IN(%s)', implode(',', $a_app_ids_tmp)) . ($d_search ? ' AND '. $d_search : '');

	    global $db_url;
	    /*
	     * @todo: fix hack here.
	     *
	    if (array_key_exists('data2', $db_url)) {
	      db_set_active('data2');
	    }
	    else {
	      db_set_active('data');
	    }
	    */

	    // Temporary table
	    /**
	     * @todo: fix this to db_query_temp
	     */
	    mediamosa_db::db_query('
	      CREATE TEMPORARY TABLE {assets_all} (
	        asset_id VARCHAR(32) NOT NULL,
	        videotimestampmodified TIMESTAMP NULL DEFAULT NULL,
	        PRIMARY KEY  (asset_id)
	      ) ENGINE=MEMORY');

		    // Modified original query
		    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.videotimestampmodified';
		    unset($a_query[mediamosa_db_query::A_ORDER_BY]);
		    unset($a_query[mediamosa_db_query::LIMIT]);
		    unset($a_query[mediamosa_db_query::OFFSET]);
		    $s_query = mediamosa_db_query::query_select($a_query);
		    mediamosa_db::db_query('INSERT INTO {assets_all} '. $s_query);

		    // Deleted asset query
		    mediamosa_db::db_query('INSERT INTO {assets_all} SELECT a.asset_id, a.videotimestampmodified FROM {asset_delete} AS a'. ($d_where ? ' WHERE '. $d_where : ''));
		    //db_query('INSERT INTO {assets_all} SELECT a.asset_id, a.videotimestampmodified FROM {asset_delete} AS a'. ($d_where ? ' WHERE '. $d_where : '') .' LIMIT '. ($d_offset + $d_limit + 1));

		    // Harvesting the data
		    $result = mediamosa_db::db_query('SELECT DISTINCT a.asset_id FROM {assets_all} AS a'. ($d_ordby ? ' ORDER BY '. $d_ordby : '') . ($d_limit ? ($d_offset ? ' LIMIT '. $d_offset .','. $d_limit : ' LIMIT '. $d_limit) : ''));
		    $a_asset_ids = array();
		    foreach ($result as $a_row) {
		      $a_asset_ids[] = $a_row[mediamosa_asset_db::ID];
		    }

		    // How much assets we have globally
		    $i_found_rows = db_query('SELECT COUNT(*) FROM {assets_all}')->fetchField();

		    // Drop temporary table
		    db_query('DROP TABLE {assets_all}');

		    // Restore.
		    $a_query = $tmp_a_query;
		  }

	  // Now create the query that will include all other results that are not included here
	  // but only when a order by was present

	  /**
	   * @todo: fix hack here. ($a_query[mediamosa_db_query::LIMIT] > 6)
	   */

	  /*
	  if (($order_by != '') && ($a_query[mediamosa_db_query::LIMIT] > 6)) {
	    // Special case. If we have an order by, we need to attach the NULL values to the end result
	    // Even if we had enough of the other query, we still need to figure out the total result
	    // of rows of the 1st query AND the total of the NULL values.
	    // Lucky for us, the NULL value query does not require an order by and can match our index 1 on 1

	    $a_query_2 = $a_query; // copy 1st query

	    // Remove the order by from the query
	    unset($a_query_2[mediamosa_db_query::A_ORDER_BY]);

	    $a_asset_property = _media_management_searchsort_is_type($a_funcparam, $order_by);
	    $order_by_alias = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by, TRUE);
	    $order_by_alias_full = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by);

	    // Remove the where for order by
	    unset($a_query_2[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY]);

	    if ($a_asset_property) {
	      unset($a_query_2[mediamosa_db_query::A_JOIN]['asset_property'][$order_by_alias]);// so its inserted as last in array(!)

	      // Note; the force index is kinda hack, mysql is stupid, it uses the index for datetime by default, make sure the index exists(!)
	 //     $a_query_2[mediamosa_db_query::A_JOIN]['asset_property'][$order_by_alias] = 'LEFT JOIN {asset_property} AS '. $order_by_alias . (substr($order_by_alias_full, -strlen('.val_char_sort')) == '.val_char_sort' ? ' FORCE INDEX ('. VPX_IDX_KEY_ASSET_PROP_VAL_CHAR .')' : '') .' ON '. $order_by_alias .'.asset_id=a.asset_id AND '. $order_by_alias .'.prop_id='. $a_asset_property['prop_id'];
	      $a_query_2[mediamosa_db_query::A_JOIN]['asset_property'][$order_by_alias] = sprintf('LEFT JOIN {asset_property} AS %s ON %s.prop_id = %d AND %s.asset_id = a.asset_id', $order_by_alias, $order_by_alias, $a_asset_property['prop_id'], $order_by_alias);

	      // add to where to include only descriptions from this property for ordering (if property)
	      $a_query_2[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY][mediamosa_db_query::WHERE_OR][] = $order_by_alias_full .' IS NULL';

	      if ($a_asset_property['type'] == VPX_PROP_DEF_TYPE_CHAR) {
	        $a_query_2[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY][mediamosa_db_query::WHERE_OR][] = $order_by_alias_full .'=''';
	      }
	    }
	    else {
	      $a_query_2[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY][mediamosa_db_query::WHERE_OR][] = $order_by_alias_full .' IS NULL';

	      if ($a_funcparam['a_sort'][self::ORDER_BY]['type'] == VPX_TYPE_ALPHANUM) {
	        $a_query_2[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY][mediamosa_db_query::WHERE_OR][] = $order_by_alias_full .'=''';
	      }
	    }

	    if ($i_found_rows < $a_query[mediamosa_db_query::OFFSET]) {
	      // Offset starts beyond the end the 1st query.
	      $a_query_2[mediamosa_db_query::OFFSET] = $a_query[mediamosa_db_query::OFFSET] - $i_found_rows; // correct position
	      // limit stays the same.
	    }
	    else {
	      // offset is inside the 1st query
	      $a_query_2[mediamosa_db_query::OFFSET] = 0;
	      $a_query_2[mediamosa_db_query::LIMIT] = max(($a_query[mediamosa_db_query::LIMIT] - count($a_asset_ids)), 1); // make sure its at least 1
	    }

	    // Create the query
	    $s_query = mediamosa_db_query::select($a_query_2, array(mediamosa_query_db::SQL_CALC_FOUND_ROWS => TRUE));

	    // Ok, fill it up
	    if (array_key_exists('data2', $db_url)) {
	      db_set_active('data2');
	    }
	    else {
	      db_set_active('data');
	    }
	    $db_result = vpx_db_query($s_query);
	    $s_query = 'SELECT found_rows()';
	    $db_result_rows = db_query($s_query);
	    db_set_active();

	    // Count the total of the NULL query
	    $i_found_rows += db_result($db_result_rows);

	    while (count($a_asset_ids) < $a_query[mediamosa_db_query::LIMIT] && ($a_row = db_fetch_array($db_result))) {
	      $a_asset_ids[] = $a_row['asset_id'];
	    }
	  }
*/
	  // Just return.
	  if (!count($a_asset_ids)) {
	  	return;
	  }

    $o_mediamosa->item_total_count = $i_found_rows;
    $o_mediamosa->item_offset = $offset;

    if ($return_asset_ids) {
	    foreach ($a_asset_ids as $asset_id) {
	      $o_mediamosa->add_item(array('asset_id' => $asset_id));
	    }
	  }
	  else {
	  	mediamosa_asset::asset_collect($o_mediamosa, $a_asset_ids, $a_app_ids, $aut_user_id, $granted, $a_aut_group_id, $aut_domain, $aut_realm, $is_app_admin, $show_stills, $show_collections);
	  }
  }
}