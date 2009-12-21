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
 * Deze functie is de motor van het schedulen en verwerken van jobs.
 * Elke keer zal voor de lijst van servers de status van alle jobs
 * op die server worden opgevraagd en verwerkt.
 * Daarna zal voor alle beschikbare slots per server een job worden
 * gestart.
 * Deze functie zal periodiek dienen te worden gestart.
 * (bijvoorbeeld elke 15s aangeroepen te worden.
 */
function vpx_jobhandler_parse_queue($a_args) {
  $testtag = vpx_get_parameter_2($a_args['get'], 'testtag', 'FALSE');

  # FIX ME
  /* controle inbouwen voor het stoppen van de handler zelf.
   * Dit zou dan een toekomstige functionaliteit zijn.
   */
  // ga alle server pollen voor de status en verwerk de resultaten in de database
  _vpx_jobhandler_check_status_all_servers($testtag);

  // voor elke server kies een job om uit te voeren en stuur deze naar de server, update de database
  _vpx_jobhandler_start_new_jobs($testtag);

  // controleer of er uploads jobs in de queue staan die al 2 min geen
  // update hebben ontvangen
  _vpx_jobhandler_check_upload_timeout();

  // controleer of er transcode jobs zijn die al langer dan 2 uur lopen.
  // minimum requirement, de server_job tabel is gevuld.
  _vpx_jobhandler_check_transcode_timeout();

  // wanneer een server de status close heeft gekregen, dient deze
  // op stop te worden gezet wanneer er geen lopende jobs voor de server zijn.
  _vpx_jobhandler_check_server_on_status_close();

  // geef altijd okay terug. (cron job doet toch niets met deze info)
  // eventuele fouten worden gegeschreven in watchdog
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
} // end of vpx_jobhandler_parse_queue


/**
 * Haal de lijst op met alle servers en laat voor elke server de
 * jobstatus bij werken.
 */
function _vpx_jobhandler_check_status_all_servers($testtag) {
  // get all the active servers from the database and retrieve their status

  db_set_active();
  $query_result = db_query("
    SELECT ip_address
    FROM {transcoding_server}
    WHERE status <> '%s' AND is_test_server = '%s'", SERVERSTATUS_OFF, $testtag);

  while ($query_result_row = db_fetch_object($query_result)) {
    _vpx_jobhandler_update_server_job_status($query_result_row->ip_address, $testtag);
  }
  // for each server get the status
}


/**
 * Met behulp van een REST call wordt de status van de jobs van een
 * server opgevraagd. Deze informatie wordt vervolgens verwerkt.
 * vb. sprintf("http://%s/server/joblist", $ip_address);
 */
function _vpx_jobhandler_update_server_job_status($ip_address, $testtag) {
  // stel de url en query samen
  $rest_url = sprintf("http://%s/%s/internal/server/joblist?testtag=%s", $ip_address, VPX_BUILD_URL, $testtag);

  $response = drupal_http_request(
    $rest_url, // url
    array(), // headers
    'GET', // method
    null // data
  );

  if ($response->code != "200") {
    return;
  }

  $response_data = new SimpleXMLElement($response->data);

  // stel de update query samen.
  for ($i = 0; $i < $response_data->header->item_count; $i++) {
    $query = sprintf("UPDATE {job} SET status = '%s', progress = '%s', error_description = '%s'",
                      $response_data->items->item[$i]->status,
                      $response_data->items->item[$i]->progress,
                      db_escape_string($response_data->items->item[$i]->error_description));

    // voeg de starttijd toe wanneer deze bekent is.
    if ($response_data->items->item[$i]->started_unix != "") {
      $query .= sprintf(", started=from_unixtime(%s)",
                        $response_data->items->item[$i]->started_unix);
    }

    // voeg de eindtijd toe wanneer de job is afgerond.
    if ($response_data->items->item[$i]->finished_unix != "") {
      $query .= sprintf(", finished=from_unixtime(%s) ",
                        $response_data->items->item[$i]->finished_unix);
    }
    $query .= sprintf("WHERE job_id = %d", $response_data->items->item[$i]->job_id);

    db_set_active("data");
    $status = db_result(db_query_range("SELECT status FROM {job} WHERE job_id = %d", $response_data->items->item[$i]->job_id, 0, 1));
    $query_result = db_query($query);
    db_set_active();
    vpx_jobs_notify_transcoding($status, $response_data->items->item[$i]->status, $response_data->items->item[$i]->job_id);

    // wanneer een job is afgerond, verwijder dan de job uit tijdelijke
    // server job relatie en laat de job van de jobserver verwijderen.
    if ($response_data->items->item[$i]->status == JOBSTATUS_FINISHED ||
        $response_data->items->item[$i]->status == JOBSTATUS_FAILED ||
        $response_data->items->item[$i]->status == JOBSTATUS_CANCELLED) {

      // verwijder de actieve server-job relatie
      _vpx_jobhandler_clear_serverjob($response_data->items->item[$i]->job_id);

      // stuur een verwijder afgesloten job naar de server
      _vpx_jobhandler_remove_job_on_server($ip_address, $response_data->items->item[$i]->job_id);
    }

    if ($response_data->items->item[$i]->status == JOBSTATUS_FINISHED) {
      switch ($response_data->items->item[$i]->job_type) {

        case JOBTYPE_TRANSCODE:
          // als het een transcode job is die compleet is, voeg de mediafile toe
          _vpx_jobhandler_parse_finished_transcode($response_data->items->item[$i]->job_id, $response_data->items->item[$i]->mediafile_dest, $testtag);
          break;
        case JOBTYPE_ANALYSE:
          // als het een analyse job is die compleet is, voeg technische metadata toe
          _vpx_jobhandler_update_analyse_job_info_to_mediafile($response_data->items->item[$i]->job_id, $response_data->items->item[$i]->analyse_result);
          break;
        case JOBTYPE_STILL:
          // als het een still job is die compleet is, voeg de still aan de db toe
          // We serialize it, because of the multiple stills
          _vpx_jobhandler_add_still_to_db($response_data->items->item[$i]->job_id, unserialize($response_data->items->item[$i]->mediafile_dest));
          break;
      }
    }
    elseif ($response_data->items->item[$i]->status == JOBSTATUS_FAILED ||
            $response_data->items->item[$i]->status == JOBSTATUS_CANCELLED) {

      switch ($response_data->items->item[$i]->job_type) {
        case JOBTYPE_TRANSCODE:
          // als transcode faalt....
          _vpx_jobhandler_parse_failed_transcode($response_data->items->item[$i]->job_id, $response_data->items->item[$i]->mediafile_dest, $testtag);
          break;
      }
    }
  }
}

/**
 * Haal een lijst op van alle servers waarbij geteld wordt hoeveel
 * jobs een server nog mag starten. Per beschikbaar 'slot' wordt een
 * job uitgezocht om uit te voeren.
 */
function _vpx_jobhandler_start_new_jobs($testtag) {
  $error_code = ERRORCODE_OKAY;
  // voor elke server actieve server (status op ON)
  // tel het aantal jobs dat niet FINISHED, FAILED of CANCELLED
  // en waarvan het job_type niet upload is (wordt niet afgehandeld door een server)
  db_set_active("data");
  $query_result = db_query(
    "SELECT transcoding_server_id as server_id, ip_address, slots,
           (SELECT count(ssj1.job_id)
            FROM {server_job} AS ssj1
            INNER JOIN {job} AS sj1 on ssj1.job_id = sj1.job_id
            WHERE ssj1.server_id = ss1.transcoding_server_id AND sj1.job_type <> '%s'
                  AND sj1.status not in ('%s', '%s', '%s')) AS jobcount
    FROM {%s.transcoding_server} AS ss1
    WHERE status = '%s' and is_test_server='%s'",
     JOBTYPE_UPLOAD,
     JOBSTATUS_FINISHED, JOBSTATUS_FAILED, JOBSTATUS_CANCELLED,
     vpx_db_query_dbname('default'),
     SERVERSTATUS_ON, $testtag);
  db_set_active();
  while ($query_result_row = db_fetch_object($query_result)) {
    // voor elke server, probeer net zoveel jobs te starten als
    // mogelijk is voor het aantal gespecificeerde slots
    for ($i=$query_result_row->jobcount; $i < $query_result_row->slots; $i++) {

      // haal 1 nieuwe job op
      $job_info = _vpx_jobhandler_fetch_single_available_job($query_result_row->server_id, $testtag);

      if (count($job_info) > 0) {
        if (_vpx_jobhandler_is_valid_job($job_info["job_id"],
                                         $job_info["job_type"],
                                         $job_info["mediafile_id"],
                                         $testtag)) {
          _vpx_jobhandler_start_server_job($job_info["job_id"],
                                           $job_info["job_type"],
                                           $query_result_row->server_id,
                                           $query_result_row->ip_address,
                                           $job_info["mediafile_id"],
                                           $testtag);
        }
      }
    } // end of for loop selecteren nieuwe job
  } // end of while loop, verwerken server lijst
} // end of _vpx_jobhandler_start_new_jobs

/**
 * Voordat een nieuwe job wordt gestart, wordt gecontroleerd of de job
 * informatie valide is om een job te starten.
 * Op dit moment betekent dat dat de still en transcode job alleen van
 * een orgineel bestand mag.
 */
function _vpx_jobhandler_is_valid_job($job_id, $job_type, $mediafile_id, $testtag) {
  // test of er een orginele mediafile bestaat voor de asset
  $result = TRUE;
  switch ($job_type) {
    case JOBTYPE_TRANSCODE:
      db_set_active("data");
      $query_mediafile = db_query(
        "SELECT count(mediafile_id)
         FROM {mediafile} md1
         WHERE mediafile_id = '%s'
         AND is_original_file ='TRUE'", $mediafile_id, $testtag);
      db_set_active();

      if (db_result($query_mediafile) == 0) {
        $result = FALSE;
      }
    break;
    case JOBTYPE_STILL:
    break;
  }

  // als een job niet voldoet $result == FALSE, fail de job
  #FIX ME error melding
  if (!($result)) {
    db_set_active("data");
    $query_result = db_query("
      UPDATE {job}
      SET status = '%s', finished = NOW(), error_description = '%s'
      WHERE job_id = %d", JOBSTATUS_FAILED, 'Het te transcoderen bestand is niet het bronbestand.', $job_id);
    db_set_active();
    vpx_jobs_notify_transcoding('NONE', JOBSTATUS_FAILED, $job_id);
  }
  return $result;
}

/**
 * Start 1 job op een server.
 * Op basis van job_type wordt extra informatie opgehaald.
 * Daarna wordt de jobinformatie via een REST call naar de server gestuurd.
 * Tot slot wordt de job in de server_job tabel gekoppeld.
 */
function _vpx_jobhandler_start_server_job($job_id, $job_type, $server_id, $ip_address, $mediafile_id, $testtag) {
  watchdog("jobhandler", sprintf("Start job: jobid=%s, job_type=%s, serverip=%s, mediafile_id=%s", $job_id, $job_type, $ip_address, $mediafile_id), null);

  switch ($job_type) {
    case JOBTYPE_TRANSCODE:
      $errorcode = _vpx_jobhandler_start_server_transcode_job($job_id, $ip_address, $mediafile_id, $testtag);
      break;
    case JOBTYPE_STILL:
      $errorcode = _vpx_jobhandler_start_server_still_job($job_id, $ip_address, $mediafile_id, $testtag);
      break;
    case JOBTYPE_ANALYSE:
      $errorcode = _vpx_jobhandler_start_server_analyse_job($job_id, $ip_address, $mediafile_id, $testtag);
      break;
  }

  if ($job_type == JOBTYPE_DELETE_MEDIAFILE) {
      _vpx_jobhandler_start_server_delete_job($job_id, $mediafile_id, $testtag);
      watchdog("jobhandler", sprintf("Verwerk %s job: mediafile %s verwijderd", $job_id, $mediafile_id), null);
  }
  else {
    // if okay update the database with the job info
    if ($errorcode == ERRORCODE_OKAY) {

      // Check if job still exists
      // to be 100%, we might have to lock table ?
      db_set_active('data');
      $exists = db_result(db_query("SELECT COUNT(*) FROM {job} WHERE job_id=%d", $job_id));
      db_set_active();

      if ($exists > 0) {
        db_set_active("data");
        $query_result = db_query(
          "INSERT INTO {server_job}
           (server_id, job_id, testtag) VALUES
           (%d, %d, '%s')", $server_id, $job_id, $testtag);
        db_set_active();
        watchdog("jobhandler", sprintf("Stuur %s job: %s naar server op: %s", $job_type, $job_id, $ip_address));
      }
      else {
        watchdog("jobhandler", sprintf("Aanmaak %s server job gestopt, job %s was verwijderd tijdens process", $job_type, $job_id));
      }
    }
  }

  return new rest_response(vpx_return_error($errorcode));
}

/**
 * Function for starting a transcode job
 */
function _vpx_jobhandler_start_server_transcode_job($job_id, $ip_address, $mediafile_id, $testtag) {
  // haal de specifieke jobgegevens van de server
  $job_parameters = _jobhandler_get_job_parameters($job_id, JOBTYPE_TRANSCODE, $mediafile_id);

  // stuur de job naar de job server
  $rest_url = sprintf("http://%s/%sinternal/server/jobstart", $ip_address, VPX_BUILD_URL);
  $rest_url = $rest_url . sprintf("?job_id=%s&job_type=%s&mediafile_src=%s&tool=%s&file_extension=%s&command=%s&testtag=%s",
           $job_id, JOBTYPE_TRANSCODE, $mediafile_id, $job_parameters["tool"],
           $job_parameters["file_extension"], $job_parameters["command"], $testtag);
//  $data = http_build_query(array_merge(array("job_id" => $job_id,
//            "job_type" => $job_type, "mediafile_src" => $mediafile_src),
//             $job_parameters));
  watchdog("jobhandler", "Start job:". $rest_url, null);

  $data = "";
  $response = drupal_http_request(
    $rest_url, // url
    array("Content-Type: application/x-www-form-urlencoded",
          "Content-Length: " . strlen($data) ), // headers
    'POST', // method
    $data
  );
  if (($response->code == "201") || ($response->code == "200")) {
    $response_data = new SimpleXMLElement($response->data);
    $errorcode = $response_data->header->request_result_id;
  }
  else {
    watchdog("jobhandler", "errorcode $response->code bij url $rest_url", null);
    $errorcode = $response->code;
  }
  return $errorcode;
}


/**
 * Function for starting a analyse job
 */
function _vpx_jobhandler_start_server_analyse_job($job_id, $ip_address, $mediafile_id, $testtag) {
  // verwijder oude metadata voor zover deze bestaat
  db_set_active("data");
  db_query("DELETE FROM {mediafile_metadata} WHERE mediafile_id = '%s'", $mediafile_id);
  db_set_active();

  // stuur de job naar de job server
  $rest_url = sprintf("http://%s/%s", $ip_address, VPX_BUILD_URL);
  $rest_url = $rest_url ."/internal/server/jobstart";
  $rest_url = $rest_url . sprintf("?job_id=%s&job_type=%s&mediafile_src=%s&testtag=%s",
           $job_id, JOBTYPE_ANALYSE, $mediafile_id, $testtag);
//  $data = http_build_query(array_merge(array("job_id" => $job_id,
//            "job_type" => $job_type, "mediafile_src" => $mediafile_src),
//             $job_parameters));
    $data = "";
  $response = drupal_http_request(
    $rest_url, // url
    array("Content-Type: application/x-www-form-urlencoded",
          "Content-Length: " . strlen($data) ), // headers
    'POST', // method
    $data
  );
  if (($response->code == "201") || ($response->code == "200")) {
    $response_data = new SimpleXMLElement($response->data);
    $errorcode = $response_data->header->request_result_id;
  }
  else {
    watchdog("jobhandler", "errorcode $response->code bij url $rest_url", null);
    $errorcode = $response->code;
  }
  return $errorcode;
}


/**
 * Function for starting a still job
 */
function _vpx_jobhandler_start_server_still_job($job_id, $ip_address, $mediafile_id, $testtag){
  // haal de specifieke jobgegevens van de server
  $job_parameters = _jobhandler_get_job_parameters($job_id, JOBTYPE_STILL, $mediafile_id);

  // eerst controleren of de opgegeven frametime voorbij de duration van het bestand ligt.
  db_set_active("data");
  $mm = db_fetch_array(db_query_range("SELECT file_duration, fps
                                         FROM {mediafile_metadata}
                                         WHERE mediafile_id = '%s'",
                                         $mediafile_id, 0, 1));
  db_set_active();
  $actual_duration = $mm['file_duration'];
  $fps = $mm['fps'];
  if ($actual_duration && $actual_duration != "") {
    // haal de ms van de tijd af.
    $actual_duration = substr($actual_duration, 0, 8);
    @list($uren, $minuten, $seconden) = explode(':', $actual_duration);
    $video_duration = (($uren * 3600) + ($minuten * 60) + $seconden);
    if  ($video_duration < $job_parameters["frametime"]) {
/*
      $still_error = vpx_return_error(ERRORCODE_JOB_FRAMETIME_GREATER_THEN_DURATION);
      db_set_active("data");
      $query_result = db_query("
        UPDATE {job}
        SET status = '%s', error_description = '%s', finished=NOW()
        WHERE job_id = %d",
        JOBSTATUS_FAILED, $still_error['description'], $job_id);
      db_set_active();

      watchdog("jobhandler", "Still job is gefaald omdat de gekozen frametime hoger is dan de lengte van de film", null);

      return ERRORCODE_JOB_FRAMETIME_GREATER_THEN_DURATION;
 */
      // If the video duration is less than still picture time, we decrease the still picture time to half of video duration
      $job_parameters["frametime"] = ($video_duration >> 1);
    }
  }

  // stuur de job naar de job server
  $rest_url = sprintf("http://%s/%s", $ip_address, VPX_BUILD_URL);
  $rest_url = $rest_url ."/internal/server/jobstart";
  $rest_url = $rest_url . sprintf("?job_id=%s&job_type=%s&mediafile_src=%s&frametime=%s&size=%s&h_padding=%s&v_padding=%s&blackstill_check=%s&testtag=%s&still_type=%s&still_per_mediafile=%s&still_every_second=%s&start_frame=%s&end_frame=%s&video_duration=%s&fps=%s&tag=%s",
           $job_id, JOBTYPE_STILL, $mediafile_id,
           $job_parameters["frametime"],
           $job_parameters["size"],
           $job_parameters["h_padding"],
           $job_parameters["v_padding"],
           $job_parameters["blackstill_check"],
           $testtag,
           $job_parameters['still_parameters']['still_type'],
           $job_parameters['still_parameters']['still_per_mediafile'],
           $job_parameters['still_parameters']['still_every_second'],
           $job_parameters['still_parameters']['start_frame'],
           $job_parameters['still_parameters']['end_frame'],
           $video_duration,
           $fps,
           $job_parameters['still_parameters']['tag']);
//  $data = http_build_query(array_merge(array("job_id" => $job_id,
//            "job_type" => $job_type, "mediafile_src" => $mediafile_src),
//             $job_parameters));
    $data = "";

  $response = drupal_http_request(
    $rest_url, // url
    array("Content-Type: application/x-www-form-urlencoded",
          "Content-Length: " . strlen($data) ), // headers
    'POST', // method
    $data
  );
  if (($response->code == "201") || ($response->code == "200")) {
    $response_data = new SimpleXMLElement($response->data);
    $errorcode = $response_data->header->request_result_id;
  }
  else {
    watchdog("jobhandler", "errorcode $response->code bij url $rest_url", null);
    $errorcode = $response->code;
  }
  return $errorcode;
}


/**
 * Function for starting a delete job
 */
function _vpx_jobhandler_start_server_delete_job($job_id, $mediafile_id, $testtag) {
  $a_parameters = array();
  $a_parameters['uri'] = array('mediafile_id' => $mediafile_id);
  $a_parameters['get'] = array('app_id' => 1);
  $error = media_management_delete_mediafile($a_parameters);
  if (!vpx_check_result_for_error($error)) {
    $status_parameters = array(
        'uri' => array('job_id' => $job_id),
        'post' => array('status' => JOBSTATUS_FINISHED,
        'progress' => "1.000"),
    );
  }
  else {
    $status_parameters = array(
        'uri' => array('job_id' => $job_id),
        'post' => array('status' => JOBSTATUS_FAILED,
                        'progress' => "1.000",
                        'error_description' =>
            vpx_shared_return_message(ERRORCODE_MEDIAFILE_NOT_FOUND,
                               array('@mediafile_id' => $mediafile_id)))
    );
  }
  vpx_jobs_set_job_status($status_parameters);
}


/**
 * Haal op basis van de job_type, speciefieke informatie op voer de job
 * vanuit de hulp tabellen.
 */
function _jobhandler_get_job_parameters($job_id, $job_type, $mediafile_id) {
  $result = array();

  switch ($job_type) {
    case JOBTYPE_TRANSCODE:
      db_set_active("data");
      $query_job = db_query(
          "SELECT transcode_profile_id, tool, file_extension, command FROM {transcode_job}
            WHERE job_id = %d", $job_id);
      db_set_active();
      if ($query_job_row = db_fetch_object($query_job)) {
        $result["profile_id"] = $query_job_row->transcode_profile_id;
        $result["tool"] = $query_job_row->tool;
        $result["file_extension"] = $query_job_row->file_extension;
        $result["command"] = _vpx_jobhandler_map_parameters($query_job_row->tool, $query_job_row->command, $mediafile_id);
      }
      break;
    case JOBTYPE_STILL:
      db_set_active("data");
      $query_job = db_query(
          "SELECT frametime, size, h_padding, v_padding, blackstill_check, still_parameters FROM {still_job} WHERE job_id = %d", $job_id);
      db_set_active();
      if ($query_job_row = db_fetch_object($query_job)) {
        //$result["frametime"] = $query_job_row->frametime;
        //$result["size"] = $query_job_row->size;
        //$result["h_padding"] = $query_job_row->h_padding;
        //$result["v_padding"] = $query_job_row->v_padding;
        $result["blackstill_check"] = $query_job_row->blackstill_check;
        $result["still_parameters"] = unserialize($query_job_row->still_parameters);
        $result["frametime"] = $result["still_parameters"]['frametime'];
        //$result["size"] = $result["still_parameters"]['size'];
        $result["h_padding"] = $result["still_parameters"]['h_padding'];
        $result["v_padding"] = $result["still_parameters"]['v_padding'];
        $result["tag"] = $result["still_parameters"]['tag'];

        // Calculating aspect ratio
        // Base value
        //$target_size = $query_job_row->size;
        $target_size = $result["still_parameters"]['size'];
        if ($query_job_row->size == 'sqcif') {
          $target_size = '128x96';
        }
        elseif ($query_job_row->size == 'qcif') {
          $target_size = '176x144';
        }
        elseif ($query_job_row->size == 'cif') {
          $target_size = '352x288';
        }
        elseif ($query_job_row->size == '4cif') {
          $target_size = '704x576';
        }
        elseif ($query_job_row->size == 'qqvga') {
          $target_size = '160x120';
        }
        elseif ($query_job_row->size == 'qvga') {
          $target_size = '320x240';
        }
        elseif ($query_job_row->size == 'vga') {
          $target_size = '640x480';
        }
        elseif ($query_job_row->size == 'svga') {
          $target_size = '800x600';
        }
        elseif ($query_job_row->size == 'xga') {
          $target_size = '1024x768';
        }
        elseif ($query_job_row->size == 'uxga') {
          $target_size = '1600x1200';
        }
        elseif ($query_job_row->size == 'qxga') {
          $target_size = '2048x1536';
        }
        elseif ($query_job_row->size == 'sxga') {
          $target_size = '1280x1024';
        }
        elseif ($query_job_row->size == 'qsxga') {
          $target_size = '2560x2048';
        }
        elseif ($query_job_row->size == 'hsxga') {
          $target_size = '5120x4096';
        }
        elseif ($query_job_row->size == 'wvga') {
          $target_size = '852x480';
        }
        elseif ($query_job_row->size == 'wxga') {
          $target_size = '1366x768';
        }
        elseif ($query_job_row->size == 'wsxga') {
          $target_size = '1600x1024';
        }
        elseif ($query_job_row->size == 'wuxga') {
          $target_size = '1920x1200';
        }
        elseif ($query_job_row->size == 'woxga') {
          $target_size = '2560x1600';
        }
        elseif ($query_job_row->size == 'wqsxga') {
          $target_size = '3200x2048';
        }
        elseif ($query_job_row->size == 'wquxga') {
          $target_size = '3840x2400';
        }
        elseif ($query_job_row->size == 'whsxga') {
          $target_size = '6400x4096';
        }
        elseif ($query_job_row->size == 'whuxga') {
          $target_size = '7680x4800';
        }
        elseif ($query_job_row->size == 'cga') {
          $target_size = '320x200';
        }
        elseif ($query_job_row->size == 'ega') {
          $target_size = '640x350';
        }
        elseif ($query_job_row->size == 'hd480') {
          $target_size = '852x480';
        }
        elseif ($query_job_row->size == 'hd720') {
          $target_size = '1280x720';
        }
        elseif ($query_job_row->size == 'hd1080') {
          $target_size = '1920x1080';
        }
        // First get source width and height.
        db_set_active("data");
        $width  = db_result(db_query("SELECT width FROM {mediafile_metadata} where mediafile_id = '%s' ", $mediafile_id));
        $height = db_result(db_query("SELECT height FROM {mediafile_metadata} where mediafile_id = '%s' ", $mediafile_id));
        db_set_active();
        // Get the parameter string.
        //watchdog('server', 'vpx_jobhandler $result: '. print_r($result, TRUE));
        //watchdog('server', "vpx_jobhandler: $width $height $target_size");
        $cmmd = _vpx_jobhandler_calc_aspect_ratio($width, $height, $target_size, FALSE, $result["h_padding"], $result["v_padding"]);
        //watchdog('server', 'vpx_jobhandler $cmmd: '. print_r($cmmd, TRUE));
        // Set result
        if ($cmmd) {
          $result['size'] = $cmmd['width'] .'x'. $cmmd['height'];
          $result['h_padding'] = $cmmd['h_padding'];
          $result['v_padding'] = $cmmd['v_padding'];
        }
        else {
          if ($width && $height) {
            $result['size'] = $width .'x'. $height;
          }
          else {
            $result['size'] = '176x144';
          }
          $result['h_padding'] = 0;
          $result['v_padding'] = 0;
        }

      }
      else {
        // Something went wrong in the analyse script
        // Fall back to the default values
        $result["frametime"] = VPX_STILL_DEFAULT_FRAME_TIME;
        $result["size"] = '176x144';
        $result["h_padding"] = 0;
        $result["v_padding"] = 0;
        $result["blackstill_check"] = 'FALSE';
        $result["tag"] = '';
      }
      break;
    case JOBTYPE_ANALYSE:
      break;
  }
  return $result;
}


/**
 * Calculate ffmpeg parameters if we have to maintain aspect ratio.
 *
 * @param $w_source
 *   Width of video source.
 * @param $h_source
 *   Height of video source.
 * @param $w_target
 *   Width of video to create.
 * @param $h_target
 *   Height of video to create.
 * @return
 *   Parameter string to use with ffmpeg.
 */
function _vpx_jobhandler_calc_aspect_ratio($w_source, $h_source, $target_size, $response_string = TRUE, $h_padding = NULL, $v_padding = NULL) {

  // Get target width and height, format is 'wxh'.
  preg_match('/(\d+)x(\d+)/',  $target_size, $matches);
  if ((is_array($matches)) && (count($matches) == 3)) {
    $w_target = $matches[1];
    $h_target = $matches[2];
  }
  else {
    return '';
  }

  if (!(($w_target > 0) && ($h_target > 0) && ($w_source > 0) && ($h_source > 0))) {
    return '';
  }

  if (isset($h_padding) && is_numeric($h_padding) && $h_padding >= 0 && isset($v_padding) && is_numeric($v_padding) && $v_padding >= 0) {
    $param_string = sprintf("-s:%dx%d;-padtop:%d;-padbottom:%d;-padleft:%d;-padright:%d", (int)$w_target, (int)$h_target, (int)$h_padding, (int)$h_padding, (int)$v_padding, (int)$v_padding);
    $param_array = array('width' => (int)$w_target, 'height' => (int)$h_target, 'h_padding' => (int)$h_padding, 'v_padding' => $v_padding);
    return ( $response_string ? $param_string : $param_array );
  }

  $w_ratio = $w_source / $w_target;
  $h_ratio = $h_source / $h_target;

  if ($w_ratio > $h_ratio) {
    // Adjust height padding.
    $padding = $h_target - ($h_source / $w_ratio); // Total size of padding.
    $padding = (($padding - ($padding % 4)) /2);   // Single size of padding (must be even).
    $new_height = $h_target - (2 * $padding);
    $new_height = ($new_height - ($new_height % 2)); // must be even.
    $param_string = sprintf("-s:%dx%d;-padtop:%d;-padbottom:%d", (int)$w_target, (int)$new_height, (int)$padding, (int)$padding);
    $param_array = array('width' => (int)$w_target, 'height' => (int)$new_height, 'h_padding' => (int)$padding, 'v_padding' => 0);
  }
  else {
    // Adjust width padding.
    $padding = $w_target - ($w_source / $h_ratio); // Total size of padding.
    $padding = (($padding - ($padding % 4)) /2);   // Single size of padding (must be even).
    $new_width = $w_target - (2 * $padding);
    $new_width = ($new_width - ($new_width % 2)); // must be even.
    $param_string = sprintf("-s:%dx%d;-padleft:%d;-padright:%d", (int)$new_width, (int)$h_target, (int)$padding, (int)$padding);
    $param_array = array('width' => (int)$new_width, 'height' => (int)$h_target, 'h_padding' => 0, 'v_padding' => (int)$padding);
  }
  return ( $response_string ? $param_string : $param_array );
}


/**
 * Voor tanscoding worden de 'mooie' parameters vertaald naar de
 * parameters die door de tool gebruikt worden.
 */
function _vpx_jobhandler_map_parameters($tool, $command, $mediafile_id) {
  $cli = "";

  // first determine if we have to alter the aspect ratio.
  $alter_size_param = FALSE;
  if ((strstr($command, 'maintain_aspect_ratio:yes') !== FALSE) && (strstr($command, 'size:') !== FALSE)) {
      $alter_size_param = TRUE;
  }

  // op basis van de toolname worden alle parameters die gebruikt kunnen
  // worden opgevraagd. De opgegeven parameters worden hier mee gecontroleerd.
  $query_mapping = db_query(
      "SELECT nice_parameter, tool_parameter, min_value, max_value,
              allowed_value, default_value, required
       FROM {transcode_mapping}
        WHERE tool = '%s'", $tool);
  $cli = "";
  while ($query_mapping_row = db_fetch_object($query_mapping)) {
    if (($query_mapping_row->nice_parameter == 'size') && ($alter_size_param)) {
      $parameters = create_named_array($command, ";", ":");
      $target_size = $parameters['size'];// We skip the size here.
    }
    else {
      if ($cli != "") {
        $cli .= ";";
      }
      $cli .= _job_handler_mapping_value(
        $query_mapping_row->nice_parameter,
        $query_mapping_row->tool_parameter,
        $query_mapping_row->min_value,
        $query_mapping_row->max_value,
        $query_mapping_row->allowed_value,
        $query_mapping_row->default_value,
        $query_mapping_row->required,
        $command);
    }
  }

  // Adjust for aspect ratio? makes only sense if size is set, else we
  // convert to size same as source (default ffmpeg behaviour).
  if ($alter_size_param) {
    // First get source width and height.
    db_set_active("data");
    $width  = db_result(db_query("SELECT width FROM {mediafile_metadata} where mediafile_id = '%s' ", $mediafile_id));
    $height = db_result(db_query("SELECT height FROM {mediafile_metadata} where mediafile_id = '%s' ", $mediafile_id));
    db_set_active();

    // Get the parameter string.
    $cli .= ";" . _vpx_jobhandler_calc_aspect_ratio($width, $height, $target_size);
  }

  return $cli;
}


/**
 * Verwerk de analyse metadata van een string naar een named array
 */
function _vpx_jobhandler_parse_metadata($metadata) {

  $a_metadata = create_named_array($metadata, "\n", ": ");

  // MIME-type
  $a_parameters["mime_type"] = isset($a_metadata["MIME-type"]) ? $a_metadata["MIME-type"] : "";

  // Video-codec
  $a_parameters["video_codec"] = isset($a_metadata["Video-codec"]) ? $a_metadata["Video-codec"] : "";

  // Video-colorspace
  $a_parameters["colorspace"] = isset($a_metadata["Video-colorspace"]) ? $a_metadata["Video-colorspace"] : "";

  // Video-size
  list($width, $height) = isset($a_metadata["Video-size"]) ? explode("x", $a_metadata["Video-size"]) : array(0, 0);
  $a_parameters["width"] = $width;
  $a_parameters["height"] = $height;

  // Video-framespersecond
  $a_parameters["fps"] = isset($a_metadata["Video-framespersecond"]) ? $a_metadata["Video-framespersecond"] : "";

  // Audio-codec
  $a_parameters["audio_codec"] = isset($a_metadata["Audio-codec"]) ? $a_metadata["Audio-codec"] : "";

  // Audio-frequency
  $a_parameters["sample_rate"] = isset($a_metadata["Audio-frequency"]) ? $a_metadata["Audio-frequency"] : "";

  // Audio-channels
  $a_parameters["channels"] = isset($a_metadata["Audio-channels"]) ? $a_metadata["Audio-channels"] : "";

  // file-duration
  $a_parameters["file_duration"] = isset($a_metadata["File-duration"]) ? $a_metadata["File-duration"] : "";

  // container
  $a_parameters["container_type"] = isset($a_metadata["File-type"]) ? $a_metadata["File-type"] : "";

  // file-bitrate
  $a_parameters["bitrate"] = isset($a_metadata["File-bitrate"]) ? $a_metadata["File-bitrate"] : "";

  // bpp
  $a_parameters["bpp"] = _vpx_jobhandler_calculate_bpp($width, $height, $a_parameters["fps"], $a_parameters["bitrate"]);

  // is hinted
  $a_parameters["is_hinted"] = isset($a_metadata["Is-hinted"]) ? $a_metadata["Is-hinted"] : "no";

  // has inserted extra metadata
  $a_parameters["is_inserted_md"] = isset($a_metadata["Is-inserted-md"]) ? $a_metadata["Is-inserted-md"] : "no";

  // The output of ffmpeg
  $a_parameters["ffmpeg_output"] = isset($a_metadata['ffmpeg-output']) ? implode("\n", explode("}-{", $a_metadata['ffmpeg-output'])) : '';

  return $a_parameters;
}

// controleer de opgegeven waarde tegen de database waarden en geef
// een toegestane waarde terug.
function _job_handler_mapping_value($nice_parameter, $tool_parameter, $min_value, $max_value,
                           $allowed_value, $default_value, $required, $command) {
  $parameters = create_named_array($command, ";", ":");
  $result = "";

  foreach ($parameters as $name => $value) {
    if (($name == $nice_parameter) && ($tool_parameter != "")) {
      $result = sprintf("%s:%s", $tool_parameter, $value);

      // valideer de ontvangen waarde tegen de toegestane waarden
      if ($min_value != null) {
        if ($min_value > $value) {
          return sprintf("%s:%s", $tool_parameter, $default_value);
        }
      }

      if ($max_value != null) {
        if ($max_value < $value) {
          return sprintf("%s:%s", $tool_parameter, $default_value);
        }
      }

      if ($allowed_value != null) {
        $allowed_values = split(";", $allowed_value);
        if (!(in_array($value, $allowed_values))) {
          return sprintf("%s:%s", $tool_parameter, $default_value);
        }
      }
      // alle checks overleefd, nu mogen we eruit
      return $result;
    }
  }

  // controleren of we de parameter uberhaubt hadden gevonden
  // wanneer dat niet het geval is maar de parameter is wel verplicht
  // dan wordt de default waarde gezet.
  if ($result == "") {
    if ($required == "TRUE") {
      return sprintf(";%s:%s", $tool_parameter, $default_value);
    }
  }

  return $result;
}

function _vpx_jobhandler_create_analyse_job($owner, $asset_id, $mediafile_id, $app_id, $testtag) {
  if ($testtag != "TRUE") {
    $testtag = "FALSE";
  }
  watchdog("jobhandler", sprintf("Start een analyse job: mediafileid=%s", $mediafile_id), null);
  # FIX ME, wat kan er allemaal fout gaan? vang die fouten op.
  db_set_active("data");
  $query_result = db_query(
    "INSERT INTO {job}
    (asset_id, mediafile_id, owner, priority, job_type, created, app_id, testtag)
    VALUES
    ('%s', '%s', '%s', -1, '%s', NOW(), %d, '%s')",
      $asset_id, $mediafile_id, $owner, JOBTYPE_ANALYSE, $app_id, $testtag);
  db_set_active();
}


function _vpx_jobhandler_calculate_bpp($width, $height, $fps, $bitrate) {
  $result = "";
  if (($width != "") && ($height != "") && ($fps != "") && ($bitrate != "")) {
    $result = round((($bitrate * 1000) / ($fps * $width * $height)), 2);
  }

  return $result;
}

/**
 * Verwijder de job-server-relatie van de job lijst.
 */
function _vpx_jobhandler_clear_serverjob($job_id) {
  db_set_active("data");
  $query_result = db_query(
    "DELETE FROM {server_job}
    WHERE job_id = %d", $job_id);
  db_set_active();
}

/**
 * Stuur een job-cancel naar de server. De afgesloten job zal dan uit de
 * joblist van de jobserver worden verwijderd.
 */
function _vpx_jobhandler_remove_job_on_server($ip_address, $job_id) {

  // stuur de job naar de job server
  $rest_url = sprintf("http://%s/%s", $ip_address, VPX_BUILD_URL);
  $rest_url = $rest_url . sprintf("/internal/server/%s/delete", $job_id);
  //$rest_url = $rest_url . sprintf("?killjob=TRUE");
//  $data = http_build_query(array_merge(array("job_id" => $job_id,
//            "job_type" => $job_type, "mediafile_src" => $mediafile_src),
//             $job_parameters));
  $data = "";
  $response = drupal_http_request(
    $rest_url, // url
    array("Content-Type: application/x-www-form-urlencoded",
          "Content-Length: " . strlen($data) ), // headers
    'POST', // method
    $data
  );
}

/**
 * Haal de informatie van een transcodejob op met daarbij gekoppeld de
 * filename van het orginele bron bestand
 * velden: asset_id, filename
 */
function _vpx_jobhandler_get_transcodejob_info($job_id) {
  $result = array();

  db_set_active("data");
  $query_result = db_query(
    "SELECT sj1.asset_id, filename, sj1.job_type, sj1.owner, sj1.app_id,
            tj1.tool, tj1.transcode_profile_id,
            tj1.file_extension, tj1.command, mf1.mediafile_id
            FROM {job} sj1
      INNER JOIN {mediafile} mf1 ON sj1.asset_id = mf1.asset_id
      INNER JOIN {transcode_job} tj1 ON tj1.job_id = sj1.job_id
      WHERE sj1.job_id = %d AND is_original_file = 'TRUE'",
    $job_id);
  db_set_active();

  if ($query_result_row = db_fetch_object($query_result)) {
    $new_filename = get_base_filename($query_result_row->filename) .".". $query_result_row->file_extension;
    $result = array("asset_id" => $query_result_row->asset_id,
                    "owner" => $query_result_row->owner,
                    "app_id" => $query_result_row->app_id,
                    "filename" => $new_filename,
                    "transcode_profile_id" => $query_result_row->transcode_profile_id,
                    "tool" => $query_result_row->tool,
                    "file_extension" => $query_result_row->file_extension,
                    "command" => $query_result_row->command,
                    "mediafile_id" => $query_result_row->mediafile_id);
  }
  else {
    $mm_link = _vpx_jobserver_get_asset_link($job_id);
    watchdog("handler", sprintf("Het bijbehorende orginele bestand is niet gevonden voor job_id %s <br /><br />%s", $response_data->items->item[$i]->job_id, $mm_link));
  }
  return $result;
}


/**
 * Verwerk de informatie in de database die terugkomt van een stilljob
 */
function _vpx_jobhandler_add_still_to_db($job_id, $filenames) {
  // Scene changes
  $destination_path = SAN_NAS_BASE_PATH . DS . DATA_LOCATION . DS . 'transcode' . DS;
  $my_file = $destination_path . $job_id .'_scene.txt';
  $fh = @fopen($my_file, 'r') or $fh = NULL;
  $scenes = array();
  if ($fh) {
    while(!feof($fh)) {
      $scenes[] = fgets($fh);
    }
    fclose($fh);
  }

  // haal de asset_id op
  db_set_active("data");
  $query_result = db_query("SELECT asset_id, owner, app_id, still_parameters, mediafile_id FROM {job} WHERE job_id=%d", $job_id);
  $a_job = db_fetch_array($query_result);
  $asset_id = $a_job["asset_id"];
  $still_parameters = unserialize($a_job["still_parameters"]);

  $query_result = db_query("SELECT parent_id FROM {asset} WHERE asset_id='%s'", $asset_id);
  $parent_id = db_result($query_result);

  if (!$parent_id) {
    $parent_id = $asset_id;
  }

  db_set_active();

  watchdog("jobhandler", sprintf("Start aanmaken DB still, e.g. %s, job: %d", $filenames[0], $job_id));

  // verwijder de oude stills
  // We have multiple stills now, so we don't delete the old ones
  // And deleting with asset_id is definetly not a good idea, while we have multiple stills per mediafile
  //_media_management_delete_still($asset_id);

  // voeg een record toe aan de mediafile metadata tabel.
  if (is_array($filenames)) {
    $frametime = $still_parameters['frametime'];
    $duration = $still_parameters['duration'];
    if (isset($still_parameters['framerate']) && is_numeric($still_parameters['framerate'])) {
      $second = 1/$still_parameters['framerate'];
    }
    $tag = $still_parameters['tag'];

    $order = 0;
    $sec = 0;
    if (isset($frametime) && is_numeric($frametime)) {
      $sec = $frametime;
    }
    $i = 0;
    foreach ($filenames as $filename) {
      media_management_create_still($asset_id, $parent_id, $filename, $a_job["app_id"], $a_job["owner"], "", $order, !$order, $still_parameters, ( $scenes == array() ? $sec : $scenes[$i] ), $a_job["mediafile_id"], $tag);
      $order++;
      if (isset($second) && is_numeric($second)) {
        $sec += $second;
      }

      $i++;
    }
  }

  @unlink($my_file);
}

/**
 * Kies een job geldt die voldoet aan de volgende voorwaarden:
 * - er is GEEN andere job voor de zelfde asset in behandeling (op enige server)
 * - job status is WAITING
 * - er bestaat GEEN actieve upload job voor de asset
 * - lage prioriteit gaat eerst
 * - de job mag niet op een andere server zijn gestart
 */
function _vpx_jobhandler_fetch_single_available_job($server_id, $testtag) {
  $result = array();
  db_set_active("data");

  $query_activejob = db_query(
    "SELECT job_id, job_type, asset_id, mediafile_id, status, progress FROM {job} AS j1 WHERE
          j1.status = '%s'
        AND
          j1.asset_id NOT IN
            (SELECT asset_id FROM {job} AS j2
             INNER JOIN {server_job} AS sj1 USING(job_id)
             WHERE j2.status IN ('%s', '%s', '%s'))
        AND
          j1.asset_id NOT IN
            (SELECT asset_id FROM {job} AS j3
             WHERE j3.job_type = '%s' AND j3.status IN ('%s', '%s'))
        AND
          j1.testtag = '%s'
        AND
          j1.job_type <> '%s'
        AND
          (
            (j1.job_type = '%s' AND
              (SELECT COUNT(*) FROM transcode_job AS tj1
                INNER JOIN %s.transcoding_server_tool USING(tool)
                WHERE transcoding_server_id=%d AND tj1.job_id = j1.job_id LIMIT 1) = 1)
          OR
            (j1.job_type = '%s' AND
              (SELECT COUNT(*) FROM %s.transcoding_server_tool AS tsc1
                WHERE tsc1.tool = '%s' AND transcoding_server_id=%d LIMIT 1) = 1)
          OR
            (j1.job_type = '%s' AND
              (SELECT COUNT(tsc1.tool) FROM %s.transcoding_server_tool AS tsc1
                WHERE tsc1.tool = '%s' AND transcoding_server_id=%d LIMIT 1) = 1)
          OR
            (j1.job_type = '%s')
          )
        AND
          j1.job_id NOT IN
            (SELECT job_id FROM {server_job})
        ORDER BY priority, job_id
        LIMIT 0,1",
        JOBSTATUS_WAITING,
        JOBSTATUS_INPROGRESS, JOBSTATUS_CANCELLING, JOBSTATUS_WAITING,
        JOBTYPE_UPLOAD, JOBSTATUS_INPROGRESS, JOBSTATUS_CANCELLING,
        $testtag,
        JOBTYPE_UPLOAD,
        JOBTYPE_TRANSCODE,
        vpx_db_query_dbname('default'),
        $server_id,
        JOBTYPE_ANALYSE,
        vpx_db_query_dbname('default'),
        JOBTYPE_ANALYSE, $server_id,
        JOBTYPE_STILL,
        vpx_db_query_dbname('default'),
        JOBTYPE_STILL, $server_id,
        JOBTYPE_DELETE_MEDIAFILE
  );

  if ($query_activejob_row = db_fetch_object($query_activejob)) {
    $result["job_id"] = $query_activejob_row->job_id;
    $result["job_type"] = $query_activejob_row->job_type;
    $result["mediafile_id"] = $query_activejob_row->mediafile_id;
    $result["asset_id"] = $query_activejob_row->asset_id;
  }

  db_set_active();// RBL: added, was missing ???

  return $result;
}

function _vpx_jobhandler_create_wmv_job($orginele_job_id, $asset_id, $mediafile_id, $testtag) {
  if ($testtag != "TRUE") {
    $testtag = "FALSE";
  }

  watchdog("jobhandler", sprintf("Start job wmv job: orginelejobid=%s, mediafileid=%s", $orginele_job_id, $mediafile_id), null);
  db_set_active("data");
  // haal de orginele parameters op
  $query_job = db_query(
    "SELECT command, owner, app_id FROM {job} sj1
      INNER JOIN {transcode_job} tj1 ON tj1.job_id = sj1.job_id
      WHERE sj1.job_id = %d", $orginele_job_id);
  if ($query_job_row = db_fetch_object($query_job)) {
    $command = $query_job_row->command;
    $owner = $query_job_row->owner;
    $app_id = $query_job_row->app_id;
  }
  else {
    // RBL:
    // Hmmm geen orginele job? Is dan niet beter om de wmv job niet aan te maken?
    // Nu transcode je een file waarvan er geen owner, command, app_id aan hangt....
    $command = "";
    $owner = "";
    $app_id = 0;
    watchdog("jobhandler", "Een wmv job zonder transcode_job!", array(), WATCHDOG_EMERG);
  }
  // voeg een nieuwe job toe
  $query_result = db_query(
    "INSERT INTO {job}
    (asset_id, mediafile_id, owner, app_id, priority, job_type, created, testtag)
    VALUES
    ('%s', '%s', '%s', %d, -1, '%s', NOW(), '%s')",
      $asset_id, $mediafile_id, $owner, $app_id, JOBTYPE_TRANSCODE, $testtag);
  $job_id = db_last_insert_id("job", "job_id");

  // en voeg de transcode job toe
  $query_result = db_query(
    "INSERT INTO {transcode_job}
    (job_id, transcode_profile_id, tool, command, file_extension)
    VALUES
    ('%d', NULL, '%s', '%s', '%s')",
    $job_id, JOBWINDOWS_TOOL, $command .";internal_previous_job:". $orginele_job_id, "wmv");
  db_set_active();
}

/**
 * Called when transcode was successfully finished
 *
 * @param integer $job_id
 * @param string $mediafile_dest
 * @param string $testtag
 */
function _vpx_jobhandler_parse_finished_transcode($job_id, $mediafile_dest, $testtag) {
  // haal de asset id en original filename op basis van een job op
  $job_info = _vpx_jobhandler_get_transcodejob_info($job_id);

  // controleer of de vorige job_id naar boven is te halen.
  $command_parameters = create_named_array($job_info["command"], ";", ":");
  $original_job_id = $command_parameters['internal_previous_job'];
  if ($original_job_id > 0) {

    // controleer op basis van de file_extension of er een extra job
    // aangemaakt moet worden tbv windows transcoding
    if ($job_info["file_extension"] == JOBRAW_FILE_EXTENSION) {
      // 1st pass: the raw intermediate has been created, now convert it to wmv
      db_set_active("data");
      // NOTE should *not* be converted to api call: the API sets
      // ownership, and we do not want this intermediate result to
      // influence quota &c.
      $query_result = db_query(
        "INSERT INTO {mediafile}
        (mediafile_id, asset_id, filename, is_original_file, app_id, testtag, sannas_mount_point)
        VALUES
        ('%s', '%s', '%s', 'TRUE', %d, '%s', '%s')",
          $mediafile_dest, $job_info["asset_id"], $job_info["filename"], $job_info["app_id"], $testtag, SAN_NAS_BASE_PATH);
      db_set_active();
      // voeg een nieuwe job toe aan de scheduler
      _vpx_jobhandler_create_wmv_job($original_job_id, $job_info["asset_id"], $mediafile_dest, $testtag);

      // RBL:
      // Update the original job for progress
      db_set_active("data");
      db_query("UPDATE {job} SET progress = '0.666' WHERE job_id=%d", $original_job_id);
      db_set_active();

      // bail out: the mediafile record for the intermediate result
      // has been created; we don't want any further action here.
      return;
    }
    else {
      // 2nd pass: the raw intermediate has been converted to wmv, now delete the intermediate
      _vpx_jobhandler_wmv_remove_raw_file($job_id);

      // Set the original job to finished
      db_set_active("data");
      $status = db_result(db_query_range("SELECT status FROM {job} WHERE job_id = %d", $original_job_id, 0, 1));
      db_query("UPDATE {job} SET status = '%s', progress = '1.000' WHERE job_id = '%d'", JOBSTATUS_FINISHED, $original_job_id);
      db_set_active();
      vpx_jobs_notify_transcoding($status, JOBSTATUS_FINISHED, $original_job_id);

    }
  }

  // NOTE this should be converted to api call
  db_set_active("data");

  $asset_id_root = db_result(db_query("SELECT parent_id FROM {asset} WHERE asset_id='%s'", $job_info["asset_id"]));
  $asset_id_root = ((is_null($asset_id_root) || ($asset_id_root == "")) ? $job_info["asset_id"] : $asset_id_root);

  $query_result = db_query(
    "INSERT INTO {mediafile}
    (mediafile_id, asset_id, asset_id_root, filename, is_original_file,
     testtag, sannas_mount_point, transcode_profile_id,
     tool, file_extension, command, owner_id, app_id)
    VALUES
    ('%s', '%s', '%s', '%s', '%s', 'FALSE', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
      $mediafile_dest, $job_info["asset_id"], $asset_id_root, $job_info["filename"], $testtag,
      SAN_NAS_BASE_PATH, $job_info["transcode_profile_id"], $job_info["tool"],
    $job_info["file_extension"], $job_info["command"], $job_info["owner"], $job_info["app_id"]);

    // Check if we need to copy ACL rights from original
    $transcode_inherits_acl = db_result(db_query("SELECT transcode_inherits_acl FROM {mediafile} WHERE mediafile_id = '%s'", $job_info['mediafile_id']));
    if (vpx_shared_boolstr2bool($transcode_inherits_acl)) {
       vpx_acl_replace_mediafile_to_mediafile($job_info['mediafile_id'], $mediafile_dest);
    }

    db_set_active();

  // add analyse job
  _vpx_jobhandler_create_analyse_job($job_info["owner"], $job_info["asset_id"], $mediafile_dest, $job_info["app_id"], $testtag);
}

/**
 * Called when transcode failed or was canceled
 *
 * @param integer $job_id
 * @param string $mediafile_dest
 * @param string $testtag
 */
function _vpx_jobhandler_parse_failed_transcode($job_id, $mediafile_dest, $testtag) {
  // haal de asset id en original filename op basis van een job op
  $job_info = _vpx_jobhandler_get_transcodejob_info($job_id);

  // controleer of de vorige job_id naar boven is te halen.
  $command_parameters = create_named_array($job_info["command"], ";", ":");
  $original_job_id = $command_parameters['internal_previous_job'];
  if ($original_job_id > 0) {
    if ($job_info["file_extension"] != JOBRAW_FILE_EXTENSION) {

      // Remove the raw file
      _vpx_jobhandler_wmv_remove_raw_file($job_id);

      // Set the original job to failed
      db_set_active("data");
      $status = db_result(db_query_range("SELECT status FROM {job} WHERE job_id = %d", $original_job_id, 0, 1));
      db_query("UPDATE {job} SET status = '%s', progress = '1.000' WHERE job_id = '%d'", JOBSTATUS_FAILED, $original_job_id);
      db_set_active();
      vpx_jobs_notify_transcoding($status, JOBSTATUS_FAILED, $original_job_id);

    }
  }
}

/**
 * Remove the raw avi file for wmv transcode
 *
 * @param integer $job_id
 */
function _vpx_jobhandler_wmv_remove_raw_file($job_id) {
  db_set_active("data");
  $mediafile_id = db_result(db_query("SELECT mediafile_id FROM {job} WHERE job_id=%d", $job_id));
  db_set_active();
  assert($mediafile_id);

  if ($mediafile_id) {
    // Delete the raw file
    unlink(vpx_get_san_nas_base_path() . DS . DATA_LOCATION . DS . $mediafile_id{0} . DS . $mediafile_id);

    // Delete the mediafile
    db_set_active("data");
    db_query("DELETE FROM {mediafile} WHERE mediafile_id='%s'", $mediafile_id);
    db_set_active();

    watchdog("jobhandler", sprintf("Tijdelijke mediafile '%s' van job '%d' is verwijderd.", $mediafile_id, $job_id));
  }
  else{
    watchdog("jobhandler", sprintf("Job %d niet gevonden in de database, kan de tijdelijke mediafile niet verwijderen.", $job_id), array(), WATCHDOG_ERROR);
  }
}


/**
 * Check Jobs which have no progress. Change status to FAILED.
 */
function _vpx_jobhandler_check_upload_timeout() {

  // Upload jobs in progress and waiting which have no activity.
  db_set_active("data");
  $query_result = db_query(
     "UPDATE
      {job} SET status = '%s', FINISHED= NOW(),
      error_description = 'UPLOAD changed timeout expired (%ds)'
      WHERE status in ('%s', '%s') AND job_id IN
      (SELECT job_id FROM {upload_job} WHERE NOW()-changed > %d)",
      JOBSTATUS_FAILED, JOB_UPLOAD_TIMEOUT, JOBSTATUS_INPROGRESS, JOBSTATUS_WAITING, JOB_UPLOAD_TIMEOUT
  );
  // Jobs with no activity for 3 hours.
  $query_result = db_query(
     "UPDATE
      {job} SET status = '%s', FINISHED= NOW(),
      error_description = 'JOB changed timeout expired (%ds)'
      WHERE status = '%s' AND NOW()-changed > %d",
      JOBSTATUS_FAILED, JOB_JOB_TIMEOUT, JOBSTATUS_INPROGRESS, JOB_JOB_TIMEOUT
  );
  db_set_active();
}



/**
 * Controleer of er transcoding jobs zijn waarvoor geldt dat al meer dan
 * x sec draaien. (bijv 1 uur)
 */
function _vpx_jobhandler_check_transcode_timeout() {

  db_set_active("data");
  $query_result = db_query(
     "SELECT jb1.job_id, ip_address
      FROM {job} jb1
      INNER JOIN {server_job} sj1 ON jb1.job_id = sj1.job_id
      INNER JOIN {%s.transcoding_server} ts1 ON sj1.server_id = ts1.transcoding_server_id
      WHERE NOT started IS NULL AND TIME_TO_SEC( TIMEDIFF(NOW() ,jb1.started)) > %d",
      vpx_db_query_dbname('default'),
      JOB_TRANSCODE_TIMEOUT
  );
  db_set_active();
  while ($query_result_row = db_fetch_object($query_result)) {
    // voor elke gevonden job stuur een verwijder request naar de transcoding server
    _vpx_jobhandler_remove_job_on_server($query_result_row->ip_address, $query_result_row->job_id);

    // verwijder de job uit de koppeltabel
    _vpx_jobhandler_clear_serverjob($query_result_row->job_id);

    // update de status van de job.
    $error_message = vpx_shared_return_message(ERRORCODE_JOB_TRANSCODE_TIMEOUT, array("@timeout" => JOB_TRANSCODE_TIMEOUT));
    db_set_active("data");

    $status = db_result(db_query_range("SELECT status FROM {job} WHERE job_id = %d", $query_result_row->job_id, 0, 1));
    db_query(
      "UPDATE {job} SET status='%s', error_description='%s'
      WHERE job_id = %d",
      JOBSTATUS_FAILED, $error_message,  $query_result_row->job_id);
    db_set_active();
    vpx_jobs_notify_transcoding($status, JOBSTATUS_FAILED, $query_result_row->job_id);

    watchdog("jobhandler", sprintf("Transcode job verwijderd vanwege timeout, job_id: %d", $query_result_row->job_id), null);
  }
}


/**
 * Controleer of er transcode servers zijn die op CLOSE staan en geen
 * geen jobs meer hebben toegewezen. Zet deze dan op OFF
 */
function _vpx_jobhandler_check_server_on_status_close() {

  db_set_active();
  $query_result = db_query(
     "UPDATE {transcoding_server}
      SET status = '%s' WHERE
      status = '%s' AND transcoding_server_id NOT IN
      (SELECT DISTINCT server_id FROM {%s.server_job})",
      SERVERSTATUS_OFF, SERVERSTATUS_CLOSE, vpx_db_query_dbname('data'));
}

/**
 * Geeft informatie terug over een job:
 * - een lijst terug van de jobs op basis van een ownerid.
 * - een lijst terug van de jobs op basis van een asset_id.
 * - een job op basis van job_id
 * Wanneer er een fout optreed wordt er een errorstatus melding teruggegeven
 * In alle andere gevallen wordt een okay-status met eventuele informatie teruggegeven.
 */
function vpx_jobs_get_job_list($a_parameters) {
  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  $where = sprintf("jb1.owner = '%s' AND jb1.app_id = %d ",
     $a_parameters['user_id']['value'], $a_parameters['app_id']['value']);

  if ($a_parameters['job_id']['value'] != "") {
    $where .= " AND jb1.job_id = ". $a_parameters['job_id']['value'];
  }
  if (isset($a_parameters['asset_id']) && $a_parameters['asset_id']['value'] != "") {
    $where .= " AND (jb1.asset_id = '". $a_parameters['asset_id']['value'] ."' ";
    $where .= "OR as1.parent_id = '". $a_parameters['asset_id']['value'] ."')";
  }
  if (isset($a_parameters['mediafile_id']) && $a_parameters['mediafile_id']['value'] != "") {
    $where .= " AND jb1.mediafile_id = '". $a_parameters['mediafile_id']['value'] ."' ";
  }

  // voer de query uit op de 'data' database
  db_set_active("data");
  $db_result = db_query(
    "SELECT jb1.job_id, jb1.owner, jb1.status, jb1.asset_id, jb1.progress,
            jb1.priority, jb1.job_type, UNIX_TIMESTAMP(jb1.started) AS started,
            jb1.testtag, error_description ".
    "FROM {job} jb1 ".
    "INNER JOIN {asset} as1 on as1.asset_id = jb1.asset_id ".
    "WHERE jb1.testtag = '%s' AND ". $where
    ." ORDER BY jb1.asset_id, jb1.priority",
      $a_parameters['testtag']['value']
  );

  db_set_active();

  // vul de response met de jobs.
  $rest_jobs = new rest_response();
  while ($db_result_row = db_fetch_object($db_result)) {
    $job = array();
    $job["id"] = $db_result_row->job_id;
    $job["owner"] = $db_result_row->owner;
    $job["status"] = $db_result_row->status;
    $job["progress"] = $db_result_row->progress;
    $job["priority"] = $db_result_row->priority;
    $job["job_type"] = $db_result_row->job_type;
    $job["started"] = ($db_result_row->started == null) ? "" : date("d-m-Y H:i", $db_result_row->started);
    $job["started_unix"] = ($db_result_row->started == null) ? "" : $db_result_row->started;
    $job["error_description"] = $db_result_row->error_description;
    if ($a_parameters['testtag']['value'] == "TRUE") {
      $job["testtag"] = $db_result_row->testtag;
    }
    $rest_jobs->add_item($job);
  }

  // Voeg de status van de request samen met de lijst van jobs
    $rest_jobs->set_result(
      vpx_return_error(ERRORCODE_OKAY)
    );
    return $rest_jobs;

} // end of vpx_jobs_get_job_list_user

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



/**
 * Functie voor het toevoegen van een nieuwe upload job aan de database
 * Voor een upload job wordt de file_size opgegeven. Deze wordt gecontroleerd
 * tegen het uiteindelijk upgeloade bestand om te bepalen of de upload gelukt is
 */
function _vpx_jobs_create_new_job_upload($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'job_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', TYPE_GROUP_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'mediafile_id', VPX_TYPE_ALPHANUM, TRUE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'file_size', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'retranscode', VPX_TYPE_BOOL, FALSE, 'FALSE');
    vpx_funcparam_add_post($a_funcparam, $a_args, 'create_still', VPX_TYPE_BOOL, FALSE, 'FALSE');

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');
    $job_id = vpx_funcparam_get_value($a_funcparam, 'job_id');
    $file_size = vpx_funcparam_get_value($a_funcparam, 'file_size');
    $retranscode = vpx_funcparam_get_value($a_funcparam, 'retranscode');
    $create_still = vpx_funcparam_get_value($a_funcparam, 'create_still');
    $mediafile_id = vpx_funcparam_get_value($a_funcparam, 'mediafile_id');

    // test of er niet nog een actieve/waiting upload job is
    db_set_active("data");
    $db_result = db_query("SELECT COUNT(*) FROM {job}".
      vpx_db_simple_where(
        array(
          "job_type = '%s'",
          "mediafile_id = '%s'",
          "job_id <> %d",
          "status IN ('%s','%s','%s')"
        )
      ),
      JOBTYPE_UPLOAD,
      $mediafile_id,
      $job_id,
      JOBSTATUS_WAITING, JOBSTATUS_INPROGRESS, JOBSTATUS_CANCELLING
    );
    db_set_active();

    if (db_result($db_result)) {
      throw new vpx_exception(ERRORCODE_UPLOAD_ALREADY_EXISTS);
    }

    // voeg een record toe aan de trancode_job tabel
    db_set_active("data");
    $db_result = db_query("INSERT INTO {upload_job}".
      vpx_db_simple_set(
        array(
          "job_id = %d",
          "file_size = %d",
          "retranscode = '%s'",
          "create_still = '%s'"
        )
      ),
      $job_id,
      $file_size,
      $retranscode,
      $create_still
    );
    db_set_active();

    // log de upload
    _vpx_statistics_log_file_upload($app_id, $user_id, $group_id, $file_size);

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * Functie voor het toevoegen van een nieuwe still job aan de database
 * Voor de still is een frame_time en size noodzakelijk; als deze niet
 * zijn opgegeven dan worden de defaults gebruikt (vpx_shared.defines)
 */
function _vpx_jobs_create_new_job_still($job_id, $a_args) {
  //watchdog('server', 'create job still: '. print_r($a_args, TRUE));
  $a_parameters = array(
    'job_id' => array(
      'value' => $job_id,
      'type' => 'int',
      'required' => TRUE
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
      'type' => 'skip'
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
      'type' => VPX_TYPE_ALPHA,
    ),
    'frametime' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'frametime', VPX_STILL_DEFAULT_FRAME_TIME),
      'type' => 'int'
    ),
  );

  // valideer op nieuw de extra velden die noodzakelijk zijn voor het toevoegen van een transcode job
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    $rest_response = new rest_response($result);
    return $rest_response;
  }

  // If the user didn't give us a size parameter, it becames default
  if (!$a_parameters['size']['value']) {
    $a_parameters['size']['value'] = VPX_STILL_DEFAULT_SIZE;
  }

  $blackstill_check = 'TRUE';
  switch ($a_parameters['still_type']['value']) {
    case 'NORMAL':
      $still_per_mediafile = $a_parameters['still_per_mediafile']['value'];
      $start_frame = $a_parameters['start_frame']['value'];
      $end_frame = $a_parameters['end_frame']['value'];
      $blackstill_check = 'FALSE';
      break;
    case 'SECOND':
      $still_every_second = $a_parameters['still_every_second']['value'];
      $start_frame = $a_parameters['start_frame']['value'];
      $end_frame = $a_parameters['end_frame']['value'];
      $blackstill_check = 'FALSE';
      break;
    case 'SCENE':
      $blackstill_check = 'FALSE';
      break;
    default:
      break;
  }

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
    //'width' => $a_parameters['width']['value'],
    //'height' => $a_parameters['height']['value'],
  );

  // voeg een record toe aan de still_job tabel
  db_set_active("data");
  $db_result = db_query("
    INSERT INTO {still_job}
    (job_id, frametime, size, h_padding, v_padding, blackstill_check, still_parameters) VALUES
    (%d, %d, '%s', %d, %d, '%s', '%s')",
    $a_parameters['job_id']['value'], $a_parameters['frametime']['value'], $a_parameters['size']['value'], $a_parameters['h_padding']['value'], $a_parameters['v_padding']['value'], $blackstill_check, serialize($still_parameters));
  db_set_active();

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

/**
 * Voor het kunnen transcoderen van een mediafile naar een windows formaat
 * moet eerst een raw transcode worden uitgevoerd.
 */

function _vpx_jobs_create_raw_job($original_job_id, $testtag) {
  if ($testtag != "TRUE") {
    $testtag = "FALSE";
  }
  db_set_active("data");

  // RBL: Changed to inprogress, so we can keep track of the status of the job...
  $db_result = db_query(
        "UPDATE {job}
         SET status = '%s', STARTED = NOW(), progress = '0.333'
         WHERE job_id = '%s'",
        JOBSTATUS_INPROGRESS, $original_job_id);

  $db_result = db_query(
    "SELECT asset_id, mediafile_id, owner, app_id, testtag
      FROM {job} WHERE job_id = %s", $original_job_id);

  if ($db_result_row = db_fetch_object($db_result)) {
    $asset_id = $db_result_row->asset_id;
    $mediafile_id = $db_result_row->mediafile_id;
    $owner = $db_result_row->owner;
    $app_id = $db_result_row->app_id;
    $testtag = $db_result_row->testtag;
  }
  else {
    db_set_active();
    return;
  }

  $db_result = db_query(
    "INSERT INTO {job}
    (asset_id, mediafile_id, owner, app_id,
     priority, job_type, created, testtag)
    VALUES
    ('%s', '%s', '%s', %d, -1, '%s', NOW(), '%s')",
      $asset_id, $mediafile_id, $owner, $app_id, JOBTYPE_TRANSCODE, $testtag);
  $job_id = db_last_insert_id("job", "job_id");

  # voeg de transcoding job toe met als 1 van de parameters het orginele
  # job_id (nodig voor het straks opzoeken van de gegevens voor de windows transcode)
  $db_result = db_query(
    "INSERT INTO {transcode_job}
    (job_id, transcode_profile_id, tool, command, file_extension)
    VALUES
    ('%d', NULL, '%s', '%s', '%s')",
    $job_id, JOBRAW_TOOL, sprintf(JOBRAW_COMMAND, $original_job_id), JOBRAW_FILE_EXTENSION);
  db_set_active();
}

/**********************************************************************
 * Testen van de opgegeven parameters voor een trancode job
 *********************************************************************/

/**
 * Controleer het opgegeven commando tegen de transcode mapping tabel.
 */
function _vpx_jobs_check_command_parameters($tool, $command) {
  // maak een named array van de string
  $parameters = create_named_array($command, ";", ":");

  // op basis van de toolname worden alle parameters die gebruikt kunnen
  // worden opgevraagd. De opgegeven parameters worden hier mee gecontroleerd.
  foreach ($parameters as $key => $value) {
    db_set_active();
    $query_mapping = db_query(
        "SELECT nice_parameter, tool_parameter, min_value,
                max_value, allowed_value
         FROM {transcode_mapping}
          WHERE tool = '%s' AND nice_parameter = '%s'", $tool, $key);

    if ($query_mapping_row = db_fetch_object($query_mapping)) {
      // valideer de ontvangen waarde tegen de toegestane waarden
      if ($query_mapping_row->min_value != null) {
        if (!($error = _vpx_validate_helper(array("type" => "float", "value" => $value)))) {
          return new rest_response(vpx_return_error(ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT, array("@key" => $key, "@value" => $value)));
        }
        if ($query_mapping_row->min_value > $value) {
          return new rest_response(vpx_return_error(ERRORCODE_JOB_TRANSCODE_PARAMETER_TOO_LOW, array("@key" => $key, "@min_value" => $query_mapping_row->min_value, "@value" => $value)));
        }
      }

      if ($query_mapping_row->max_value != null) {
        if (!($error = _vpx_validate_helper(array("type" => "float", "value" => $value)))) {
          return new rest_response(vpx_return_error(ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT, array("@key" => $key, "@value" => $value)));
        }
        if ($query_mapping_row->max_value < $value) {
          return new rest_response(vpx_return_error(ERRORCODE_JOB_TRANSCODE_PARAMETER_TOO_HIGH, array("@key" => $key, "@max_value" => $query_mapping_row->max_value, "@value" => $value)));
        }
      }

      if ($query_mapping_row->allowed_value != null) {
        $allowed_values = split(";", $query_mapping_row->allowed_value);
        if (!(in_array($value, $allowed_values))) {
          return new rest_response(vpx_return_error(ERRORCODE_JOB_TRANSCODE_PARAMETER_WRONG_VALUE, array("@key" => $key, "@value" => $value)));
        }
      }
    }
    else {
      return new rest_response(vpx_return_error(ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FOUND, array("@key" => $key)));
    }
  }
  return FALSE;
} // end of _check_command_parameters


/**********************************************************************
 * Verwijderen van een bestaande job
 *********************************************************************/

/**
 * Verwijderen van een job
 * Een job kan alleen verwijderd worden wanneer de status WAITING,
 * FINISHED of FAILED is, en deze niet is toegewezen aan een server.
 *
 * De job wordt ook uit alle hulptabellen verwijderd.
 */
function _vpx_jobs_cancel_job($a_args) {
  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');

    vpx_funcparam_add_uri($a_funcparam, $a_args, 'job_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);

    $job_id = vpx_funcparam_get_value($a_funcparam, 'job_id');
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');

    // controleer of de job bestaat
    vpx_shared_must_exist("job", array("job_id" => $job_id));

    db_set_active("data");
    $db_result = db_query(
      "SELECT * FROM {job} WHERE
        job_id = %d AND
        status IN ('%s', '%s', '%s') AND
        job_id NOT IN (SELECT job_id FROM server_job)",
       $job_id,
       JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);
    db_set_active();

    $a_row_job = db_fetch_array($db_result);
    if ($a_row_job === FALSE) {
      throw new vpx_exception_error_job_could_not_be_removed(array("@job_id" => $a_parameters['job_id']['value']));
    }

    // controleer de gebruiker rechten
    vpx_acl_owner_check($app_id, $user_id, $a_row_job["app_id"], $a_row_job["owner"]);

    $job_status = $a_row_job["status"];
    $job_type = $a_row_job["job_type"];

    // Als we een analyze proberen te stoppen, mag dat alleen als er al een
    // metadata bestaat. We zouden anders een mediafile krijgen zonder metadata
    if ($job_type == JOBTYPE_ANALYSE && $job_status == JOBSTATUS_WAITING) {
      // Check voor een metadata entry
      db_set_active('data');
      $exists = db_result(db_query("SELECT COUNT(*) FROM {mediafile_metadata} WHERE mediafile_id='%s'", $a_row_job["mediafile_id"]));
      db_set_active();

      if ($exists == 0) {
        throw new vpx_exception_error_job_could_not_be_removed(array("@job_id" => $job_id));
      }
    }

    if (!_vpx_jobs_delete_job($job_id)) {
      throw new vpx_exception_error_job_could_not_be_removed(array("@job_id" => $a_parameters['job_id']['value']));
    }

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
} // end of _vpx_jobs_cancel_job


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
 * Notify the user about the status changes on transcoding job
 */
function vpx_jobs_notify_transcoding($old_status, $new_status, $job_id) {
    if ($old_status != $new_status) {
      db_set_active("data");
      $completed_transcoding_url = db_result(db_query_range("SELECT completed_transcoding_url FROM {transcode_job} WHERE job_id = %d", $job_id, 0, 1));
      db_set_active();

      if (!empty($completed_transcoding_url)) {
        watchdog('completed_transcoding_url', $completed_transcoding_url . $new_status);
        exec('wget -O - -q -t 1 ' . escapeshellcmd($completed_transcoding_url . $new_status .' >/dev/null 2>/dev/null &'));
      }
    }
}

/**
 * Zet de progress van een job op basis van een job_id
 */
function vpx_jobs_set_upload_progress($a_args) {
  // Haal de parameters op ..
  $a_parameters = array(
    'job_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'job_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'uploaded_file_size' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'uploaded_file_size'),
      'type' => 'int',
      'required' => TRUE,
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
  );

  /*
   * FIX ME
   * Authorisatie
   */

  // ... en valideer deze op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    $rest_response = new rest_response($result);
    return $rest_response;
  }

  // controleer of de job bestaat
  if (!vpx_count_rows("job", array("job_id", $a_parameters['job_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_JOB_NOT_FOUND, array("@job_id" => $a_parameters['job_id']['value'])));
  }

  $uploaded_file_size = $a_parameters['uploaded_file_size']['value'];

  if (!$uploaded_file_size) {
    db_set_active("data");
    $file_size = db_result(db_query("SELECT file_size FROM {upload_job} WHERE job_id=%d", $a_parameters['job_id']['value']));
    db_set_active();

    // If the filesize is 0, then we are done...
    if (!$file_size) {
      $status_parameters = array(
        'uri' => array('job_id' => $a_parameters['job_id']['value']),
        'post' => array('status' => JOBSTATUS_INPROGRESS,
                        'progress' => "1.000")
      );
      vpx_jobs_set_job_status($status_parameters); // put progress on 1.000

      return new rest_response(vpx_return_error(ERRORCODE_OKAY));
    }
  }

  db_set_active("data");
  $db_result = db_query("
      UPDATE {upload_job}
      SET uploaded_file_size = %d
      WHERE job_id = %d",
        $uploaded_file_size,
        $a_parameters['job_id']['value']);
  db_set_active();

  db_set_active("data");
  $progress = db_result(db_query("SELECT uploaded_file_size / file_size FROM {upload_job} WHERE job_id = %d", $a_parameters['job_id']['value']));
  db_set_active();

  $status_parameters = array(
    'uri' => array('job_id' => $a_parameters['job_id']['value']),
    'post' => array(
      'status' => JOBSTATUS_INPROGRESS,
      'progress' => $progress,
      'create_still' => $a_parameters['create_still']['value'],
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
    )
  );
  vpx_jobs_set_job_status($status_parameters);

  // Alles is goed gegaan, geef okay terug
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
} // end of vpx_jobscheduler_set_job_progress


function _vpx_jobs_add_retranscode_jobs($asset_id, $mediafile_id, $owner, $app_id) {

  db_set_active("data");
  $db_result = db_query(
    "SELECT tool, command, file_extension
     FROM {mediafile}
     WHERE asset_id = '%s' AND
           is_original_file = 'FALSE' AND
           NOT tool IS NULL AND
           NOT command IS NULL AND
           NOT file_extension IS NULL",
      $asset_id
  );
  db_set_active();

  // vul de response met de jobs.
  $counter = 0;
  while ($db_result_row = db_fetch_object($db_result)) {
    $counter++;
    $tool = $db_result_row->tool;
    $command = $db_result_row->command;
    $file_extension = $db_result_row->file_extension;

    $job_parameters = array(
      "mediafile_id" => array("value" => $mediafile_id),
      "user_id" => array("value" => $owner),
      "app_id" => array("value" => $app_id),
      "job_type" => array("value" => JOBTYPE_TRANSCODE),
      "command" => array("value" => $command),
      "tool" => array("value" => $tool),
      "file_extension" => array("value" => $file_extension),
      "testtag" => array("value" => 'FALSE')
    );
    _vpx_jobs_create_new_job(FALSE, $job_parameters);
  }
  return $counter;
}

/**
 * Delete/Cancels all running jobs of asset
 *
 * @param string $asset_id
 */
function _vpx_jobs_cancel_job_by_asset($asset_id) {
  db_set_active("data");

  // controleer de transcoding tabel
  db_query("DELETE tj FROM {transcode_job} AS tj JOIN {job} AS j USING(job_id) WHERE j.asset_id='%s' AND j.status IN ('%s', '%s', '%s')",
    $asset_id, JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);

  // controleer de still tabel
  db_query("DELETE sj FROM {still_job} AS sj JOIN {job} AS j USING(job_id) WHERE j.asset_id='%s' AND j.status IN ('%s', '%s', '%s')",
    $asset_id, JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);

  // controleer de upload tabel
  db_query("DELETE uj FROM {upload_job} AS uj JOIN {job} AS j USING(job_id) WHERE j.asset_id='%s' AND j.status IN ('%s', '%s', '%s')",
    $asset_id, JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);

  // Verwijder uit job...
  db_query("DELETE FROM {job} WHERE asset_id='%s' AND status IN ('%s', '%s', '%s')",
    $asset_id, JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);

  $exists = db_result(db_query("SELECT COUNT(*) AS total FROM {job} WHERE asset_id='%s'", $asset_id));

  db_set_active();

  return ($exists ? FALSE : TRUE); // returns TRUE when job has been deleted...
}

/**
 * Deletes the job. Moved into function for re-use.
 *
 * @param integer $job_id
 * @return bool TRUE; job has been deleted
 */
function _vpx_jobs_delete_job($job_id) {
  db_set_active("data");

  // controleer de transcoding tabel
  db_query("DELETE tj FROM {transcode_job} AS tj JOIN {job} AS j USING(job_id) WHERE j.job_id=%d AND j.status IN ('%s', '%s', '%s')",
    $job_id, JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);

  // controleer de still tabel
  db_query("DELETE sj FROM {still_job} AS sj JOIN {job} AS j USING(job_id) WHERE j.job_id=%d AND j.status IN ('%s', '%s', '%s')",
    $job_id, JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);

  // controleer de upload tabel
  db_query("DELETE uj FROM {upload_job} AS uj JOIN {job} AS j USING(job_id) WHERE j.job_id=%d AND j.status IN ('%s', '%s', '%s')",
    $job_id, JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);

  // Verwijder uit job...
  db_query("DELETE FROM {job} WHERE job_id=%d AND status IN ('%s', '%s', '%s')",
    $job_id, JOBSTATUS_WAITING, JOBSTATUS_FINISHED, JOBSTATUS_FAILED);

  $exists = db_result(db_query("SELECT COUNT(*) AS total FROM {job} WHERE job_id=%d", $job_id));

  db_set_active();

  return ($exists ? FALSE : TRUE); // returns TRUE when job has been deleted...
}


function vpx_jobs_analyse_mediafile($a_args) {
  $a_parameters = array(
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'mediafile_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
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
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_app_admin', 'false'),
      'type' => 'bool',
    ),
  );

  // Controleer de parameters
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    $rest_response = new rest_response($result);
    return $rest_response;
  }
  $is_app_admin = vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']);

  $a_analyse_parameters = array(
    'mediafile_id' => array('value' => $a_parameters['mediafile_id']['value']),
    'user_id' => array('value' => $a_parameters['user_id']['value']),
    'app_id' => array('value' => $a_parameters['app_id']['value']),
    'job_type' => array('value' => JOBTYPE_ANALYSE),
    'testtag' => array('value' => 'false'),
    'create_still' => array('value' => 'false'),
    'is_app_admin' => array('value' => $is_app_admin),
  );
  $output = _vpx_jobs_create_new_job(FALSE, $a_analyse_parameters);

  return $output;
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

/**
 * Based on the jobserver id and mediafile_src a number of stills is generated,
 * the still with the most different colors is returned
 *
 * @param integer $jobserver_id
 * @param string $mediafile_src
 * @return mixed FALSE when there are no stills or the name of the still
 */
function _vpx_jobserver_still_validate($jobserver_id, $mediafile_src) {
  // Retrieve the job settings
  // CRap they are already removed here
  db_set_active("jobserver");
  $query_result = db_query("
    SELECT frametime, size, h_padding, v_padding, blackstill_check
    FROM {jobserver_still_job} jsj
    LEFT JOIN {jobserver_job} jj ON jj.jobserver_job_id = jsj.jobserver_job_id
    WHERE jj.job_id = '%s'", $jobserver_id);
  db_set_active();

  $execution_string = "";
  $query_result_row = db_fetch_object($query_result);
  if (!empty($query_result_row)) {
    $frametime = $query_result_row->frametime;
    $size = $query_result_row->size;
    $h_padding = $query_result_row->h_padding;
    $v_padding = $query_result_row->v_padding;
    $blackstill_check = $query_result_row->blackstill_check;
  }

  if ($blackstill_check == 'FALSE') {
    return $mediafile_src;
  }

  // Retrieve duration of the movie
  db_set_active('data');

  $result = db_query("
    SELECT file_duration
    FROM {mediafile_metadata} m
    LEFT JOIN {job} j ON m.mediafile_id = j.mediafile_id
    WHERE job_id = %d
    LIMIT 1", $jobserver_id);
  $file_duration = db_result($result);
  db_set_active();

  $duration_array = explode(':', $file_duration);
  $seconds = $duration_array[0] * 3600;
  $seconds += $duration_array[1] * 60;
  $seconds += $duration_array[2];

  $base_filename = get_base_filename($mediafile_src);
  $base_path = vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename;

  // Create our extra stills
  $stills = array();

  // Move the original still to safety
  $path = $base_path . sprintf(VPX_STILL_EXTENSION, 1) .'.jpeg';
  $new_path = $base_path .'_0.jpeg';
  rename($path, $new_path);

  // Add our still to an array
  $stills[0] = array(
    'base_filename' => $base_filename . '_0',
    'path' => $new_path,
  );

  $offset = 0;
  for ($i = 1; $i <= VPX_STILLS_AMOUNT; $i++) {
    // Calc the time offsets
    if ($i%2) {
      $offset = floor($i * VPX_STILL_INTERVAL * VPX_STILL_INTERVAL_JITTER);
      $time = $frametime + $offset;
    }
    else {
      $time = $frametime - $offset;
    }
    //$time = $i * $seconds / (VPX_STILLS_AMOUNT+1);

    // Stay within the movie duration
    $time = ($time <= 0)? 1 : $time;
    $time = ($time >= $seconds)? $seconds-1 : $time;

    $duration = 1;
    $framerate = 1;
    $execution_string = sprintf("%s %s %s jpeg ".VPX_STILL_STRING . " > /dev/null",
    VPX_STILL_FILE, SAN_NAS_BASE_PATH . DS . DATA_LOCATION, $base_filename, $size, $h_padding, $h_padding, $v_padding, $v_padding, $time, $duration, $framerate);

    // Execute the LUA call
    exec($execution_string);

    // Move it
    $path = $base_path . sprintf(VPX_STILL_EXTENSION, 1) .'.jpeg';
    $new_path = $base_path .'_'. $i .'.jpeg';
    rename($path, $new_path);

    // Add our still to an array
    $stills[$i] = array(
      'base_filename' => $base_filename . '_'. $i,
      'path' => $new_path,
    );
  }

  // Check the colors all the stills
  foreach ($stills as $key => $value) {
    if (file_exists($stills[$key]['path']) && filesize($stills[$key]['path']) > 0) {
      $stills[$key]['colors'] = _vpx_jobserver_still_colors($stills[$key]['path']);
    }
    else {
      // The still does not exist
      @unlink($stills[$key]['path']);
    }
  }

  // Determine the still with the most colors
  if (count($stills) > 0) {
    $maxvalue = 0;
    foreach ($stills as $key => $value) {
      if (count($value['colors']) > $maxvalue) {
        $maxvalue = count($value['colors']);
        $maxkey = $key;
      }
    }

    // $stills[$maxkey] should contain the most colors and should be ok.
    // Adjust the filename

    $file = explode('_', $stills[$maxkey]['base_filename']); // That is why there was an _ in the filename
    $still = $file[0];
    $path = $stills[$maxkey]['path'];
    $new_path = vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $file[0] . sprintf(VPX_STILL_EXTENSION, 1) .'.jpeg';
    rename($path, $new_path);

    // Remove everything in $stills since we no longer need it
    foreach ($stills as $key => $value) {
      @unlink($stills[$key]['path']);
    }

    // Return the best still
    return $still;
  }
  else {
    return FALSE;
  }
}

/**
 * Return all the colors sorted by dominance.
 * @param string $image Path to the image
 * @return array All the colors as hexidecimal values ordered by occurance.
 */
function _vpx_jobserver_still_colors($image) {
  // Resize the image, we need only the most significant colors
  $size = getimagesize($image);
  $scale = 1;
  if ($size[0]>0) {
    $scale = min(VPX_STILL_RESAMPLE_WIDTH / $size[0], VPX_STILL_RESAMPLE_HEIGHT / $size[1]);
  }

  if ($scale < 1) {
    $width = floor($scale * $size[0]);
    $height = floor($scale * $size[1]);
  }
  else {
    $width = $size[0];
    $height = $size[1];
  }

  $image_resized = imagecreatetruecolor($width, $height);
  switch ($size[2]) {
    case 1:
      $image_orig = imagecreatefromgif($image);
      break;
    case 2:
      $image_orig = imagecreatefromjpeg($image);
      break;
    case 3:
      $image_orig = imagecreatefrompng($image);
      break;
    default:
      $image_orig = imagecreatefromjpeg($image);
      break;
  }

  // Nearest neightbour as it does not alter the colors
  imagecopyresampled($image_resized, $image_orig, 0, 0, 0, 0, $width, $height, $size[0], $size[1]);

  $im = $image_resized;
  $img_width = imagesx($im);
  $img_height = imagesy($im);

  for ($y=0; $y < $img_height; $y++) {
    for ($x=0; $x < $img_width; $x++) {
      $index = imagecolorat($im,$x,$y);
      $rgb = imagecolorsforindex($im,$index);
      // Round the colors to reduce the amount of them
      $rgb['red']   = intval((($rgb['red']) + 15) / 32) * 32;
      $rgb['green'] = intval((($rgb['green']) + 15)/32) * 32;
      $rgb['blue']  = intval((($rgb['blue']) + 15) / 32) * 32;
      if ($rgb['red'] >= 256) {
        $rgb['red'] = 240;
      }

      if ($rgb['green'] >= 256) {
        $rgb['green'] = 240;
      }

      if ($rgb['blue'] >= 256) {
        $rgb['blue'] = 240;
      }

      $hex_array[] = rgb2hex($rgb);
    }
  }
  $hex_array = array_count_values($hex_array);
  natsort($hex_array);
  $hex_array = array_reverse($hex_array, TRUE);

  return $hex_array;
}

/**
 * Calculate the average color by resampling the image to 1px.
 * @param string $image Image path
 * @return array RGB color
 */
function _vpx_jobserver_still_averagecolor($image) {
  $image_tmp = imagecreatetruecolor(1, 1);
  $size = getimagesize($image);

  switch ($size[2]) {
    case IMAGETYPE_GIF:
      $image_orig = imagecreatefromgif($image);
      break;
    case IMAGETYPE_JPEG:
      $image_orig = imagecreatefromjpeg($image);
      break;
    case IMAGETYPE_PNG:
      $image_orig = imagecreatefrompng($image);
      break;
    default:
      $image_orig = imagecreatefromjpeg($image);
      break;
  }

  imagecopyresampled($image_tmp, $image_orig, 0, 0, 0, 0, 1, 1, $size[0], $size[1]);

  $color = imagecolorat($image_tmp, 0, 0);
  $rgb = imagecolorsforindex($image_tmp, $color);

  return $rgb;
}

/**
 * Difference in lumosity
 * @param integer $r1
 * @param integer $g1
 * @param integer $b1
 * @param integer $r2
 * @param integer $g2
 * @param integer $b2
 * @return integer Difference in color
 */
function lumdiff($r1, $g1, $b1, $r2, $g2, $b2){
    $l1 = 0.2126 * pow($r1/255, 2.2) +
          0.7152 * pow($g1/255, 2.2) +
          0.0722 * pow($b1/255, 2.2);

    $l2 = 0.2126 * pow($r2/255, 2.2) +
          0.7152 * pow($g2/255, 2.2) +
          0.0722 * pow($b2/255, 2.2);

    if ($l1 > $l2) {
        return ($l1+0.05) / ($l2+0.05);
    }
    else {
        return ($l2+0.05) / ($l1+0.05);
    }
}

function coldiff($r1, $g1, $b1, $r2, $g2, $b2){
    return max($r1, $r2) - min($r1, $r2) +
           max($g1, $g2) - min($g1, $g2) +
           max($b1, $b2) - min($b1, $b2);
}

/**
 * Difference in color distance
 * @param integer $r1
 * @param integer $g1
 * @param integer $b1
 * @param integer $r2
 * @param integer $g2
 * @param integer $b2
 * @return integer Difference in color
 */
function pythdiff($r1, $g1, $b1, $r2, $g2, $b2) {
    $rd = $r1 - $r2;
    $gd = $g1 - $g2;
    $bd = $b1 - $b2;

    return  sqrt($rd * $rd + $gd * $gd + $bd * $bd);
}

/**
 * Difference in brightness
 * @param integer $R1
 * @param integer $G1
 * @param integer $B1
 * @param integer $R2
 * @param integer $G2
 * @param integer $B2
 * @return integer Difference in color
 */
function brghtdiff($r1, $g1, $b1, $r2, $g2, $b2){
    $br1 = (299 * $r1 + 587 * $g1 + 114 * $b1) / 1000;
    $br2 = (299 * $r2 + 587 * $g2 + 114 * $b2) / 1000;

    return abs($br - $br2);
}

/**
 * Convert Hex to RGB
 * @param string $hexcolor Hexidecimal color
 * @return array RGB color
 */
function hex2rgb($hexcolor = "") {
  $rgb = array();
  $rgb['red'] = hexdec(substr($hexcolor, 0, 2));
  $rgb['green'] = hexdec(substr($hexcolor, 2, 2));
  $rgb['blue'] = hexdec(substr($hexcolor, 4, 2));

  return $rgb;
}

/**
 * Convert RGB to Hex
 * @param array $rgb Color values
 * @return string RGB value converted to hexidecimal
 */
function rgb2hex($rgb = array()) {
  $hex = "";
  $hex =  substr("0". dechex($rgb['red']), -2) .
          substr("0". dechex($rgb['green']), -2) .
          substr("0". dechex($rgb['blue']), -2);
  return $hex;
}
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
 * creer de string die naar de vpx-transcode script wordt gestuurd.
 */
function _vpx_jobserver_get_transcode_string($jobserver_id, $mediafile_src) {
  // haal de transcode gegevens van de job op.
  db_set_active("jobserver");
  $query_result = db_query("
    SELECT tool, file_extension, command
    FROM {jobserver_transcode_job}
    WHERE jobserver_job_id = '%s'", $jobserver_id);
  db_set_active();

  $execution_string = "";

  if (($query_result_row = db_fetch_object($query_result)) !== FALSE) {
    $tool = $query_result_row->tool;
    $file_extension = $query_result_row->file_extension;
    $command = $query_result_row->command;
  }
  else {
    watchdog("server", sprintf("Transcode job not found, jobserver_id: %s", $jobserver_id), null);
    return $execution_string;
  }

  // bewerk de parameters zodat er een mooie string ontstaat.
  $parameters = create_named_array($command, ";", ":");
  $parameter_string = "'";
  foreach ($parameters as $name => $value) {
    $parameter_string .= sprintf("%s %s ", $name, $value);
  }

  $parameter_string .= "'";
  $parameter_string = trim($parameter_string);
  if ($parameter_string == "''") {
    $parameter_string = "";
  }

  // combineer alles op basis van de gekozen tool.
  switch ($tool) {
    case "ffmpeg":
      $execution_string = sprintf("%s %s %s %s %s > /dev/null &", VPX_FFMPEG_TRANSCODE_FILE, SAN_NAS_BASE_PATH . DS . DATA_LOCATION, $mediafile_src, $file_extension, $parameter_string);
      break;
    case "windows":
      $execution_string = sprintf("%s %s %s %s", VPX_WINDOWS_TRANSCODE_FILE, $mediafile_src, $file_extension, $parameter_string);
      break;
    default:
      return "";
  }

  return $execution_string;
}

/**
 * creer de string die naar het vpx-analyse script wordt gestuurd.
 */
function _vpx_jobserver_get_analyse_string($mediafile_src, $job_id) {

  db_set_active('data');
  $app_id = db_result(db_query("SELECT app_id FROM {job} WHERE job_id=%d", $job_id));
  db_set_active();

  assert($app_id);
  $dbrow = db_fetch_array(db_query("SELECT always_hint_mp4, always_insert_md FROM {client_applications} WHERE id=%d", $app_id));
  assert($dbrow);

  $a_options = array();

  if (strcasecmp($dbrow['always_hint_mp4'], 'true') == 0) {
    $a_options[] =  VPX_ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
  }

  if (strcasecmp($dbrow['always_insert_md'], 'true') == 0) {
    $a_options[] =  VPX_ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
  }

  $execution_string = sprintf("%s %s %s", VPX_ANALYSE_FILE, SAN_NAS_BASE_PATH . DS . DATA_LOCATION, $mediafile_src);
  $execution_string .= (count($a_options) ? ' ' . implode(' ', $a_options) : '');

  return $execution_string;
}

/**
 * creer de string die naar het vpx-transcode script wordt gestuurd
 * voor het genereren van een still
 */
function _vpx_jobserver_get_still_string($jobserver_id, $mediafile_src) {
  // haal de still gegevens uit de database.
  db_set_active("jobserver");
  $query_result = db_query("
    SELECT frametime, size, h_padding, v_padding, still_parameters
    FROM {jobserver_still_job}
    WHERE jobserver_job_id = '%s'", $jobserver_id
  );
  $job_id = db_result(db_query_range("SELECT job_id FROM {jobserver_job} WHERE jobserver_job_id = %d", $jobserver_id, 0, 1));
  db_set_active();

  $execution_string = "";

  if ($query_result_row = db_fetch_object($query_result)) {
    $frametime = $query_result_row->frametime;
    $size = $query_result_row->size;
    $h_padding = $query_result_row->h_padding;
    $v_padding = $query_result_row->v_padding;

    $still_parameters = unserialize($query_result_row->still_parameters);
    $video_duration = $still_parameters['video_duration'];
    $fps = $still_parameters['fps'];
    $start_frame = $still_parameters['start_frame'];
    $end_frame = $still_parameters['end_frame'];
    if (!$start_frame || !is_numeric($start_frame) || $start_frame < 0) {
      $start_frame = 0;
    }
    //$ef = $video_duration * $fps;
    if (!$end_frame || !is_numeric($end_frame) || $end_frame > $video_duration-1) {
      $end_frame = $video_duration-1;
    }
    if ($start_frame > $video_duration-1) {
      $start_frame = $video_duration-1;
    }
    if ($end_frame < 0) {
      $end_frame = 0;
    }
    if (!$video_duration || !is_numeric($video_duration)) {
      // An error, falls back to the original behaviour
      $still_parameters['still_type'] = 'NONE';
    }
    if ($start_frame > $end_frame) {
      $tmp = $start_frame;
      $start_frame = $end_frame;
      $end_frame = $tmp;
    }

    // Data check
    switch ($still_parameters['still_type']) {
      case 'NORMAL':
        if (!is_numeric($still_parameters['still_per_mediafile']) || $still_parameters['still_per_mediafile'] < 1) {
          $still_parameters['still_type'] = 'NONE';
        }
        break;
      case 'SECOND':
        if (!is_numeric($still_parameters['still_every_second']) || $still_parameters['still_every_second'] < 0) {
          $still_parameters['still_type'] = 'NONE';
        }
        break;
      case 'SCENE':
        if (!$fps || !is_numeric($fps)) {
          // An error, falls back to the original behaviour
          $still_parameters['still_type'] = 'NONE';
        }
        break;
      default:
        break;
    }

    $frametime = $start_frame;
    $duration = 1;
    $framerate = 1;
    switch ($still_parameters['still_type']) {
      case 'NORMAL':
        $still_per_mediafile = $still_parameters['still_per_mediafile'];
        $frametime = $start_frame;
        $duration = ($end_frame - $start_frame);
        // |--------- video length -----------|
        // Stills per mediafile = 4
        // |------S------S------S------S------|
        // $framesecond = duration / (4 + 1)
        // .......|-- video length -----------|
        // frametime += $framesecond
        // duration -= $framesecond
        $framesecond = $duration / ($still_per_mediafile + 1);
        $frametime += $framesecond;
        $duration -= $framesecond;
        // Frames per second
        $framerate = 1/$framesecond;
        // Safety check
        if ($duration / $framesecond > VPX_STILL_MAXIMUM) {
          $duration = VPX_STILL_MAXIMUM * $framesecond;
        }
        break;
      case 'SECOND':
        $still_every_second = $still_parameters['still_every_second'];
        $frametime = $start_frame;
        $duration = ($end_frame - $start_frame);
        //
        //$frametime += $still_every_second;
        //$duration -= $still_every_second;
        $framerate = 1/$still_every_second;
        // Safety check
        if ($duration / $still_every_second > VPX_STILL_MAXIMUM) {
          $duration = VPX_STILL_MAXIMUM * $still_every_second;
        }
        break;
      case 'SCENE':
        watchdog('server', 'Scene detection starts');
        $destination_path = SAN_NAS_BASE_PATH . DS . DATA_LOCATION . DS . 'transcode' . DS;
        //watchdog('server', sprintf('Scene: destination path = %s', $destination_path));
        $input = SAN_NAS_BASE_PATH . DS . DATA_LOCATION . DS . $mediafile_src{0} . DS . $mediafile_src;

        // Clean up I.
        $order = sprintf('rm %s', $destination_path . $mediafile_src .'_scene.avi');
        //watchdog('server', $order);
        exec($order);
        $order = sprintf('rm %s', $destination_path . $mediafile_src .'_list.el');
        //watchdog('server', $order);
        exec($order);

/*
// Old code
        $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $input, $destination_path . $mediafile_src .'_scene.avi');
        //watchdog('server', $order);
        exec($order);
*/

// Hot fix
        $order = sprintf('ffmpeg -i %s %s', $input, $destination_path . $mediafile_src .'_scene.wmv');
        //watchdog('server', $order);
        exec($order);

        $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $destination_path . $mediafile_src .'_scene.wmv', $destination_path . $mediafile_src .'_scene.avi');
        //watchdog('server', $order);
        exec($order);

        $order = sprintf('rm %s', $destination_path . $mediafile_src .'_scene.wmv');
        //watchdog('server', $order);
        exec($order);
// Hot fix - end

        $order = sprintf('lav2yuv -S %s %s', $destination_path . $mediafile_src .'_list.el', $destination_path . $mediafile_src .'_scene.avi');
        watchdog('server', $order);
        exec($order);

        $output = NULL;
        $order = sprintf('cat %s', $destination_path . $mediafile_src .'_list.el');
        //watchdog('server', $order);
        exec($order, $output);
        //watchdog('server', sprintf('output = %s', print_r($output, TRUE)));

        // Clean up II.
        $order = sprintf('rm %s', $destination_path . $mediafile_src .'_scene.avi');
        //watchdog('server', $order);
        exec($order);
        $order = sprintf('rm %s', $destination_path . $mediafile_src .'_list.el');
        //watchdog('server', $order);
        exec($order);

        // Analyze
        if ($output && is_array($output) && $output != array()) {
          //watchdog('server', sprintf('Output is array with data'));

          // tbr & map from the best stream
          $map = NULL;
          $tbr = NULL;
          $kbs = NULL;
          $order = sprintf('ffmpeg -i %s 2>&1', $input);
          //watchdog('server', $order);
          exec($order, $details);
          //watchdog('server', 'Details: '. print_r($details, TRUE));
          foreach($details as $line) {
            if (stripos($line, ' Stream ') !== FALSE && stripos($line, ' Video: ') !== FALSE) {
              if (ereg('Stream #[0-9]*\.([0-9]*)', $line, $reg_map) && ereg(' ([0-9]*) kb/s', $line, $reg_kbs)) {
                if (!$map || !$kbs || $kbs < $reg_kbs[1]) {
                  $map = $reg_map[1];
                  $kbs = $reg_kbs[1];
                }
                if (ereg(' ([0-9]*) tb', $line, $regs)) {
                  $tbr = $regs[1];
                }
              }
              elseif (!$map && ereg(' ([0-9]*) tb', $line, $regs)) {
                $tbr = $regs[1];
              }
            }
          }
          //watchdog('server', 'tbr: '. print_r($tbr, TRUE) .', map: '. print_r($map, TRUE) .', kbs: '. print_r($kbs, TRUE));
          if (!$tbr) {
            // This is the base value
            $tbr = 25;
          }
          // Go further, if we have tbr
          if ($tbr) {

            $scene_frame = NULL;
            $i = 1;
            $scenes = array();
            foreach ($output as $line) {
              //watchdog('server', sprintf('Line: %s', print_r($line, TRUE)));
              $line_args = explode(' ', $line);
              //watchdog('server', sprintf('Line args: %s', print_r($line_args, TRUE)));
              if ($line_args && is_array($line_args) && $line_args != array()) {
                //watchdog('server', sprintf('Line args is array with data'));
                if (isset($line_args[0]) && isset($line_args[1]) && isset($line_args[2]) && !isset($line_args[3]) && is_numeric($line_args[0]) && is_numeric($line_args[1]) && is_numeric($line_args[2])) {
                  //watchdog('server', sprintf('Line args has 3 numbers'));
                  // $line_args[1] + num = a little bit after the change
                  $change_frame = $line_args[1] + VPX_STILL_SCENE_AFTER;
                  //watchdog('server', 'change_frame: '. print_r($change_frame, TRUE) .', start_frame: '. print_r($start_frame, TRUE) .', end_frame: '. print_r($end_frame, TRUE));
                  if ($change_frame >= ($start_frame*$fps) && $change_frame <= ($end_frame*$fps)) {
                    // $scene_frame + num = minimal distance with two changes
                    if (!$scene_frame || $scene_frame + VPX_STILL_SCENE_DISTANCE < $change_frame) {
                      $scene_frame = $change_frame;
                      //watchdog('server', sprintf('New scene frame: %s', $scene_frame));
                      $scene_sec = (int)($scene_frame/$tbr);

                      // VPX_STILL_SCENE_STRING = ffmpeg -i %s -s %s -padtop %d -padbottom %d -padleft %d -padright %d -an -deinterlace -y -ss %d -t 1 -r 1 %s -f image2 %s
                      $order = sprintf(VPX_STILL_SCENE_STRING, $input, $size, $h_padding, $h_padding, $v_padding, $v_padding, $scene_sec, ($map ? '-map 0.'. $map : ''), $destination_path . $mediafile_src . sprintf(VPX_STILL_EXTENSION, $i) .".jpeg");
                      //watchdog('server', $order);
                      exec($order, $details);
                      //watchdog('server', 'Details: '. print_r($details, TRUE));

                      $scenes[] = $scene_sec;

                      $i++;
                      if ($i > VPX_STILL_MAXIMUM) {
                        // Emergency break
                        break;
                      }
                    }
                  }
                }
              }
            }

          }

        }

        // Save the sec data (see: vpx_jobhandler.module)
        if (is_array($scenes) && $scenes != array()) {
          $my_file = $destination_path . $job_id .'_scene.txt';
          $fh = fopen($my_file, 'w') or $fh = NULL;
          if ($fh) {
            foreach ($scenes as $scene) {
              fwrite($fh, $scene ."\n");
            }
            fclose($fh);
          }

          // Do nothing after this
          return 'echo "Scene stills are rock"';
        }

        // We reached this point, so something went wrong in the creation of scene stills
        // So we are creating still type NONE (original behaviour)
        break;
      default:
        break;
    }

  }
  else {
    watchdog("server", sprintf("Still job not found, jobserver_id: %s", $jobserver_id), null);
    return $execution_string;
  }

  $still_parameters['frametime'] = $frametime;
  $still_parameters['duration'] = $duration;
  $still_parameters['framerate'] = $framerate;

  db_set_active("data");
  db_query("UPDATE {job} SET still_parameters = '%s' WHERE job_id = %d", serialize($still_parameters), $job_id);
  db_set_active();

  // combineer de string (alleen voor linux, windows genereert geen stills)
  $execution_string = sprintf("%s %s %s jpeg ".VPX_STILL_STRING, VPX_STILL_FILE, SAN_NAS_BASE_PATH . DS . DATA_LOCATION, $mediafile_src, $size, $h_padding, $h_padding, $v_padding, $v_padding, $frametime, $duration, $framerate);
  $execution_string .= " > /dev/null &";

  return $execution_string;
}
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
 * Check the created still and save it if everything is ok.
 *
 * @param string $job_id Current job id.
 * @param string $mediafile_src Contains a file path to the mediafile
 * @return string Contains the error message
 */
function _vpx_jobserver_store_new_still($job_id, $mediafile_src) {
  require_once 'job_server_still_check.inc';
  $base_filename = get_base_filename($mediafile_src);

  // Is black still working?
  if (@filesize(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename .'_0.jpeg')) {
    return JOBSTATUS_INPROGRESS;
  }

  // Check if there really is an image ($file_size > 0)
  if (filesize(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename . sprintf(VPX_STILL_EXTENSION, 1) .".jpeg") == 0) {
    // Something failed, very likely the frametime was too high. Remove the files and fail the job.
    $still_error = vpx_return_error(ERRORCODE_JOB_FRAMETIME_GREATER_THEN_DURATION);
    db_set_active("jobserver");
    $query_result = db_query("
        UPDATE {jobserver_job}
        SET status = '%s', error_description = '%s'
        WHERE job_id = %d",
        JOBSTATUS_FAILED,
        $still_error['description'],
        $job_id);
    db_set_active();

    // Remove all of the still images
    $i = 1;
    while (file_exists(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename . sprintf(VPX_STILL_EXTENSION, $i) .".jpeg") && $i <= VPX_STILL_MAXIMUM) {
      unlink(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename . sprintf(VPX_STILL_EXTENSION, $i) .".jpeg");
      $i++;
    }
    unlink(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename .".status");

    # FIX ME. Remove error notice from table.
    watchdog("server", sprintf("Job failed, the new frametime is higher than the duration of the movie?"), null);
    _vpx_jobserver_set_job_status($job_id, JOBSTATUS_FAILED, "1.000", "Job failed, the new frametime is higher than the duration of the movie?");
    return JOBSTATUS_FAILED;
  }

  // Check if the frame has any usefull content. We do this by checking the amount of dominant colors.
  $mediafile_src = _vpx_jobserver_still_validate($job_id, $mediafile_src);

  if ($mediafile_src === FALSE) {
    watchdog("server", sprintf("Job failed, are there any colors in this movie?"), NULL);
    _vpx_jobserver_set_job_status($job_id, JOBSTATUS_FAILED, "1.000", "Job failed, are there any colors in this movie?");
    return JOBSTATUS_FAILED;
  }

  $i = 1;
  $mediafile_dest = array();
  while (file_exists(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename . sprintf(VPX_STILL_EXTENSION, $i) .".jpeg")) {
    if ($i <= VPX_STILL_MAXIMUM) {
      // Generate new hash
      $filename = vpx_create_hash($mediafile_src, $job_id);

      // Everything went ok, move the still and remove other files
      rename(
        vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename . sprintf(VPX_STILL_EXTENSION, $i) .".jpeg",
        vpx_get_san_nas_base_path() . DS . STILL_LOCATION . DS . $filename{0} . DS . $filename
      );
      $mediafile_dest[] = $filename;
    }
    else {
      // Reached the maximum, just delete the remain stills
      unlink(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename . sprintf(VPX_STILL_EXTENSION, $i) .".jpeg");
    }
    $i++;
  }
  unlink(vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $base_filename .".status");

  // Update mediafile_dest of the job
  db_set_active("jobserver");
  $query_result = db_query("
      UPDATE {jobserver_job}
      SET mediafile_dest = '%s'
      WHERE job_id = %d", serialize($mediafile_dest), $job_id);
  db_set_active();
  watchdog("server", "Job finished: Still saved as e.g.: ". vpx_get_san_nas_base_path() . DS . STILL_LOCATION . DS . $filename{0} . DS . $filename, NULL);
  return JOBSTATUS_FINISHED;
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
 * Maak een array van het status bestand
 */
function _vpx_jobserver_get_status($filename, $orig = FALSE) {
  $statusfile = vpx_get_san_nas_base_path() . DS . VPX_TRANSCODE_TMP_DIR . DS . $filename .".status";
  if (!file_exists($statusfile)) {
    return array();
  }

  $a_result = $a_lines = array();

  $handle = fopen($statusfile, "r");
  while (!feof($handle)) {
    $a_lines[] = fgets($handle);
  }
  fclose($handle);

  // strip the garbage from the file
  foreach ($a_lines as $line) {
    list($name, $value) = explode(":", $line, 2);
    if ($name == "Progress" || $name == "Status" || $name == "Errors") {
      $a_result[$name] = trim($value);
    }
    elseif ($name == 'ffmpeg-output') {
      $a_result[$name] = implode("\n", explode('}-{', trim($value)));
    }
  }

  return ($orig ? implode('', $a_lines) : $a_result);
}

/**
 * Update de job status op de server. Wanneer de status op FINISHED
 * wordt gezet zal de finished datum worden gezet op NOW().
 */
function _vpx_jobserver_set_job_status($job_id, $status, $progress, $error_description = "") {
  $updatefinished = "";
  if ($status == JOBSTATUS_FINISHED || $status == JOBSTATUS_FAILED || $status == JOBSTATUS_CANCELLED) {
    $updatefinished = ", finished = NOW()";
  }

  // test of de job nog niet gestart is en moet worden gestart.
  db_set_active("jobserver");
  $curstatus = db_result(db_query("
      SELECT status FROM {jobserver_job}
      WHERE job_id = %d", $job_id));
  db_set_active();

  // zet dan de starttijd op nu.
  $updatestarted = "";
  if ($curstatus == JOBSTATUS_WAITING && $status == JOBSTATUS_INPROGRESS) {
    $updatestarted = ", started = NOW()";
  }

  if ($error_description != "") {
    $error = sprintf("error_description = '%s', ", db_escape_string($error_description));
  }
  else {
    $error = "";
  }

  db_set_active("jobserver");
  $query_result = db_query("
    UPDATE {jobserver_job}
    SET status = '%s', ". $error ."progress= '%s'%s%s
    WHERE job_id = %d", $status, $progress,
                        $updatefinished, $updatestarted, $job_id);
  db_set_active();
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

/**
 * Create a link to the parent asset belonging to a given job id
 * @param int $job_id
 * @return string Link to an asset.
 */
function _vpx_jobserver_get_asset_link($job_id) {
  $job_details = _vpx_jobs_get_job_info($job_id);
  db_set_active('data');
  $asset_id = db_result(db_query("SELECT parent_id FROM {asset} WHERE asset_id = '%s'", $job_details['asset_id']));
  db_set_active();

  if (empty($asset_id)) {
    $asset_id = $job_details['asset_id'];
  }

  $link = l(t('Go to asset @asset_id', array('@asset_id' => $asset_id)), 'vpx/vpx_beheer_mm/asset/' . $asset_id);

  return $link;
}

