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
 * This function lists all mediafiles of the given asset id
 *
 * @param string $app_id
 * @param string $asset_id
 * @return: array
 */
/*
function media_management_get_asset_mediafiles($a_args) {
  try {
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'tag', VPX_TYPE_STRING, FALSE, '');
    vpx_funcparam_add($a_funcparam, $a_args, 'is_still', VPX_TYPE_BOOL, FALSE, 'FALSE');

    return _media_management_search_mediafiles($a_funcparam);
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}
*/

/**
 * This function retrieves all info about the given mediafile id
 *
 * @param string $app_id
 * @param string $asset_id
 * @return: array
 */
/*
function media_management_get_mediafile($a_args) {

  try {
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'mediafile_id', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);

    $mediafile_id = vpx_funcparam_get_value($a_funcparam, 'mediafile_id', "");
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');

    vpx_shared_must_exist("mediafile", array("mediafile_id" => $mediafile_id));

    // ACL check
    vpx_acl_read_single_object(VPX_ACL_AUT_TYPE_MEDIAFILE, $mediafile_id, $app_id);

    $mediafiles = array($mediafile_id);

    // get the media file
    return _media_management_return_mediafile_list($mediafiles, $app_id);
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}
*/

/*
function media_management_delete_mediafile($a_args) {

  try {
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'mediafile_id', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL);
    vpx_funcparam_add($a_funcparam, $a_args, 'delete', VPX_TYPE_IGNORE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $mediafile_id = vpx_funcparam_get_value($a_funcparam, 'mediafile_id', "");
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));
    $delete = vpx_funcparam_get_value($a_funcparam, 'delete', "");

    // controleer of de webservice aan staat
    vpx_shared_webservice_must_be_active('media_management', $app_id);

    // Mediafile moet bestaan
    vpx_shared_must_exist('mediafile', array("mediafile_id" => $mediafile_id));

    $a_mediafile = vpx_acl_get_data_from_mediafile($mediafile_id);

    // Check rechten
    vpx_acl_owner_check($app_id, $user_id, $a_mediafile["app_id"], $a_mediafile["owner_id"], $is_app_admin);

    // controleer of deze mediafile als enige onder een sub-asset hangt
    db_set_active('data');
    $a_asset = db_fetch_array(
      db_query(
        "SELECT a.parent_id, a.asset_id FROM {asset} AS a ".
        "JOIN {mediafile} AS mf ON a.asset_id = mf.asset_id ".
        "WHERE mf.mediafile_id = '%s'",
        $mediafile_id
      )
    );

    // Tel de aantal mediafiles onder mijn asset.
    $count = (int)db_result(db_query("SELECT COUNT(*) FROM {mediafile} WHERE asset_id = '%s' AND is_still = 'FALSE'", $a_asset['asset_id']));
    db_set_active();

    // Moet een subasset zijn en alleen deze mediafile als kind.
    $b_delete_asset = (!is_null($a_asset['parent_id']) && $count == 1);

    if ($delete == 'cascade') {
      // Deletes the stills
      db_set_active('data');
      $asset_id_root = db_result(db_query("SELECT asset_id_root FROM {mediafile} WHERE mediafile_id = '%s' AND is_still = 'FALSE'", $mediafile_id));
      db_set_active();
      _media_management_delete_still($asset_id_root, $mediafile_id);
    }

    // verwijder de mediafile
    if (!_media_management_delete_mediafile($mediafile_id)) {
      throw new vpx_exception_error_unexpected_error();
    }

    if ($b_delete_asset) {
      if (!_media_management_delete_asset($a_asset['asset_id'])) {
        throw new vpx_exception_error_unexpected_error();
      }
    }

    // update de timestamps van de asset
    _media_management_update_asset_timestamps($a_asset['asset_id']);

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

function media_management_create_mediafile_wrapper($a_args) {
  if (isset($a_args['post']['asset_id'])) {
    $a_args['uri']['asset_id'] = $a_args['get']['asset_id'];
  }

  return media_management_create_mediafile($a_args);
}

function media_management_create_mediafile($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id'),
      'type' => TYPE_GROUP_ID,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'uri' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'uri'),
      'type' => 'stream_uri',
    ),
    'is_downloadable' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_downloadable'),
      'type' => 'bool',
    ),
    'sannas_mount_point' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'sannas_mount_point', SAN_NAS_BASE_PATH),
      'type' => 'skip',
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'mediafile_id'),
      'type' => 'alphanum',
    ),
    'is_original_file' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'is_original_file', 'true'),
      'type' => 'bool',
    ),
    'filename' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'filename'),
      'type' => 'filename',
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_app_admin', 'FALSE'),
      'type' => 'bool',
    ),
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // kijk of is_app_admin gezet is
  $is_app_admin = vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']);

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  // uri en (filename of is_downloadable) kunnen niet samen worden gebruikt
  if (!is_null($a_parameters['uri']['value']) && (!is_null($a_parameters['filename']['value']) || !is_null($a_parameters['is_downloadable']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_MIX_OF_URI_AND_FILE));
  }

  // maak een uniek mediafile_id aan indien niet opgegeven
  if (is_null($a_parameters['mediafile_id']['value'])) {
    $a_parameters['mediafile_id']['value'] = vpx_create_hash($a_parameters['app_id']['value'], $a_parameters['user_id']['value']);
  }

  // kijk of de asset bestaat
  if ($a_parameters['is_original_file']['value'] == 'true') { // bij originele files mag de asset geen sub-asset zijn...
    if (!vpx_count_rows("asset", array("asset_id", $a_parameters['asset_id']['value'], "parent_id", NULL))) {
      return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $a_parameters['asset_id']['value'])));
    }
  }
  else {
    if (!vpx_count_rows("asset", array("asset_id", $a_parameters['asset_id']['value']))) {
      return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $a_parameters['asset_id']['value'])));
    }
  }

  // acl check op het asset
  db_set_active('data');
  $dbrow_asset = db_fetch_array(db_query("SELECT app_id, owner_id, parent_id FROM {asset} where asset_id  = '%s' ", $a_parameters['asset_id']['value']));
  $asset_owner  = $dbrow_asset["owner_id"];
  $asset_app_id = $dbrow_asset["app_id"];
  $asset_id_root = is_null($dbrow_asset["parent_id"]) ? $a_parameters['asset_id']['value'] : $dbrow_asset["parent_id"];

  // Make sure we test owner on the asset root!
  if ($asset_id_root != $a_parameters['asset_id']['value']) {
    $dbrow_asset = db_fetch_array(db_query("SELECT app_id, owner_id FROM {asset} where asset_id  = '%s' ", $asset_id_root));
    $asset_owner  = $dbrow_asset["owner_id"];
    $asset_app_id = $dbrow_asset["app_id"];
  }

  db_set_active();

  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $asset_app_id, $asset_owner, $is_app_admin);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // kijk waar de nieuwe mediafile ondergebracht moet worden

  if ($a_parameters['is_original_file']['value'] == 'true') {
    // originele file, dus een nieuwe sub-asset aanmaken

    // Indien er al een sub-asset bestaat, maak dan een nieuwe aan.
    if (vpx_count_rows("asset", array("parent_id", $a_parameters['asset_id']['value']))) {
      // opgegeven asset is reeds een 'parent asset'
      $a_args = array(
        'internal' => array(
          'parent_id' => $a_parameters['asset_id']['value']
        ),
        'get' => array(
          'app_id' => $a_parameters['app_id']['value'],
          'user_id' => $a_parameters['user_id']['value'],
          'group_id' => $a_parameters['group_id']['value'],
        ),
      );
      $result = media_management_create_asset($a_args);
      $a_parameters['asset_id']['value'] = $result->response['items'][1]['asset_id'];
    }
    elseif (vpx_count_rows("mediafile", array("asset_id", $a_parameters['asset_id']['value'], "is_still", 'FALSE'))) {

      // er bestaat al een mediafile onder de asset. deze moet verplaatst worden naar een niewe 'sub asset'
      for ($i = 0; $i < 2; $i++) { // maak 2 nieuwe sub assets aan
        $a_args = array(
          'internal' => array(
            'parent_id' => $a_parameters['asset_id']['value']
          ),
          'get' => array(
            'app_id' => $a_parameters['app_id']['value'],
            'user_id' => $a_parameters['user_id']['value'],
            'group_id' => $a_parameters['group_id']['value'],
          ),
        );
        $result = media_management_create_asset($a_args);
        $a_sub_asset[$i] = $result->response['items'][1]['asset_id'];
      }

      // verplaats de bestaande mediafile naar een nieuwe asset_id
      db_set_active('data');
      db_query("UPDATE {mediafile} SET asset_id = '%s' WHERE asset_id = '%s'", $a_sub_asset[0], $a_parameters['asset_id']['value']);
      db_set_active();

      // maak een nieuwe mediafile aan op de volgende asset_id
      $a_parameters['asset_id']['value'] = $a_sub_asset[1];
    }
  }
  // Niet orginele file mag alleen onder een hoofdasset worden geplaatst
  elseif (vpx_count_rows("asset", array("parent_id", $a_parameters['asset_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR));
  }

  // stel de query samen
  $a_set = array();
  foreach (
    array(
      'asset_id' => 'asset_id',
      'app_id' => 'app_id',
      'group_id' => 'group_id',
      'owner_id' => 'user_id',
      'is_downloadable' => 'is_downloadable',
      'is_original_file' => 'is_original_file',
      'sannas_mount_point' => 'sannas_mount_point',
      'mediafile_id' => 'mediafile_id',
      'filename' => 'filename',
      'uri' => 'uri') as $column => $param_name) {

    if (isset($a_parameters[$param_name]['value'])) {
      if (!is_null($a_parameters[$param_name]['value'])) {
        $a_set[] = sprintf("%s='%s'", $column, db_escape_string($a_parameters[$param_name]['value']));
      }
    }
  }
  assert(!empty($a_set));
  // Add the root asset_id, even if asset_id already points to the root.
  $a_set[] = sprintf("asset_id_root='%s'", db_escape_string($asset_id_root));
  $query = "INSERT INTO {mediafile} SET ". implode(", ", $a_set);

  // voer het unieke id in de database in
  db_set_active('data');
  db_query($query);
  db_set_active();

  // Set the external
  _media_management_update_is_external($a_parameters['mediafile_id']['value']);

  // retourneer de gegenereerde mediafile_id
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $rest_response->add_item(array(
    "mediafile_id" => $a_parameters['mediafile_id']['value'],
  ));
  return $rest_response;
}


function media_management_update_mediafile($a_args) {
  $a_parameters = array(
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
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'mediafile_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'filename' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'filename'),
      'type' => 'filename',
    ),
    'uri' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'uri'),
      'type' => 'stream_uri',
    ),
    'is_downloadable' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'is_downloadable'),
      'type' => 'bool',
    ),
    'transcode_inherits_acl' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'transcode_inherits_acl', 'FALSE'),
      'type' => 'bool',
    ),
    'tag' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'tag'),
      'type' => VPX_TYPE_IGNORE,
    ),
  );
  if (isset($a_args['internal']) && count($a_args['internal'])) {
    $a_parameters = array_merge($a_parameters, $a_args['internal']);
  }

  // uri en (filename of is_downloadable) kunnen niet samen worden gebruikt
  if (!is_null($a_parameters['uri']['value']) && (!is_null($a_parameters['filename']['value']) || !is_null($a_parameters['is_downloadable']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_MIX_OF_URI_AND_FILE));
  }

  // uri en (filename of is_downloadable) sluiten elkaar uit
  if (!is_null($a_parameters['uri']['value'])) {
    $type = 'uri';
  }
  else {
    $type = 'file';
  }

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  // kijk of de mediafile bestaat
  if (!vpx_count_rows("mediafile", array("mediafile_id", $a_parameters['mediafile_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_MEDIAFILE_NOT_FOUND, array("@mediafile_id" => $a_parameters['mediafile_id']['value'])));
  }

  // controleer de rechten van de gebruiker op de mediafile
  db_set_active('data');
  $mf_app_id = db_result(db_query("SELECT app_id FROM {mediafile} where mediafile_id  = '%s' ", $a_parameters['mediafile_id']['value']));
  $mf_owner  = db_result(db_query("SELECT owner_id FROM {mediafile} where mediafile_id  = '%s' ", $a_parameters['mediafile_id']['value']));
  db_set_active();

  // controleer de gebruiker rechten
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $mf_app_id, $mf_owner);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // controleer of het huidige mediafile type (video of uri) niet veranderd
  db_set_active('data');
  $a_mediafile = db_fetch_array(db_query("SELECT uri, filename, asset_id FROM {mediafile} WHERE mediafile_id = '%s'", $a_parameters['mediafile_id']['value']));
  db_set_active();
  if ((!is_null($a_mediafile['uri']) && $type === 'file') || (!is_null($a_mediafile['filename']) && $type === 'uri')) {
    return new rest_response(vpx_return_error(ERRORCODE_CHANGE_URI_AND_FILE)); // verbouwing van type mediafile
  }

  // update de timestamps van de asset
  _media_management_update_asset_timestamps($a_mediafile['asset_id']);

  // stel de query samen
  $a_update = array();
  foreach (array('sannas_mount_point', 'is_original_file', 'filename', 'uri', 'is_downloadable', 'transcode_inherits_acl', 'tag') as $subject) {
    if (isset($a_parameters[$subject]['value'])) {
      if (!is_null($a_parameters[$subject]['value'])) {
        $a_update[] = sprintf("%s = '%s'", $subject, db_escape_string($a_parameters[$subject]['value']));
      }
    }
  }

  // controleer of er wel iets veranderd wordt
  if (count($a_update) == 0) {
    return new rest_response(vpx_return_error(ERRORCODE_NO_CHANGES));
  }

  $query = sprintf("UPDATE {mediafile} SET %s WHERE mediafile_id = '%s'", implode(", ", $a_update), $a_parameters['mediafile_id']['value']);

  // voer de wijzigingen door in de database
  db_set_active('data');
  $result = db_query($query);
  db_set_active();

  if (!$result) {
    return new rest_response(vpx_return_error(2346)); // unknown error
  }

  // retourneer een ok
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}


function media_management_internal_update_mediafile($a_args) {
  $a_internal = array(
    'internal' => array(
      'sannas_mount_point' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'sannas_mount_point'),
        'type' => 'skip',
        'required' => FALSE,
      ),
      'is_original_file' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'is_original_file'),
        'type' => 'bool',
        'required' => TRUE,
      ),
    )
  );

  // retourneer een ok
  return media_management_update_mediafile(array_merge($a_args, $a_internal));
}

function media_management_is_playable($mediafile_id) {
  // Check if the file can be transcoded, might be type 'application/x-empty'
  db_set_active("data");
  $is_unplayable = db_result(db_query("SELECT COUNT(*) FROM {mediafile_metadata} WHERE mediafile_id = '%s' AND mime_type = '%s'",
    $mediafile_id,
    VPX_MIME_TYPE_APPLICATION_X_EMPTY
  ));
  db_set_active();

  return ($is_unplayable ? "This mediafile has an empty filesize." : NULL);
}

function media_management_is_transcodable($mediafile_id) {
  // Check if the file can be played, might be type 'application/x-empty'
  db_set_active("data");
  $is_untranscodeable = db_result(db_query("SELECT COUNT(*) FROM {mediafile_metadata} WHERE mediafile_id = '%s' AND mime_type = '%s'",
    $mediafile_id,
    VPX_MIME_TYPE_APPLICATION_X_EMPTY
  ));
  db_set_active();

  return ($is_untranscodeable ? "This mediafile has an empty filesize." : NULL);
}
*/

/**
 * count mediafiles
 */
/*
function media_management_mediafile_count($a_args) {
  return _media_management_count_items('mediafile', $a_args);
}
*/
/**
 * Delete watermark image
 * Not implemented yet!
 */
/*
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
*/