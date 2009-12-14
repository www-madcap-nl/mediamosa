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
 *
 */


/**
 * REST CALL | GET user/$user_id
 *
 * @param array $a_args
 * @return object
 */
function user_management_get_user($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');

    if ($user_id === FALSE) {
      throw new vpx_exception_error_userman_invalid_user(array("@user_id" => $user_id));
    }

    // Test of user bestaat, zowel in de quota_user als in de coll/ass/mediafile tabellen (owner_id)
    db_set_active('data');

    $query = sprintf(
      "SELECT COUNT(*) AS total FROM " .
      " ((SELECT name FROM {quota_user} AS qu WHERE qu.name = '%s' AND qu.app_id=%d) " .
      "     UNION (SELECT owner_id FROM {collection} AS c WHERE c.owner_id = '%s' AND c.app_id = %d) " .
      "     UNION (SELECT owner_id FROM {asset} AS a WHERE a.owner_id = '%s' AND a.app_id = %d) " .
      "     UNION (SELECT owner_id FROM {mediafile} AS m WHERE m.owner_id = '%s' AND m.app_id = %d) " .
      " ) tmp",
      addslashes($user_id), $app_id,
      addslashes($user_id), $app_id,
      addslashes($user_id), $app_id,
      addslashes($user_id), $app_id
    );

    $total = intval(db_result(db_query($query)));
    db_set_active();

    if (!$total) {
      throw new vpx_exception_error_userman_invalid_user(array("@user_id" => $user_id));
    }

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    // Haal quota user
    db_set_active('data');
    $a_user = db_fetch_array(db_query("SELECT group_id, created, changed FROM {quota_user} WHERE name = '%s' AND app_id = %d", $user_id, $app_id));
    db_set_active();

    if ($a_user === FALSE) {
      $a_user = array(
        "group_id" => "",
        "created" => "",
        "changed" => "",
      );
    }

    $a_quota = _user_management_get_quota($app_id, $user_id, $a_user['group_id']);

    // owner is unclear: use user for output
    $a_quota['user_quota_mb'] = (int)$a_quota['owner_quota_mb'];
    $a_quota['group_quota_mb'] = (int)$a_quota['group_quota_mb']; // int so we have '0' as value when empty
    unset($a_quota['owner_quota_mb']);

    $diskspace_free = _user_management_check_user_quota($app_id, $user_id, $a_user['group_id'], TRUE);

    $a_diskspace_used = array(
      'user' => _user_management_get_diskspace('owner', $user_id, $app_id),
      'group' => _user_management_get_diskspace('group', $a_user['group_id'], $app_id),
      'app' => _user_management_get_diskspace('app', $app_id, $app_id)
    );

    $a_result = array_merge(
      $a_quota,
      array(
        'app_diskspace_used_mb' => $a_diskspace_used['app']['diskspace_used_mb'],
        'group_diskspace_used_mb' => $a_diskspace_used['group']['diskspace_used_mb'],
        'user_diskspace_used_mb' => $a_diskspace_used['user']['diskspace_used_mb'],
        'quota_available_mb' => $diskspace_free,
        'user_over_quota' => ($diskspace_free < 0 ? "true" : "false")
      ),
      $a_user
    );

    $o_rest_reponse->add_item($a_result);
    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * REST CALL | GET group/$group_id
 *
 * @param array $a_args
 * @return object
 */
function user_management_get_group($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'group_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE
    ),
  );
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  //test of group bestaat, zowel in de quota_group als in de coll/ass/mediafile tabellen (group_id)
  db_set_active('data');
  if ($a_parameters["group_id"]["value"] !== FALSE) {
    $query = sprintf(
      "SELECT count(*) aantal FROM " .
      " ((SELECT group_id FROM quota_group qg WHERE qg.group_id = '%s' AND qg.app_id=%d) " .
      "     UNION (SELECT group_id FROM collection c WHERE c.group_id = '%s' AND c.app_id = %d) " .
      "     UNION (SELECT group_id FROM asset a WHERE a.group_id = '%s' AND a.app_id = %d) " .
      "     UNION (SELECT group_id FROM mediafile m WHERE m.group_id = '%s' AND m.app_id = %d) " .
      " ) tmp",
      $a_parameters['group_id']['value'], $a_parameters['app_id']['value'],
      $a_parameters['group_id']['value'], $a_parameters['app_id']['value'],
      $a_parameters['group_id']['value'], $a_parameters['app_id']['value'],
      $a_parameters['group_id']['value'], $a_parameters['app_id']['value']
    );
    $aantal = db_result(db_query($query));
    if (!($aantal >= 1)) {
      return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_GROUP, array("@group_id" => $a_parameters['group_id']['value'])));
    }
  }
  db_set_active();

  $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

  $quota = _user_management_get_quota($a_parameters['app_id']['value'], "", $a_parameters['group_id']['value']);
  unset($quota['owner_quota_mb']);

  db_set_active('data');
  $a_group = db_fetch_array(db_query("SELECT created, changed FROM {quota_group} WHERE group_id = '%s' AND app_id = %d", $a_parameters['group_id']['value'], $a_parameters['app_id']['value']));
  if (!$a_group) {
    $a_group = array();
  }
  db_set_active();

  $diskspace_free = _user_management_check_user_quota($a_parameters['app_id']['value'], "", $a_parameters['group_id']['value'], TRUE);

  $diskspace_used = array(
    'group' => _user_management_get_diskspace('group', $a_parameters['group_id']['value'], $a_parameters['app_id']['value']),
    'app' => _user_management_get_diskspace('app', $a_parameters['app_id']['value'], $a_parameters['app_id']['value'])
  );
  $o_rest_reponse->add_item(
    array_merge(
      $quota,
      array(
        'app_diskspace_used_mb' => $diskspace_used['app']['diskspace_used_mb'],
        'group_diskspace_used_mb' => $diskspace_used['group']['diskspace_used_mb'],
        'quota_available_mb' => $diskspace_free,
        'group_over_quota' => ($diskspace_free < 0) ? "true" : "false"
      ),
      $a_group
    )
  );
  return $o_rest_reponse;
}

/**
 * REST CALL | GET user
 *
 * @param array $a_args
 * @return object
 */
function user_management_list_user($a_args) {
  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, TRUE, 10, 1, MAX_RESULT_COUNT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', TYPE_GROUP_ID);

    return _user_management_fetch_items(
      'users',
      vpx_funcparam_get_value($a_funcparam, 'app_id'),
      vpx_funcparam_get_value($a_funcparam, 'offset'),
      vpx_funcparam_get_value($a_funcparam, 'limit'),
      vpx_funcparam_get_value($a_funcparam, 'group_id')
    );
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

function user_management_list_group($a_args) {
  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, TRUE, 10, 1, MAX_RESULT_COUNT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);

    return _user_management_fetch_items(
      'groups',
      vpx_funcparam_get_value($a_funcparam, 'app_id'),
      vpx_funcparam_get_value($a_funcparam, 'offset'),
      vpx_funcparam_get_value($a_funcparam, 'limit')
     );
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


function _user_management_fetch_items($subject, $app_id, $offset = NULL, $limit = NULL, $group_id = NULL) {

  $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

  $is_users = ($subject == 'users');
  $element = $is_users ? 'owner_id' : 'group_id';

  foreach(array("mediafile", "asset", "collection") as $table) {
    $a_union[] = sprintf("(SELECT DISTINCT a.%s FROM %s AS a WHERE a.app_id = %d and a.%s IS NOT NULL)", $element, $table, $app_id, $element);
  }

  $a_union[] = sprintf(
    "(SELECT DISTINCT a.%s FROM %s AS a WHERE a.app_id = %d)",
    $is_users ? 'name AS owner_id' : 'group_id',
    $is_users ? 'quota_user' : 'quota_group',
    $app_id
  );

  $a_select = array("qu.quotum", "qu.created", "qu.changed"/*, "ca.quota * 1024 AS app_quota_mb"*/);

 // $join_group = "LEFT JOIN vpx_data.{quota_group} qg ON ca.id = qg.app_id AND qg.group_id = tmp.group_id ";
  //$join_user =  "LEFT JOIN vpx_data.{quota_user} qu ON ca.id = qu.app_id AND qu.name = tmp.owner_id ";

// stel de query samen
  $default_db = vpx_getting_dbs('default');
  $query = sprintf(
    "SELECT SQL_CALC_FOUND_ROWS DISTINCT tmp.%s,".
    implode(",", $a_select) .
    "\nFROM (". implode(" \nUNION ", $a_union) .") AS tmp ".
    "\nLEFT JOIN %s AS qu ON qu.%s = tmp.%s AND qu.app_id = %d ".
    "\nLEFT JOIN $default_db.{client_applications} AS ca ON ca.id=%d ".
  //  "\n%s".
    "%s%s%s",
    ($is_users ? 'owner_id AS user_id, group_id' : 'group_id'),
    ($is_users ? 'quota_user' : 'quota_group'),
    ($is_users ? 'name' : 'group_id'),
    ($is_users ? 'owner_id' : 'group_id'),
    $app_id,
    $app_id,// vpx.{client_applications}
 //   ($is_users ? $join_user : $join_group),
    !is_null($group_id) ? sprintf(" WHERE group_id='%s'", db_escape_string($group_id)) : "",
    !is_null($limit) ? sprintf(" LIMIT %d", $limit) : "",
    !is_null($offset) ? sprintf(" OFFSET %d", $offset) : ""
  );

  if (!is_null($offset)) {
    $o_rest_reponse->item_offset = $offset;
  }

/*
 return db_fetch_array(db_query(
    "SELECT quota * 1024 AS app_quota_mb, qg.quotum AS group_quota_mb, qu.quotum AS owner_quota_mb ".
    "FROM vpx.{client_applications} ca ".
    "LEFT JOIN vpx_data.{quota_group} qg ON ca.id = qg.app_id AND qg.group_id = '%s' ".
    "LEFT JOIN vpx_data.{quota_user} qu ON ca.id = qu.app_id AND qu.name = '%s' ".
    "WHERE ca.id = %d LIMIT 1",
    $group_id,
    $user_id,
    $app_id
  ));
*/
//print "<pre>";
//print htmlspecialchars(print_r(str_replace(array("{", "}"), "", $query), TRUE));
//print "</pre>";

// voer de query uit
  db_set_active("data");
  $db_result = db_query($query);
  $o_rest_reponse->item_total_count = db_result(db_query("SELECT found_rows()"));
  db_set_active();

// haal de info uit de DB en voeg dat aan de rest_response toe
  while ($dbrow = db_fetch_array($db_result)) {
    $o_rest_reponse->add_item($dbrow);
  }

  if (!$o_rest_reponse->item_count) {
    $o_rest_reponse->set_result(vpx_return_error(ERRORCODE_EMPTY_RESULT));
  }

  return $o_rest_reponse;
}


/**
 * REST CALL | POST group/create
 *
 */
function user_management_create_group($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
    'quotum' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'quotum', 0),
      'type' => 'int'
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id'),
      'type' => TYPE_GROUP_ID,
      'required' => TRUE
    )
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of de group bestaat
  if (vpx_count_rows("quota_group", array("group_id", $a_parameters['group_id']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_GROUP_EXISTS, array("@group_id" => $a_parameters['group_id']['value'])));
  }

  db_set_active('data');
  db_query(
    "INSERT INTO {quota_group} (app_id, group_id, quotum) VALUES (%d, '%s', '%s')",
    $a_parameters['app_id']['value'],
    $a_parameters['group_id']['value'],
    $a_parameters['quotum']['value']
  );
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * REST CALL | POST group/$group/delete
 *
 */
function user_management_delete_group($a_args) {
  $a_parameters = array(
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'group_id'),
      'type' => TYPE_GROUP_ID,
      'required' => TRUE
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    )
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of de group bestaat
  if (!vpx_count_rows("quota_group", array("group_id", $a_parameters['group_id']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_GROUP, array("@group_id" => $a_parameters['group_id']['value'])));
  }

// kijk of de group nog users bevat
  if (vpx_count_rows("quota_user", array("group_id", $a_parameters['group_id']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_GROUP_NOT_EMPTY));
  }

// verwijder de group
  db_set_active('data');
  db_query(
    "DELETE FROM {quota_group} WHERE app_id = %d AND group_id = '%s'",
    $a_parameters['app_id']['value'],
    $a_parameters['group_id']['value']
  );
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * REST CALL | POST user/create
 *
 */
function user_management_create_user($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
    'user' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user'),
      'type' => TYPE_USER_ID,
      'required' => TRUE
    ),
    'quotum' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'quotum', 0),
      'type' => 'int'
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id', ""),
      'type' => TYPE_GROUP_ID
    )
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  if ($a_parameters["group_id"]["value"] !== "") {
// kijk of de group bestaat
    if (!vpx_count_rows("quota_group", array("group_id", $a_parameters['group_id']['value'], "app_id", $a_parameters['app_id']['value']))) {
      return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_GROUP, array("@group_id" => $a_parameters['group_id']['value'])));
    }
  }
// kijk of de user reeds bestaat
  if (vpx_count_rows("quota_user", array("name", $a_parameters['user']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_USER_EXISTS));
  }

  db_set_active('data');
  db_query(
    "INSERT INTO {quota_user} (group_id, app_id, name, quotum) VALUES ('%s', %d, '%s', %d)",
    $a_parameters['group_id']['value'],
    $a_parameters['app_id']['value'],
    $a_parameters['user']['value'],
    $a_parameters['quotum']['value']
  );
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * REST CALL | POST user/$user/delete
 *
 */
function user_management_delete_user($a_args) {
  $a_parameters = array(
    'user' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'user'),
      'type' => TYPE_USER_ID,
      'required' => TRUE
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of de user bestaat
  if (!vpx_count_rows("quota_user", array("name", $a_parameters['user']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_USER));
  }

// verwijder de user
  db_set_active('data');
  db_query(
    "DELETE FROM {quota_user} WHERE name='%s' AND app_id='%d'",
    $a_parameters["user"]["value"],
    $a_parameters["app_id"]["value"]
  );
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

function user_management_update_user($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'user', VPX_TYPE_USER_ID, TRUE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'group_id', VPX_TYPE_GROUP_ID);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'quotum', VPX_TYPE_INT);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user = vpx_funcparam_get_value($a_funcparam, 'user');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');
    $quotum = vpx_funcparam_get_value($a_funcparam, 'quotum');

    $isset_group_id = vpx_funcparam_isset($a_funcparam, 'group_id');
    $isset_quotum = vpx_funcparam_isset($a_funcparam, 'quotum');

    if (!$isset_group_id && !$isset_quotum) {
      throw new vpx_exception_error_empty_result();
    }

    vpx_shared_must_exist("quota_user", array("name" => $user, "app_id" => $app_id));

    if ($isset_group_id) {
      vpx_shared_must_exist("quota_group", array("group_id" => $group_id, "app_id" => $app_id));
      $a_set[] = sprintf("group_id = '%s'", $group_id);
    }

    if ($isset_quotum) {
      $a_set[] = sprintf("quotum = %d", $quotum);
    }

    $a_where[] = sprintf("name = '%s'", $user);
    $a_where[] = sprintf("app_id = %d", $app_id);

    db_set_active("data");
    $result = vpx_db_query("UPDATE {quota_user}". vpx_db_simple_set($a_set) . vpx_db_simple_where($a_where));
    db_set_active();

    if ($result === FALSE) {
      throw new vpx_exception_error_query_error();
    }

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * REST CALL | POST user/$user/set_group
 * Deprecated
 */
function user_management_set_group($a_args) {
  $a_parameters = array(
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id'),
      'type' => VPX_TYPE_GROUP_ID,
      'required' => TRUE
    ),
    'user' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'user'),
      'type' => VPX_TYPE_USER_ID,
      'required' => TRUE
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    )
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of de user bestaat
  if (!vpx_count_rows("quota_user", array("name", $a_parameters['user']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_USER, array("@user_id" => $a_parameters['user']['value'])));
  }

// kijk of de group bestaat
  if (!vpx_count_rows("quota_group", array("group_id", $a_parameters['group_id']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_GROUP, array("@group_id" => $a_parameters['group_id']['value'])));
  }

  db_set_active("data");
  if (db_query("update quota_user set group_id='%s', changed=now() where name='%s' and app_id='%d'",
               $a_parameters["group_id"]["value"], $a_parameters["user"]["value"],
               $a_parameters["app_id"]["value"]) == FALSE) {
    db_set_active();
    return new rest_response(vpx_return_error(ERRORCODE_QUERY_ERROR));
  }
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * REST CALL | POST user/$user/set_quotum
 * Deprecated
 */
function user_management_set_user_quotum($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
    'user' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'user'),
      'type' => TYPE_USER_ID,
      'required' => TRUE
    ),
    'quotum' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'quotum'),
      'type' => 'int',
      'required' => TRUE
    )
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of de user bestaat
  if (!vpx_count_rows("quota_user", array("name", $a_parameters['user']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_USER, array("@user_id" => $a_parameters['user_id']['value'])));
  }

  db_set_active('data');
  if (db_query(
    "UPDATE {quota_user} SET quotum = %d WHERE app_id = %d AND name = '%s'",
    $a_parameters["quotum"]["value"],
    $a_parameters["app_id"]["value"],
    $a_parameters["user"]["value"]
  ) === FALSE) {
    return new rest_response(vpx_return_error(ERRORCODE_QUERY_ERROR));
  }
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * REST CALL | POST group/$group_id/set_quotum
 *
 */
function user_management_set_group_quotum($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'group_id'),
      'type' => TYPE_GROUP_ID,
      'required' => TRUE
    ),
    'quotum' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'quotum'),
      'type' => 'int',
      'required' => TRUE
    )
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of de group bestaat
  if (!vpx_count_rows("quota_group", array("group_id", $a_parameters['group_id']['value'], "app_id", $a_parameters['app_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_GROUP, array("@group_id" => $a_parameters['group_id']['value'])));
  }

  db_set_active('data');
  if (db_query(
    "UPDATE {quota_group} SET quotum = %d WHERE app_id = %d AND group_id = '%s'",
    $a_parameters["quotum"]["value"],
    $a_parameters["app_id"]["value"],
    $a_parameters["group_id"]["value"]
  ) === FALSE) {
    return new rest_response(vpx_return_error(ERRORCODE_QUERY_ERROR));
  }
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * Helper functie voor het ophalen van de totale schrijfruimte van een user/group
 *
 * @param string $used_by
 * @param string $id
 * @return array
 */
function _user_management_get_diskspace($subject, $id, $app_id) {
  db_set_active("data");
  $i_mbytes = db_result(db_query(
    "SELECT SUM(d.filesize) / 1024 / 1024 ".
    "FROM {mediafile} m, {mediafile_metadata} d ".
    "WHERE m.%s_id = '%s' AND m.mediafile_id = d.mediafile_id AND m.app_id = %s",
    $subject,
    $id,
    $app_id
  ));
  db_set_active();
  return array(
    'diskspace_used_mb' => (int)round($i_mbytes),
  );
}

function _user_management_get_quota($app_id, $user_id = "", $group_id = "") {
  $default_db = vpx_getting_dbs('default');
  $data_db = vpx_getting_dbs('data');
  return db_fetch_array(db_query(
    "SELECT quota * 1024 AS app_quota_mb, qg.quotum AS group_quota_mb, qu.quotum AS owner_quota_mb ".
    "FROM $default_db.{client_applications} ca ".
    "LEFT JOIN $data_db.{quota_group} qg ON ca.id = qg.app_id AND qg.group_id = '%s' ".
    "LEFT JOIN $data_db.{quota_user} qu ON ca.id = qu.app_id AND qu.name = '%s' ".
    "WHERE ca.id = %d LIMIT 1",
    $group_id,
    $user_id,
    $app_id
  ));
}

function _user_management_check_user_quota($app_id, $user_id = "", $group_id = "", $return_free = FALSE) {
  $quota = _user_management_get_quota($app_id, $user_id, $group_id); // quota is reported in MB
  foreach (array('app' => $app_id, 'group' => $group_id, 'owner' => $user_id) as $subject => $var) {
    if (($var !== "") || ($var !== null)) {
      $diskspace_used = _user_management_get_diskspace($subject, $var, $app_id);
      $quota[$subject .'_used_mb'] = $diskspace_used['diskspace_used_mb'];
      $quota[$subject .'_free_mb'] = $quota[$subject .'_quota_mb'] - $diskspace_used['diskspace_used_mb'];
      if ($quota[$subject .'_quota_mb'] != 0) { // unlimited quota
        (!isset($quota['quota_available_mb'])) ? $quota['quota_available_mb'] = $quota[$subject .'_free_mb'] : $quota['quota_available_mb'] = min($quota['quota_available_mb'], $quota[$subject .'_free_mb']);
      }
    }
  }

  if ($return_free) {
    return (int)$quota['quota_available_mb'];
  }

  return $quota['quota_available_mb'] >= 0 ? TRUE : FALSE;
}


define("USER_FAV_TYPE_ASSET", "ASSET");
define("USER_FAV_TYPE_COLLECTION", "COLLECTION");

/*

fav_user_id - linked with quota_user->name
app_id - linked with quota_user->app_id
fav_type - either ASSET or COLLECTION (matched with either table asset or collection)
fav_id - linked to either asset_id or coll_id

CREATE TABLE `user_favorites` (
`name` varchar(255) NOT NULL,
`app_id` int(11) NOT NULL,

`fav_type` ENUM ('ASSET', 'COLLECTION') NOT NULL,
`fav_id` varchar(32) character set latin1 collate latin1_general_cs NOT NULL,

PRIMARY KEY  (`name`,`app_id`,`fav_type`,`fav_id`),
KEY (`fav_type`,`fav_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;
*/

/**
 * Create link between user (quota_user) and either asset or collection
 *
 * @param array $a_args
 * @return rest_object
 */
function user_management_favorites_add($a_args) {

  $a_parameters = array(
    'fav_user_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'fav_user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE
    ),
  'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
  'fav_type' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'fav_type'),
      'type' => 'alphanum',
      'required' => TRUE
    ),
   'fav_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'fav_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // kijk of de user bestaat
//  if (!vpx_count_rows("quota_user", array("name", $a_parameters['fav_user_id']['value'], "app_id", $a_parameters['app_id']['value']))) {
//    return new rest_response(vpx_return_error(ERRORCODE_USERMAN_INVALID_USER, array("@user_id" => $a_parameters['fav_user_id']['value'])));
//  }

  // Process the fav type
  switch (strtoupper($a_parameters['fav_type']['value'])) {
    case USER_FAV_TYPE_ASSET:
      // kijk of de asset bestaat en of het geen sub asset is, mag niet linken naar een sub asset
      if (!vpx_count_rows("asset", array(
      "asset_id", $a_parameters['fav_id']['value'],
      "parent_id", NULL
      ))) {
        return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $a_parameters['fav_id']['value'])));
      }
      break;

    case USER_FAV_TYPE_COLLECTION:
      if (!vpx_count_rows("collection", array("coll_id", $a_parameters['fav_id']['value']))) {
        return new rest_response(vpx_return_error(ERRORCODE_COLLECTION_NOT_FOUND, array("@coll_id" => $a_parameters['fav_id']['value'])));
      }
      break;

    default:
      // Any other type is not allowed
      return new rest_response(vpx_return_error(ERRORCODE_INVALID_FAV_TYPE, array("@type" => $a_parameters['fav_type']['value'], "@valid_types" => implode(",", array(USER_FAV_TYPE_ASSET, USER_FAV_TYPE_COLLECTION)))));
  }

  // Check if the link already exists
  db_set_active('data');
  $o_result = db_fetch_object(db_query("SELECT * FROM {user_favorites} WHERE name='%s' AND app_id='%s' AND fav_type='%s' AND fav_id='%s'",
    $a_parameters['fav_user_id']['value'],
    $a_parameters['app_id']['value'],
    $a_parameters['fav_type']['value'],
    $a_parameters['fav_id']['value']
  ));

  if (!$o_result) {
    db_query("INSERT INTO {user_favorites} SET name='%s', app_id='%s', fav_type='%s', fav_id='%s'",
      $a_parameters['fav_user_id']['value'],
      $a_parameters['app_id']['value'],
      $a_parameters['fav_type']['value'],
      $a_parameters['fav_id']['value']
    );
  }
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * Delete the link between user (quota_user) and either asset or collection.
 *
 * @param array $a_args
 * @return rest_object
 */
function user_management_favorites_delete($a_args) {
  $a_parameters = array(
    'fav_user_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'fav_user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
    'fav_type' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'fav_type'),
      'type' => 'alphanum',
      'required' => TRUE
    ),
    'fav_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'fav_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  db_set_active('data');
  db_query("DELETE FROM {user_favorites} WHERE name='%s' AND app_id='%s' AND fav_type='%s' AND fav_id='%s'",
    $a_parameters['fav_user_id']['value'],
    $a_parameters['app_id']['value'],
    $a_parameters['fav_type']['value'],
    $a_parameters['fav_id']['value']
  );
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

function user_management_favorites_list($a_args) {
  switch (strtoupper(vpx_get_parameter_2($a_args['get'], 'fav_type'))) {
    case USER_FAV_TYPE_ASSET:
      return user_management_favorites_list_asset($a_args);
    case USER_FAV_TYPE_COLLECTION:
      return user_management_favorites_list_collection($a_args);
  }

  // Any other type is not allowed
  return new rest_response(vpx_return_error(ERRORCODE_INVALID_FAV_TYPE, array("@type" => vpx_get_parameter_2($a_args['get'], 'fav_type'), "@valid_types" => implode(",", array(USER_FAV_TYPE_ASSET, USER_FAV_TYPE_COLLECTION)))));
  //return vpx_return_error(ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array("@param" => 'fav_type', "@type" => 'string'));
}

function user_management_favorites_list_collection($a_args) {

  if (isset($a_args['uri']['fav_user_id'])) {
    $a_args['get']['fav_user_id'] = $a_args['uri']['fav_user_id'];
  }
  return media_management_get_collection_search($a_args, TRUE);
}

function user_management_favorites_list_asset($a_args) {

  if (isset($a_args['uri']['fav_user_id'])) {
    $a_args['get']['fav_user_id'] = $a_args['uri']['fav_user_id'];
  }

  // just make fav_user_id required
  return media_management_get_asset_search($a_args, TRUE);
}
