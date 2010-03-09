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
  * @file
  * The job module.
  */
class mediamosa_job {

  // ------------------------------------------------------------------ Consts.
  // Indicate windows tool.
  const JOBWINDOWS_TOOL = 'windows';

  // Raw job
  const JOBRAW_TOOL = 'ffmpeg';
  const JOBRAW_FILE_EXTENSION = 'avi';
  const JOBRAW_COMMAND = 'videocodec:rawvideo;audiocodec:pcm_s16le;internal_previous_job:%d';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Log for job.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa::log($message, $variables, $severity, 'job');
  }

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
  static public function get($job_id, array $a_fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $a_fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the job from the main job table.
   * @param integer $mediafile_id
   *  The mediafile ID.
   */
  static public function get_by_mediafileid($mediafile_id, array $a_fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $a_fields)
      ->condition(mediamosa_job_db::MEDIAFILE_ID, $mediafile_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Store the error description.
   *
   * @param integer $job_id
   *  The job ID.
   * @param string $error_description
   *  Text to store.
   */
  static public function store_error_description($job_id, $error_description) {
    $a_fields = array(
      mediamosa_job_db::ERROR_DESCRIPTION => $error_description,
    );

    // Add changed.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_db::ID, $job_id);
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

  /**
   * Update the progress in the job.
   *
   * @param integer $job_id
   *  The job ID.
   * @param string $progress
   *  In format x.xxx (0.000 to 1.000)
   * @param bool $started
   */
  static public function set_progress($job_id, $progress, $started = FALSE, $job_status = NULL) {
    assert(mediamosa_unicode::strlen($progress) <= 5);

    $a_fields = array(
      mediamosa_job_db::PROGRESS => $progress,
    );

    if ($started) {
      $a_fields[mediamosa_job_db::STARTED] = mediamosa::db_current_timestamp_now();
      $a_fields[mediamosa_job_db::JOB_STATUS] = mediamosa_job_db::JOB_STATUS_INPROGRESS;
    }

    if ($job_status) {
      $a_fields[mediamosa_job_db::JOB_STATUS] = $job_status;
    }

    return mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute();
  }

  /**
   * Create an extra raw job for windows transcoding.
   *
   * @param integer $job_id
   *  The job ID.
   */
  static public function create_raw($job_id) {
    // Update job for progress.
    self::set_progress($job_id, '0.333', TRUE);

    // Get the job.
    $a_job = self::get($job_id);
    if (!$a_job) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_NOT_FOUND, array('@job_id' => $job_id));
    }

    // Fields to insert.
    $a_fields = array(
      mediamosa_job_db::ASSET_ID => $a_job[mediamosa_job_db::ASSET_ID],
      mediamosa_job_db::MEDIAFILE_ID => $a_job[mediamosa_job_db::MEDIAFILE_ID],
      mediamosa_job_db::OWNER_ID => $a_job[mediamosa_job_db::OWNER_ID],
      mediamosa_job_db::APP_ID => $a_job[mediamosa_job_db::APP_ID],
      mediamosa_job_db::PRIORITY => $a_job[mediamosa_job_db::PRIORITY],
    );

    // Add created/changed.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    $job_id_raw = mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();

    // Create a new transcode job.
    mediamosa_job_transcode::create(
      $a_job[mediamosa_job_db::APP_ID],
      $job_id_raw,
      mediamosa_job::JOBRAW_FILE_EXTENSION,
      sprintf(mediamosa_job::JOBRAW_COMMAND, $job_id),
      mediamosa_job::JOBRAW_TOOL
    );
  }

  /**
   * Add retranscode jobs.
   *
   * 1.x: _vpx_jobs_add_retranscode_jobs
   *
   * @param string $asset_id
   *  The asset ID.
   * @param string $mediafile_id
   *  The mediafile ID.
   * @param string $user_id
   *  The owner.
   * @param integer $app_id
   *  The application ID.
   */
  static public function add_retranscode_jobs($asset_id, $mediafile_id, $user_id, $app_id) {
    // Get all mediafiles that are transcoded, so we can retranscode them.
    $a_mediafiles = mediamosa_asset_mediafile::get_all_transcoded($asset_id);

    $counter = 0;
    foreach ($a_mediafiles as $a_mediafile) {
      $counter++;

      // Create new job.
      try {
        mediamosa_job::create_job_transcode($app_id, $user_id, '', FALSE, $mediafile_id, FALSE, array(), $a_mediafile[mediamosa_asset_mediafile_db::FILE_EXTENSION], $a_mediafile[mediamosa_asset_mediafile_db::COMMAND], $a_mediafile[mediamosa_asset_mediafile_db::TOOL]);
      }
      catch (mediamosa_exception $e) {
        // ignore..
      }
    }

    return $counter;
  }

  /**
   * Create a transcode job
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param string $file_extension
   * @param string $command
   * @param string $tool
   * @param integer $profile_id
   * @param string $completed_transcoding_url
   * @param string $mediafile_id
   */
  static public function create_job_transcode($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $create_still, $a_still_parameters, $file_extension, $command, $tool = '', $profile_id = '', $completed_transcoding_url = '', $priority = NULL) {

    // Must have user_quota.
    mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);

    // Must be transcodable.
    mediamosa_asset_mediafile_metadata::is_transcodable($mediafile_id);

    // Create job.
    $job_id = self::create(mediamosa_job_db::JOB_TYPE_TRANSCODE, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still, $a_still_parameters, $priority);

    // Create sub transcode job.
    mediamosa_job_transcode::create($app_id, $job_id, $file_extension, $command, $tool, $profile_id, $completed_transcoding_url, $mediafile_id);

    return $job_id;
  }

  /**
   * Create still job.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param array $a_still_parameters
   */
  static public function create_job_still($app_id, $user_id, $is_app_admin, $mediafile_id, array $a_still_parameters = array()) {

    // Create job.
    $job_id = self::create(mediamosa_job_db::JOB_TYPE_STILL, $app_id, $user_id, $is_app_admin, $mediafile_id, FALSE, $a_still_parameters);

    // Create sub still job.
    mediamosa_job_still::create($job_id, $a_still_parameters);

    return $job_id;
  }

  /**
   * Create upload job.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $group_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param integer $file_size
   * @param bool $retranscode
   * @param bool $create_still
   */
  static public function create_job_upload($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $file_size, $retranscode, $create_still, $a_still_parameters) {

    // Enough quota?
    mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);

    // Create job.
    $job_id = self::create(mediamosa_job_db::JOB_TYPE_UPLOAD, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still, $a_still_parameters);

    // Create sub upload job.
    mediamosa_job_upload::create($app_id, $job_id, $user_id, $group_id, $mediafile_id, $file_size, $retranscode, $create_still);

    // Return the new job_id.
    return $job_id;
  }

  /**
   * Create a job.
   *
   * @param string $job_type
   * @param integer $app_id
   * @param string $user_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param bool $create_still
   * @param array $a_still_parameters
   */
  static public function create($job_type, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still = FALSE, $a_still_parameters = array(), $priority = NULL, $no_hint = FALSE) {

    // Get the mediafile.
    $a_mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Owner check.
    mediamosa_aut::owner_check($app_id, $user_id, $a_mediafile[mediamosa_asset_mediafile_db::APP_ID], $a_mediafile[mediamosa_asset_mediafile_db::OWNER_ID], $is_app_admin);

    // Re-analysing still.
    if ($job_type == mediamosa_job_db::JOB_TYPE_ANALYSE && $a_mediafile[mediamosa_asset_mediafile_db::IS_STILL] == mediamosa_asset_mediafile_db::IS_STILL_TRUE) {
      // Get the metadata.
      $a_mediafile_metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id);

      if (!$a_mediafile_metadata) {
        $still_id = $a_mediafile['mediafile_id'];

        // Destination.
        $destination = mediamosa_configuration_storage::still_filename_get($still_id);

        // Still size.
        $size = getimagesize($destination);
        $width = $size[0];
        $height = $size[1];

        // File type.
        $file_type = '';
        $pos = strrpos($size['mime'], '/');
        if ($pos !== FALSE) {
          $file_type = substr($size['mime'], $pos+1);
        }
        $filesize = filesize($destination);

        mediamosa_asset_mediafile_metadata::create_metadata_still_default($still_id, $width, $height, $filesize, $size['mime'], $file_type);
      }

      return 0;
    }

    // Get the asset ID.
    $asset_id = $a_mediafile[mediamosa_asset_mediafile_db::ASSET_ID];

    // Create the job.
    $a_fields = array(
      mediamosa_job_db::ASSET_ID => $asset_id,
      mediamosa_job_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_job_db::OWNER_ID => $user_id,
      mediamosa_job_db::APP_ID => $app_id,
      mediamosa_job_db::JOB_TYPE => $job_type,
      mediamosa_job_db::CREATE_STILL => $create_still ? mediamosa_job_db::CREATE_STILL_TRUE : mediamosa_job_db::CREATE_STILL_FALSE,
      mediamosa_job_db::STILL_PARAMETERS => serialize($a_still_parameters),
      mediamosa_job_db::NO_HINT => $no_hint ? mediamosa_job_db::NO_HINT_TRUE : mediamosa_job_db::NO_HINT_FALSE,
      mediamosa_job_db::PROGRESS => '0.000',
    );

    if (isset($priority)) {
      $a_fields[mediamosa_job_db::PRIORITY] = $priority;
    }

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  // Create a analyse job, after upload has been completed.
  static public function create_analyse_job_with_jobext($a_job_ext_upload) {

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
      // Job failed.
      // Change status to failed.
      return array('status' => mediamosa_job_db::JOB_STATUS_FAILED, 'progress' => '1.000');
    }

    // Do analyse.

    // Check if we need to retranscode.
    $retranscode = $a_job_ext_upload[mediamosa_job_upload_db::RETRANSCODE];
    $create_still = $a_job_ext_upload[mediamosa_job_upload_db::CREATE_STILL];

    if ($retranscode == mediamosa_job_upload_db::RETRANSCODE_TRUE) {
      self::add_retranscode_jobs(
        $a_job_ext_upload[mediamosa_job_db::ASSET_ID],
        $a_job_ext_upload[mediamosa_job_db::MEDIAFILE_ID],
        $a_job_ext_upload[mediamosa_job_db::OWNER_ID],
        $a_job_ext_upload[mediamosa_job_db::APP_ID]
      );
    }
    else {
      // Delete the unoriginal mediafiles of this asset.
      mediamosa_asset_mediafile::delete_transcoded_unoriginal_mediafiles($a_job_ext_upload[mediamosa_job_db::ASSET_ID]);
    }

    // Lets try to get it so we know we either UPDATE or INSERT.
    $a_mediafile_metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($a_job_ext_upload[mediamosa_job_db::MEDIAFILE_ID]);

    // Exists, so we UPDATE.
    if (!empty($a_mediafile_metadata)) {
      $a_fields = array(
        mediamosa_asset_mediafile_metadata_db::FILESIZE => $uploaded_filesize,
      );

      // Add changed.
      $a_fields = mediamosa_db::db_update_enrich($a_fields);

      mediamosa_db::db_update(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
        ->fields($a_fields)
        ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $a_job_ext_upload[mediamosa_job_db::MEDIAFILE_ID])
        ->execute();
    }
    else {
      // INSERT.
      $a_fields = array(
        mediamosa_asset_mediafile_metadata_db::FILESIZE => $uploaded_filesize,
        mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => $a_job_ext_upload[mediamosa_job_db::MEDIAFILE_ID],
      );

      // Add changed/created.
      $a_fields = mediamosa_db::db_insert_enrich($a_fields);

      // Insert it anyway.
      mediamosa_db::db_insert(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
        ->fields($a_fields)
        ->execute();
    }

    // Analyse the mediafile.
    $a_analyse_result = mediamosa_asset_mediafile::analyse($a_job_ext_upload[mediamosa_job_db::APP_ID], $a_job_ext_upload[mediamosa_job_db::MEDIAFILE_ID]);
    assert(is_array($a_analyse_result));

    // Store it.
    mediamosa_asset_mediafile_metadata::store_analyse($a_job_ext_upload[mediamosa_job_db::ID], $a_analyse_result);
  }

  /**
   * Create basic analyse job.
   *
   * @param string $owner
   * @param string $asset_id
   * @param string $mediafile_id
   * @param integer $app_id
   */
  static public function create_analyse_job($owner, $asset_id, $mediafile_id, $app_id) {
    self::log('Starting analyse job for #mediafile_id', array('#mediafile_id' => (string)$mediafile_id));

    $a_fields = array(
      mediamosa_job_db::ASSET_ID => $asset_id,
      mediamosa_job_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_job_db::OWNER_ID => $owner,
      mediamosa_job_db::PRIORITY => -1,
      mediamosa_job_db::JOB_TYPE => mediamosa_job_db::JOB_TYPE_ANALYSE,
      mediamosa_job_db::APP_ID => $app_id,
    );

    // Add created.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Create.
    mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Trigger completed_transcoding_url.
   *
   * 1.x: vpx_jobs_notify_transcoding
   *
   * @param string $old_status
   *  Old status.
   * @param string $new_status
   *  New status.
   * @param integer $job_id
   *  The job ID.
   */
  static public function notify_transcoding($old_status, $new_status, $job_id) {
    if ($old_status == $new_status) {
      return;
    }

    $a_job_transcode = mediamosa_job_transcode::get($job_id);

    if (!empty($a_job_transcode[mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL])) {
      $completed_transcoding_url = $a_job_transcode[mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL];
      self::log($completed_transcoding_url . $new_status, array(), WATCHDOG_NOTICE);

      // Do the call (through proxy).
      mediamosa_http::do_head_call($completed_transcoding_url . $new_status, array('use_curlproxy' => TRUE));
    }
  }

  /**
   * Change the job status.
   *
   * @param integer job_id
   * @param string $job_status
   * @param string $error_description
   */
  static public function set_job_status_failed($job_id, $error_description, $old_status = 'NONE') {
    $a_fields = array(
      mediamosa_job_db::JOB_STATUS => mediamosa_job_db::JOB_STATUS_FAILED,
      mediamosa_job_db::FINISHED => mediamosa::db_current_timestamp_now(),
      mediamosa_job_db::ERROR_DESCRIPTION => $error_description,
    );

    // Add changed.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    // Update
    mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute();

    // Trigger completed url.
    self::notify_transcoding($old_status, mediamosa_job_db::JOB_STATUS_FAILED, $job_id);
  }

  /**
   * Make sure if the job information to start new job is correct.
   *
   * 1.x: _vpx_jobhandler_is_valid_job
   *
   * @param $job_id
   *  Job ID.
   * @param $job_type
   *  Job type.
   * @param $mediafile_id
   *  The Mediafile ID.
   */
  static public function is_valid_job($job_id, $job_type, $mediafile_id) {

    switch ($job_type) {
      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id);
        if (!$a_mediafile || !mediamosa_asset_mediafile::is_original($a_mediafile)) {
          self::set_job_status_failed(mediamosa::t('The original mediafile (ID: @mediafile_id is not present or not the original.', array('@mediafile_id' => $mediafile_id)));
          return FALSE;
        }
        break;
    }

    return TRUE;
  }

  /**
   * Update the status of the job.
   *
   * @param array $a_job_ext
   * @param string $progress
   * @param string $status
   * @param string $error_description
   */
  static public function update_status(array $a_job_ext, $status, $progress = '', $error_description = '') {

    /**
     * If JOB is upload and we are setting the status on FINISHED, then start analyse job.
     */
    if ($a_job_ext[mediamosa_job_db::JOB_TYPE] == mediamosa_job_db::JOB_TYPE_UPLOAD &&
        $progress == '1.000' &&
        $a_job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FINISHED) {

      // Create an analyse job.
      $resp = mediamosa_job::create_analyse_job_with_jobext($a_job_ext);
      if (is_array($resp) && isset($resp['status']) && isset($resp['progress'])) {
        $status = $resp['status'];
        $progress = $resp['progress'];
      }
    }

    $a_fields = array();

    // Set progress.
    if ($progress != '') {
      if ($a_job_ext[mediamosa_job_db::PROGRESS] != $progress) {
        $a_fields[mediamosa_job_db::PROGRESS] = $progress;
      }

      if (($progress == '1.000') &&
          ( ($status != mediamosa_job_db::JOB_STATUS_FINISHED) &&
            ($status != mediamosa_job_db::JOB_STATUS_FAILED) &&
            ($a_job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FINISHED) &&
            ($a_job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FAILED))) {
         $status = mediamosa_job_db::JOB_STATUS_FINISHED;
      }
    }

    // Start value.
    if ($a_job_ext[mediamosa_job_db::STARTED] == '' && $status != mediamosa_job_db::JOB_STATUS_WAITING) {
      $a_fields[mediamosa_job_db::STARTED] = mediamosa::db_current_timestamp_now();
    }

    // Finished value.
    if ($a_job_ext[mediamosa_job_db::FINISHED] == '' &&
        (($status == mediamosa_job_db::JOB_STATUS_FAILED) ||
         ($status == mediamosa_job_db::JOB_STATUS_FINISHED) )) {
      $a_fields[mediamosa_job_db::FINISHED] = mediamosa::db_current_timestamp_now();
    }

    // Set status
    if ($a_job_ext[mediamosa_job_db::JOB_STATUS] != $status) {
      $a_fields[mediamosa_job_db::JOB_STATUS] = $status;
    }

    // Set de error description.
    if ($a_job_ext[mediamosa_job_db::ERROR_DESCRIPTION] != $error_description && $error_description) {
      $a_fields[mediamosa_job_db::ERROR_DESCRIPTION] = $error_description;
    }

    if (!empty($a_fields)) {
      $current_status = $a_job_ext[mediamosa_job_db::JOB_STATUS];

      mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
        ->fields($a_fields)
        ->condition(mediamosa_job_db::ID, $a_job_ext[mediamosa_job_db::ID])
        ->execute();

      // Trigger url.
      self::notify_transcoding($current_status, $status, $a_job_ext[mediamosa_job_db::ID]);
    }
  }

  /**
   * Give a list of jobs and as much as information.
   *
   * 1.x: vpx_jobs_get_job_list
   *
   * @param $a_parameters
   */
  static public function get_job_list($app_id, $user_id, $is_app_admin = FALSE, $job_id = 0, $asset_id = '', $mediafile_id = '') {

    $query = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j');
    $query->fields('j');
    $query->join(mediamosa_asset_db::TABLE_NAME, 'a', 'a.asset_id = j.asset_id');

    if (!$is_app_admin) {
      $query->condition('j.' . mediamosa_job_db::APP_ID, $app_id);
      $query->condition('j.' . mediamosa_job_db::OWNER_ID, $user_id);
    }

    if (!empty($job_id)) {
      $query->condition('j.' . mediamosa_job_db::ID, $job_id);
    }

    if (!empty($asset_id)) {
      $query->condition(db_or()->condition('a.' . mediamosa_asset_db::ID, $asset_id)->condition('a.' . mediamosa_asset_db::PARENT_ID, $asset_id));
    }

    if (!empty($mediafile_id)) {
      $query->condition('j.' . mediamosa_job_db::MEDIAFILE_ID, $mediafile_id);
    }
    $query->orderBy('j.' . mediamosa_job_db::ASSET_ID, 'ASC');
    $query->orderBy('j.' . mediamosa_job_db::PRIORITY, 'ASC');

    return $query->execute()->fetchAll();
  }

  /**
   * Deletes the job. Moved into function for re-use.
   *
   * 1.x: _vpx_jobs_delete_job
   *
   * @param integer $job_id
   * @return bool TRUE; job has been deleted
   */
  static public function delete_job($job_id) {
    mediamosa_db::db_query(
      'DELETE tj FROM {#mediamosa_job_transcode} AS tj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.job_id = :job_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_transcode' => mediamosa_job_transcode_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query(
      'DELETE sj FROM {#mediamosa_job_still} AS sj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.job_id = :job_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_still' => mediamosa_job_still_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query(
      'DELETE uj FROM {#mediamosa_job_upload} AS uj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.job_id = :job_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_upload' => mediamosa_job_upload_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query('DELETE FROM {#mediamosa_job} WHERE job_id = :job_id AND status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    // See if delete has worked.
    $num_rows = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'mj')
      ->fields('mj', array(mediamosa_job_db::ID))
      ->condition(mediamosa_job_db::ID, $job_id)
      ->countQuery()->execute()->fetchField();

    return ($num_rows ? FALSE : TRUE);
  }

  /**
   * Deletes the job. Moved into function for re-use.
   *
   * _vpx_jobs_cancel_job_by_asset
   *
   * @param integer $job_id
   * @return bool TRUE; job has been deleted
   */
  static public function delete_job_by_asset($asset_id) {
    mediamosa_db::db_query(
      'DELETE tj FROM {#mediamosa_job_transcode} AS tj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.asset_id = :asset_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_transcode' => mediamosa_job_transcode_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':asset_id' => $asset_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query(
      'DELETE sj FROM {#mediamosa_job_still} AS sj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.asset_id = :asset_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_still' => mediamosa_job_still_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':asset_id' => $asset_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query(
      'DELETE uj FROM {#mediamosa_job_upload} AS uj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.asset_id = :asset_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_upload' => mediamosa_job_upload_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':asset_id' => $asset_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query('DELETE FROM {#mediamosa_job} WHERE asset_id = :asset_id AND status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':asset_id' => $asset_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    // See if delete has worked.
    $num_rows = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'mj')
      ->fields('mj', array(mediamosa_job_db::ASSET_ID))
      ->condition(mediamosa_job_db::ASSET_ID, $asset_id)
      ->countQuery()->execute()->fetchField();

    return ($num_rows ? FALSE : TRUE);
  }

  /**
   * Cancel job.
   * Can only be removed if status is WAITING, FINISHED or FAILED and
   * not assigned to server.
   *
   * 1.x: _vpx_jobs_cancel_job
   *
   */
  static public function cancel_job($app_id, $job_id, $user_id) {

    // Must exist to cancel.
    self::must_exists($job_id);

    // Check if can be removed.
    $a_row_job = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_job} WHERE
       #job_id = :job_id AND
       #status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED) AND
       #job_id NOT IN (SELECT #job_id FROM #mediamosa_server_job)',
      array(
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        '#mediamosa_server_job' => mediamosa_server_job_db::TABLE_NAME,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
        '#job_id' => mediamosa_job_db::ID,
        '#status' => mediamosa_job_db::JOB_STATUS,
      )
    )->fetchAssoc();

    if ($a_row_job == FALSE) {
      throw new mediamosa_exception_error_job_could_not_be_removed(array('@job_id' => $job_id));
    }

    // Must be owner.
    mediamosa_aut::owner_check($app_id, $user_id, $a_row_job['app_id'], $a_row_job['owner']);

    $job_status = $a_row_job['status'];
    $job_type = $a_row_job['job_type'];

    // If we try to delete a analyse job, then make sure its has a metadata row.
    if ($job_type == mediamosa_job_db::JOB_TYPE_ANALYSE && $job_status == mediamosa_job_db::JOB_STATUS_WAITING) {

      $num_rows = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mamm')
        ->fields('mamm', array(mediamosa_asset_mediafile_metadata_db::ID))
        ->condition(mediamosa_asset_mediafile_metadata_db::ID, $a_row_job['mediafile_id'])
        ->countQuery()->execute()->fetchField();

      if (!$num_rows) {
        throw new mediamosa_exception_error_job_could_not_be_removed(array('@job_id' => $job_id));
      }
    }

    // Delete it.
    if (!self::delete_job($job_id)) {
      throw new mediamosa_exception_error_job_could_not_be_removed(array('@job_id' => $job_id));
    }
  }

  static public function update_progress_upload($job_id, $uploaded_file_size) {

      // Get complete job.
    $a_job_ext = mediamosa_job::get_job_ext($job_id);

    if (!$uploaded_file_size  && $a_job_ext[mediamosa_job_db::JOB_TYPE] == mediamosa_job_db::JOB_TYPE_UPLOAD) {
      // If the filesize is 0, then we are done...
      if (!$a_job_ext[mediamosa_job_upload_db::FILE_SIZE]) {

        // Update the job.
        mediamosa_job::update_status($a_job_ext, mediamosa_job_db::JOB_STATUS_INPROGRESS, '1.000');
        return; // done
      }
    }

    // Set uploaded size.
    $a_fields = array(
      mediamosa_job_upload_db::UPLOADED_FILE_SIZE => $uploaded_file_size,
    );

    // Set fields.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    // Update.
    mediamosa_db::db_update(mediamosa_job_upload_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_upload_db::JOB_ID, $job_id)
      ->execute();

    // Get progress
    $progress = mediamosa_db::db_query(
      'SELECT #uploaded_file_size / #file_size FROM {#mediamosa_job_upload} WHERE #job_id = :job_id',
      array(
        '#uploaded_file_size' => mediamosa_job_upload_db::UPLOADED_FILE_SIZE,
        '#file_size' => mediamosa_job_upload_db::FILE_SIZE,
        '#mediamosa_job_upload' => mediamosa_job_upload_db::TABLE_NAME,
        '#job_id' => mediamosa_job_upload_db::JOB_ID,
        ':job_id' => $job_id,
      )
    )->fetchField();

    // 1.xxx.
    $progress = number_format(floatval($progress), 3, '.', '');

    // FIXME: all rest calls with vpx_jobs_set_job_status are broken, still params not used~!

    // Update the job.
    mediamosa_job::update_status($a_job_ext, mediamosa_job_db::JOB_STATUS_INPROGRESS, $progress);
  }
}
