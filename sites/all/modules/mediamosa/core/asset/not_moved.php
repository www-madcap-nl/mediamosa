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
 * Code that we will not moved (yet).
 */

/*
+--------+--------------------------+---------------+--------+----------------------------------+--------+-------------+
| uri_id | uri                      | response_type | method | function_call                    | status | vpx_version |
+--------+--------------------------+---------------+--------+----------------------------------+--------+-------------+
|     71 | internal/check_existence | text/xml      | GET    | media_management_check_existence | active | 1.1.0       |
+--------+--------------------------+---------------+--------+----------------------------------+--------+-------------+
 */
function media_management_check_existence($a_args) {
  $a_parameters = array(
    'table' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'table'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'cell' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'cell'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'id'),
      'type' => 'skip',
      'required' => TRUE,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  if (vpx_count_rows($a_parameters['table']['value'], array($a_parameters['cell']['value'], $a_parameters['id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    switch ($a_parameters['table']['value']) {
      case "mediafile": return new rest_response(vpx_return_error(ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $a_parameters['id']['value']))); break;
      case "asset": return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $a_parameters['id']['value']))); break;
      default: return new rest_response(vpx_return_error(ERRORCODE_EMPTY_RESULT));
    }
  }
}




/**
 * Delete watermark image
 * Not implemented yet!
 */
function media_management_delete_watermark() {
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));

  $a_parameters = array(
    'watermark_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'watermark_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
  );

  // Validate all of the parameters
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // Check whether is the webservice on
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  $watermark_id = $a_parameters['watermark_id']['value'];

// kijk of de asset bestaat en of het geen sub asset is
  if (!vpx_count_rows("asset", array(
    "asset_id", $asset_id,
    "parent_id", NULL
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $asset_id)));
  }
// controleer of de still bestaat
  if (!vpx_count_rows("mediafile", array("asset_id_root", $asset_id))) {
    return new rest_response(vpx_return_error(ERRORCODE_STILL_NOT_FOUND, array("@asset_id" => $asset_id)));
  }

  // controleer of de gebruiker rechten heeft om de still te verwijderen
  // get owner info
  db_set_active('data');
  $asset_app_id = db_result(db_query("SELECT app_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  $asset_owner  = db_result(db_query("SELECT owner_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  db_set_active();

  // controleer of de gebruiker rechten heeft om de metadata aan te passen
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $asset_app_id, $asset_owner);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // verwijder de still
  if (($error = _media_management_delete_still($asset_id, $mediafile_id, $still_id)) === TRUE) {
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    return $error;
  }
}
