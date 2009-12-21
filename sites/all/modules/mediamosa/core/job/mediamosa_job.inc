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
  * The job module.
  */
class mediamosa_job {

  /**
   * Make sure the job exists.
   *
   * @param integer $job_id
   *
   * @return array
   *  The found job.
   */
  static public function must_exists($job_id) {
    return mediamosa_db::db_must_exists(mediamosa_job_db::TABLE_NAME, array(mediamosa_job_db::ID => $job_id));
  }

  /**
   * Get the job from the main job table.
   * @param integer $job_id
   *  The job ID.
   */
  static public function get($job_id) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j')
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get all information of this job in the database.
   *
   * 1.x -> 2.0 change:
   * will now merge with extra table and
   * will not contain NULL fields from other child tables.
   *
   * 1.x: _vpx_jobs_get_job_info
   *
   * @param integer $job_id
   */
  static public function get_job_ext($job_id) {

    // Get the basic job.
    $a_job_ext = self::get($job_id);

    // Any extra information stored here.
    $a_job_child = NULL;

    // Based on type, we will get the sub-table info.
    switch ($a_job_ext[mediamosa_job_db::JOB_TYPE]) {
      case mediamosa_job_db::JOB_TYPE_STILL:
        $a_job_child = mediamosa_job_still::get($job_id);
        break;
      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        $a_job_child = mediamosa_job_transcode::get($job_id);
        break;
      case mediamosa_job_db::JOB_TYPE_UPLOAD:
        $a_job_child = mediamosa_job_upload::get($job_id);
        break;
    }

    // If set, then merge.
    if ($a_job_child) {
      // Lapping info will prefer what is in job_ext.
      return array_merge($a_job_child, $a_job_ext);
    }

    return $a_job_ext;
  }

  // _vpx_jobs_create_new_job
  /*
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
   */

  static public function create($app_id, $user_id, $is_app_admin, $job_type, $mediafile_id, $create_still = FALSE, $a_still_parameters = array()) {

    // Get the mediafile.
    $a_mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Owner check.
    mediamosa_aut::owner_check($app_id, $user_id, $a_mediafile[mediamosa_asset_mediafile_db::APP_ID], $a_mediafile[mediamosa_asset_mediafile_db::OWNER_ID], $is_app_admin);

    // Get the asset ID.
    $asset_id = $a_mediafile[mediamosa_asset_mediafile_db::ASSET_ID];

    // Create the job.
    $a_fields = array(
      mediamosa_job_db::ASSET_ID => $asset_id,
      mediamosa_job_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_job_db::OWNER_ID => $user_id,
      mediamosa_job_db::APP_ID => $app_id,
      mediamosa_job_db::JOB_TYPE => $job_type,
      mediamosa_job_db::CREATE_STILL => $create_still,
      mediamosa_job_db::STILL_PARAMETERS => serialize($a_still_parameters),
    );

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    $job_id = mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();

    switch ($job_type) {
      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        break;
      case mediamosa_job_db::JOB_TYPE_STILL:
        break;
      case mediamosa_job_db::JOB_TYPE_UPLOAD:
        break;
    }

    return $job_id;
  }
/*
function _vpx_jobs_create_new_job($a_args, $a_parameters) {


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

 */


  // Create a analyse job, after upload has been completed.
  static function create_analyse_job($a_job_ext_upload) {

    assert(isset($a_job_ext_upload[mediamosa_job_upload_db::FILE_SIZE]));

    // Get the mediafile ID.
    $mediafile_id = $a_job_ext_upload[mediamosa_job_db::MEDIAFILE_ID];

    // Get the mediafile.
    $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id);

    // Get the newly uploaded filename.
    $uploaded_filename = mediamosa_configuration_storage::mediafile_filename_get($a_mediafile);

    // Reset IO cache, so we are sure we get the latest information about the file.
    mediamosa_io::clearstatcache();

    // Get the filesize of the uploaded file.
    $uploaded_filesize = mediamosa_io::filesize($uploaded_filename);

    if ($uploaded_filesize != $a_job_ext_upload[mediamosa_job_upload_db::FILE_SIZE]) {
      // Job failed
      // @todo
      return;
    }

    // Do analyse.

    // Check if we need to retranscode.
    $retranscode = $a_job_ext_upload[mediamosa_job_upload_db::RETRANSCODE];
    $create_still = $a_job_ext_upload[mediamosa_job_upload_db::CREATE_STILL];

    if ($retranscode == mediamosa_job_upload_db::RETRANSCODE_TRUE) {

    }


  }

}

/*
 *
 *


        // controleer of er een retranscode moet plaatsvinden.
        db_set_active("data");
        $db_result = db_query("SELECT retranscode, create_still FROM {upload_job} WHERE job_id = %d ", $a_parameters['job_id']['value']);
        assert($db_result);
        $dbrow_upload_job = db_fetch_array($db_result);
        $retranscode = $dbrow_upload_job["retranscode"];
        $create_still = $dbrow_upload_job["create_still"];
        db_set_active();

        if ($retranscode == 'TRUE') {
          _vpx_jobs_add_retranscode_jobs($jobinfo['asset_id'],
                    $jobinfo['mediafile_id'],
                    $jobinfo['owner'],
                    $jobinfo['app_id']);
        }
        else { // verwijder alle bestaande transcodings
          db_set_active('data');
          $result = db_query("SELECT mediafile_id FROM {mediafile} WHERE asset_id = '%s' AND is_original_file = 'FALSE'", $jobinfo['asset_id']);
          db_set_active();
          while ($delete_mediafile_id = db_result($result)) {
            _media_management_delete_mediafile($delete_mediafile_id);
          }
        }

        // Zet de filesize in de database (workaround voor de trage analyse/interval)
        $db = db_set_active('data');
        db_query("UPDATE {mediafile_metadata} SET filesize = %d WHERE mediafile_id = '%s'", $current_filesize, $jobinfo['mediafile_id']);
        if (!db_affected_rows()) {
          db_query("INSERT INTO {mediafile_metadata} (filesize, mediafile_id) VALUES (%d, '%s')", $current_filesize, $jobinfo['mediafile_id']);
        }
        db_set_active($db);

        if ($jobinfo['app_id'] != 9999) { // Workaround for unittest 504.
          // start analyse job
          $a_analyse_parameters = array(
            "mediafile_id" => array("value" => $jobinfo['mediafile_id']),
            "user_id" => array("value" => $jobinfo['owner']),
            "app_id" => array("value" => $jobinfo['app_id']),
            "job_type" => array("value" => JOBTYPE_ANALYSE),
            "testtag" => array("value" => $jobinfo['testtag']),
            //"create_still" => array("value" => $create_still),
            'create_still' => array("value" => $a_parameters['create_still']['value']),
            'still_type' => array("value" => $a_parameters['still_type']['value']),
            'still_per_mediafile' => array("value" => $a_parameters['still_per_mediafile']['value']),
            'still_every_second' => array("value" => $a_parameters['still_every_second']['value']),
            'start_frame' => array("value" => $a_parameters['start_frame']['value']),
            'end_frame' => array("value" => $a_parameters['end_frame']['value']),
            'size' => array("value" => $a_parameters['size']['value']),
            'h_padding' => array("value" => $a_parameters['h_padding']['value']),
            'v_padding' => array("value" => $a_parameters['v_padding']['value']),
            'tag' => array("value" => $a_parameters['tag']['value']),
            'frametime' => array("value" => $a_parameters['frametime']['value']),
            'width' => array("value" => $a_parameters['width']['value']),
            'height' => array("value" => $a_parameters['height']['value']),
          );

          $res = vpx_jobs_get_analysis_upload($jobinfo['app_id'], $jobinfo['mediafile_id']);

          _vpx_jobhandler_update_analyse_job_info_to_mediafile($a_parameters['job_id']['value'], implode("\n", $res));

          // Do not create new job
          //_vpx_jobs_create_new_job(FALSE, $a_analyse_parameters);
        }
      }
      else {
        $a_parameters['status']['value'] = JOBSTATUS_FAILED;
        $a_parameters['error_description']['value'] = "Het mediabestand is niet compleet";
      }
  }

 */


