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
 * /ftp_batch/
 *
 * @param array $a_args
 * @return object
 */
function vpx_ftp_batch_list($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);

    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, TRUE, 10, 1, MAX_RESULT_COUNT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');

    // Arbitrary columns to list.
    //
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "batch_id";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "owner_id";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "group_id";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "vuf";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "started";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "finished";
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "email_address";
    $a_query[VPX_DB_QUERY_A_FROM][] = "{ftp_batch}";

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "app_id=" . $app_id;
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = "batch_id ASC";

    $a_query[VPX_DB_QUERY_I_LIMIT] = vpx_funcparam_get_value($a_funcparam, 'limit');
    $a_query[VPX_DB_QUERY_I_OFFSET] = vpx_funcparam_get_value($a_funcparam, 'offset');

    $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    $s_query = "SELECT found_rows()";
    $db_result_rows = db_query($s_query);
    db_set_active();

    $o_rest_reponse->item_offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $o_rest_reponse->item_total_count = db_result($db_result_rows);

    while ($a_row = db_fetch_array($db_result)) {
      $o_rest_reponse->add_item($a_row);
    }

    if ($o_rest_reponse->item_count === 0) {
      throw new vpx_exception_error_empty_result();
    }

    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * REST CALL | GET
 *
 * /ftp_batch/$batch_id/ | GET
 *
 * @param array $a_args
 * @return object
 */
function vpx_ftp_batch_get($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'batch_id', VPX_TYPE_INT, TRUE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $batch_id = vpx_funcparam_get_value($a_funcparam, 'batch_id');

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    // Test if the ftp batch exists (exception is thrown if it doesn't).
    //
    vpx_shared_must_exist("ftp_batch", array("app_id" => $app_id, "batch_id" => $batch_id));

    db_set_active('data');
    $db_result = db_query("SELECT * FROM {ftp_batch} where batch_id='%d'", $batch_id);
    db_set_active();

    $o_rest_reponse->add_item(db_fetch_array($db_result));
    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * REST CALL | POST
 *
 * @param array $a_args
 * @return object
 */
function vpx_ftp_batch_create($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', TYPE_GROUP_ID, FALSE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $owner_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');

    // The batch_id column auto increments (therefore is not added
    // explicitly).
    //
    db_set_active('data');
    db_query("INSERT INTO {ftp_batch} SET app_id='%d',owner_id='%s',group_id='%s'", $app_id, $owner_id, $group_id);
    $batch_id = vpx_db_get_last_id();
    db_set_active();

    $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
    $o_rest_response->add_item(array(
          'batch_id' => $batch_id,
          'result' => ERRORMESSAGE_OKAY,
          'result_id' => ERRORCODE_OKAY,
          'result_description' => '',
        ));

    return $o_rest_response;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * REST CALL | POST
 *
 * /ftp_batch/$batch_id/ | POST

 * @param array $a_args
 * @return object
 */
function vpx_ftp_batch_update($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', TYPE_GROUP_ID, FALSE);

    vpx_funcparam_add_uri($a_funcparam, $a_args, 'batch_id', VPX_TYPE_INT, TRUE);

    vpx_funcparam_add_post($a_funcparam, $a_args, 'vuf', VPX_TYPE_STRING, TRUE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'started', VPX_TYPE_DATETIME, TRUE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'finished', VPX_TYPE_DATETIME, TRUE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'email_address', VPX_TYPE_STRING, TRUE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'email_contents', VPX_TYPE_STRING, TRUE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $batch_id = vpx_funcparam_get_value($a_funcparam, 'batch_id');
    $owner_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');
    $vuf = vpx_funcparam_get_value($a_funcparam, 'vuf');
    $started = vpx_funcparam_get_value($a_funcparam, 'started');
    $finished = vpx_funcparam_get_value($a_funcparam, 'finished');
    $email_address = vpx_funcparam_get_value($a_funcparam, 'email_address');
    $email_contents = vpx_funcparam_get_value($a_funcparam, 'email_contents');

    // Test if the ftp batch exists (exception is thrown if it doesn't).
    //
    vpx_shared_must_exist("ftp_batch", array("app_id" => $app_id, "batch_id" => $batch_id));

    db_set_active('data');

    db_query(sprintf(
      "UPDATE {ftp_batch} SET owner_id='%s',group_id='%s',vuf='%s',started='%s',finished='%s',email_address='%s',email_contents='%s' WHERE batch_id='%d'",
      db_escape_string($owner_id),
      db_escape_string($group_id),
      db_escape_string($vuf),
      db_escape_string($started),
      db_escape_string($finished),
      db_escape_string($email_address),
      db_escape_string($email_contents), $batch_id));

    db_set_active();

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * REST CALL | POST
 *
 * /ftp_batch/$batch_id/delete | POST
 *
 * @param array $a_args
 * @return object
 */
function vpx_ftp_batch_delete($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);

    vpx_funcparam_add_uri($a_funcparam, $a_args, 'batch_id', VPX_TYPE_INT, TRUE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $batch_id = vpx_funcparam_get_value($a_funcparam, 'batch_id');

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    // Test if the ftp batch exists (exception is thrown if it doesn't).
    //
    vpx_shared_must_exist("ftp_batch", array("app_id" => $app_id, "batch_id" => $batch_id));

    db_set_active('data');

    // Remove all rows with batch_id from {ftp_batch_asset}.
    //
    db_query("DELETE FROM {ftp_batch_asset} WHERE batch_id=%d", $batch_id);

    // Remove row with batch_id from {ftp_batch}.
    //
    db_query("DELETE FROM {ftp_batch} WHERE batch_id=%d", $batch_id);

    db_set_active();

    // Return an OK.
    //
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * REST CALL | GET
 *
 * /ftp_batch/$batch_id/assets/ | GET
 *
 * Get list of assets for batch with batch_id.
 */
function vpx_ftp_batch_assets_get($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);

    vpx_funcparam_add_uri($a_funcparam, $a_args, 'batch_id', VPX_TYPE_INT, TRUE);

    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, TRUE, 10, 1, MAX_RESULT_COUNT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);

    $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $batch_id = vpx_funcparam_get_value($a_funcparam, 'batch_id');

    // Test if the ftp batch exists (exception is thrown if it doesn't).
    //
    vpx_shared_must_exist("ftp_batch", array("app_id" => $app_id, "batch_id" => $batch_id));

    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "asset_id";
    $a_query[VPX_DB_QUERY_A_FROM][] = "{ftp_batch_asset}";

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "batch_id=" . $batch_id;
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = "asset_id ASC";

    $a_query[VPX_DB_QUERY_I_LIMIT] = vpx_funcparam_get_value($a_funcparam, 'limit');
    $a_query[VPX_DB_QUERY_I_OFFSET] = vpx_funcparam_get_value($a_funcparam, 'offset');

    $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

    db_set_active('data');
    $db_result = vpx_db_query($s_query);
    $s_query = "SELECT found_rows()";
    $db_result_rows = db_query($s_query);
    db_set_active();

    $o_rest_reponse->item_offset = vpx_funcparam_get_value($a_funcparam, 'offset');
    $o_rest_reponse->item_total_count = db_result($db_result_rows);

    while ($a_row = db_fetch_array($db_result)) {
      $o_rest_reponse->add_item($a_row);
    }

    if ($o_rest_reponse->item_count === 0) {
      throw new vpx_exception_error_empty_result();
    }

    return $o_rest_reponse;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * REST CALL | POST
 *
 * /ftp_batch/$batch_id/assets/add | POST
 *
 * Add list of assets to batch with batch_id.
 */
function vpx_ftp_batch_assets_add($a_args) {
  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', TYPE_GROUP_ID, FALSE);

    vpx_funcparam_add_uri($a_funcparam, $a_args, 'batch_id', VPX_TYPE_INT, TRUE);

    vpx_funcparam_add_post_array($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM, TRUE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $batch_id = vpx_funcparam_get_value($a_funcparam, 'batch_id');
    $owner_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');
    $asset_ids = vpx_funcparam_get_value($a_funcparam, 'asset_id');

    $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    // Test if the ftp batch exists (exception is thrown if it doesn't).
    //
    vpx_shared_must_exist("ftp_batch", array("app_id" => $app_id, "batch_id" => $batch_id));

    if (!is_array($asset_ids)) {
      $asset_ids = array($asset_ids);
    }

    // Now process all asset_ids that where passed via the asset_id[]
    // parameter construct. There can be lots of these.
    //
    foreach ($asset_ids as $asset_id) {
      $asset = array(
        'asset_id' => array(
          'value' => $asset_id,
          'type' => 'alphanum',
          'required' => TRUE
        ),
      );
      // Check if $asset_id has a valid format. If not then add
      // an error message to the response for the current asset.
      //
      $result_object = vpx_validate($asset);
      if (vpx_check_result_for_error($result_object)) {
        $o_rest_response->add_item(array(
          'asset_id' => $asset_id,
          'result' => $result_object->response['header']['request_result'],
          'result_id' => $result_object->response['header']['request_result_id'],
          'result_description' => $result_object->response['header']['request_result_description'],
        ));
      }
      else {
        // Check if [$asset_id,$app_id] exists in asset table. Treat as fatal,
        // hence throw an exception for this error.
        //
        db_set_active('data');
        $db_result = db_query("SELECT asset_id FROM {asset} where asset_id='%s' AND app_id='%d'",
                              $asset_id,
                              $app_id);
        db_set_active();
        if (! db_fetch_object($db_result)) {
          throw new vpx_exception_error(
            ERRORCODE_VALIDATE_FAILED,
            array("@param" => "asset_id[]=" . $asset_id, "@type" => VPX_TYPE_ALPHANUM));
        }

        // Check if [$asset_id, $batch_id] is not already present. If not
        // then add an error message to the response for the current asset.
        //
        db_set_active('data');
        $db_result = db_query("SELECT id FROM {ftp_batch_asset} where batch_id='%d' AND asset_id='%s'",
                              $batch_id,
                              $asset_id);
        db_set_active();
        if (db_fetch_object($db_result)) {
          $o_rest_response->add_item(array(
            'asset_id' => $asset_id,
            'result' => 'ERRORCODE_VALIDATE_FAILED',
            'result_id' => ERRORCODE_VALIDATE_FAILED,
            'result_description' => 'ERRORCODE_VALIDATE_FAILED'
          ));
        }
        else {
          // Add this asset as row in {ftp_batch_asset} with [$asset_id, $batch_id].
          //
          db_set_active('data');
          db_query("INSERT INTO {ftp_batch_asset} SET asset_id = '%s', batch_id = '%d'", $asset_id, $batch_id);
          db_set_active();
        }
      }
    }

    // Return response, which can contain several error reports (one per asset_id[]).
    //
    return $o_rest_response;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}
