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
 * Access layer.
 */

class mediamosa_acl {

  // ------------------------------------------------------------------ Consts.
  const AUT_TYPE_MEDIAFILE = 'MEDIAFILE';
  const AUT_TYPE_ASSET = 'ASSET';
  const AUT_TYPE_COLLECTION = 'COLLECTION';
  const AUT_TYPE_STILL = 'STILL';
  const AUT_TYPE_ASSET_METADATA = 'ASSET_METADATA';

// Current rights
  const RIGHT_ACCESS = 'ACCESS';

  const AUT_NAME_TYPE_DOMAIN = 'DOMAIN';
  const AUT_NAME_TYPE_REALM = 'REALM';
  const AUT_NAME_TYPE_USER = 'USER';
  const AUT_NAME_TYPE_USER_GROUP = 'USER_GROUP';

// Indirect use from aut_app_master_slave, is no longer a type in aut_name(!)
  const AUT_NAME_TYPE_APP = 'APP';

// Link types
  const OBJ_LINK_TYPE_NAME = 'NAME';
  const OBJ_LINK_TYPE_GROUP = 'GROUP';

// Dummy for result rights
  const OBJ_LINK_TYPE_APP = 'APP';

  const MEDIAFILE_IS_PROTECTED_FALSE = 'FALSE';
  const MEDIAFILE_IS_PROTECTED_TRUE = 'TRUE';
  const MEDIAFILE_IS_PROTECTED_USER_USERGROUP = 'USER_USERGROUP';
  const MEDIAFILE_IS_PROTECTED_DOMAIN_REALM = 'DOMAIN_REALM';

  // ------------------------------------------------------------------ Functions (statics).
  /**
   * Returns the join for joining with aut_object table
   *
   * @param enum $MEDIAMOSA_ACL_aut_type
   *  See MEDIAMOSA_ACL_AUT_TYPE_*
   * @param string $on
   * @return enum
   */
  function join_aut_object_get($aut_type) {
    switch ($aut_type) {
      case mediamosa_acl::AUT_TYPE_MEDIAFILE:
        $on = "mf.mediafile_id";
        break;
      case mediamosa_acl::AUT_TYPE_ASSET:
        $on = "a.asset_id";
        break;
      case mediamosa_acl::AUT_TYPE_ASSET_METADATA:
      case mediamosa_acl::AUT_TYPE_COLLECTION:
      case mediamosa_acl::AUT_TYPE_STILL:
      default:
        assert(0);
        return '';
    }

    return sprintf("LEFT JOIN {aut_object} AS aut_obj ON aut_obj.aut_object_type='%s' AND %s=aut_obj.aut_object_id", $aut_type, $on);
  }

/**
 * Returns the join for joining with aut_app_slave table
 *
 * @param enum $aut_type
 *  See AUT_TYPE_*
 * @param string $on
 * @return enum
 */
function join_aut_app_master_slave_get($aut_type) {
  switch ($aut_type) {
    case mediamosa_acl::AUT_TYPE_MEDIAFILE:
      $on = "mf.mediafile_id";
      break;
    case mediamosa_acl::AUT_TYPE_COLLECTION:
      $on = "c.coll_id";
      break;
    case mediamosa_acl::AUT_TYPE_ASSET:
      $on = "a.asset_id";
      break;
    case mediamosa_acl::AUT_TYPE_ASSET_METADATA:
    case mediamosa_acl::AUT_TYPE_STILL:
      assert(0);
      return;
  }

  return sprintf("LEFT JOIN {aut_app_master_slave} AS aut_ms ON aut_ms.aut_object_type='%s' AND %s=aut_ms.aut_object_id", $aut_type, $on);
}

/**
 * Returns the aut_ids of the given domain and realms
 *
 * @param integer/array $mixed_app_id
 * @param string $aut_user_id
 * @param array $a_aut_group_id
 *  These are user_id groups, like mf.group_id
 * @param array $aut_domain
 * @param array $aut_realm
 */
function build_access($mixed_app_id, $aut_user_id = NULL, $a_aut_group_id = NULL, $aut_domain = NULL, $aut_realm = NULL, $a_slaves = NULL) {
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "{aut_name}.aut_name_id";
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "{aut_name}.aut_group_id";
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "{aut_name}.aut_type";
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "{aut_name}.app_id";

  // Prevent mysql choosing the uni index above the needed index.
  $a_query[VPX_DB_QUERY_A_FROM][] = "{aut_name} FORCE INDEX (idx_type_groupid_name_prefix)";

  assert(is_array($a_aut_group_id) || is_null($a_aut_group_id));

  if (empty($a_aut_group_id)) {
    $a_aut_group_id = NULL;
  }
  if ($aut_domain == "") {
    $aut_domain = NULL;
  }
  if ($aut_realm == "") {
    $aut_realm = NULL;
  }

  $empty_access = TRUE;// make sure we do at least one of the wheres besides app_id, or else we will have access on all


  // First the app_id
  if (is_array($mixed_app_id)) {
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["app_id"][VPX_DB_WHERE_OR][] = sprintf("{aut_name}.app_id IN(%s)", implode(",", $mixed_app_id));
  }
  else {
    assert($mixed_app_id);
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["app_id"][] = sprintf("{aut_name}.app_id=%s", $mixed_app_id);
  }
/*
  // Maybe there is a link to the app_id to other objects
  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["app"] = "{aut_name}.aut_type='APP'";
*/

  // Do we have entry for our user_id and is not in a group (future exp. but lets do it now anyway)
  if (!is_null($aut_user_id) && $aut_user_id != "") {
    $empty_access = FALSE;

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["user"][VPX_DB_WHERE_AND][] = "{aut_name}.aut_type='USER'";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["user"][VPX_DB_WHERE_AND][] = "{aut_name}.aut_group_id IS NULL";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["user"][VPX_DB_WHERE_AND][] = sprintf("{aut_name}.aut_name%s", vpx_db_query_value($aut_user_id));
  }

  // Do we have entry for our user_id and is not in a group (group type is never in a group, we will not group groups here)
  if (!is_null($a_aut_group_id) && !empty($a_aut_group_id)) {
    // Make sure there are no 'empty' group_ids in our list, or else we will gain access by mistake
    foreach ($a_aut_group_id as $key => $s_group_id) {
      if (trim($s_group_id) == "") {
        unset($a_aut_group_id[$key]);
        reset($a_aut_group_id); // Hmm lets reset the pointer, dont know if the unset might cause skipping of items.
      }
    }

    if (!empty($a_aut_group_id)) {
      $empty_access = FALSE;

      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["group"][VPX_DB_WHERE_AND][] = "{aut_name}.aut_type='USER_GROUP'";
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["group"][VPX_DB_WHERE_AND][] = "{aut_name}.aut_group_id IS NULL";

      if (count($a_aut_group_id) == 1) {
          $s_group_id = reset($a_aut_group_id);
          $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["group"][VPX_DB_WHERE_AND][] = sprintf("{aut_name}.aut_name%s", vpx_db_query_value($s_group_id));
      }
      else {
        foreach ($a_aut_group_id as $s_group_id) {
          if (!empty($s_group_id)) {
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["group"][VPX_DB_WHERE_AND]["group"][VPX_DB_WHERE_OR][] = sprintf("{aut_name}.aut_name%s", vpx_db_query_value($s_group_id));
          }
        }
      }
    }
  }

  // Does my domain match with any in the listing?
  if (!is_null($aut_domain)) {
    $empty_access = FALSE;

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["domain"][VPX_DB_WHERE_AND][] = "{aut_name}.aut_type='DOMAIN'";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["domain"][VPX_DB_WHERE_AND]['aut_group_id'][VPX_DB_WHERE_OR][] = "{aut_name}.aut_group_id IS NULL";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["domain"][VPX_DB_WHERE_AND]['aut_group_id'][VPX_DB_WHERE_OR][] = "{aut_name}.aut_group_id IS NOT NULL";

    // Split the domain on its '.'
    $a_domain_parts = explode(".", $aut_domain);

    // Now reverse it
    $a_domain_parts = array_reverse($a_domain_parts);

    // Now walk through and each time add one more
    $a_domain_parts_to_join = array(reset($a_domain_parts));

    while (1) {
      $domain_part = next($a_domain_parts);

      $aut_domain = implode(".", array_reverse($a_domain_parts_to_join));
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["domain"][VPX_DB_WHERE_AND]['aut_name'][VPX_DB_WHERE_OR][] = sprintf("{aut_name}.aut_name%s", vpx_db_query_value($aut_domain));

      if ($domain_part === FALSE) {
        break; // from while
      }

      $a_domain_parts_to_join[] = $domain_part; // next part plz.
    }

    // Domain type does not use prefix field so, we dont need to test IS NULL either
  }

  if (!is_null($aut_realm)) {
    $empty_access = FALSE;

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["realm"][VPX_DB_WHERE_AND][] = "{aut_name}.aut_type='REALM'";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["realm"][VPX_DB_WHERE_AND]["aut_group_id"][VPX_DB_WHERE_OR][] = "{aut_name}.aut_group_id IS NULL";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["realm"][VPX_DB_WHERE_AND]["aut_group_id"][VPX_DB_WHERE_OR][] = "{aut_name}.aut_group_id IS NOT NULL";

    // Realm check is special, very special
    $array_to_compare = array();

    // Must contain @, if not, include it
    if (strpos($aut_realm, "@") === FALSE) {
      $aut_realm = "@" . $aut_realm;
    }

    list($aut_prefix, $s_aut_domain) = explode("@", trim($aut_realm));
    if (trim($aut_prefix) == "") {
      $aut_prefix = NULL;
    }

    // do the same thing as we did on domain

    // Split the domain on its '.'
    $a_domain_parts = explode(".", $s_aut_domain);

    // Now reverse it
    $a_domain_parts = array_reverse($a_domain_parts);

    // Now walk through and each time add one more
    $a_domain_parts_to_join = array(reset($a_domain_parts));

    while (1) {
      $aut_realm = "@" . implode(".", array_reverse($a_domain_parts_to_join));

      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["realm"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['aut_name'][VPX_DB_WHERE_AND]["aut_prefix"][VPX_DB_WHERE_OR][] = sprintf("{aut_name}.aut_name%s", vpx_db_query_value($aut_realm));

      $domain_part = next($a_domain_parts);
      if ($domain_part === FALSE) {
        break; // from while
      }

      $a_domain_parts_to_join[] = $domain_part;
    }

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["realm"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['aut_name'][VPX_DB_WHERE_AND][] = "{aut_name}.aut_prefix IS NULL";

    if (!is_null($aut_prefix)) {
      // if prefix was given we also check on the full [name] @ [domain] access
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["realm"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['aut_name_full'][VPX_DB_WHERE_AND][] = sprintf("{aut_name}.aut_name%s", vpx_db_query_value("@" . $s_aut_domain));// its a realm, even though its called domain in its var name
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]["realm"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['aut_name_full'][VPX_DB_WHERE_AND][] = sprintf("{aut_name}.aut_prefix%s", vpx_db_query_value($aut_prefix));
    }
  }

  // If no access was checked with where, then we have no access on any protected object.
  if ($empty_access) {
    return array();
  }

  $s_query = vpx_db_query_select($a_query);

  db_set_active('data');
  $db_result = db_query($s_query);
  db_set_active();

  // When an array was provided for app_id, we also split the result into app_id.
  if (!is_array($mixed_app_id)) {
    $mixed_app_id = array($mixed_app_id);// make array
  }

  // Create empty result
  foreach ($mixed_app_id as $app_id) {
    $a_result[$app_id] = array("a_name" => array(), "a_group" => array());
  }

  while ($a_row = db_fetch_array($db_result)) {
    if (intval($a_row['aut_group_id'])) {
      $a_result[$a_row['app_id']]['a_group'][] = $a_row['aut_group_id'];
    }
    else {
      $a_result[$a_row['app_id']]['a_name'][] = $a_row['aut_name_id'];
    }
  }

  $a_groups = array();
  foreach ($a_result as $app_id => $a_row) {
    if (!count($a_row['a_group']) && !count($a_row['a_name'])) {
      unset($a_result[$app_id]);
      continue;
    }

    // make them unique
    array_unique($a_result[$app_id]['a_group']);
    array_unique($a_result[$app_id]['a_name']);

    $a_groups = array_merge($a_groups, $a_result[$app_id]['a_group']);

    // sort them, is a bit faster for query
    sort($a_result[$app_id]['a_group'], SORT_NUMERIC);
    sort($a_result[$app_id]['a_name'], SORT_NUMERIC);
  }

  // $groups
  array_unique($a_groups);

  // some comments here please.
  // Hier zoeken we alias groupids op basis...
  if (isset($a_slaves) && count($a_groups)) {

    // Collect all names
    db_set_active('data');
    $result = db_query("SELECT aut_group_id, aut_group_name, aut_group_type FROM {aut_group} WHERE aut_group_id IN (%s)", implode(',', $a_groups));
    db_set_active();

    while ($a_group = db_fetch_array($result)) {
      db_set_active('data');
      $result2 = db_query("SELECT app_id, aut_group_id FROM {aut_group} WHERE aut_group_name = '%s' AND aut_group_type = '%s'",
        $a_group['aut_group_name'], $a_group['aut_group_type']);

      //echo sprintf("SELECT app_id, aut_group_id FROM {aut_group} WHERE aut_group_name = '%s' AND aut_group_type = '%s'",
        //$a_group['aut_group_name'], $a_group['aut_group_type'])."<br>";

       //echo  "looping: "; print_r($a_group); echo "<br>";
      db_set_active();

      while ($a_group_2 = db_fetch_array($result2)) {
        if (!isset($a_slaves[$a_group_2['app_id']])) {
          continue;
        }

        $app_master_id = $a_group_2['app_id'];

        $a_result['slaves']['a_group'][$a_group_2['app_id']][] = $a_group_2['aut_group_id'];
         //echo '<pre>' .  __FILE__.'::',__LINE__.'::'; print_r($a_groups); echo '</pre>';
         //echo '<pre>' .  __FILE__.'::',__LINE__.'::'; print_r($a_group_2); echo '</pre>';
        if (!in_array($a_group_2['aut_group_id'], $a_groups)) {
          $a_result['slaves_convert']['a_group'][$a_group_2['app_id']][] = $a_group_2['aut_group_id'];
          //echo "ADD!<br>";
        }

         //echo "<hr>";

      }
    }

    // Collect all names
    db_set_active('data');
    $result = db_query("SELECT aut_group_id, aut_group_name, aut_group_type FROM {aut_group} WHERE aut_group_id IN (%s)", implode(',', $a_groups));
    db_set_active();

    while ($a_group = db_fetch_array($result)) {

       //echo  "looping: "; print_r($a_group); echo "<br>";

      db_set_active('data');
      $result2 = db_query("SELECT app_id, aut_group_id FROM {aut_group} WHERE aut_group_name = '%s' AND aut_group_type = '%s'
        AND app_id <> %d AND aut_group_id IN(%s)",
        $a_group['aut_group_name'], $a_group['aut_group_type'], $app_id, implode(',', $a_groups));

      db_set_active();

      while ($a_group_2 = db_fetch_array($result2)) {

        //echo "group 2: "; print_r($a_group_2); echo "<br>";
        db_set_active('data');
        $result3 = db_query("SELECT aut_group_id FROM {aut_group} WHERE aut_group_name = '%s' AND aut_group_type = '%s' AND app_id = %d",
          $a_group['aut_group_name'], $a_group['aut_group_type'], $app_id);
        db_set_active();

        if ($a_group_3 = db_fetch_array($result3)) {
          //print_r($a_group_3); echo "<br>";

          $a_result['slaves_convert']['a_group'][$app_master_id][] = $a_group_3['aut_group_id'];
        }
      }
    }
  }

  return (count($a_result) ? $a_result : array());
}

/**
 * Creates the sql to include in your sql call on object to test access
 *
 * @param string $aut_type
 *  AUT_TYPE_* ID
 * @param array $a_query
 * @param string $aut_type
 *   see AUT_TYPE_*
 * @param integer $object_id
 *  If given, we check only on this object for rights.
 *  This is also the main switch between a search query and an object search
 *  Specify NULL for non-direct matching.
 * @param integer $app_id
 * @param string $aut_user_id
 * @param array $a_aut_group_id
 * @param string $aut_domain
 * @param string $aut_realm
 * @param array $a_aut_ids
 * @param bool $b_is_slave
 *  optional when providing own auth_ids
 */
function build_access_where(&$a_query, $aut_type, $object_id, $mixed_app_id, $aut_user_id, $a_aut_group_id, $aut_domain, $aut_realm, $a_slaves = NULL, $is_app_admin = FALSE, $do_master_slave_only = FALSE) {
  assert(is_bool($is_app_admin));

  $a_app_ids = $mixed_app_id;
  if (!is_array($a_app_ids)) {
    $a_app_ids = array($a_app_ids);
  }

  if (is_null($a_slaves)) {
    $a_slaves = slave_get($a_app_ids, $aut_type);
  }

  // Also require the rights from the other possible slave app(s)
  $a_app_ids_tmp = $a_app_ids;
  foreach ($a_slaves as $app_id_slave => $a_slave) {
    $a_app_ids_tmp[] = $app_id_slave;
  }

  // Get of this current user the aut_name/aut_group ids so we know his/her access.
  $a_aut_ids = build_access($a_app_ids_tmp, $aut_user_id, $a_aut_group_id, $aut_domain, $aut_realm, $a_slaves);

  if (!is_null($aut_user_id) || (!is_null($a_aut_group_id) && !empty($a_aut_group_id)) || count($a_aut_ids) || count($a_slaves)) {
    switch ($aut_type) {
      case mediamosa_acl::AUT_TYPE_MEDIAFILE:

        $a_query[VPX_DB_QUERY_A_JOIN]["mediafile"] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root";
        // Even if we return all stuff where we have access to, we must still grant only access to master/slave apps
        if (!$do_master_slave_only) {
          // If $do_master_slave_only is TRUE, granted is TRUE, meaning we need to return all assets
          // even if we dont have access. This switch prevents returning all assets of the other
          // slave apps.

          // If we need to check aut_object
          if (count($a_aut_ids)) {
            $a_query[VPX_DB_QUERY_A_JOIN]["aut_object"] = join_aut_object_get($aut_type);
          }

          // allow assets without mediafile for master app_id
          $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["no_mf"][VPX_DB_WHERE_AND][] = sprintf("a.app_id IN(%s)", implode(",", $a_app_ids));
          $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["no_mf"][VPX_DB_WHERE_AND][] = "mf.asset_id IS NULL";

          if (is_null($object_id)) {
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND][] = "mf.is_original_file='TRUE'";
          }
          else {
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND][] = "mf.mediafile_id='". db_escape_string($object_id) ."'";
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND][] = "mf.is_original_file IS NOT NULL";// always TRUE, because its never NULL
          }

          // If we have aut id for the master app, check them
          foreach ($a_app_ids as $app_id) {
            if (isset($a_aut_ids[$app_id]) && count($a_aut_ids[$app_id])) {
              // Master app_id checks
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND][] = "mf.app_id=" . db_escape_string($app_id);

              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR][] = "mf.is_protected='FALSE'";
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][] = "mf.is_protected!='FALSE'";

              if (count($a_aut_ids[$app_id]["a_name"])) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['name'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='NAME'";
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['name'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids[$app_id]["a_name"]) .")";
              }

              if (count($a_aut_ids[$app_id]["a_group"]) || (isset($a_aut_ids['slaves']) && isset($a_aut_ids['slaves']['a_group'][$app_id]))) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='GROUP'";
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids[$app_id]["a_group"]) .")";
              }
            }
            else {
              // allow master app_id mediafiles when not protected only
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND][] = "mf.app_id=" . db_escape_string($app_id);
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR][] = "mf.is_protected='FALSE'";
            }

            if (!is_null($aut_user_id)) {
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]['user_id'] = "mf.owner_id='". db_escape_string($aut_user_id) ."'";
            }
            // group_id
            if (!is_null($a_aut_group_id)) {
              assert(is_array($a_aut_group_id));

              foreach ($a_aut_group_id as $s_group_id) {
                if (!empty($s_group_id)) {
                  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]['group_id'][] = "mf.group_id='". db_escape_string($s_group_id) ."'";
                }
              }
            }
          }
        }
        else {
          // When $do_master_slave_only is TRUE, we have always access to mediafiles of our own app_id(s)

          // Master app_id checks
          // Because the $a_app_ids is in the app_id selection of assets, this one is a bit useles...
          if (count($a_app_ids)) {
          // Master app_id checks
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR]['master_app_id'][VPX_DB_WHERE_AND][] = sprintf("a.app_id IN(%s)", implode("," , $a_app_ids));
          }
        }

        // possible slaves
        if (count($a_slaves)) {
          $a_query[VPX_DB_QUERY_A_JOIN]["aut_app_master_slave"] = join_aut_app_master_slave_get($aut_type);

          // If we need to check aut_object
          if (count($a_aut_ids)) {
            $a_query[VPX_DB_QUERY_A_JOIN]["aut_object"] = join_aut_object_get($aut_type);
          }

          foreach ($a_slaves as $app_id_slave => $a_slave) {
            assert($app_id_slave);// should not happen
            // Only for app_ids that are present

            // Must be same app_id for slaves
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = "a.app_id=" . db_escape_string($app_id_slave);

            // Any case, a link to app must be present if we allow access to the MF
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = sprintf("aut_ms.app_master_id IN(%s)", implode(",", $a_app_ids));
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = sprintf("aut_ms.app_slave_id=%d", $app_id_slave);

            if ($do_master_slave_only) {
              // If we only do master/slave, we include all that has been slaved and protected
              continue;
            }

            // 2 cases;
            // 1. The object is protected only app_id (is_protected == FALSE)
            //   then only access if app_id == slave_app_id
            // 2. The object is protected by other rights (is_protected == TRUE)
            //   then only access if app_id == slave_app_id and rights match with other rights

            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_FALSE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_USER_USERGROUP);

            if (isset($a_aut_ids[$app_id_slave]) && count($a_aut_ids[$app_id_slave])) {
/*               if ($_SESSION['DDD']) { */
/*                 echo "IF1"; */
/*               } */
              if (count($a_aut_ids[$app_id_slave]["a_name"])) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);
              }

              // Even if we dont have access we still want it in our result when granted == TRUE, so skip it when granted is TRUE
              if (count($a_aut_ids[$app_id_slave]["a_name"])) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['name'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='NAME'";
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['name'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids[$app_id_slave]["a_name"]) .")";
              }

              /*              if (in_array($app_id_slave, $a_app_ids)) {
                if (count($a_aut_ids[$app_id_slave]["a_group"])) {
                  if (!count($a_aut_ids[$app_id_slave]["a_name"])) {
                    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);
                  }

                  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='GROUP'";
                  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids[$app_id_slave]["a_group"]) .")";
                }
                }*/
            }

            if (isset($a_aut_ids['slaves_convert']) && isset($a_aut_ids['slaves_convert']['a_group'][$app_id_slave])) {
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='GROUP'";
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids['slaves_convert']['a_group'][$app_id_slave]) .")";
            }
          }
        }

        break;
      case mediamosa_acl::AUT_TYPE_ASSET:

        if (!$do_master_slave_only) {
          // If $do_master_slave_only is TRUE, granted is TRUE, meaning we need to return all assets
          // even if we dont have access. This switch prevents returning all assets of the other
          // slave apps.

          // If we need to check aut_object
          if (count($a_aut_ids)) {
            $a_query[VPX_DB_QUERY_A_JOIN]["aut_object"] = join_aut_object_get($aut_type);
          }

          // If we have aut id for the master app, check them
          foreach ($a_app_ids as $app_id) {
            if (isset($a_aut_ids[$app_id]) && count($a_aut_ids[$app_id])) {
              // Master app_id checks
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND][] = "a.app_id=" . db_escape_string($app_id);

              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR][] = "a.is_protected='FALSE'";
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][] = "a.is_protected!='FALSE'";

              if (count($a_aut_ids[$app_id]["a_name"])) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['name'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='NAME'";
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['name'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids[$app_id]["a_name"]) .")";
              }

              if (count($a_aut_ids[$app_id]["a_group"]) || (isset($a_aut_ids['slaves']) && isset($a_aut_ids['slaves']['a_group'][$app_id]))) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='GROUP'";
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids[$app_id]["a_group"]) .")";
              }
            }
            else {

              // allow master app_id mediafiles when not protected only
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND][] = "a.app_id=" . db_escape_string($app_id);
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR][] = "a.is_protected='FALSE'";
            }

            if (!is_null($aut_user_id)) {
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]['user_id'] = "a.owner_id='". db_escape_string($aut_user_id) ."'";
            }
            // group_id
            if (!is_null($a_aut_group_id)) {
              assert(is_array($a_aut_group_id));

              foreach ($a_aut_group_id as $s_group_id) {
                if (!empty($s_group_id)) {
                  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]['group_id'][] = "a.group_id='". db_escape_string($s_group_id) ."'";
                }
              }
            }
          }
        }
        else {
          // When $do_master_slave_only is TRUE, we have always access to mediafiles of our own app_id(s)

          // Master app_id checks
          // Because the $a_app_ids is in the app_id selection of assets, this one is a bit useles...
          if (count($a_app_ids)) {
          // Master app_id checks
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR]['master_app_id'][VPX_DB_WHERE_AND][] = sprintf("a.app_id IN(%s)", implode("," , $a_app_ids));
          }
        }

        // possible slaves
        if (count($a_slaves)) {
          $a_query[VPX_DB_QUERY_A_JOIN]["aut_app_master_slave"] = join_aut_app_master_slave_get($aut_type);

          // If we need to check aut_object
          if (count($a_aut_ids)) {
            $a_query[VPX_DB_QUERY_A_JOIN]["aut_object"] = join_aut_object_get($aut_type);
          }

          foreach ($a_slaves as $app_id_slave => $a_slave) {
            assert($app_id_slave);// should not happen
            // Only for app_ids that are present

            // Must be same app_id for slaves
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = "a.app_id=" . db_escape_string($app_id_slave);

            // Any case, a link to app must be present if we allow access to the MF
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = sprintf("aut_ms.app_master_id IN(%s)", implode(",", $a_app_ids));
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = sprintf("aut_ms.app_slave_id=%d", $app_id_slave);

            if ($do_master_slave_only) {
              // If we only do master/slave, we include all that has been slaved and protected
              continue;
            }

            // 2 cases;
            // 1. The object is protected only app_id (is_protected == FALSE)
            //   then only access if app_id == slave_app_id
            // 2. The object is protected by other rights (is_protected == TRUE)
            //   then only access if app_id == slave_app_id and rights match with other rights

            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_FALSE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_USER_USERGROUP);

            if (isset($a_aut_ids[$app_id_slave]) && count($a_aut_ids[$app_id_slave])) {

              if (count($a_aut_ids[$app_id_slave]["a_name"])) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);
              }

              // Even if we dont have access we still want it in our result when granted == TRUE, so skip it when granted is TRUE
              if (count($a_aut_ids[$app_id_slave]["a_name"])) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['name'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='NAME'";
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['name'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids[$app_id_slave]["a_name"]) .")";
              }

              if (in_array($app_id_slave, $a_app_ids)) {
                if (count($a_aut_ids[$app_id_slave]["a_group"])) {
                  if (!count($a_aut_ids[$app_id_slave]["a_name"])) {
                    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);
                  }

                  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='GROUP'";
                  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids[$app_id_slave]["a_group"]) .")";
                }
              }
            }
            elseif (isset($a_aut_ids['slaves']) && isset($a_aut_ids['slaves']['a_group'][$app_id_slave])) {
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_type='GROUP'";
              $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]["is_protected_TRUE"][VPX_DB_WHERE_AND][VPX_DB_WHERE_OR]['group'][VPX_DB_WHERE_AND][] = "aut_obj.aut_id IN(". implode(",", $a_aut_ids['slaves']['a_group'][$app_id_slave]) .")";
            }
          }
/*
          Slaves do not have access through aut_users or aut_groups (for now).
          if (!is_null($aut_user_id)) {
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]['user_id'] = "a.owner_id='". db_escape_string($aut_user_id) ."'";
          }
          // group_id
          if (!is_null($a_aut_group_id)) {
            assert(is_array($a_aut_group_id));

            foreach ($a_aut_group_id as $s_group_id) {
              if (!empty($s_group_id)) {
                $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["mf"][VPX_DB_WHERE_AND]["org"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND]["is_protected"][VPX_DB_WHERE_OR]['group_id'][] = "a.group_id='". db_escape_string($s_group_id) ."'";
              }
            }
          }
*/
        }

        break;
      case mediamosa_acl::AUT_TYPE_COLLECTION:
        // possible slaves

        // Allow master app always
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR][0][VPX_DB_WHERE_AND][] = sprintf("c.app_id IN(%s)", implode(",", $a_app_ids));

        if (count($a_slaves)) {
          $a_query[VPX_DB_QUERY_A_JOIN]["aut_app_master_slave"] = join_aut_app_master_slave_get($aut_type);

          foreach ($a_slaves as $app_id_slave => $a_slave) {
            // Must be same app_id for slaves
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = "c.app_id=" . db_escape_string($app_id_slave);

            // Any case, a link to app must be present if we allow access to the MF
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = sprintf("aut_ms.app_master_id IN(%s)", implode(",", $a_app_ids));
            $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR][$app_id_slave][VPX_DB_WHERE_AND][] = sprintf("aut_ms.app_slave_id=%d", $app_id_slave);
          }
        }

        break;
      case mediamosa_acl::AUT_TYPE_ASSET_METADATA:
      case mediamosa_acl::AUT_TYPE_STILL:
        assert(0);
        return;
    }
  }
  else {
    switch ($aut_type) {
      case mediamosa_acl::AUT_TYPE_MEDIAFILE:
        // Include the mediafile table for join
        $a_query[VPX_DB_QUERY_A_JOIN]["mediafile"] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root";

        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["no_mf"] = "mf.asset_id IS NULL";
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["app_id"][VPX_DB_WHERE_AND][] = "mf.is_original_file='TRUE'";
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["app_id"][VPX_DB_WHERE_AND][] = sprintf("mf.app_id IN(%s)", implode(",", $a_app_ids));

        if (!$do_master_slave_only) {
          $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["app_id"][VPX_DB_WHERE_AND][] = "mf.is_protected='FALSE'";
        }
        break;
      case mediamosa_acl::AUT_TYPE_ASSET:
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["app_id"][VPX_DB_WHERE_AND][] = sprintf("a.app_id IN(%s)", implode(",", $a_app_ids));

        if (!$do_master_slave_only) {
          $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]["access"][VPX_DB_WHERE_OR]["app_id"][VPX_DB_WHERE_AND][] = "a.is_protected='FALSE'";
        }
        break;
      case mediamosa_acl::AUT_TYPE_COLLECTION:
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['collection'][] = sprintf("c.app_id IN(%s)", implode(",", $a_app_ids));

        break;
      case mediamosa_acl::AUT_TYPE_ASSET_METADATA:
      case mediamosa_acl::AUT_TYPE_STILL:
        assert(0);
        return;
    }
  }
}

/**
 * Check access on objects using given array asset_ids
 *
 * @param string $aut_type
 * @param array $a_asset_ids
 * @param integer $app_id
 * @param string $aut_user_id
 * @param array $a_aut_group_id
 * @param string $s_aut_domain
 * @param string $s_aut_realm
 * @return array
 *  returns the array with asset_id(s) on which we have access on
 */
function access_check_mediafiles($a_asset_ids, $mixed_app_id, $aut_user_id, $a_aut_group_id, $s_aut_domain, $s_aut_realm, $is_app_admin = FALSE) {

  $a_app_ids = $mixed_app_id;
  if (!is_array($a_app_ids)) {
    $a_app_ids = array($a_app_ids);
  }

  $a_query = array();

  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.asset_id";
  $a_query[VPX_DB_QUERY_A_FROM][] = "{asset} AS a";

  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = sprintf("a.asset_id IN('%s')", implode("','", $a_asset_ids));
  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = "(a.parent_id IS NOT NULL OR a.parent_id IS NULL)";

  // Build the access where
  build_access_where($a_query, mediamosa_acl::AUT_TYPE_MEDIAFILE, NULL, $a_app_ids, $aut_user_id, $a_aut_group_id, $s_aut_domain, $s_aut_realm, NULL, $is_app_admin);

  $s_query = vpx_db_query_select($a_query);

  db_set_active('data');// set to data
  $db_result = db_query($s_query);
  db_set_active();

  $a_result = array();
  while ($a_row = db_fetch_array($db_result)) {
    $a_result[$a_row['asset_id']] = $a_row['asset_id'];
  }

  return $a_result;
}

/**
 * Check access on a single object
 *
 * @param string $aut_type
 * @param string $asset_id
 * @param string $object_id
 * @param integer $app_id
 * @param string $aut_user_id
 * @param array $a_aut_group_id
 * @param string $s_aut_domain
 * @param string $s_aut_realm
 *
 * Throws vpx_exception_error_access_denied (ERRORCODE_NOT_AUTHORIZED) when access is denied
 */
function access_check_on_object($aut_type, $asset_id, $object_id, $app_id, $aut_user_id, $a_aut_group_id, $s_aut_domain, $s_aut_realm, $is_app_admin = FALSE) {

  if ($is_app_admin) {
    return;
  }

  assert(!is_null($object_id));
  switch ($aut_type) {
    case mediamosa_acl::AUT_TYPE_MEDIAFILE:

      $a_query = array();

      $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.asset_id";
      $a_query[VPX_DB_QUERY_A_FROM][] = "{asset} AS a";

      // Build the access where
      build_access_where($a_query, $aut_type, $object_id, $app_id, $aut_user_id, $a_aut_group_id, $s_aut_domain, $s_aut_realm, NULL, $is_app_admin);

      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = sprintf("a.asset_id ='%s'", $asset_id);
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = "(a.parent_id IS NOT NULL OR a.parent_id IS NULL)";

      if (!$is_app_admin) {
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_OR][] = "a.is_unappropiate = 'FALSE'";
        if ($aut_user_id) {// if provided, then we must be owner to access unappropiate assets
          $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_OR][] = sprintf("(a.is_unappropiate = 'TRUE' AND a.owner_id='%s')", db_escape_string($aut_user_id));
        }
      }
/*
      niks hier achter, dus hoef niet te matchen met index....
      else {
         just to match index
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_OR][] = "a.isprivate = 'FALSE' OR a.is_unappropiate = 'FALSE' OR a.is_unappropiate = 'TRUE'";// only 3, will match always
      }
*/
      $s_query = vpx_db_query_select($a_query);

      db_set_active('data');
      $db_result = db_query($s_query);
      db_set_active();
      if (db_fetch_array($db_result) === FALSE) {
        throw new vpx_exception_error_access_denied(array('@reason' => 'ACL ???'));
      }

      break;

    case mediamosa_acl::AUT_TYPE_ASSET:

      $a_query = array();

      $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.asset_id";
      $a_query[VPX_DB_QUERY_A_FROM][] = "{asset} AS a";

      // Build the access where
      build_access_where($a_query, $aut_type, $object_id, $app_id, $aut_user_id, $a_aut_group_id, $s_aut_domain, $s_aut_realm, NULL, $is_app_admin);

      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = sprintf("a.asset_id ='%s'", $asset_id);
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = "(a.parent_id IS NOT NULL OR a.parent_id IS NULL)";

      if (!$is_app_admin) {
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_OR][] = "a.is_unappropiate = 'FALSE'";
        if ($aut_user_id) {// if provided, then we must be owner to access unappropiate assets
          $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_OR][] = sprintf("(a.is_unappropiate = 'TRUE' AND a.owner_id='%s')", db_escape_string($aut_user_id));
        }
      }

      $s_query = vpx_db_query_select($a_query);

      db_set_active('data');
      $db_result = db_query($s_query);
      db_set_active();

      if (db_fetch_array($db_result) === FALSE) {
        throw new vpx_exception_error_access_denied();
      }

      break;

    default:
      throw new vpx_exception_error_unexpected_error();
  }
}

/**
 * Return the app_id(s) of EGAs that are slaves of my supplied EGA (master)
 *
 *
 * @param integer $app_id
 *  The app ID.
 * @param string $aut_type
 *  The type of acl (mediamosa_acl::AUT_TYPE_*)
 * @param string $object_id (optional)
 *  The ID of object you want to know slaves on.
 * @return array
 *  returns array with info about the slaves
 */
function slave_get($mixed_app_id, $mediamosa_acl::aut_type, $object_id = NULL) {
  $a_app_id = $mixed_app_id;
  if (!is_array($a_app_id)) {
    $a_app_id = array($a_app_id);
  }

  db_set_active('data');
  if (isset($object_id)) {
    $db_result = db_query("SELECT app_slave_id FROM {aut_app_master_slave} WHERE aut_object_type='%s' AND aut_object_id = '%s' AND app_master_id IN(%s) GROUP BY app_slave_id", $aut_type, $object_id, implode(",", $a_app_id));
  }
  else {
    $db_result = db_query("SELECT app_slave_id FROM {aut_app_master_slave} WHERE aut_object_type='%s' AND app_master_id IN(%s) GROUP BY app_slave_id", $aut_type, implode(",", $a_app_id));
  }
  db_set_active();

  $array_result = array();
  while ($a_row = db_fetch_array($db_result)) {
    $array_result[$a_row["app_slave_id"]] = array("app_id" => $a_row["app_slave_id"]);
  }

  return $array_result;
}

/**
 * Check if the object has a valid slave record when it's a foreign object
 */
function read_single_object($type, $id, $app_ids) {
  if (!is_array($app_ids)) {
    $app_ids = array($app_ids);
  }

  switch ($type) {
    case mediamosa_acl::AUT_TYPE_ASSET:

      // Basic app_id check
      $db = db_set_active('data');
      $asset_id_app_id = db_result(db_query("SELECT app_id FROM {asset} WHERE asset_id = '%s'", $id));
      db_set_active($db);

      if (in_array($asset_id_app_id, $app_ids)) {
        break;
      }

      // Master/slave check (based on mediafiles!)
      // @todo: Hhmm in current context with rights on asset, this might not be the right approache.
      $db = db_set_active('data');
      $resource = db_query(
        "SELECT 1 FROM {asset} AS a
        JOIN {mediafile} AS m ON a.asset_id = m.asset_id_root
        JOIN {aut_app_master_slave} AS ms ON ms.aut_object_type = 'MEDIAFILE'
          AND ms.aut_object_id = m.mediafile_id
          AND (ms.app_master_id IN (%s) OR ms.app_slave_id IN (%s))
        WHERE a.asset_id = '%s'",
        implode(",", $app_ids), implode(",", $app_ids),
        $id
      );
      db_set_active($db);
      if (!db_result($resource)) {
        throw new vpx_exception_error_access_denied(array('@reason' => 'application not allowed'));
      }
      break;

    case mediamosa_acl::AUT_TYPE_MEDIAFILE:
      // Basic app_id check
      $db = db_set_active('data');
      $mediafile_id_app_id = db_result(db_query("SELECT app_id FROM {mediafile} WHERE mediafile_id = '%s'", $id));
      db_set_active($db);

      if (in_array($mediafile_id_app_id, $app_ids)) {
        break;
      }

      // Master/slave check
      $db = db_set_active('data');
      $resource = db_query(
        "SELECT 1 FROM {mediafile} AS m
        JOIN {aut_app_master_slave} AS ms ON ms.aut_object_type = 'MEDIAFILE' AND ms.aut_object_id = m.mediafile_id AND ms.app_master_id IN (%s)
        WHERE m.mediafile_id = '%s'",
        implode(",", $app_ids),
        $id
      );
      db_set_active($db);

      if (!db_result($resource)) {
        throw new vpx_exception_error_access_denied(array('@reason' => 'application not allowed'));
      }
      break;

    default:
      assert(0); // should not happen, but when it does, we will know.
      throw new vpx_exception_error_access_denied(array('@reason' => 'application not allowed'));
  }
}
