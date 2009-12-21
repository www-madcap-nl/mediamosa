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
 * Job Transcode functions.
 */
class mediamosa_job_transcode {

  /**
   * Get the job transcode data.
   * @param integer $job_id
   *  The job ID.
   */
  static public function get($job_id) {
    return mediamosa_db::db_select(mediamosa_job_transcode_db::TABLE_NAME, 'jt')
      ->fields('jt')
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  // 1.x: _vpx_jobs_create_new_job_transcode
  static public function create($app_id, $job_id, $mediafile_id, $file_extension, $command, $completed_transcoding_url, $tool = '', $profile_id = '') {

    // Get the default transcode profile settings when not supplied.
    if ($profile_id == '' && $tool == '') {
      $a_transcode_profile = mediamosa_transcode_profile::get_default($app_id);

      if (!$a_transcode_profile) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_DEFAULT_TRANSCODE_PROFILE);
      }

      $profile_id = $a_transcode_profile[mediamosa_transcode_profile_db::ID];
      $tool = $a_transcode_profile[mediamosa_transcode_profile_db::TOOL];
      $file_extension = $a_transcode_profile[mediamosa_transcode_profile_db::FILE_EXTENSION];
      $command = $a_transcode_profile[mediamosa_transcode_profile_db::COMMAND];
    }
    else {
      $profile_id = NULL;
    }

  }


}


/**
 * Functie voor het toevoegen van een nieuwe transcode job aan de database
 * Op basis van de opgegeven parameters worden de tool, file_extension en
 * extra parameters bepaald (default profiel, gekozen profiel of zelf
 * meegegeven.
 * Als de gegevens incompleet zijn, of wanneer de parameters niet kloppen
 * wordt een foutmelding teruggegeven. Anders okay
 *
 * Wanneer er een windows wordt aangemaakt moet eerst een job worden
 * aangemaakt om een transcoding naar een raw formaat te maken.
 */


/*


  }
  else if (($a_parameters['profile_id']['value'] != "")) {
    $profile_id = $a_parameters['profile_id']['value'];
    db_set_active();
    $db_result = db_query("
        SELECT tool, file_extension, command
        FROM {transcode_profile}
        WHERE transcode_profile_id = %d", $profile_id);
    if ($db_result_row = db_fetch_object($db_result)) {
      $tool = $db_result_row->tool;
      $file_extension = $db_result_row->file_extension;
      $command = $db_result_row->command;
    }
    else {
      // de transcode profile is niet gevonden in de database
      return new rest_response(vpx_return_error(ERRORCODE_UNKNOWN_TRANSCODE_PROFILE));
    }
  }
  else {
    $profile_id = -1;
    $tool = $a_parameters['tool']['value'];
    $file_extension = $a_parameters['file_extension']['value'];
    $command = $a_parameters['command']['value'];
  }

  // zowel de tool als de file_extensie zijn verplichte velden.
  if (($tool == "") || ($file_extension == "")) {
    return new rest_response(vpx_return_error(ERRORCODE_NO_TRANSCODE_PARAMETERS));
  }

  // controleer of de opgegeven fileextensie wel is toegestaan voor deze tool
  if ($errorcommand = _vpx_jobs_check_command_parameters($tool, "internal_file_extension:". $file_extension)) {
    return new rest_response(vpx_return_error(ERRORCODE_JOB_TRANSCODE_PARAMETER_COMBINATION, array("@tool" => $tool, "@file_extension" => $file_extension)));
  }

  // controleer of de opgegeven parameters toegestaan zijn voor de tool
  if ($errorcommand = _vpx_jobs_check_command_parameters($tool, $command)) {
    return $errorcommand;
  }

  if ($profile_id == -1) {
    $profile_id = "NULL";
  }

  // Build the completed_transcoding_url
  $completed_transcoding_url = $a_parameters['completed_transcoding_url']['value'];
  if (!empty($completed_transcoding_url)) {
    $a_query = array();
    $pos = strpos($completed_transcoding_url, '?');// Lets see if there is a http query
    if ($pos !== FALSE) {
      if (strlen($completed_transcoding_url) > $pos + 1) {
        parse_str(substr($completed_transcoding_url, $pos+1), $a_query);
      }
      $completed_transcoding_url = substr($completed_transcoding_url, 0, $pos);
    }

    $a_query['job_id'] = $a_parameters['job_id']['value'];
    $a_query['mediafile_id'] = $a_parameters['mediafile_id']['value'];
    // The status code will be filled later
    $a_query['status_code'] = '';

    $completed_transcoding_url .= '?' . http_build_query($a_query, '', '&');
  }

  // voeg een record toe aan de trancode_job tabel
  db_set_active("data");
  $db_result = db_query("
    INSERT INTO {transcode_job}
    (job_id, transcode_profile_id, tool, file_extension, command, completed_transcoding_url) VALUES
    (%d, %s, '%s', '%s', '%s', '%s')",
    $a_parameters['job_id']['value'], $profile_id, $tool, $file_extension, $command, $completed_transcoding_url);
  db_set_active();

  // detecteer of een job eigenlijk naar een windows server moet
  // als dat zo is dan moet er eerst op een linux server een job worden
  // aangemaakt voor een vertaling naar xxx.raw
  if ($tool == JOBWINDOWS_TOOL) {
    _vpx_jobs_create_raw_job($job_id, $a_parameters['testtag']['value']);
  }

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
} // end of _vpx_jobs_create_new_job_transcode
*/
