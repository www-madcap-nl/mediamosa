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



/**********************************************************************
 * Create job functies
 *********************************************************************/

/**
 * Maak een nieuwe job aan.
 * Op basis van het type job wordt ook een hulptabel gevuld.
 * Als dit goed gaat, geef dan een job_id terug, zo niet, geef een foutmelding
 */

function _vpx_jobs_create_new_job($a_args, $a_parameters) {
  //watchdog('server', 'vpx_jobs $a_args'. print_r($a_args, TRUE));
  //watchdog('server', 'vpx_jobs $a_parameters'. print_r($a_parameters, TRUE));

  db_set_active('data');
  $dbrow_result = db_fetch_array(db_query("SELECT app_id, owner_id FROM {mediafile} WHERE mediafile_id = '%s' ", $a_parameters['mediafile_id']['value']));
  db_set_active();

  if ($dbrow_result === FALSE) {
    throw new vpx_exception_error_job_mediafile_not_found(array("@mediafile_id" => $a_parameters['mediafile_id']['value']));
  }

  // controleer de gebruiker rechten
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $dbrow_result["app_id"], $dbrow_result["owner_id"], vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']));
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  $asset_info = _vpx_jobs_get_asset_info($a_parameters['mediafile_id']['value']);

  $create_still = isset($a_parameters['create_still']['value']) ? strtoupper($a_parameters['create_still']['value']) : 'FALSE';
  $still_parameters = array(
    'still_type' => $a_parameters['still_type']['value'],
    'still_per_mediafile' => $a_parameters['still_per_mediafile']['value'],
    'still_every_second' => $a_parameters['still_every_second']['value'],
    'start_frame' => $a_parameters['start_frame']['value'],
    'end_frame' => $a_parameters['end_frame']['value'],
    'size' => $a_parameters['size']['value'],
    'h_padding' => $a_parameters['h_padding']['value'],
    'v_padding' => $a_parameters['v_padding']['value'],
    'tag' => $a_parameters['tag']['value'],
    'frametime' => $a_parameters['frametime']['value'],
    'width' => $a_parameters['width']['value'],
    'height' => $a_parameters['height']['value'],
  );

  // voeg een job toe aan de database
  db_set_active("data");
  db_query("START TRANSACTION");
  $db_result = db_query("
      INSERT INTO {job}
      (asset_id, mediafile_id, owner, app_id, job_type, created, testtag, create_still, still_parameters) VALUES
      ('%s', '%s', '%s', %d, '%s', NOW(), '%s', '%s', '%s')",
      $asset_info["asset_id"], $a_parameters['mediafile_id']['value'],
      $a_parameters['user_id']['value'], $a_parameters['app_id']['value'],
      $a_parameters['job_type']['value'], $a_parameters['testtag']['value'],
      $create_still, serialize($still_parameters));

  if ($db_result === FALSE) {
    db_query("ROLLBACK");
    db_set_active();
    throw new vpx_exception(ERRORCODE_CREATING_JOB);
  }

  $job_id = db_last_insert_id("job", "job_id");

  // als de record succesvol is toegevoegd dan is er een n_job_id ingevuld
  if ($job_id) {
    db_set_active();

    try {
      // op basis van een job type en een transcoding profile wordt een keuze gemaakt om de rest van de gegevens vast te leggen.
      switch ($a_parameters['job_type']['value']) {
        // afhandelen van een transcode job
        case 'TRANSCODE':
          $error = _vpx_jobs_create_new_job_transcode($job_id, $a_args);
          break;
        case 'STILL':
          $error = _vpx_jobs_create_new_job_still($job_id, $a_args);
          break;
        case 'UPLOAD':
          $a_args['get']['job_id'] = $job_id;
          $a_args['get']['app_id'] = $a_parameters['app_id']['value'];
          $a_args['get']['user_id'] = $a_parameters['user_id']['value'];
          $a_args['get']['group_id'] = $a_parameters['group_id']['value'];
          $a_args['post']['retranscode'] = isset($a_parameters['retranscode']['value']) ? $a_parameters['retranscode']['value'] : 'false';
          $a_args['post']['create_still'] = $create_still;
          $a_args['get']['mediafile_id'] = $a_parameters['mediafile_id']['value'];

          $error = _vpx_jobs_create_new_job_upload($a_args);
          break;
      }
    }
    catch (vpx_exception $e) {
      db_set_active("data");
      db_query("ROLLBACK");
      db_set_active();
      throw $e; // rethrow
    }
  }
  else {
    db_query("ROLLBACK");
    db_set_active();
    throw new vpx_exception(ERRORCODE_CREATING_JOB);
  }

  // heeft er zich een error voor gedaan? doe dan een rollback
  db_set_active("data");
  if (isset($error) && vpx_check_result_for_error($error)) {
    db_query("ROLLBACK");
    db_set_active();
    return $error;
  }

  // toevoegen is goed gegaan, geef de job_id terug.
  db_query("COMMIT");
  db_set_active();
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  if ($job_id) {
     $rest_response->add_item(array("job_id" => $job_id));
  }

  return $rest_response;
}

/**********************************************************************
 * Testen van de opgegeven parameters voor een trancode job
 *********************************************************************/



/**********************************************************************
 * Verwijderen van een bestaande job
 *********************************************************************/



function _vpx_jobs_get_asset_info($mediafile_id) {
  $result = array();

  db_set_active("data");
  $db_result = db_query("
      SELECT asset_id FROM {mediafile}
      WHERE mediafile_id = '%s'", $mediafile_id);
  $asset_id = db_result($db_result);
  db_set_active();
  if ($asset_id == "") {
    return $result;
  }
  $result["asset_id"] = $asset_id;

  return $result;
}

/**
 * Geef een lijst terug van de jobs die voor een user in het systeem bestaan.
 * het resultaat bestaat uit de status en per job een sectie
 * Wanneer er een fout optreed wordt er een errorstatus melding teruggegeven
 * In alle andere gevallen wordt een okaystatus met eventuele jobs teruggegeven.
 */
function vpx_jobscheduler_get_user_job_list($a_args) {
  // Haal de parameters op ..
  $parameters = array(
      'user_id' => array(
        'value' => vpx_get_parameter_2($a_args['uri'], 'user_id'),
        'type' => TYPE_USER_ID,
        'required' => TRUE
      ),
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => 'int',
        'required' => TRUE
      ),
      'testtag' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE'),
        'type' => 'alphanum',
      )
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  return vpx_jobs_get_job_list($parameters);

} // end of vpx_jobscheduler_get_job_list_user


/**
 * Geef een lijst terug van de jobs die voor een asset in het systeem bestaan.
 * het resultaat bestaat uit de status en per job een sectie
 * Wanneer er een fout optreed wordt er een errorstatus melding teruggegeven
 * In alle andere gevallen wordt een okaystatus met eventuele jobs teruggegeven.
 */
function vpx_jobscheduler_get_asset_job_list($a_args) {
  // Haal de parameters op ..
  $parameters = array(
      'asset_id' => array(
        'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
        'type' => 'alphanum',
        'required' => TRUE
      ),
      'user_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
        'type' => TYPE_USER_ID,
        'required' => TRUE
      ),
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => 'int',
        'required' => TRUE
      ),
      'testtag' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE'),
        'type' => 'alphanum',
      )
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // kijk of de asset bestaat
  if (!vpx_count_rows("asset", array("asset_id", $parameters['asset_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $parameters['asset_id']['value'])));
  }


  return vpx_jobs_get_job_list($parameters);

} // end of vpx_jobscheduler_get_asset_job_list

/**
 * Geef een lijst terug van de jobs die voor een mediafile in het systeem bestaan.
 * het resultaat bestaat uit de status en per job een sectie
 * Wanneer er een fout optreed wordt er een errorstatus melding teruggegeven
 * In alle andere gevallen wordt een okaystatus met eventuele jobs teruggegeven.
 */
function vpx_jobscheduler_get_mediafile_job_list($a_args) {
  // Haal de parameters op ..
  $parameters = array(
      'mediafile_id' => array(
        'value' => vpx_get_parameter_2($a_args['uri'], 'mediafile_id'),
        'type' => 'alphanum',
        'required' => TRUE
      ),
      'user_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
        'type' => TYPE_USER_ID,
        'required' => TRUE
      ),
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => 'int',
        'required' => TRUE
      ),
      'testtag' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE'),
        'type' => 'alphanum',
      )
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // kijk of de mediafile bestaat
  if (!vpx_count_rows("mediafile", array("mediafile_id", $parameters['mediafile_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@mediafile_id" => $parameters['mediafile_id']['value'])));
  }

  return vpx_jobs_get_job_list($parameters);

} // end of vpx_jobscheduler_get_mediafile_job_list

/**
 * Geef de details terug van een enkele job op basis van het job_id
 * Wanneer er een fout optreed wordt er een errorstatus melding teruggegeven
 * In alle andere gevallen wordt een okaystatus met eventuele jobdetails teruggegeven.
 */
function vpx_jobscheduler_get_job_details($a_args) {
  $parameters = array(
    'job_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'job_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
    ),
    'testtag' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE'),
      'type' => 'alphanum',
    )
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // kijk of de asset bestaat
  // controleer of de job bestaat
  if (!vpx_count_rows("job", array("job_id", $parameters['job_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_JOB_NOT_FOUND, array("@job_id" => $parameters['job_id']['value'])));
  }


  return vpx_jobs_get_job_list($parameters);

} // end of vpx_jobscheduler_get_job_details


/**********************************************************************
 * create job functies
 *********************************************************************/


/**
 * Aanmaken van een nieuwe job
 */
function vpx_jobscheduler_create_new_job($a_args) {
  // Haal de parameters op ..
  //print_r($a_args);
  $parameters = array(
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id', FALSE),
      'type' => TYPE_GROUP_ID,
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'mediafile_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'job_type' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'job_type'),
      'type' => 'job_type',
      'required' => TRUE,
      'custom_error' => ERRORCODE_UNKNOWN_JOB_TYPE
    ),
    'testtag' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE'),
      'type' => 'alphanum'
    ),
    // Stills
    'create_still' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'create_still', 'FALSE'),
      'type' => VPX_TYPE_BOOL
    ),
    'still_type' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'still_type', NULL),
      'type' => VPX_TYPE_ALPHA,
    ),
    'still_per_mediafile' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'still_per_mediafile', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'still_every_second' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'still_every_second', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'start_frame' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'start_frame', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'end_frame' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'end_frame', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'size' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'size', NULL),
      'type' => VPX_TYPE_IGNORE,
    ),
    'h_padding' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'h_padding', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'v_padding' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'v_padding', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'tag' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'tag', NULL),
      'type' => VPX_TYPE_STRING,
    ),
    // Still parameters for backward compatibility
    'frametime' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'frametime', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'width' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'width', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'height' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'height', NULL),
      'type' => VPX_TYPE_INT,
    ),
    // End stills
    'completed_transcoding_url' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'completed_transcoding_url'),
      'type' => VPX_TYPE_URL,
    ),
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // controleer of de ega een job mag starten
  if (!vpx_shared_webservice_is_active('jobs', $parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }
  /*
   * FIX ME
   * Authorisatie
   */

  // controleer of de mediafile bestaat
  if (!vpx_count_rows("mediafile", array("mediafile_id", $parameters['mediafile_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_JOB_MEDIAFILE_NOT_FOUND, array("@mediafile_id" => $parameters['mediafile_id']['value'])));
  }
// controleer of de user niet boven quota zit indien het een transcode of upload is
  if (($parameters['job_type']['value'] === JOBTYPE_UPLOAD) || ($parameters['job_type']['value'] === JOBTYPE_TRANSCODE)) {
    $result = _user_management_check_user_quota($parameters['app_id']['value'], $parameters['user_id']['value'], $parameters['group_id']['value']);
    if (vpx_check_result_for_error($result)) {
      return new rest_response(vpx_return_error(ERRORCODE_QUOTA_REACHED));
    }
  }

  // controleer of het een upload job betreft en er een URI is ingevuld
  // de mediafile_id
  db_set_active("data");
  $has_uri = db_result(db_query("
      SELECT uri IS NULL FROM mediafile WHERE mediafile_id = '%s'",
      $parameters['mediafile_id']['value']
  ));
  db_set_active();

  if ($has_uri != 1) {
    return new rest_response(vpx_return_error(ERRORCODE_CHANGE_URI_AND_FILE, array("@mediafile_id" => $parameters['mediafile_id']['value'])));
  }

  if ($parameters['job_type']['value'] === JOBTYPE_TRANSCODE) {
    $reason = media_management_is_transcodable($parameters['mediafile_id']['value']);

    if (!is_null($reason)) {
      return new rest_response(vpx_return_error(ERRORCODE_CANT_TRANSCODE_MEDIAFILE, array("@mediafile_id" => $parameters['mediafile_id']['value'], "@reason" => $reason)));
    }
  }

  return _vpx_jobs_create_new_job($a_args, $parameters);
}

/**
 * starten van een transcode job
 */
function vpx_jobscheduler_create_new_transcode_job($a_args) {
  // Haal de parameters op ..
  $a_args['post']['job_type'] = JOBTYPE_TRANSCODE;
  $a_args['get']['mediafile_id'] = $a_args['uri']['mediafile_id'];

  return vpx_jobscheduler_create_new_job($a_args);
}

/**
 * starten van een still job
 */
function vpx_jobscheduler_create_new_still_job($a_args) {
  // Haal de parameters op ..
  $a_args['post']['job_type'] = JOBTYPE_STILL;
  $a_args['get']['mediafile_id'] = $a_args['uri']['mediafile_id'];

  // Backward compatibility
  if (!$a_args['post']['still_type'] || $a_args['post']['still_type'] == 'NONE') {
    if (!$a_args['post']['start_frame'] && $a_args['post']['frametime'] && is_numeric($a_args['post']['frametime'])) {
      $a_args['post']['start_frame'] = $a_args['post']['frametime'];
    }
    if (!$a_args['post']['size'] && $a_args['post']['width'] && $a_args['post']['height'] && is_numeric($a_args['post']['width']) && is_numeric($a_args['post']['height'])) {
      $a_args['post']['size'] = (int)($a_args['post']['width']) .'x'. (int)($a_args['post']['height']);
    }
  }
  unset($a_args['post']['frametime']);
  unset($a_args['post']['width']);
  unset($a_args['post']['height']);

  return vpx_jobscheduler_create_new_job($a_args);
}

/**
 * Creating still(s) for a mediafile
 * /mediafile/mediafile_id/still/create
 */
function vpx_jobscheduler_create_new_still_job_for_mediafile($a_args) {
  if (!is_array($a_args['get'])) {
    $a_args['get'] = array();
  }

  $a_args['get']['mediafile_id'] = $a_args['uri']['mediafile_id'];

  return vpx_jobscheduler_create_new_still_job_2($a_args);
}

/**
 * Replaced version of above
 * asset/$asset_id/still/create
 */
function vpx_jobscheduler_create_new_still_job_2($a_args) {
  $a_args_2 = array(
    'uri' => $a_args['uri'],
    'get' => array("app_id" => $a_args['get']['app_id']),
    'post' => array(),
    'internal' => $a_args['internal']
  );

  $a_args_2['post']['job_type'] = JOBTYPE_STILL;

  // Backward compatibility
  if (!$a_args['post']['still_type'] || $a_args['post']['still_type'] == 'NONE') {
    if (!$a_args['post']['start_frame'] && $a_args['post']['frametime'] && is_numeric($a_args['post']['frametime'])) {
      $a_args['post']['start_frame'] = $a_args['post']['frametime'];
    }
    if (!$a_args['post']['size'] && $a_args['post']['width'] && $a_args['post']['height'] && is_numeric($a_args['post']['width']) && is_numeric($a_args['post']['height'])) {
      $a_args['post']['size'] = (int)($a_args['post']['width']) .'x'. (int)($a_args['post']['height']);
    }
  }
  unset($a_args['post']['frametime']);
  unset($a_args['post']['width']);
  unset($a_args['post']['height']);

  $a_args_2['post']['still_type'] = $a_args['post']['still_type'];
  $a_args_2['post']['still_per_mediafile'] = $a_args['post']['still_per_mediafile'];
  $a_args_2['post']['still_every_second'] = $a_args['post']['still_every_second'];
  $a_args_2['post']['start_frame'] = $a_args['post']['start_frame'];
  $a_args_2['post']['end_frame'] = $a_args['post']['end_frame'];
  $a_args_2['post']['size'] = $a_args['post']['size'];
  $a_args_2['post']['h_padding'] = $a_args['post']['h_padding'];
  $a_args_2['post']['v_padding'] = $a_args['post']['v_padding'];
  $a_args_2['post']['tag'] = $a_args['post']['tag'];

  if (isset($a_args['get']['mediafile_id'])) {
    $a_args_2['get']['mediafile_id'] = $a_args['get']['mediafile_id'];
  }

  if (isset($a_args['get']['user_id'])) {
    $a_args_2['get']['user_id'] = $a_args['get']['user_id'];
  }

  // required checks are in here...
  return vpx_jobscheduler_create_new_job($a_args_2);
}

/**
 * Verwijderen van een job
 */
function vpx_jobscheduler_cancel_job($a_args) {

  return _vpx_jobs_cancel_job($a_args);
} // end of vpx_jobscheduler_delete_job


// start of job_server_still_check

/**
 * @file
 * Various image function to determine color difference, dominant colors and average colors.
 * This can be used for measuring a still image to see if it contains usefull data.
 */




// end of job_server_still_check

/**
 * Deze functie is de motor van de server voor het verwerken van jobs.
 * parse_queue haalt de lijst op van alle jobs op de server waarvoor
 * geldt dat ze niet afgerond zijn.
 * Voor alle nieuwe jobs (status is WAITING) wordt de job gestart.
 * Voor alle lopende jobs wordt de status file uitgelezen en verwerkt.
 *
 * Deze functie zal periodiek dienen te worden gestart.
 * (bijvoorbeeld elke 15s aangeroepen te worden.
 */
function vpx_jobserver_parse_queue($a_args) {
  $testtag = vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE');

  // selecteer alle jobs op deze server, loop ze 1 voor 1 langs en
  // voer een actie uit op basis van de status
  db_set_active("jobserver");
  $query_result = db_query("
    SELECT job_id, job_type, mediafile_src, status
    FROM {jobserver_job}
    WHERE NOT status IN ('%s', '%s', '%s')
    AND testtag = '%s'
    ORDER BY jobserver_job_id asc",
      JOBSTATUS_FINISHED, JOBSTATUS_FAILED, JOBSTATUS_CANCELLED, $testtag);
  db_set_active();
  while ($query_result_row = db_fetch_object($query_result)) {
    // handel een job af
    $job_id = $query_result_row->job_id;
    $job_type = $query_result_row->job_type;
    $status = $query_result_row->status;
    $mediafile_src = $query_result_row->mediafile_src;
    if ($status == JOBSTATUS_WAITING) {
      // job is nieuw, start de job
      _vpx_jobserver_startjob($job_id);
    }
    else {
      // de job loopt al, verwerkt de status
      _vpx_jobserver_updatestatus($job_id);
    }
  }

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * Start de job; op basis van het type job wordt de juiste manier gekozen
 */
function _vpx_jobserver_startjob($job_id) {
  // als in de beheer interface van de webservice_management module jobs op status uit is gezet,
  // voorkomen dat jobs kunnen worden gestart.
  // RBL: weet niet, maar dit lijkt mij niet het goeie tabel 'webservice_management', dit zou moeten zijn
  //      webservice_management_capp? webservice_management bevat de beschikbare opties voor de interface...
  db_set_active();
  $status = db_result(db_query("SELECT status FROM {webservice_management} WHERE handle='jobs' LIMIT 1"));
  if ($status == 'FALSE') {
    watchdog("jobserver", "Webservice management module for jobs is not enabled!");
    return FALSE;
  }

  // haal de info van 1 job op
  db_set_active("jobserver");
  $query_result = db_query("SELECT jobserver_job_id, job_type, mediafile_src FROM {jobserver_job} WHERE job_id='%s'", $job_id);
  db_set_active();

  $query_result_row = db_fetch_object($query_result);
  if (!$query_result_row) {
    watchdog("server", "Error, job not found: ". $job_id);
    return FALSE;
  }

  $jobserver_id = $query_result_row->jobserver_job_id;
  $job_type = $query_result_row->job_type;
  $mediafile_src = $query_result_row->mediafile_src;

  // zet de status van de job op inprogress
  _vpx_jobserver_set_job_status($job_id, JOBSTATUS_INPROGRESS, "0.000");

  // Controleer of the mediafile bestaat of dat we toegang hebben, voordat we verder gaan...
  if (!(file_exists(vpx_get_san_nas_base_path() . DS . DATA_LOCATION . DS . $mediafile_src{0} . DS . $mediafile_src))) {
    $mm_link = _vpx_jobserver_get_asset_link($job_id);
    watchdog('job', "File '". vpx_get_san_nas_base_path() . DS . DATA_LOCATION . DS . $mediafile_src{0} . DS . $mediafile_src ."' not found <br /><br />". $mm_link, WATCHDOG_ERROR);
    _vpx_jobserver_set_job_status($job_id, JOBSTATUS_FAILED, "1.000", "File not found");
    return FALSE;
  }

  // op basis van het type job wordt een script gestart
  $execution_string = "";
  switch ($job_type) {
    case JOBTYPE_TRANSCODE:
      $execution_string = _vpx_jobserver_get_transcode_string($jobserver_id, $mediafile_src);
      break;
    case JOBTYPE_STILL:
      $execution_string = _vpx_jobserver_get_still_string($jobserver_id, $mediafile_src);
      break;
    case JOBTYPE_ANALYSE:
      $execution_string = _vpx_jobserver_get_analyse_string($mediafile_src, $job_id);
      break;
    default:
      watchdog('job', 'Unknown job type: %s in job id: %d.', $job_type, $job_id);
      _vpx_jobserver_set_job_status($job_id, JOBSTATUS_FAILED, "0.000", sprintf("Unknown job type: %s.", $job_type));
      return FALSE;
  }

  watchdog("server", sprintf("About to start %s job: %s calling exec: %s", $job_type, $job_id, $execution_string), null);
  if ($job_type == JOBTYPE_ANALYSE) {
    $a_output = array();
    $s_output = exec($execution_string . " 2>&1", $a_output);

    $mm_link = _vpx_jobserver_get_asset_link($job_id);
    watchdog('server', sprintf("%s job returned output: %s - %s <br /><br />%s", $job_type, $s_output, implode("\n", $a_output), $mm_link));

    if ($s_output != "") {
      _vpx_jobserver_set_job_status($job_id, JOBSTATUS_FINISHED, "1.000");
    }
    else {
      _vpx_jobserver_set_job_status($job_id, JOBSTATUS_FAILED, "1.000", "Empty result, analyse failed.");
    }

    watchdog("jobserver", sprintf("Starting for job %s new job to process analyse.", $job_id));
    db_set_active("jobserver");
    db_query("UPDATE {jobserver_analyse_job} SET analyse_result = '%s' WHERE jobserver_job_id = %d", implode("\n", $a_output), $jobserver_id);
    db_set_active();
  }
  else {
    exec($execution_string);
    watchdog('server', sprintf("Started %s job", $job_type));
  }

  return TRUE;
}

/**
 * Haal voor een job de status op. Deze staat in de transcode directory
 * met een mediafile_id.status.
 * Voor transcode en still bestaat de file en wordt de status uitgelezen.
 * Wanneer een transcode of still job is afgerond worden deze op de
 * juiste plek gezet.
 */

function _vpx_jobserver_updatestatus($job_id) {
  // Haal een job op
  db_set_active("jobserver");
  $query_result = db_query("
    SELECT job_type, mediafile_src
    FROM {jobserver_job}
    WHERE job_id = '%s'", $job_id);
  db_set_active();

  if ($query_result_row = db_fetch_object($query_result)) {
    $job_type = $query_result_row->job_type;
    $mediafile_src = $query_result_row->mediafile_src;

    // lees de status van de job van de mediafile_id.status in.
    $jobstatus = _vpx_jobserver_get_status($mediafile_src);
    $status = JOBSTATUS_INPROGRESS;
    $updatefinished = "";

    // controleer of de job een transcodejob is en verwerk de resultaten
    // wanneer deze is afgerond.
    if ($job_type == JOBTYPE_TRANSCODE) {
      if ($jobstatus["Status"] == "done" && $jobstatus["Errors"] == "none") {
        $status = JOBSTATUS_FINISHED;
        _vpx_jobserver_store_new_mediafile($job_id, $mediafile_src);
        watchdog("server", sprintf("Einde %s job: %s met status: %s", $job_type, $job_id, $status), null);
      }
      elseif ($jobstatus["Status"] == "error" && $jobstatus["Errors"] != "none") {
        $status = JOBSTATUS_FAILED;
        $mm_link = _vpx_jobserver_get_asset_link($job_id);
        watchdog("server", sprintf("Einde %s job: %s met status: %s <br /><br />%s", $job_type, $job_id, $status, $mm_link), null);
        watchdog("server", sprintf("Info (%s job) statusfile: %s", $job_id, print_r(_vpx_jobserver_get_status($mediafile_src, TRUE), TRUE)), null);
      }

      // zet de status van de job
      if ($jobstatus["Errors"] != "none") {
        _vpx_jobserver_set_job_status($job_id, $status, $jobstatus["Progress"], isset($jobstatus["ffmpeg-output"]) ? ($jobstatus["Errors"] != "" ? $jobstatus["Errors"] . "-\n" : '') . $jobstatus["ffmpeg-output"] : $jobstatus["Errors"]);
      }
      else {
        _vpx_jobserver_set_job_status($job_id, $status, $jobstatus["Progress"]);
      }
    } // end of jobtype == transcode
    elseif ($job_type == JOBTYPE_STILL) {
      // Scene still?
      $statusfile = vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $job_id ."_scene.txt";

      if (file_exists($statusfile) || ($jobstatus["Status"] == "done" && $jobstatus["Errors"] == "none")) {
        $status = _vpx_jobserver_store_new_still($job_id, $mediafile_src);
        watchdog("server", sprintf("Einde %s job: %s met status: %s", $job_type, $job_id, $status), null);
      }
      else if ($jobstatus["Status"] == "error" && $jobstatus["Errors"] != "none") {
        $status = JOBSTATUS_FAILED;
        $mm_link = _vpx_jobserver_get_asset_link($job_id);
        watchdog("server", sprintf("Einde %s job: %s met status: %s <br /><br />%s", $job_type, $job_id, $status, $mm_link), null);
        watchdog("server", sprintf("Info (%s job) statusfile: %s", $job_id, print_r(_vpx_jobserver_get_status($mediafile_src, TRUE), TRUE)), null);
      }

      if (!file_exists($statusfile) && $jobstatus["Errors"] != "none") {
        _vpx_jobserver_set_job_status($job_id, $status, $jobstatus["Progress"], $jobstatus["Errors"]);
      }
      else {
        _vpx_jobserver_set_job_status($job_id, $status, $jobstatus["Progress"]);
      }
    }
  } // end of fetch job from db
  else {
    watchdog("server", "Error, job not found: ". $job_id, null);
    return FALSE;
  }

  return TRUE;
}  // end of _vpx_jobserver_updatestatus




/**
 * Sla de nieuwe mediafile op.
 */
function _vpx_jobserver_store_new_mediafile($job_id, $old_filename) {
  // genereer een nieuwe hash
  $new_filename = vpx_create_hash($old_filename, $job_id);

  // haal de transcode gegevens op
  db_set_active("jobserver");
  $query_result = db_query("
    SELECT command, file_extension
    FROM {jobserver_transcode_job} jtj1
    INNER JOIN {jobserver_job} jj1
    ON jtj1.jobserver_job_id = jj1.jobserver_job_id
    WHERE job_id = '%s'", $job_id);
  db_set_active();


  if ($query_result_row = db_fetch_object($query_result)) {
    $file_extension = $query_result_row->file_extension;
  }
  else {
    watchdog("server", sprintf("Transcode job not found, job_id: %s", $job_id), null);
    return;
  }

  // detecteer of dit een hulp job is voor windows, dan moet nl een
  // een file extensie aan het doel bestand worden gegeven
  if ($file_extension == JOBRAW_FILE_EXTENSION) {
    $new_filename .= ".". JOBRAW_FILE_EXTENSION;
  }

/*  rename(VPX_TRANSCODE_TMP_DIR ."/". $filename[0] .".". $extension,
 *         sprintf("%s/%s/%s.%s", SAN_NAS_BASE_PATH, DATA_LOCATION, $new_filename, $extension));
 *
 * $query_result = db_query("
 *     UPDATE {tmp_server_job}
 *     SET mediafile_dest = '%s.%s'
 *     WHERE job_id = %d", $new_filename, $extension, $job_id);
 *     watchdog("server",sprintf("job klaar, nieuwe mediafile opslaan als: %s/%s/%s.%s", SAN_NAS_BASE_PATH, DATA_LOCATION, $new_filename, $extension ), null);
*/

  // verplaats de mediafile en verwijder de status file.
  rename(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $old_filename .".". $file_extension,
          vpx_get_san_nas_base_path() . DS . DATA_LOCATION . DS . $new_filename{0} . DS . $new_filename);
  unlink(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $old_filename .".status");

  // update de jostatus
  db_set_active("jobserver");
  $query_result = db_query("
      UPDATE {jobserver_job}
      SET mediafile_dest = '%s'
      WHERE job_id = %d", $new_filename, $job_id);
  db_set_active();
  watchdog("server", "job $job_id klaar, nieuwe mediafile opslaan als: ". vpx_get_san_nas_base_path() . DS . DATA_LOCATION . DS . $new_filename{0} . DS . $new_filename, null);
}



/**
 * Voor het starten van een job wordt deze functie aangeroepen
 * Als parameters wordt alle info meegestuurd die noodzakelijk is voor
 * het uitvoeren van een job. Wanneer het een analyse job betreft wordt
 * een hulptabel gevuld.
 */
function vpx_jobserver_add_job($a_args) {
  // als in de beheer interface van de webservice_management module jobs op status uit is gezet,
  // voorkomen dat jobs kunnen worden gestart.
  db_set_active();
  $status = db_result(db_query("SELECT status FROM {webservice_management} WHERE handle='jobs' LIMIT 1"));
  if ($status == 'FALSE') {
    return new rest_response(vpx_return_error(ERRORCODE_STARTING_JOB_FAILED));
  }

  $a_parameters = array(
    'job_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'job_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'job_type' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'job_type'),
      'type' => 'job_type',
      'required' => TRUE,
    ),
    'mediafile_src' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'mediafile_src'),
      'type' => 'skip',
      'required' => TRUE,
    ),
    'tool' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'tool'),
      'type' => 'skip',
      'required' => FALSE,
    ),
    'file_extension' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'file_extension'),
      'type' => 'skip',
      'required' => FALSE,
    ),
    'command' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'command'),
      'type' => 'skip',
      'required' => FALSE,
    ),
    'frametime' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'frametime'),
      'type' => VPX_TYPE_IGNORE,
      'required' => FALSE,
    ),
    'size' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'size'),
      'type' => VPX_TYPE_IGNORE,
      'required' => FALSE,
    ),
    'h_padding' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'h_padding'),
      'type' => VPX_TYPE_IGNORE,
      'required' => FALSE,
    ),
    'v_padding' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'v_padding'),
      'type' => VPX_TYPE_IGNORE,
      'required' => FALSE,
    ),
    'tag' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'tag'),
      'type' => VPX_TYPE_STRING,
      'required' => FALSE,
    ),
    'blackstill_check' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'blackstill_check', 'FALSE'),
      'type' => VPX_TYPE_IGNORE,
      'required' => FALSE,
    ),
    'still_type' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'still_type', NULL),
      'type' => VPX_TYPE_ALPHA,
    ),
    'still_per_mediafile' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'still_per_mediafile', NULL),
      'type' => VPX_TYPE_INT,
      'required' => FALSE,
    ),
    'still_every_second' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'still_every_second', NULL),
      'type' => VPX_TYPE_INT,
      'required' => FALSE,
    ),
    'start_frame' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'start_frame', NULL),
      'type' => VPX_TYPE_INT,
      'required' => FALSE,
    ),
    'end_frame' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'end_frame', NULL),
      'type' => VPX_TYPE_INT,
      'required' => FALSE,
    ),
    'video_duration' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'video_duration', NULL),
      'type' => VPX_TYPE_INT,
    ),
    'fps' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'fps', NULL),
      'type' => VPX_TYPE_IGNORE,
    ),
    'testtag' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE'),
      'type' => 'alphanum',
      'required' => FALSE,
    )
  );

  // .. en valideer deze op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    $rest_response = new rest_response($result);
    return $rest_response;
  }

  // controleer of de job_id niet al een keer is toegevoegd.
  db_set_active("jobserver");
  if (db_result(db_query(
      "SELECT COUNT(job_id) FROM {jobserver_job} WHERE job_id = %d",
      $a_parameters['job_id']['value'])) != 0) {
      db_set_active();
      return new rest_response(vpx_return_error(ERRORCODE_STARTING_JOB_FAILED));
  }
  db_set_active();


  // voeg de query toe aan de server_job tabel.
  db_set_active("jobserver");
  $query_result = db_query(
    "INSERT INTO {jobserver_job}
     (job_id, job_type, status, mediafile_src, testtag) VALUES
     (%d, '%s', '%s', '%s', '%s')", $a_parameters['job_id']['value'],
      $a_parameters['job_type']['value'],
      JOBSTATUS_WAITING,
      $a_parameters['mediafile_src']['value'],
      $a_parameters['testtag']['value']);
  $jobserver_id = db_last_insert_id("jobserver_job", "jobserver_job_id");
  db_set_active();


  // voor een analyse job wordt een hulp table gevuld.
  if ($a_parameters['job_type']['value'] == JOBTYPE_ANALYSE) {
    db_set_active("jobserver");
    $query_result = db_query(
      "INSERT INTO {jobserver_analyse_job}
       (jobserver_job_id, analyse_result) VALUES
       (%d, '')", $jobserver_id);
    db_set_active();
  }

  // voor een still job wordt een hulp table gevuld met frametime
  if ($a_parameters['job_type']['value'] == JOBTYPE_STILL) {
    $still_parameters = array(
      'still_type' => $a_parameters['still_type']['value'],
      'still_per_mediafile' => $a_parameters['still_per_mediafile']['value'],
      'still_every_second' => $a_parameters['still_every_second']['value'],
      'start_frame' => $a_parameters['start_frame']['value'],
      'end_frame' => $a_parameters['end_frame']['value'],
      //'size' => $a_parameters['size']['value'],
      //'h_padding' => $a_parameters['h_padding']['value'],
      //'v_padding' => $a_parameters['v_padding']['value'],
      'tag' => $a_parameters['tag']['value'],
      //'frametime' => $a_parameters['frametime']['value'],
      //'width' => $a_parameters['width']['value'],
      //'height' => $a_parameters['height']['value'],
      'video_duration' => $a_parameters['video_duration']['value'],
      'fps' => $a_parameters['fps']['value'],
    );

    db_set_active("jobserver");
    $query_result = db_query(
      "INSERT INTO {jobserver_still_job}
      (jobserver_job_id, frametime, size, h_padding, v_padding, blackstill_check, still_parameters) VALUES
      (%d, %d, '%s', %d, %d, '%s', '%s')",
      $jobserver_id,
      $a_parameters['frametime']['value'],
      $a_parameters['size']['value'],
      $a_parameters['h_padding']['value'],
      $a_parameters['v_padding']['value'],
      $a_parameters['blackstill_check']['value'],
      serialize($still_parameters));
    db_set_active();
  }

  // voor een transcode job wordt een hulptabel gevuld met tool,
  // file_extension en command.
  if ($a_parameters['job_type']['value'] == JOBTYPE_TRANSCODE) {
    db_set_active("jobserver");
    $query_result = db_query(
      "INSERT INTO {jobserver_transcode_job}
       (jobserver_job_id, tool, file_extension, command) VALUES
       (%d, '%s', '%s', '%s')", $jobserver_id, $a_parameters['tool']['value'],
                  $a_parameters['file_extension']['value'],
                  $a_parameters['command']['value']);
    db_set_active();
  }

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}


/**
 * Verwijder de jobs van de database
 * todo: sent a kill signal to the process.
 */
function vpx_jobserver_remove_job_on_server($a_args) {
  $a_parameters = array(
    'job_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'job_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'killjob' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE'),
      'type' => 'alphanum',
      'required' => FALSE,
    )
  );

  // .. en valideer deze op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    $rest_response = new rest_response($result);
    return $rest_response;
  }

  // haal de status van de job op (en de mediafile_id)
  db_set_active("jobserver");
  $query_result = db_query("
    SELECT jobserver_job_id, job_type, mediafile_src, status
    FROM {jobserver_job}
    WHERE job_id = '%s'", $a_parameters['job_id']['value']);

  if ($query_result_row = db_fetch_object($query_result)) {
    $job_type = $query_result_row->job_type;
    $jobserver_job_id = $query_result_row->jobserver_job_id;
    $mediafile_src = $query_result_row->mediafile_src;
    $status = $query_result_row->status;

    // is er een killjob commando gegeven, stop dan eerste de job
    if ($a_parameters['killjob']['value'] == "TRUE") {
      db_query("
        UPDATE {jobserver_job}
        SET status = '%s'
        WHERE job_id = '%s'", JOBSTATUS_CANCELLED, $a_parameters['job_id']['value']);

      // stuur een kill commando naar de specifieke job

      // verwijder de bestanden
      // unlink
    }

    // verwijder de tabellen.
    $query_result = db_query(
      "DELETE FROM {jobserver_analyse_job}
      WHERE jobserver_job_id = '%s'", $jobserver_job_id);
    $query_result = db_query(
      "DELETE FROM {jobserver_still_job}
      WHERE jobserver_job_id = '%s'", $jobserver_job_id);
    $query_result = db_query(
      "DELETE FROM {jobserver_transcode_job}
      WHERE jobserver_job_id = '%s'", $jobserver_job_id);
    $query_result = db_query(
      "DELETE FROM {jobserver_job}
      WHERE jobserver_job_id = '%s'", $jobserver_job_id);

  }

  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}


