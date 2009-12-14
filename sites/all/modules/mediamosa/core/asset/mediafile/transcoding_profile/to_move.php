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
function transcoding_profiles_get_default_transcode_profile($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// zoek de preview_profile_id op
  $response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $response->add_item(db_fetch_array(db_query("SELECT preview_profile_id FROM {client_applications} WHERE id = %d", $a_parameters['app_id']['value'])));
  return $response;
}

/*
 * Start move code vpx_jobscheduler code.
 */

/**
 * Get the transcode list
 */
function _transcoding_profiles_get_transcode_list($a_args) {
  $parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    )
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // Get the listing.
  $db_result = db_query("SELECT * FROM {transcode_profile} WHERE app_id = 0 OR app_id = %d", $parameters['app_id']['value']);

  // Build response.
  $rest_transcodes = new rest_response;

  while ($db_result_row = db_fetch_object($db_result)) {
    $transcode = array();
    $transcode["profile_id"] = $db_result_row->transcode_profile_id;
    $transcode["profile"] = $db_result_row->profile;
    $transcode["default"] = $db_result_row->is_default_profile;
    $transcode["global"] = $db_result_row->app_id > 0 ? 'FALSE' : 'TRUE';
    $rest_transcodes->add_item($transcode);
  }

  // Set the OKAY.
  $rest_transcodes->set_result(vpx_return_error(ERRORCODE_OKAY));

  return $rest_transcodes;
}

/**
 * Get single transcode.
 */
function _transcoding_profiles_get_transcode($a_args) {
  $parameters = array(
      'profile_id' => array(
        'value' => vpx_get_parameter_2($a_args['uri'], 'profile_id'),
        'type' => 'alphanum',
        'required' => TRUE
      ),
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => 'int',
        'required' => TRUE
      )
  );

  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  $rest_transcodes = new rest_response;

  $db_result = db_query("SELECT * FROM {transcode_profile} WHERE transcode_profile_id = %d AND (app_id = 0 OR app_id = %d)",
    $parameters['profile_id']['value'],
    $parameters['app_id']['value']
  );

  $has_result = FALSE;
  while ($db_result_row = db_fetch_object($db_result)) {
    $has_result = TRUE;
    $transcode = array();
    $transcode["profile_name"] = $db_result_row->profile;
    $transcode["profile_id"] = $db_result_row->transcode_profile_id;
    $transcode["default"] = $db_result_row->is_default_profile;
    $transcode["global"] = $db_result_row->app_id > 0 ? 'FALSE' : 'TRUE';
    $transcode["file_extension"] = $db_result_row->file_extension;
    $transcode["created"] = $db_result_row->created;
    $transcode["changed"] = $db_result_row->changed;
    $transcode["version"] = $db_result_row->version;
    $transcode["command"] = $db_result_row->command;
    $transcode["tool"] = $db_result_row->tool;

    $rest_transcodes->add_item($transcode);
  }

  // Status.
  $rest_transcodes->set_result(vpx_return_error($has_result ? ERRORCODE_OKAY : ERRORCODE_EMPTY_RESULT));

  return $rest_transcodes;
}

/**
 * Create transcode profile.
 */
function _transcoding_profiles_create_transcode($a_args) {
  $parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => VPX_TYPE_INT,
      'required' => TRUE,
    ),
    'name' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'name'),
      'type' => VPX_TYPE_STRING,
      'required' => TRUE,
      'length_max' => 100,
    ),
    'tool' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'tool'),
      'type' => VPX_TYPE_STRING,
      'required' => TRUE,
      'length_max' => 10,
    ),
    'default' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'default', 'FALSE'),
      'type' => VPX_TYPE_BOOL,
      'required' => FALSE
    ),
    'version' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'version'),
      'type' => VPX_TYPE_INT,
      'required' => FALSE
    ),
    'file_extension' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'file_extension'),
      'type' => VPX_TYPE_STRING,
      'required' => TRUE,
      'length_max' => 4,
    ),
    'command' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'command'),
      'type' => VPX_TYPE_STRING,
      'required' => TRUE,
      'length_max' => 1000,
    ),
  );

  // Hotfix: Only global app can set default.
  if ($parameters['default']['value'] != 0) {
    $parameters['default']['value'] = 'FALSE';
  }

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  $rest_transcodes = new rest_response;

  // Must not exist.
  if (vpx_shared_count_rows('transcode_profile', array('profile' => $parameters['name']['value'], 'app_id' => $parameters['app_id']['value']), '')) {
    $rest_transcodes->set_result(vpx_return_error(ERRORCODE_TRANSCODE_PROFILE_EXISTS, array('@profile_name' => $parameters['name']['value'])));
    return $rest_transcodes;
  }

  // Reset the defaults to FALSE if a new default is specified.
  if ($parameters['default']['value'] == 'TRUE') {
    db_query(
      "UPDATE {transcode_profile} SET is_default_profile = 'FALSE' WHERE app_id = %d",
      $parameters['app_id']['value']
    );
  }

  db_query(
    "INSERT INTO {transcode_profile}
    (version, profile, is_default_profile, tool, file_extension, command, `changed`, `created`, app_id)
    VALUES
    ('%s', '%s', '%s', '%s', '%s', '%s', NOW(), NOW(), %d)",
    $parameters['version']['value'],
    $parameters['name']['value'],
    $parameters['default']['value'],
    $parameters['tool']['value'],
    $parameters['file_extension']['value'],
    $parameters['command']['value'],
    $parameters['app_id']['value']
  );

  // Get the last inserted ID.
  $last_id = vpx_db_get_last_id();
  assert($last_id);

  // Add Item.
  $rest_transcodes->add_item(array('profile_id' => $last_id));

  // Set Result.
  $rest_transcodes->set_result(vpx_return_error(ERRORCODE_OKAY));

  return $rest_transcodes;
}

/**
 * Update transcode.
 */
function _transcoding_profiles_update_transcode($a_args) {
   // Haal de parameters op ..
  $parameters = array(
      'profile_id' => array(
        'value' => vpx_get_parameter_2($a_args['uri'], 'profile_id'),
        'type' => VPX_TYPE_INT,
        'required' => TRUE,
      ),
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => VPX_TYPE_INT,
        'required' => TRUE,
      ),
      'name' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'name'),
        'type' => VPX_TYPE_STRING,
        'required' => TRUE,
        'length_max' => 100,
      ),
      'tool' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'tool'),
        'type' => VPX_TYPE_STRING,
        'required' => TRUE,
        'length_max' => 10,
      ),
      'default' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'default', !isset($a_args['post']['default']) ? 'FALSE' : 'TRUE'),
        'type' => VPX_TYPE_BOOL,
        'required' => FALSE
      ),
      'version' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'version'),
        'type' => VPX_TYPE_INT,
        'required' => FALSE
      ),
      'file_extension' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'file_extension'),
        'type' => VPX_TYPE_STRING,
        'required' => TRUE,
        'length_max' => 4,
      ),
      'command' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'command'),
        'type' => VPX_TYPE_STRING,
        'required' => TRUE,
        'length_max' => 1000,
      ),
  );

  // Hotfix: Only global app can set default.
  if ($parameters['default']['value'] != 0) {
    $parameters['default']['value'] = 'FALSE';
  }

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  $rest_transcodes = new rest_response;

  // Make sure it exists.
  if (!vpx_shared_count_rows('transcode_profile', array('transcode_profile_id' => $parameters['profile_id']['value'], 'app_id' => $parameters['app_id']['value']), '')) {
    $rest_transcodes->set_result(vpx_return_error(ERRORCODE_TRANSCODE_PROFILE_NOT_FOUND, array('@profile_id' => $parameters['profile_id']['value'])));
    return $rest_transcodes;
  }

  // Reset the defaults to FALSE if a new default is specified
  if ($parameters['default']['value'] == 'TRUE') {
    db_query("UPDATE {transcode_profile} SET is_default_profile = 'FALSE' WHERE app_id = %d",
      $parameters['app_id']['value']);
  }

  // voer de query uit
  db_query(
    "UPDATE {transcode_profile} SET
    version = '%s',
    profile = '%s',
    is_default_profile = '%s',
    tool = '%s',
    file_extension = '%s',
    command = '%s',
    app_id = %d
    WHERE transcode_profile_id = %d",
    $parameters['version']['value'],
    $parameters['name']['value'],
    $parameters['default']['value'],
    $parameters['tool']['value'],
    $parameters['file_extension']['value'],
    $parameters['command']['value'],
    $parameters['app_id']['value'],
    $parameters['profile_id']['value']
  );

  // Add Item.
  $rest_transcodes->add_item(array('profile_id' => $parameters['profile_id']['value']));

  // Voeg de status van de request samen met de lijst van jobs
  $rest_transcodes->set_result(vpx_return_error(ERRORCODE_OKAY));

  return $rest_transcodes;
}


/**
 * Delete transcode per application.
 */
function _transcoding_profiles_delete_transcode($a_args) {

   // Haal de parameters op ..
  $parameters = array(
      'profile_id' => array(
        'value' => vpx_get_parameter_2($a_args['uri'], 'profile_id'),
        'type' => VPX_TYPE_INT,
        'required' => TRUE,
      ),
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => VPX_TYPE_INT,
        'required' => TRUE,
      ),
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // voer de query uit
  db_query(
    "DELETE FROM {transcode_profile} WHERE transcode_profile_id = %d AND app_id = %d",
    $parameters['profile_id']['value'],
    $parameters['app_id']['value']
  );

  $rest_transcodes = new rest_response;

  // Voeg de status van de request samen met de lijst van jobs
  $rest_transcodes->set_result(vpx_return_error(ERRORCODE_OKAY));

  return $rest_transcodes;
}

/**
 * Get transcode parameters
 */
function _transcoding_profiles_transcode_parameter($a_args) {

   // Haal de parameters op ..
  $parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => VPX_TYPE_INT,
      'required' => TRUE,
    ),
  );

  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  $rest_transcodes = new rest_response;

  $db_result = db_query("SELECT * FROM {transcode_mapping}");

  while ($db_result_row = db_fetch_object($db_result)) {
    $transcode = array();
    $transcode["tool"] = $db_result_row->tool;
    $transcode["nice_parameter"] = $db_result_row->nice_parameter;
    $transcode["tool_parameter"] = $db_result_row->tool_parameter;
    $transcode["min_value"] = $db_result_row->min_value;
    $transcode["max_value"] = $db_result_row->max_value;
    $transcode["allowed_value"] = $db_result_row->allowed_value;
    $transcode["default_value"] = $db_result_row->default_value;
    $transcode["required"] = $db_result_row->required;

    $rest_transcodes->add_item($transcode);
  }

  // Voeg de status van de request samen met de lijst van jobs
  $rest_transcodes->set_result(vpx_return_error(ERRORCODE_OKAY));

  return $rest_transcodes;
}
