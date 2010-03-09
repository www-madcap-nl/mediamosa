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
   * hook_cron
   *
   * Cron functies voor het genereren van statistieken
   *
   */
function vpx_statistics_cron() {

  $last_run = variable_get('vpx_statistics_cron_last_run', 0);

  $now = time();
  $hour24 = date("G", $now);    // 24-hour format of an hour without leading zeros (0 through 23)
  $month= date("n", $now);      // Numeric representation of a month, without leading zeros (1 through 12)
  $year= date("Y", $now);       // A full numeric representation of a year, 4 digits

  // Alleen als het uur tussen 1:00 en 6:00 's nachts is
  // en als de laatste keer meer dan 12 uur geleden is.
  //
  if ((($hour24 >= 1) && ($hour24 < 6)) &&
       ($last_run < $now - (12 * 60 * 60))) {

    // Set last run variable to immediately lock out successive
    // attempts (including those due to this run taking longer
    // than the cron interval).
    //
    variable_set('vpx_statistics_cron_last_run', $now);
    vpx_statistics_calculate_used_diskspace($year, $month);
  }
}


/**
 * Opvragen van een overzicht van de laatste 50 mediafiles.
 * RTO:1
 *
 */
function vpx_statistics_get_newest_mediafiles($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);
    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM_UNDERSCORE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE, VPX_ORDER_BY_DESC);

    // An ega cant supply multi app_ids, only VPX can
    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');

    // First query only retrieves the mediafile_ids
    $a_query[VPX_DB_QUERY_A_FROM][] = "{asset} AS a FORCE INDEX (". VPX_IDX_ASSET_APP_PARENT_PRIV_UNAPP_OWNER_GROUP .")";

    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.asset_id";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.app_id";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.owner_id";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.group_id";

    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "mf.mediafile_id";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "mf.filename";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "mf.created";

    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "mfmd.container_type";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "mfmd.filesize";

    // Correct app_id(s) if given
    if (count($a_app_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("a.app_id IN(%s)", db_escape_string(implode(",", $a_app_id)));
    }
    else {
      // To match index
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "a.app_id > 0";
    }

    // Only parent assets
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "a.parent_id IS NULL";

    // Dont count private assets
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "a.isprivate='FALSE'";

    // Dont count unappropiate assets
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "a.is_unappropiate='FALSE'";

    // Join the mediafiles
    $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON mf.asset_id_root=a.asset_id";

    // MF must be original
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "mf.is_original_file='TRUE'";

    // Join the metadata stuff
    $a_query[VPX_DB_QUERY_A_JOIN]['mediafile_metadata'] = "LEFT JOIN {mediafile_metadata} AS mfmd ON mf.mediafile_id=mfmd.mediafile_id";

    // Allowed Order By List
    $a_order_by = array(
      'created' => array('column' => 'mf.created'),
      'owner_id' => array('column' => 'mf.owner_id'),
      'filename' => array('column' => 'mf.filename'),
      'app_id' => array('column' => 'a.app_id'),
      'filesize' => array('column' => 'mfmd.filesize'),
      'container_type' => array('column' => 'mfmd.container_type'),
      'asset_id' => array('column' => 'a.asset_id'),
      'mediafile_id' => array('column' => 'mf.mediafile_id'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'created';
    }

    // Order by....
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak query
    $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    $s_query = "SELECT found_rows()";
    $db_result_rows = db_query($s_query);
    db_set_active();

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    $o_rest_reponse->item_total_count = db_result($db_result_rows);

    while ($a_row = db_fetch_array($db_result)) {
      $o_rest_reponse->add_item($a_row);
    }

    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Opvragen van een overzicht van de 50 meest populaire mediafiles.
 * RTO:2
 */
function vpx_statistics_get_most_popular_mediafiles($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);

    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM_UNDERSCORE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE, VPX_ORDER_BY_DESC);

    // An ega cant supply multi app_ids, only VPX can
    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');

    $a_query[VPX_DB_QUERY_A_FROM][] = '{statistics_stream_request} AS ssr';

    // Join the mediafiles
    $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf USING(mediafile_id)";

    // Select these....
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.mediafile_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.asset_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.app_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.owner_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.group_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.container_type';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.filesize';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'COUNT(ssr.mediafile_id) AS count';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'mf.filename';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'mf.created';

     // Correct app_id(s) if given
    if (count($a_app_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("ssr.app_id IN(%s)", db_escape_string(implode(",", $a_app_id)));
    }
    else {
      // To match index
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "ssr.app_id > 0";
    }

    $a_query[VPX_DB_QUERY_A_GROUP_BY][] = 'ssr.mediafile_id';

    // Allowed Order By List
    $a_order_by = array(
      'count' => array('column' => 'count'),
      'owner_id' => array('column' => 'ssr.owner_id'),
      'group_id' => array('column' => 'ssr.group_id'),
      'filesize' => array('column' => 'ssr.filesize'),
      'container_type' => array('column' => 'ssr.container_type'),
      'app_id' => array('column' => 'ssr.app_id'),
      'created' => array('column' => 'mf.created'),
      'filename' => array('column' => 'mf.filename'),
      'asset_id' => array('column' => 'ssr.asset_id'),
      'mediafile_id' => array('column' => 'ssr.mediafile_id'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'count';
    }

    // Order by....
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak query
    $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    $s_query = "SELECT found_rows()";
    $db_result_rows = db_query($s_query);
    db_set_active();

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    $o_rest_reponse->item_total_count = db_result($db_result_rows);

    while ($a_row = db_fetch_array($db_result)) {
      $o_rest_reponse->add_item($a_row);
    }

    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Opvragen van een overzicht van de 50 nieuwste collections.
 * RTO:3
 */
function vpx_statistics_get_newest_collections($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);

    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM_UNDERSCORE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE, VPX_ORDER_BY_DESC);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);

    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');

    $a_query[VPX_DB_QUERY_A_FROM][] = '{collection} AS c';

    // Select this
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.coll_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.app_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.owner_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.title';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.description';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.created';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.changed';

    // Correct app_id(s) if given
    if (count($a_app_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("c.app_id IN(%s)", db_escape_string(implode(",", $a_app_id)));
    }
    else {
      // To match index
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "c.app_id > 0";
    }

    // Exclude tests
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "c.testtag = 'FALSE'";

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'c.app_id'),
      'name' => array('column' => 'c.title'),
      'owner_id' => array('column' => 'c.owner_id'),
      'created' => array('column' => 'c.created'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'created';
    }

    // Order by....
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak query
    $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    $s_query = "SELECT found_rows()";
    $db_result_rows = db_query($s_query);
    db_set_active();

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    $o_rest_reponse->item_total_count = db_result($db_result_rows);

    while ($a_row = db_fetch_array($db_result)) {
      $o_rest_reponse->add_item($a_row);
    }

    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Opvragen van een overzicht van de 50 meest populaire collections.
 * RTO:4
 */
function vpx_statistics_get_most_popular_collections($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);

    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM_UNDERSCORE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE, VPX_ORDER_BY_DESC);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);

    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');

    // Main select
    $a_query[VPX_DB_QUERY_A_FROM][] = '{collection} AS c';

    // Join the asset_collection
    $a_query[VPX_DB_QUERY_A_JOIN]['asset_collection'] = "INNER JOIN {asset_collection} AS ac USING(coll_id)";

    // Join the asset_collection
    $a_query[VPX_DB_QUERY_A_JOIN]['statistics_stream_request'] = "INNER JOIN {statistics_stream_request} AS ssr USING(asset_id)";

    // Select this
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.coll_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.title';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.description';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.app_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.owner_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'c.created';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'COUNT(*) AS count';

    // Correct app_id(s) if given
    if (count($a_app_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("c.app_id IN(%s)", db_escape_string(implode(",", $a_app_id)));
    }
    else {
      // To match index
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "c.app_id > 0";
    }

    // Exclude tests
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "c.testtag = 'FALSE'";

    // Group by
    $a_query[VPX_DB_QUERY_A_GROUP_BY][] = "c.coll_id";

    // Allowed Order By List
    $a_order_by = array(
      'count' => array('column' => 'count'),
      'coll_id' => array('column' => 'c.coll_id'),
      'title' => array('column' => 'c.title'),
      'description' => array('column' => 'c.description'),
      'app_id' => array('column' => 'c.app_id'),
      'owner_id' => array('column' => 'c.owner_id'),
      'created' => array('column' => 'c.created'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'count';
    }

    // Order by....
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak query
    $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    $s_query = "SELECT found_rows()";
    $db_result_rows = db_query($s_query);
    db_set_active();

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    $o_rest_reponse->item_total_count = db_result($db_result_rows);

    while ($a_row = db_fetch_array($db_result)) {
      $o_rest_reponse->add_item($a_row);
    }

    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Opvragen van een overzicht van de uploaded mediafiles.
 * STATS:1
 */
function _vpx_statistics_log_file_upload($app_id, $owner_id, $group_id, $file_size) {
  if (vpx_shared_is_simpletest_app($app_id)) {
    return TRUE;
  }

  $a_set = array();
  $a_set[] = sprintf("app_id=%d", $app_id);
  $a_set[] = sprintf("owner_id='%s'", db_escape_string($owner_id));
  $a_set[] = sprintf("group_id='%s'", db_escape_string($group_id));
  $a_set[] = sprintf("file_size=%d", $file_size);

  db_set_active('data');
  $result = vpx_db_query(sprintf("INSERT INTO {statistics_file_upload}%s", vpx_db_simple_set($a_set)));
  db_set_active();
  return $result;
}

/**
 * To log an upload (via an internal call, from the xml-parser).
 * [POST - with GET parameters]
 * STATS:1
 */
function vpx_statistics_set_historical_uploaded_mediafiles($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', VPX_TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', VPX_TYPE_GROUP_ID);
    vpx_funcparam_add($a_funcparam, $a_args, 'file_size', VPX_TYPE_INT);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');
    $file_size = vpx_funcparam_get_value($a_funcparam, 'file_size');

    if (_vpx_statistics_log_file_upload($app_id, $user_id, $group_id, $file_size) === FALSE) {
      return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR));
    }

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

function vpx_statistics_get_historical_uploaded_mediafiles($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);

    vpx_funcparam_add($a_funcparam, $a_args, 'month', VPX_TYPE_INT, TRUE, 1, 1, 12);
    vpx_funcparam_add($a_funcparam, $a_args, 'year', VPX_TYPE_INT, TRUE, 1, 2000, 2099);

    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, TRUE, STATISTICS_DEFAULT_LIMIT, 1, MAX_RESULT_COUNT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_OFFSET);

    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', VPX_TYPE_USER_ID);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', VPX_TYPE_GROUP_ID);

    $year = vpx_funcparam_get_value($a_funcparam, 'year');
    $month = vpx_funcparam_get_value($a_funcparam, 'month');
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset', 0);

    $date_begin = strtotime($year ."-". $month ."-1 00:00:00");
    $date_end   = strtotime("+1 MONTH", $date_begin);

    $a_query = array();
    $a_query[VPX_DB_QUERY_A_FROM][] = "{statistics_file_upload}";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR] = array("app_id", "owner_id", "group_id", "file_size", "timestamp");

    $a_query[VPX_DB_QUERY_A_WHERE][] = "app_id=" . intval($app_id);
    $a_query[VPX_DB_QUERY_A_WHERE][] = "UNIX_TIMESTAMP(timestamp) >= ". $date_begin;
    $a_query[VPX_DB_QUERY_A_WHERE][] = "UNIX_TIMESTAMP(timestamp) < ". $date_end;

    if (!is_null($user_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][] = sprintf("owner_id='%s'", db_escape_string($user_id));
    }

    if (!is_null($group_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][] = sprintf("group_id='%s'", db_escape_string($group_id));
    }

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    $s_query = vpx_db_query_select($a_query);

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    db_set_active();

    if ($db_result === FALSE) {
      throw new vpx_exception_error_query_error();
    }

    $o_result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
    assert($o_result);

    while ($dbrow = db_fetch_array($db_result)) {
      $o_result->add_item($dbrow);
    }

    return $o_result;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Opvragen van een overzicht van de gebruikte schijfruimte.
 * STATS:3
 */
function vpx_statistics_calculate_used_diskspace($year, $month) {
  db_set_active("data");

  // Delete existing entries for the specified [year, month]
  // (but be careful: check the validity of year and month).
  //
  if (isset($year) && is_numeric($year) &&
      isset($month) && is_numeric($month) &&
      (($month >= 1) && ($month <= 12))) {

    db_query(
      "DELETE FROM {statistics_diskspace_used} WHERE YEAR(timestamp)=%d AND MONTH(timestamp)=%d",
      $year,
      $month);
  }

  // Per gebruikersgroep, gebruiker en container
  foreach (array('group_id' => 'group', 'owner_id' => 'user', 'app_id' => 'container') as $subject => $name) {
    $resource = db_query(
      "SELECT sum(m.filesize) / 1024 / 1024 AS diskspace_mb, mf.%s, mf.app_id, m.container_type FROM {mediafile_metadata} m ".
      "JOIN {mediafile} mf ON m.mediafile_id = mf.mediafile_id ".
      "WHERE filesize > 0 AND mf.app_id NOT IN(%s) ".
      "GROUP BY app_id, %s, container_type",
      $subject,
      implode(",", vpx_shared_is_simpletest_app_get()),
      $subject
    );

    // Insert new (or updated) statistics.
    //
    while ($data = db_fetch_array($resource)) {
      db_query(
        "INSERT INTO {statistics_diskspace_used} (app_id, type, keyword, container_type, diskspace) VALUES (%d, '%s', '%s', '%s', %d)",
        $data["app_id"],
        $name,
        $data[$subject],
        $data["container_type"],
        round($data["diskspace_mb"])
      );
    }
  }

  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

function vpx_statistics_get_used_diskspace($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);

    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);
    vpx_funcparam_add($a_funcparam, $a_args, 'month', VPX_TYPE_INT, TRUE, 1, 1, 12);
    vpx_funcparam_add($a_funcparam, $a_args, 'year', VPX_TYPE_INT, TRUE, 1, 2000, 2099);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM_UNDERSCORE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE, VPX_ORDER_BY_ASC);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT, FALSE);
    vpx_funcparam_add($a_funcparam, $a_args, 'type', VPX_TYPE_ENUM, TRUE, NULL, array("container", "group", "user"));

    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');
    $type = vpx_funcparam_get_value($a_funcparam, 'type');
    $month = vpx_funcparam_get_value($a_funcparam, 'month');
    $year = vpx_funcparam_get_value($a_funcparam, 'year');

    $a_query[VPX_DB_QUERY_A_FROM][] = '{statistics_diskspace_used}';

    // Select this
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'container_type';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = sprintf('keyword AS %s_id', db_escape_string($type));
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'type';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'app_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'diskspace AS diskspace_mb';

    // Correct app_id(s) if given
    if (count($a_app_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("app_id IN(%s)", db_escape_string(implode(",", $a_app_id)));
    }
    else {
      // To match index
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = 'app_id > 0';
    }

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("type = '%s'", db_escape_string($type));

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("timestamp BETWEEN '%04d-%02d-01' AND '%04d-%02d-01'", $year, $month, ($month == 12 ? $year + 1 : $year), ($month == 12 ? 1 : $month + 1));

    // Allowed Order By List
    $a_order_by = array(
      'diskspace_mb' => array('column' => 'diskspace_mb'),
      'type' => array('column' => 'type'),
      'app_id' => array('column' => 'app_id'),
      'container_type' => array('column' => 'container_type'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'app_id';
    }

    // Order by....
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak query
    $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    $s_query = "SELECT found_rows()";
    $db_result_rows = db_query($s_query);
    db_set_active();

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    $o_rest_reponse->item_total_count = db_result($db_result_rows);

    while ($a_row = db_fetch_array($db_result)) {
      if ($type == "container") {
        unset($a_row['container_id']);
      }

      $o_rest_reponse->add_item($a_row);
    }

    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Opvragen van een overzicht van aangevraagde streams per maand.
 * STATS:4
 */
function _vpx_statistics_log_requested_stream($mediafile_id, $response_type) {
  db_set_active('data');

// haal alle info van de mediafile op
  $data = db_fetch_array(db_query(
    "SELECT IFNULL(a.parent_id, a.asset_id) AS real_asset_id, m.*, d.filesize, d.container_type FROM {mediafile} m ".
    "LEFT JOIN {mediafile_metadata} d ON m.mediafile_id = d.mediafile_id ".
    "LEFT JOIN {asset} a ON a.asset_id = m.asset_id ".
    "WHERE m.mediafile_id = '%s'",
    $mediafile_id
  ));

// log geen requests van simpletest
  if (vpx_shared_is_simpletest_app($data["app_id"])) {
    return TRUE;
  }

// gebruik de parent asset als asset_id
  $data['asset_id'] = $data['real_asset_id'];

// sla de stream request op in de database
  db_query(
    "INSERT INTO {statistics_stream_request} ".
    "(mediafile_id, asset_id, app_id, owner_id, group_id, filesize, container_type, play_type) VALUES ('%s', '%s', '%s', '%s', '%s', %d, '%s', '%s')",
    $data["mediafile_id"],
    $data["asset_id"],
    $data["app_id"],
    $data["owner_id"],
    $data["group_id"],
    $data["filesize"],
    $data["container_type"],
    $response_type
  );
  $record_id = db_last_insert_id("statistics_stream_request", "id");

// indien de asset in 1 of meerder collecties voorkomt, log deze dan ook
  $resource = db_query(
    "SELECT c.* FROM {collection} AS c, {asset_collection} r ".
    "WHERE c.coll_id = r.coll_id AND r.asset_id = '%s'",
    $data["asset_id"]
  );

  while ($data = db_fetch_array($resource)) {
    db_query(
      "INSERT INTO {statistics_stream_request_collection} ".
      "(request_id, coll_id, title, description, app_id, owner_id, group_id) VALUES (%d, '%s', '%s', '%s', %d, '%s', '%s')",
      $record_id,
      $data["coll_id"],
      $data["title"],
      $data["description"],
      $data["app_id"],
      $data["owner_id"],
      $data["group_id"]
    );
  }

  db_set_active();
  return TRUE;
}

function vpx_statistics_get_requested_streams($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);

    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);
    vpx_funcparam_add($a_funcparam, $a_args, 'month', VPX_TYPE_INT, TRUE, 1, 1, 12);
    vpx_funcparam_add($a_funcparam, $a_args, 'year', VPX_TYPE_INT, TRUE, 1, 2000, 2099);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT, FALSE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM_UNDERSCORE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE, VPX_ORDER_BY_DESC);
    vpx_funcparam_add($a_funcparam, $a_args, 'owner_id', VPX_TYPE_USER_ID);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', VPX_TYPE_GROUP_ID);
    vpx_funcparam_add($a_funcparam, $a_args, 'play_type', VPX_TYPE_ALPHANUM);

    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $month = vpx_funcparam_get_value($a_funcparam, 'month');
    $year = vpx_funcparam_get_value($a_funcparam, 'year');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');
    $play_type = vpx_funcparam_get_value($a_funcparam, 'play_type');
    $owner_id = vpx_funcparam_get_value($a_funcparam, 'owner_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');

    $a_query[VPX_DB_QUERY_A_FROM][] = '{statistics_stream_request}';

    // Select this
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = '*';

    // Correct app_id(s) if given
    if (count($a_app_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("app_id IN(%s)", db_escape_string(implode(",", $a_app_id)));
    }
    else {
      // To match index
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = 'app_id > 0';
    }

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("played BETWEEN '%04d-%02d-01' AND '%04d-%02d-01'", $year, $month, $year, $month + 1);

    if ($owner_id) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("owner_id='%s'", db_escape_string($owner_id));
    }

    if ($group_id) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("group_id='%s'", db_escape_string($group_id));
    }

    if ($play_type) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("play_type='%s'", db_escape_string($play_type));
    }

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'app_id'),
      'owner_id' => array('column' => 'owner_id'),
      'group_id' => array('column' => 'group_id'),
      'filesize' => array('column' => 'filesize'),
      'container_type' => array('column' => 'container_type'),
      'play_type' => array('column' => 'play_type'),
      'played' => array('column' => 'played'),
      'asset_id' => array('column' => 'asset_id'),
      'mediafile_id' => array('column' => 'mediafile_id'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'played';
    }

    // Order by....
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak de query
    $s_query = vpx_db_query_select($a_query);

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    db_set_active();

    if ($db_result === FALSE) {
      throw new vpx_exception_error_query_error();
    }

    $o_result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
    assert($o_result);

    while ($dbrow = db_fetch_array($db_result)) {
      $o_result->add_item($dbrow);
    }

    return $o_result;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Opvragen van een overzicht van aangemaakte streams en collecties
 * STATS:5
 */
function vpx_statistics_get_created_metadata_and_collections() {
}

/**
 * Opvragen van een overzicht van zoek en bladeracties.
 * STATS:6
 */
function vpx_statistics_get_search_queries() {
}

/**
 * Opvragen van een overzicht van populaire streams
 * STATS:7
 */
function vpx_statistics_get_most_popular_streams($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);

    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);

    vpx_funcparam_add($a_funcparam, $a_args, 'begindate', VPX_TYPE_DATE, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'enddate', VPX_TYPE_DATE, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'owner_id', VPX_TYPE_USER_ID);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', VPX_TYPE_GROUP_ID);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT, FALSE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM_UNDERSCORE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE, VPX_ORDER_BY_DESC);

    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $begindate = vpx_funcparam_get_value($a_funcparam, 'begindate');
    $enddate = vpx_funcparam_get_value($a_funcparam, 'enddate');
    $owner_id = vpx_funcparam_get_value($a_funcparam, 'owner_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');

    $a_query[VPX_DB_QUERY_A_FROM][] = '{statistics_stream_request} AS ssr';

    // Join with mediafile
    $a_query[VPX_DB_QUERY_A_JOIN][] = 'LEFT JOIN {mediafile} AS mf USING(mediafile_id)';

    // Select this
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.mediafile_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.owner_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.group_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'mf.filename';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'mf.asset_id_root AS asset_id';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'count(*) AS requested';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'ssr.app_id';

    // Correct app_id(s) if given
    if (count($a_app_id)) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("ssr.app_id IN(%s)", db_escape_string(implode(",", $a_app_id)));
    }
    else {
      // To match index
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = 'ssr.app_id > 0';
    }

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("ssr.played BETWEEN '%s' AND '%s'", $begindate, $enddate);

    if ($owner_id) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("ssr.owner_id='%s'", db_escape_string($owner_id));
    }

    if ($group_id) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("ssr.group_id='%s'", db_escape_string($group_id));
    }

    // Group by
    $a_query[VPX_DB_QUERY_A_GROUP_BY][] = 'ssr.mediafile_id';

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'ssr.app_id'),
      'owner_id' => array('column' => 'ssr.owner_id'),
      'group_id' => array('column' => 'ssr.group_id'),
      'filename' => array('column' => 'mf.filename'),
      'requested' => array('column' => 'requested'),
      'mediafile_id' => array('column' => 'ssr.mediafile_id'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'requested';
    }

    // Order by....
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak de query
    $s_query = vpx_db_query_select($a_query);

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    db_set_active();

    if ($db_result === FALSE) {
      throw new vpx_exception_error_query_error();
    }

    $o_result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
    assert($o_result);

    while ($dbrow = db_fetch_array($db_result)) {
      $o_result->add_item($dbrow);
    }

    return $o_result;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * Opvragen van een overzicht van de meest gezochte woorden
 * STATS:8
 */
function vpx_statistics_get_most_popular_words($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);

    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);

    vpx_funcparam_add($a_funcparam, $a_args, array('begindate', 'startdate'), VPX_TYPE_DATE, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'enddate', VPX_TYPE_DATE, FALSE);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT, FALSE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM_UNDERSCORE);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE, VPX_ORDER_BY_DESC);

    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $begindate = vpx_funcparam_get_value($a_funcparam, 'begindate');
    $enddate = vpx_funcparam_get_value($a_funcparam, 'enddate');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');

    $a_query[VPX_DB_QUERY_A_FROM][] = '{statistics_search_request}';

    // Select this
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'keyword AS word';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'COUNT(keyword) AS count';

    $a_query[VPX_DB_QUERY_A_WHERE][] = sprintf("searched >= '%s'", $begindate);
    if (!is_null($enddate)) {
      $a_query[VPX_DB_QUERY_A_WHERE][] = sprintf("searched < '%s' + INTERVAL 1 MONTH", $enddate);
    }

    $a_query[VPX_DB_QUERY_A_GROUP_BY][] = 'keyword';

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'app_id'),
      'word' => array('column' => 'keyword'),
      'count' => array('column' => 'count'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'count';
    }

    // Order by....
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak de query
    $s_query = vpx_db_query_select($a_query);

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    db_set_active();

    if ($db_result === FALSE) {
      throw new vpx_exception_error_query_error();
    }

    $o_result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
    assert($o_result);

    while ($dbrow = db_fetch_array($db_result)) {
      $o_result->add_item($dbrow);
    }

    return $o_result;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Opvragen van een overzicht van de meest gezochte woorden
 * STATS:9
 */
function vpx_statistics_get_searchrequest($a_args, $is_internal = FALSE) {

  try {
    $is_internal = (arg(0) == "internal" || $is_internal);

    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, !$is_internal);

    vpx_funcparam_add($a_funcparam, $a_args, 'year', VPX_TYPE_DATE, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'month', VPX_TYPE_DATE, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, STATISTICS_DEFAULT_LIMIT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT, FALSE);

    $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $year = vpx_funcparam_get_value($a_funcparam, 'year');
    $month = vpx_funcparam_get_value($a_funcparam, 'month');
    $limit = vpx_funcparam_get_value($a_funcparam, 'limit');
    $offset = vpx_funcparam_get_value($a_funcparam, 'offset');

    $a_query[VPX_DB_QUERY_A_FROM][] = '{statistics_search_request}';

    // Select this
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'keyword AS word';
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'COUNT(keyword) AS count';

    $begindate = date('Y-m-d', mktime(0, 0, 0, $month, 1, $year));

    $a_query[VPX_DB_QUERY_A_WHERE][] = sprintf("searched >= '%s'", $begindate);
    $a_query[VPX_DB_QUERY_A_WHERE][] = sprintf("searched < '%s' + INTERVAL 1 MONTH", $begindate);

    $a_query[VPX_DB_QUERY_A_GROUP_BY][] = 'keyword';

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'app_id'),
      'word' => array('column' => 'keyword'),
      'count' => array('column' => 'count'),
    );

    // Limit...
    $a_query[VPX_DB_QUERY_I_LIMIT] = $limit;

     // Offset...
    $a_query[VPX_DB_QUERY_I_OFFSET] = $offset;

    // Maak de query
    $s_query = vpx_db_query_select($a_query);

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    db_set_active();

    if ($db_result === FALSE) {
      throw new vpx_exception_error_query_error();
    }

    $o_result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
    assert($o_result);

    while ($dbrow = db_fetch_array($db_result)) {
      $o_result->add_item($dbrow);
    }

    return $o_result;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * Wegschrijven van een gezocht woord in de statistics database.
 */
function _vpx_statistics_log_search_query($args, $app_id) {
  if (is_array($app_id)) {
    $app_id = $app_id[0];
  }

// log geen requests van simpletest
  if (vpx_shared_is_simpletest_app($app_id)) {
    return TRUE;
  }

// haal een lijst op van te loggen keys
  $logged_keys = array_keys(_media_management_get_metadata_definitions(array('dublin_core', 'qualified_dublin_core', 'app_'. $app_id)));
  $searchstring = array();

// loop door de get waarden heen en log de relevante
  foreach ($args['get'] as $key => $value) {
    if (in_array($key, $logged_keys)) {
      // als array ingevoerd
      if (is_array($value)) {
        foreach ($value as $single_value) {
          $searchstring[] = $single_value;
        }
      }
      else {
        // gewoon ingevoerd
        $searchstring[] = $value;
      }
    }
  }

  // log het zoekresultaat
  $searchstring = implode(' ', $searchstring);
  if (trim($searchstring) != '') {
    db_set_active('data');
    db_query(
      "INSERT INTO {statistics_search_request} (keyword, app_id)
      VALUES ('%s', %d)",
      $searchstring,
      $app_id
    );
    db_set_active();
  }
}

/**
 * Saves searched CQL keywords
 *
 * @param array $keywords
 * @param int   $app_id
 */
function _vpx_statistics_log_cql_search_query($keywords, $app_id) {
  if (is_array($keywords)) {
    $keywords = implode(' ', $keywords);
    if (trim($keywords) != '') {
      db_set_active('data');
      db_query(
        "INSERT INTO {statistics_search_request} (keyword, app_id)
        VALUES ('%s', %d)",
        $keywords,
        $app_id
      );
      db_set_active();
    }
  }
}

function isomktime($date) {
  $explodeDate = explode("-", $date);
  return mktime(0, 0, 0, $explodeDate[1], $explodeDate[2], $explodeDate[0]);
}
