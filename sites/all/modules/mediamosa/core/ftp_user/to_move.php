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
 * @file
 * FTP users module
 */

function ftp_user_list($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('batch_upload', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  $result = new rest_response(
    array(
      "id"          => ERRORCODE_OKAY,
      "status"      => ERRORMESSAGE_OKAY,
      "description" => "FTP users")
  );

  db_set_active("ftp");
  $resultset = db_query("select * from {ftpuser} where eua_id='%d'", (int) $a_parameters['app_id']['value']);
  if (!$resultset) {
    $result = new rest_response(vpx_return_error(ERRORCODE_FTP_ERROR));
  }
  else {
    while ($a_user = db_fetch_array($resultset)) {
      $result->add_item(
        array(
          "userid" => $a_user['userid'],
          "active" => intval($a_user['active']) ? 'true' : 'false',
          "modified" => $a_user['modified'],
        )
      );
    }
  }

  db_set_active();

  return $result;
}

function ftp_user_create($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => VPX_TYPE_INT,
      'required' => TRUE,
    ),
    'user' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'user'),
      'type' => VPX_TYPE_USER_ID,
      'required' => TRUE
    ),
    'password' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'password'),
      'type' => 'skip',
      'required' => TRUE
    )
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('batch_upload', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  $username = $a_parameters["user"]["value"];
  $password = "{md5}". base64_encode(md5($a_parameters["password"]["value"], TRUE));
  $eua_name = sprintf("%03d", (int) $a_parameters['app_id']['value']);

  $data_oke = TRUE;
  if (drupal_substr($username, -3) != $eua_name) {
    $username .= $eua_name;
  }
  if ((drupal_strlen($username) < VPX_FTP_CREDENTIAL_LENGTH) || (drupal_strlen($password) < VPX_FTP_CREDENTIAL_LENGTH)) {
    return new rest_response(vpx_return_error(ERRORCODE_FTP_CREDENTIAL_LENGTH));
  }

  db_set_active("ftp");

  if (($resource = db_query("select * from {ftpuser} where userid='%s'", $username)) == FALSE) {
    db_set_active();
    return new rest_response(vpx_return_error(ERRORCODE_QUERY_ERROR));
  }
  elseif (db_fetch_array($resource) != FALSE) {
    db_set_active();
    return new rest_response(vpx_return_error(ERRORCODE_FTP_USER_EXISTS));
  }

  if (db_query("insert into {ftpuser} values('', '%d', '%s', '%s', '200', '200', '%s', ".
                "'/bin/FALSE', 1, 0, '0000-00-00 00:00:00', '0000-00-00 00:00:00')",
                $a_parameters['app_id']['value'], $username, $password,
                SAN_NAS_BASE_PATH . VPX_FTP_ROOTDIR ."/". $username) == FALSE) {
    $result = new rest_response(vpx_return_error(ERRORCODE_FTP_ERROR));
  }
  else {
    $result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }

  db_set_active();

  return $result;
}

function ftp_user_delete($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => VPX_TYPE_INT,
      'required' => TRUE,
    ),
    'user' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'user'),
      'type' => VPX_TYPE_USER_ID,
      'required' => TRUE
    ),
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('batch_upload', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  db_set_active("ftp");

  if (db_query("delete from {ftpuser} where userid='%s' and eua_id='%s'",
              $a_parameters["user"]["value"],
              $a_parameters['app_id']['value']) == FALSE) {
    $result = new rest_response(vpx_return_error(ERRORCODE_FTP_ERROR));
  }
  else if (db_affected_rows() == 1) {
    $result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    $result = new rest_response(vpx_return_error(ERRORCODE_FTP_UNKNOWN_USER));
  }

  db_set_active();

  return $result;
}

function ftp_user_update($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'user', VPX_TYPE_USER_ID, TRUE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'password', VPX_TYPE_IGNORE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'active', VPX_TYPE_BOOL);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user = vpx_funcparam_get_value($a_funcparam, 'user');
    $password = vpx_funcparam_get_value($a_funcparam, 'password');
    $active = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'active', 'true'));

    vpx_shared_webservice_must_be_active('batch_upload', $app_id);

    $isset_password = vpx_funcparam_isset($a_funcparam, 'password');
    $isset_active = vpx_funcparam_isset($a_funcparam, 'active');

    if (!$isset_password && !$isset_active) {
      throw new vpx_exception_error_empty_result();
    }

    // Moet bestaan
    vpx_shared_must_exist('ftpuser', array("userid" => $user), 'ftp');

    $a_set = array();


    if ($isset_active) {
      $a_set[] = sprintf("active = %d", $active ? 1 : 0);
    }

    $a_set[] = "modified = CURRENT_TIMESTAMP";

    $a_where[] = sprintf("userid='%s'", db_escape_string($user));
    $a_where[] = sprintf("eua_id=%d", $app_id);

    db_set_active("ftp");
    $result = vpx_db_query("UPDATE {ftpuser}". vpx_db_simple_set($a_set) . vpx_db_simple_where($a_where));
    if ($isset_password) {
      $password = "{md5}". base64_encode(md5($password, TRUE));
      db_query("UPDATE {ftpuser} set passwd = '%s' where userid='%s' and eua_id='%d'", $password, db_escape_string($user), $app_id);
    }
    db_set_active();

    if ($result === FALSE) {
      throw new vpx_exception_error_query_error();
    }

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }
}

function ftp_user_get($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'user', VPX_TYPE_USER_ID, TRUE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user = vpx_funcparam_get_value($a_funcparam, 'user');

    vpx_shared_webservice_must_be_active('batch_upload', $app_id);

    $a_where[] = sprintf("userid='%s'", db_escape_string($user));
    $a_where[] = sprintf("eua_id=%d", $app_id);

    db_set_active('ftp');
    $a_user = db_fetch_array(vpx_db_query("SELECT * FROM {ftpuser}". vpx_db_simple_where($a_where)));
    db_set_active();

    if (!$a_user) {
      throw new vpx_exception_error(ERRORCODE_FTP_UNKNOWN_USER);
    }

    $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
    $rest_response->add_item(array(
      "userid" => $a_user['userid'],
      "active" => intval($a_user['active']) ? 'true' : 'false',
      "modified" => $a_user['modified'],
    ));

    return $rest_response;
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }
}

// Deprecated
function ftp_user_change_password($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'user'),
      'type' => 'alphanum',
      'required' => TRUE
    ),
    'password' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'password'),
      'type' => 'skip',
      'required' => TRUE)
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('batch_upload', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  db_set_active("ftp");

  $password = "{md5}". base64_encode(md5($a_parameters["password"]["value"], TRUE));

  if (db_query("update {ftpuser} set passwd='%s' where userid='%s' and eua_id='%s'",
                $password,
                $a_parameters["user"]["value"],
                $a_parameters['app_id']['value']) == FALSE) {
    $result = new rest_response(vpx_return_error(ERRORCODE_FTP_ERROR));
  }
  else if (db_affected_rows() == 1) {
    $result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    $result = new rest_response(vpx_return_error(ERRORCODE_FTP_UNKNOWN_USER));
  }

  db_set_active();

  return $result;
}

// Deprecated
function ftp_user_change_status($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'user'),
      'type' => 'alphanum',
      'required' => TRUE
    ),
    'active' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'active'),
      'type' => 'bool',
      'required' => TRUE)
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('batch_upload', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  db_set_active("ftp");

  if (db_query("update {ftpuser} set active='%d' where userid='%s' and eua_id='%s'",
                (strcasecmp($a_parameters["active"]["value"], "true") == 0) ? 1 : 0,
                $a_parameters["user"]["value"],
                $a_parameters['app_id']['value']) == FALSE) {
    $result = new rest_response(vpx_return_error(ERRORCODE_FTP_ERROR));
  }
  else if (db_affected_rows() == 1) {
    $result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    $result = new rest_response(vpx_return_error(ERRORCODE_FTP_UNKNOWN_USER));
  }

  db_set_active();

  return $result;
}
