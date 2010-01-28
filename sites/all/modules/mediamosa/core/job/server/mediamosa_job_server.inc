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
  * The job server module.
  */


class mediamosa_job_server {

  /**
   * Log for job_server.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa::log($message, $variables, $severity, 'job_server');
  }

  /**
   * Log debug for job_server.
   *
   * @param string $message
   * @param array $variables
   */
  static public function log_debug($message, array $variables = array()) {
    mediamosa_debug::log_debug($message, $variables, 'job_server');
  }

  /**
   * Log debug for job_server.
   *
   * @param string $message
   * @param array $variables
   */
  static public function log_debug_high($message, array $variables = array()) {
    mediamosa_debug::log_debug_high($message, $variables, 'job_server');
  }

  /**
   * Trigger every minute.
   *
   * Will timeout on itself after 2 minutes.
   *
   */
  static public function run_parse_queue() {
    $server_id = mediamosa_settings::get_server_id();

    $run_last = variable_get('mediamosa_jobserver_cron_last_' . $server_id, NULL);

    // may run? (0 - 60 seconds)
    if (round(REQUEST_TIME - $run_last) < 60) {
      return; // Still running.
    }

    variable_set('mediamosa_jobserver_cron_last_' . $server_id, REQUEST_TIME);

    // Log it.
    self::log_debug('Start parse queue job server');

    // Run.
    self::parse_queue();

    // Log it.
    self::log_debug('Ending parse queue job server');
  }
  /**
   * Parse the queue jobs.
   *
   * This function is the engine of the job server. Should be called at
   * regular intervals.
   *
   * 1. Get a list of all jobs that are not finished, failed or canceled.
   * 2. For all jobs with status WAITING, the job is started.
   * 3. For all running jobs de status is updated.
   *
   * 1.x: vpx_jobserver_parse_queue.
   */
  static public function parse_queue() {
    // Retrieve all jobs of this server.
    $result = mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::variable_get('mediamosa_installation_id', 'default'))
      ->condition(mediamosa_job_server_db::JOB_STATUS, array(mediamosa_job_server_db::JOB_STATUS_FINISHED, mediamosa_job_server_db::JOB_STATUS_FAILED, mediamosa_job_server_db::JOB_STATUS_CANCELLED), 'NOT IN')
      ->orderBy(mediamosa_job_server_db::ID, 'ASC')
      ->execute();

    foreach ($result as $a_job_server_job) {
      if ($a_job_server_job[mediamosa_job_server_db::JOB_STATUS] == mediamosa_job_server_db::JOB_STATUS_WAITING) {
        // New job, start it.
        self::waiting_job_start($a_job_server_job);
      }
      else {
        // Running job, update it.
        self::running_job_update($a_job_server_job);
      }
    }
  }


  /**
   * Create analyse server job.
   *
   * @param $job_id
   * @param $mediafile_id_src
   */
  static public function create_job_analyse($job_id, $mediafile_id_src) {
    // Set job type.
    $job_type = mediamosa_job_server_db::JOB_TYPE_ANALYSE;

    // Create basic server job.
    $jobserver_job_id = self::create_job($job_id, $job_type, $mediafile_id_src);

    $a_values = array(
      mediamosa_job_server_analyse_db::ID => $jobserver_job_id,
      mediamosa_job_server_analyse_db::ANALYSE_RESULT => '',
    );

    // Enrich with created time.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_server_analyse_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Create transcode server job.
   *
   * @param $job_id
   */
  static public function create_job_transcode($job_id, $mediafile_id_src, $tool, $file_extension, $command) {
    // Set job type.
    $job_type = mediamosa_job_server_db::JOB_TYPE_TRANSCODE;

    // Create basic server job.
    $jobserver_job_id = self::create_job($job_id, $job_type, $mediafile_id_src);

    $a_values = array(
      mediamosa_job_server_transcode_db::ID => $jobserver_job_id,
      mediamosa_job_server_transcode_db::TOOL => $tool,
      mediamosa_job_server_transcode_db::FILE_EXTENSION => $file_extension,
      mediamosa_job_server_transcode_db::COMMAND => $command,
    );

    // Enrich with created time.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_server_transcode_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Create still server job.
   *
   * @param $job_id
   */
  static public function create_job_still($job_id, $mediafile_id_src, $frametime, $size, $h_padding, $v_padding, $blackstill_check, array $a_still_parameters) {
    // Set job type.
    $job_type = mediamosa_job_server_db::JOB_TYPE_STILL;

    // Create basic server job.
    $jobserver_job_id = self::create_job($job_id, $job_type, $mediafile_id_src);

    $a_values = array(
      mediamosa_job_server_still_db::ID => $jobserver_job_id,
      mediamosa_job_server_still_db::FRAMETIME => $frametime,
      mediamosa_job_server_still_db::SIZE => $size,
      mediamosa_job_server_still_db::H_PADDING => $h_padding,
      mediamosa_job_server_still_db::V_PADDING => $v_padding,
      mediamosa_job_server_still_db::BLACKSTILL_CHECK => ($blackstill_check ? mediamosa_job_server_still_db::BLACKSTILL_CHECK_TRUE : mediamosa_job_server_still_db::BLACKSTILL_CHECK_FALSE),
      mediamosa_job_server_still_db::STILL_PARAMETERS => serialize($a_still_parameters)

    );

    // Enrich with created time.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_server_still_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Create a job server job.
   *
   * @param $job_id
   */
  static public function create_job($job_id, $job_type, $mediafile_id_src) {

    // Get the job_server_job, make sure its not already created.
    $a_job_server_job = mediamosa_job_server::get($job_id);
    if (!empty($a_job_server_job)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STARTING_JOB_FAILED);
    }

    // Data we want to insert.
    $a_fields = array(
      mediamosa_job_server_db::JOB_ID => $job_id,
      mediamosa_job_server_db::JOB_TYPE => $job_type,
      mediamosa_job_server_db::JOB_STATUS => mediamosa_job_server_db::JOB_STATUS_WAITING,
      mediamosa_job_server_db::MEDIAFILE_ID_SRC => $mediafile_id_src,
      mediamosa_job_server_db::INSTALL_ID => mediamosa::variable_get('mediamosa_installation_id', 'default'),
    );

    // Enrich with created time.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_job_server_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Start the job.
   *
   * 1.x: _vpx_jobserver_startjob.
   */
  static public function waiting_job_start($a_job_server_job) {

    // 2.x:
    // Check if jobs module is enable has been removed.
    // Old code checked it without using the app_id. Which is unknown.
    // Think its better not to check here but to prevent new jobs from being inserted
    // when job module is turned off. Lets process jobs, even when its been disabled.

    // Job ID.
    $job_id = $a_job_server_job[mediamosa_job_server_db::JOB_ID];

    // Source Mediafile ID.
    $mediafile_id_src = $a_job_server_job[mediamosa_job_server_db::MEDIAFILE_ID_SRC];

    // Job type.
    $job_type = $a_job_server_job[mediamosa_job_server_db::JOB_TYPE];

    // Job server id (ID of the row in the mediamosa_job_server table).
    $jobserver_job_id = $a_job_server_job[mediamosa_job_server_db::ID];

    // Set status in progress.
    self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_INPROGRESS, '0.000');

    // Check if source mediafile exists.
    // but first do a clear cache.
    clearstatcache();
    $filename = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id_src);
    if (!file_exists($filename)) {
      $link_asset = self::get_asset_link($job_id);
      self::log("s: $job_id File '@file' not found,<br /><br/>@link", array('@file' => $filename, '@link' => $link_asset));
      self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'File not found (@file)', array('@file' => $filename));
      self::log("s: $job_id, memory_get_peak_usage = " . memory_get_peak_usage() . " memory_usage: " . memory_get_usage());
      return;
    }

    // op basis van het type job wordt een script gestart
    $execution_string = "";
    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        $execution_string = self::get_transcode_string($jobserver_job_id, $mediafile_id_src);
        break;

      case mediamosa_job_server_db::JOB_TYPE_STILL:
        $execution_string = self::get_still_string($jobserver_job_id, $mediafile_id_src);
        break;

      case mediamosa_job_server_db::JOB_TYPE_ANALYSE:
        $execution_string = self::get_analyse_string($mediafile_id_src, $job_id);
        break;

      default:
        self::log('Unknown job type: @job_type in job id: @job_id.', array('@job_type' => $job_type, '@job_id' => $job_id));
        self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'Unknown job type: @job_type.', array('@job_type' => $job_type));
        return FALSE;
    }

    // Log our action.
    self::log('About to start @job_type job: @job_id calling exec: @execution_string', array('@job_type' => $job_type, '@job_id' => $job_id, '@execution_string' => $execution_string));

    // Now execute.
    if ($job_type == mediamosa_job_server_db::JOB_TYPE_ANALYSE) {
      $a_output = array();
      $s_output = exec($execution_string . ' 2>&1', $a_output);

      $link_asset = self::get_asset_link($job_id);
      self::log('Job @job_type (Job ID: @job_id) returned output: @s_output - @a_output<br /><br />@link',
        array(
          '@job_type' => $job_type,
          '@job_id' => $job_id,
          '@s_output' => $s_output,
          '@a_output' => implode("\n", $a_output),
          '@link' => $link_asset,
        )
      );

      if ($s_output != "") {
        self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FINISHED, '1.000');
      }
      else {
        self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'Empty result, analyse failed.');
      }

      // Log it.
      self::log('Starting new followup analyse job for job ID: @job_id.', array('@job_id' => $job_id));

      // Update job_server_analyse row.
      $a_fields = array(
        mediamosa_job_server_analyse_db::ANALYSE_RESULT => implode("\n", $a_output),
      );

      // Enrich with changed.
      $a_fields = mediamosa_db::db_update_enrich($a_fields);

      // And update row.
      mediamosa_db::db_update(mediamosa_job_server_analyse_db::TABLE_NAME)
        ->fields($a_fields)
        ->condition(mediamosa_job_server_analyse_db::ID, $jobserver_job_id)
        ->execute();
    }
    else {
      exec($execution_string);
      self::log('Started @job_type job, Job ID: @job_id', array('@job_type' => $job_type, '@job_id' => $job_id));
    }
  }

  /**
   * Update the job.
   *
   * Updates running jobs.
   *
   * 1. Get possible status file and parse it.
   * 2. When transcode or still job is done, files will be moved to the
   *    correct location.
   *
   * 1.x: _vpx_jobserver_updatestatus.
   */
  static public function running_job_update($a_job_server_job) {

    // Job ID.
    $job_id = $a_job_server_job[mediamosa_job_server_db::JOB_ID];

    // Source Mediafile ID.
    $mediafile_id_src = $a_job_server_job[mediamosa_job_server_db::MEDIAFILE_ID_SRC];

    // Job type.
    $job_type = $a_job_server_job[mediamosa_job_server_db::JOB_TYPE];

    // Read the contents of the status file in an array.
    $a_job_status = self::get_status_contents($mediafile_id_src);

    // Defailt status.
    $status = mediamosa_job_server_db::JOB_STATUS_INPROGRESS;

    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        if ($a_job_status['Status'] == 'done' && $a_job_status['Errors'] == 'none') {
          // Status to finished.
          $status = mediamosa_job_server_db::JOB_STATUS_FINISHED;

          // Store the transcode.
          self::store_new_mediafile($job_id, $mediafile_id_src);

          // Log it.
          self::log('End job @job_type, Job ID: @job_id, status: @status', array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status));
        }
        elseif ($a_job_status['Status'] == 'error' && $a_job_status['Errors'] != 'none') {
          $status = mediamosa_job_server_db::JOB_STATUS_FAILED;
          $link_asset = self::get_asset_link($job_id);

          self::log("End @job_type job, Job ID @job_id, with status: @status<br /><br />@link", array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status, '@link' => $link_asset));
          self::log("Info @job_type job, Job ID @job_id, status file '@statusfile'", array('@job_type' => $job_type, '@job_id' => $job_id, '@statusfile' => self::get_status_contents($mediafile_id_src, TRUE)));
        }


        break;

      case mediamosa_job_server_db::JOB_TYPE_STILL:
        // Scene still filename.
        $file_scene = mediamosa_configuration_storage::still_scene_file_location_get($job_id);

        if (file_exists($file_scene) || ($a_job_status["Status"] == "done" && $a_job_status["Errors"] == "none")) {
          $status = self::store_new_still($job_id, $mediafile_id_src);
          self::log("End @job_type job, Job ID @job_id, with status: @status", array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status));
        }
        elseif ($a_job_status["Status"] == "error" && $a_job_status["Errors"] != "none") {
          $status = mediamosa_job_server_db::JOB_STATUS_FAILED;
          $link_asset = self::get_asset_link($job_id);
          self::log("End @job_type job, Job ID @job_id, with status: @status<br /><br />@link", array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status, '@link' => $link_asset));
          self::log("Info @job_type job, Job ID @job_id, status file '@statusfile'", array('@job_type' => $job_type, '@job_id' => $job_id, '@statusfile' => self::get_status_contents($mediafile_id_src, TRUE)));
        }

        // Update the status.
        if (!file_exists($file_scene) && $a_job_status['Errors'] != 'none') {
          self::set_job_status($job_id, $status, $a_job_status['Progress'], $a_job_status["Errors"]);
        }
        else {
          self::set_job_status($job_id, $status, $a_job_status['Progress']);
        }

        break;
    }
  }

  /**
   * Retrieve a listing of the jobs.
   *
   * This function may only be called from jobserver (via internal).
   *
   * 1.x: vpx_jobserver_get_jobs_status (not exact port(!)).
   *
   */
  static public function search() {
    $query = mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js');
    $query->leftJoin(mediamosa_job_server_analyse_db::TABLE_NAME, 'jsa', 'js.jobserver_job_id = jsa.jobserver_job_id');
    $query->fields('js');
    $query->fields('jsa');
    $query->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::variable_get('mediamosa_installation_id', 'default'));
    return $query->execute();
  }

  /**
   * Get the job.
   * (only from this installation(!))
   *
   * @param integer $job_id
   */
  static public function get($job_id) {
    return mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->condition(mediamosa_job_server_db::ID, $job_id)
      ->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::variable_get('mediamosa_installation_id', 'default'))
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Set the job status on a jobserver table.
   *
   * _vpx_jobserver_set_job_status
   *
   * @param $job_id
   * @param $job_status
   * @param $progress
   * @param $error_description
   */
  static public function set_job_status($job_id, $job_status, $progress, $error_description = '', $a_error_description_args = array()) {

    // Set args in description.
    if (!empty($a_error_description_args)) {
      $error_description = strtr($error_description, $a_error_description_args);
    }

    $a_fields = array(
      mediamosa_job_server_db::JOB_STATUS => $job_status,
      mediamosa_job_server_db::PROGRESS => $progress,
    );

    switch ($job_status) {
      case mediamosa_job_server_db::JOB_STATUS_FINISHED:
      case mediamosa_job_server_db::JOB_STATUS_FAILED:
      case mediamosa_job_server_db::JOB_STATUS_CANCELLED:
        $a_fields[mediamosa_job_server_db::FINISHED] = mediamosa::db_current_timestamp_now();
        break;
    }

    // Check if its started.
    $a_jobserver_job = self::get($job_id);
    if (!$a_jobserver_job) {
      assert(0);
      return;
    }

    // Set status.
    if ($a_jobserver_job[mediamosa_job_server_db::JOB_STATUS] == mediamosa_job_server_db::JOB_STATUS_WAITING && $job_status == mediamosa_job_server_db::JOB_STATUS_INPROGRESS) {
      $a_fields[mediamosa_job_server_db::STARTED] = mediamosa::db_current_timestamp_now();
    }

    if (!empty($error_description)) {
      $a_fields[mediamosa_job_server_db::ERROR_DESCRIPTION] = $error_description;
    }

    // Update.
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->execute();
  }

  /**
   * Create a link to the parent asset belonging to a given job id.
   *
   * 1.x: _vpx_jobserver_get_asset_link
   *
   * @param int $job_id
   *
   * @return string
   *  Link to an asset.
   */
  static public function get_asset_link($job_id) {

    // Get the job.
    $a_jobserver_job = mediamosa_job::get($job_id);

    // Get asset ID from job.
    $asset_id = $a_jobserver_job[mediamosa_job_db::ASSET_ID];

    // Get the asset.
    $a_asset = mediamosa_asset::get($a_jobserver_job[mediamosa_job_db::ASSET_ID]);

    // Check parent.
    if (!empty($a_asset[mediamosa_asset_db::PARENT_ID])) {
      $asset_id = $a_asset[mediamosa_asset_db::PARENT_ID];
    }

    // Return link.
    return l(mediamosa::t('Go to asset @asset_id', array('@asset_id' => $asset_id)), strtr(mediamosa_settings::URL_ASSET, array('@asset_id' => $asset_id)));
  }

  /**
   * Create the string send with the vpx-analyse script.
   *
   * 1.x: _vpx_jobserver_get_analyse_string
   *
   * @param string $mediafile_src
   * @param $job_id
   */
  static public function get_analyse_string($mediafile_id, $job_id) {

    // Get the job.
    $a_app = mediamosa_job::get($job_id, array(mediamosa_job_db::APP_ID));
    $app_id = $a_app[mediamosa_app_db::APP_ID];
    $no_hint = $a_app[mediamosa_app_db::NO_HINT];

    // Get the app.
    $a_mediamosa_app = mediamosa_app::get_by_appid($app_id, array(mediamosa_app_db::ALWAYS_HINT_MP4, mediamosa_app_db::ALWAYS_INSERT_MD));

    $a_options = array();

    if (!$no_hint) {
      if ($a_mediamosa_app[mediamosa_app_db::ALWAYS_HINT_MP4] == mediamosa_app_db::ALWAYS_HINT_MP4_TRUE) {
        $a_options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
      }

      if ($a_mediamosa_app[mediamosa_app_db::ALWAYS_INSERT_MD] == mediamosa_app_db::ALWAYS_INSERT_MD_TRUE) {
        $a_options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
      }
    }

    assert(file_exists(mediamosa_settings::analyse_file()));

    $execution_string = sprintf('%s %s %s',
      mediamosa_settings::analyse_file(),
      mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(),
      $mediafile_id
    );

    $execution_string .= (count($a_options) ? ' ' . implode(' ', $a_options) : '');

    return $execution_string;
  }

  /**
   * Generate the string that is used for the vpx_transcode script.
   *
   * 1.x: _vpx_jobserver_get_transcode_string
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id
   */
  static public function get_transcode_string($jobserver_job_id, $mediafile_id) {

    // Get it.
    $a_job_server_transcode = mediamosa_job_server_transcode::get($jobserver_job_id);

    $execution_string = '';

    if ($a_job_server_transcode) {
      $tool = $a_job_server_transcode[mediamosa_job_server_transcode_db::TOOL];
      $file_extension = $a_job_server_transcode[mediamosa_job_server_transcode_db::FILE_EXTENSION];
      $command = $a_job_server_transcode[mediamosa_job_server_transcode_db::COMMAND];
    }
    else {
      self::log('Transcode job not found, jobserver_id: @jobserver_id', array('@jobserver_id' => $jobserver_job_id));
      return $execution_string;
    }

    // bewerk de parameters zodat er een mooie string ontstaat.
    $parameters = mediamosa_lib::create_named_array($command, ';', ':');
    $parameter_string = "'";
    foreach ($parameters as $name => $value) {
      $parameter_string .= sprintf('%s %s ', $name, $value);
    }

    $parameter_string .= "'";
    $parameter_string = trim($parameter_string);
    if ($parameter_string == "''") {
      $parameter_string = "";
    }

    // Conbine based on selection.
    switch ($tool) {
      case 'ffmpeg':
        $execution_string = sprintf("%s %s %s %s %s > /dev/null &", mediamosa_settings::ffmpeg_transcode_file(), mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(), $mediafile_id, $file_extension, $parameter_string);
        break;
      case 'windows':
        $execution_string = sprintf("%s %s %s %s", mediamosa_settings::windows_transcode_file(), $mediafile_id, $file_extension, $parameter_string);
        break;
      default:
        return '';
    }

    return $execution_string;
  }

  /**
   * Generate the still execute string.
   *
   * 1.x: _vpx_jobserver_get_still_string
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id
   * @return string|string|string
   */
  static public function get_still_string($jobserver_job_id, $mediafile_id) {

    $a_job_server_still = mediamosa_job_server_still::get($jobserver_job_id);
    $a_job_server = mediamosa_job_server::get($jobserver_job_id);
    $job_id = $a_job_server[mediamosa_job_server_db::JOB_ID];

    $execution_string = "";

    if ($a_job_server_still) {
      $frametime = $a_job_server_still[mediamosa_job_server_still_db::FRAMETIME];
      $size = $a_job_server_still[mediamosa_job_server_still_db::SIZE];
      $h_padding = $a_job_server_still[mediamosa_job_server_still_db::H_PADDING];
      $v_padding = $a_job_server_still[mediamosa_job_server_still_db::V_PADDING];

      $still_parameters = unserialize($a_job_server_still[mediamosa_job_server_still_db::STILL_PARAMETERS]);
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

      // Set to null so we know we have set it.
      $error_description = NULL;

      switch (mediamosa_unicode::strtoupper($still_parameters['still_type'])) {
        case 'NORMAL':
          if (!is_numeric($still_parameters['still_per_mediafile']) || $still_parameters['still_per_mediafile'] < 1) {
            $still_parameters['still_type'] = 'NONE';
            $error_description = "Still_per_mediafile parameter is missing. Still creation fall back to still_type = 'NONE'";
          }
          break;
        case 'SECOND':
          if (!is_numeric($still_parameters['still_every_second']) || $still_parameters['still_every_second'] < 0) {
            $still_parameters['still_type'] = 'NONE';
            $error_description = "Still_every_second parameter is missing. Still creation fall back to still_type = 'NONE'";
          }
          break;
        case 'SCENE':
          if (!$fps || !is_numeric($fps)) {
            $still_parameters['still_type'] = 'NONE';
            $error_description = "Fps parameter in not numeric. Still creation fall back to still_type = 'NONE'";
          }
          break;
        default:
          break;
      }

      // Store error_description.
      if (isset($error_description)) {
        self::log($error_description);
        mediamosa_db::db_query(
          'UPDATE {#mediamosa_job_server}
           SET #error_description = :error_description
           WHERE #jobserver_job_id = :jobserver_job_id',
          array(
            '#mediamosa_job_server' => mediamosa_job_server_db::TABLE_NAME,
            '#error_description' => mediamosa_job_server_db::ERROR_DESCRIPTION,
            '#jobserver_job_id' => mediamosa_job_server_db::ID,
            ':error_description' => $error_description,
            ':jobserver_job_id' => $jobserver_job_id,
          )
        );
      }

      // Security check and base values
      if (!is_numeric($frametime) || $frametime < $start_frame || $frametime > $end_frame) {
        $frametime = $start_frame + ($end_frame - $start_frame) / 3;
      }
      $duration = 1;
      $framerate = 1;

      switch (mediamosa_unicode::strtoupper($still_parameters['still_type'])) {
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
          if ($duration / $framesecond > mediamosa_settings::STILL_MAXIMUM) {
            $duration = mediamosa_settings::STILL_MAXIMUM * $framesecond;
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
          if ($duration / $still_every_second > mediamosa_settings::STILL_MAXIMUM) {
            $duration = mediamosa_settings::STILL_MAXIMUM * $still_every_second;
          }
          break;
        case 'SCENE':
          self::log('Scene detection starts');
          $destination_path = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . DATA_LOCATION . DIRECTORY_SEPARATOR . 'transcode' . DIRECTORY_SEPARATOR;
          //self::log(sprintf('Scene: destination path = %s', $destination_path));
          $input = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id);

          // Clean up I.
          mediamosa_io::unlink($destination_path . $mediafile_id .'_scene.avi');
          mediamosa_io::unlink($destination_path . $mediafile_id .'_list.el');


  /*
  // Old code
          $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $input, $destination_path . $mediafile_id .'_scene.avi');
          //self::log($order);
          exec($order);
  */

  // Hot fix
          $order = sprintf('ffmpeg -i %s %s', $input, $destination_path . $mediafile_id .'_scene.wmv');
          //self::log($order);
          exec($order);

          $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $destination_path . $mediafile_id . '_scene.wmv', $destination_path . $mediafile_id .'_scene.avi');
          //self::log($order);
          mediamosa_io::exec($order);

          mediamosa_io::unlink($destination_path . $mediafile_id . '_scene.wmv');

  // Hot fix - end

          $order = sprintf('lav2yuv -T 1 -S %s %s', $destination_path . $mediafile_id .'_list.el', $destination_path . $mediafile_id .'_scene.avi');
          self::log($order);
          mediamosa_io::exec($order);

          $output = NULL;
          $order = sprintf('cat %s', $destination_path . $mediafile_id .'_list.el');
          //self::log($order);
          mediamosa_io::exec($order, $output);
          //self::log(sprintf('output = %s', print_r($output, TRUE)));

          // Clean up II.
          mediamosa_io::unlink($destination_path . $mediafile_id .'_list.el');

          // Analyze
          if ($output && is_array($output) && $output != array()) {
            //self::log(sprintf('Output is array with data'));

            // tbr & map from the best stream
            $map = NULL;
            $tbr = NULL;
            $kbs = NULL;
            $order = sprintf('ffmpeg -i %s 2>&1', $input);
            //self::log($order);
            $details = array();
            mediamosa_io::exec($order, $details);
            //self::log('Details: '. print_r($details, TRUE));
            foreach($details as $line) {
              if (stripos($line, ' Stream ') !== FALSE && stripos($line, ' Video: ') !== FALSE) {
                if (preg_match('/Stream #[0-9]*\.([0-9]*)/', $line, $reg_map) && preg_match('/ ([0-9]*) kb\/s/', $line, $reg_kbs)) {
                  if (!$map || !$kbs || $kbs < $reg_kbs[1]) {
                    $map = $reg_map[1];
                    $kbs = $reg_kbs[1];
                  }
                  if (preg_match('/ ([0-9]*(\.[0-9]*)+) tb/', $line, $regs)) {
                    $tbr = $regs[1];
                  }
                }
                elseif (!$map && preg_match('/ ([0-9]*(\.[0-9]*)+) tb/', $line, $regs)) {
                  $tbr = $regs[1];
                  if (preg_match('/Stream #[0-9]*\.([0-9]*)/', $line, $reg_map)) {
                    $map = $reg_map[1];
                  }
                }
              }
            }
            //self::log('tbr: '. print_r($tbr, TRUE) .', map: '. print_r($map, TRUE) .', kbs: '. print_r($kbs, TRUE));
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
                //self::log(sprintf('Line: %s', print_r($line, TRUE)));
                $line_args = explode(' ', $line);
                //self::log(sprintf('Line args: %s', print_r($line_args, TRUE)));
                if ($line_args && is_array($line_args) && $line_args != array()) {
                  //self::log(sprintf('Line args is array with data'));
                  if (isset($line_args[0]) && isset($line_args[1]) && isset($line_args[2]) && !isset($line_args[3]) && is_numeric($line_args[0]) && is_numeric($line_args[1]) && is_numeric($line_args[2])) {
                    //self::log(sprintf('Line args has 3 numbers'));
                    // $line_args[1] + num = a little bit after the change
                    $change_frame = $line_args[1] + mediamosa_settings::STILL_SCENE_AFTER;
                    //self::log('change_frame: '. print_r($change_frame, TRUE) .', start_frame: '. print_r($start_frame, TRUE) .', fps = '. $fps .', end_frame: '. print_r($end_frame, TRUE));
                    if ($change_frame >= ($start_frame*$fps) && $change_frame <= ($end_frame*$fps)) {
                      // $scene_frame + num = minimal distance with two changes
                      //self::log('scene_frame: '. print_r($scene_frame, TRUE));
                      if (!$scene_frame || $scene_frame + mediamosa_settings::STILL_SCENE_DISTANCE < $change_frame) {
                        $scene_frame = $change_frame;
                        //self::log(sprintf('New scene frame: %s', $scene_frame));
                        // The avi has 25 frame/sec
                        $scene_sec = (int)($scene_frame/25);
                        $scenes[] = $scene_sec;
                        $i++;
                        if ($i > mediamosa_settings::STILL_MAXIMUM) {
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
            $my_file = $destination_path . $job_id .'_ffmpeg_scene.txt';
            $fh = fopen($my_file, 'w') or $fh = NULL;
            if ($fh) {
              foreach ($scenes as $scene) {
                fwrite($fh, $scene ."\n");

              }
              fclose($fh);

              // This version is creating stills for the original video
              $order = sprintf(mediamosa_settings::STILL_SCENE_STRING, $input, $size, $h_padding, $h_padding, $v_padding, $v_padding, $my_file, ($map ? '-map 0.'. $map : ''), $destination_path . $mediafile_id . mediamosa_settings::STILL_EXTENSION .".jpeg");

              // This version is creating stills for the analysed file
              //$order = sprintf(mediamosa_settings::STILL_SCENE_STRING, $destination_path . $mediafile_id .'_scene.avi', $size, $h_padding, $h_padding, $v_padding, $v_padding, $my_file, '', $destination_path . $mediafile_id . mediamosa_settings::STILL_EXTENSION .".jpeg");
              self::log($order);
              mediamosa_io::exec($order, $details);
              //self::log('Details: '. print_r($details, TRUE));

              mediamosa_io::unlink($my_file);
            }

            // Clean up III.
            mediamosa_io::unlink($destination_path . $mediafile_id .'_scene.avi');

            // It has to be the last, because it triggers the further processes
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

          // Clean up IV.
          mediamosa_io::unlink($destination_path . $mediafile_id .'_scene.avi');

          // We reached this point, so something went wrong in the creation of scene stills.
          // So we are creating still type NONE (original behaviour).
          break;

        default:
          break;
      }

    }
    else {
      self::log('Still job not found, jobserver_job_id: @jobserver_job_id', array('@jobserver_job_id' => $jobserver_job_id));
      return $execution_string;
    }

    $still_parameters['frametime'] = $frametime;
    $still_parameters['duration'] = $duration;
    $still_parameters['framerate'] = $framerate;

    $a_fields = array(
      mediamosa_job_db::STILL_PARAMETERS => serialize($still_parameters),
    );

    // Enrich with changed/created.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_db::JOB_ID)
      ->execute();

    // Combine the string (only for linux, windows does not generate stills).
    $execution_string = sprintf('%s %s %s jpeg ' . mediamosa_settings::STILL_STRING, mediamosa_settings::still_file(), mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(), $mediafile_id, $size, $h_padding, $h_padding, $v_padding, $v_padding, $frametime, $duration, $framerate);
    $execution_string .= ' > /dev/null &';

    return $execution_string;
  }

  /**
   * Create an array of the status file.
   *
   * 1.x: _vpx_jobserver_get_status
   *
   * @param $filename
   * @param $orig
   */
  static public function get_status_contents($filename, $orig = FALSE) {
    $statusfile = mediamosa_configuration_storage::status_file_location_get($filename);
    if (!file_exists($statusfile)) {
      return array();
    }

    $a_result = $a_lines = array();

    // FIXME move to mediamosa_io
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
   * Check the created still and save it if everything is ok.
   *
   * 1.x: _vpx_jobserver_store_new_still
   *
   * @param string $job_id
   *  Current job id.
   * @param string $mediafile_id_src
   *  Contains a file path to the mediafile
   * @return string
   *  Contains the error message
   */
  static public function store_new_still($job_id, $mediafile_id_src) {

    $base_filename = mediamosa_io::get_base_filename($mediafile_id_src);

    // Is black still working?
    if (@filesize(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename .'_0.jpeg')) {
      return mediamosa_job_server_db::JOB_STATUS_INPROGRESS;
    }

    // Check if there really is an image ($file_size > 0)
    if (filesize(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, 1) . ".jpeg") == 0) {
      // Something failed, very likely the frametime was too high. Remove the files and fail the job.
      $still_error = mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_JOB_FRAMETIME_GREATER_THEN_DURATION);

      // Update status.
      self::set_job_status($job_id, mediamosa_job_db::JOB_STATUS_FAILED, '1.000', $still_error);

      // Remove all of the still images
      $i = 1;
      while (file_exists(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg") && $i <= mediamosa_settings::STILL_MAXIMUM) {
        unlink(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg");
        $i++;
      }
      unlink(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename .".status");

      self::log($still_error);
      return mediamosa_job_server_db::JOB_STATUS_FAILED;
    }

    // Check if the frame has any usefull content. We do this by checking the amount of dominant colors.
    $mediafile_id_src = mediamosa_job_server_still::still_validate($job_id, $mediafile_id_src);

    if ($mediafile_id_src === FALSE) {
      self::log("Job failed, are there any colors in this movie?");
      self::set_job_status($job_id, mediamosa_job_db::JOB_STATUS_FAILED, '1.000', "Job failed, are there any colors in this movie?");
      return mediamosa_job_server_db::JOB_STATUS_FAILED;
    }

    $i = 1;
    $mediafile_dest = array();
    while (file_exists(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg")) {
      if ($i <= mediamosa_settings::STILL_MAXIMUM) {
        // Generate new hash
        $filename = mediamosa_db::uuid(rand(1, 9999));

        // Everything went ok, move the still and remove other files
        mediamosa_io::rename(
          mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg",
          mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . STILL_LOCATION . DIRECTORY_SEPARATOR . $filename{0} . DIRECTORY_SEPARATOR . $filename
        );
        $mediafile_dest[] = $filename;
      }
      else {
        // Reached the maximum, just delete the remain stills
        unlink(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg");
      }

      $i++;
    }
    mediamosa_io::unlink(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename .".status");

    // Data to update.
    $a_fields = array(
      mediamosa_job_server_db::MEDIAFILE_DEST => serialize($mediafile_dest),
    );

    // Add changed.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    // Update mediafile_dest of the job
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->execute();

    self::log('Job finished: Multiple stills saved as e.g.: ' . mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . STILL_LOCATION . DIRECTORY_SEPARATOR . $filename{0} . DIRECTORY_SEPARATOR . $filename, NULL);

    return mediamosa_job_server_db::JOB_STATUS_FINISHED;
  }

  /**
   * Save a new mediafile.
   *
   * 1.x: _vpx_jobserver_store_new_mediafile
   *
   * @param $filename
   * @param $orig
   */
  static public function store_new_mediafile($job_id, $old_filename) {

    // Generate the filename.
    $new_filename = mediamosa_db::uuid(rand (1, 9999));

    $a_job_server = mediamosa_db::db_query(
      'SELECT mjst.#command, mjst.#file_extension
       FROM {#mediamosa_job_server_transcode} AS mjst
       JOIN {#mediamosa_job_server} AS mjs ON mjs.#jobserver_job_id = mjst.#jobserver_job_id
       WHERE mjs.#job_id = :job_id',
      array(
        '#command' => mediamosa_job_server_transcode_db::COMMAND,
        '#file_extension' => mediamosa_job_server_transcode_db::FILE_EXTENSION,
        '#mediamosa_job_server_transcode' => mediamosa_job_server_transcode_db::TABLE_NAME,
        '#mediamosa_job_server' => mediamosa_job_server_db::TABLE_NAME,
        '#jobserver_job_id' => mediamosa_job_server_db::ID,
        '#job_id' => mediamosa_job_server_db::JOB_ID,
        ':job_id' => $job_id,
      )
    )->fetchAssoc();

    if ($a_job_server == FALSE) {
      self::log('Transcode job not fould for job with ID @job_id', array('@job_id' => $job_id));
      return;
    }

    // Get file extension.
    $file_extension = $a_job_server[mediamosa_job_server_transcode_db::FILE_EXTENSION];

    // If extension is RAW, attach it to the filename.
    if ($file_extension == mediamosa_job::JOBRAW_FILE_EXTENSION) {
      $new_filename .= '.' . mediamosa_job::JOBRAW_FILE_EXTENSION;
    }

    // Get the filenames.
    $file_status = mediamosa_configuration_storage::status_file_location_get($old_filename);
    $file_transcode = mediamosa_configuration_storage::transcode_file_location_get($old_filename . '.' . $file_extension);
    $file_dest = mediamosa_configuration_storage::mediafile_id_filename_get($new_filename);

    // Rename transcoded file to new dest.
    mediamosa_io::rename($file_transcode, $file_dest);

    // Now remove the status file.
    mediamosa_io::unlink($file_status);

    $a_fields = array(
      mediamosa_job_server_db::MEDIAFILE_DEST => $new_filename,
    );

    // Enrich with update date.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    // Update the filename in mediafile_dest.
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id);

    // Log it.
    self::log("Job with ID @job_id ready, new mediafile stored as '@file_dest'.", array('@job_id' => $job_id, '@file_dest' => $file_dest));
  }

  /**
   * Delete a server job.
   * @param integer $job_id
   * @param bool $killjob
   */
  static public function delete_job($job_id, $killjob = FALSE) {

    // Get the job.
    $a_jobserver_job = mediamosa_job_server::get($job_id);

    if (empty($a_jobserver_job)) {
      return;
    }

    if ($killjob) {
      $a_fields = array(
        mediamosa_job_server_db::JOB_STATUS => mediamosa_job_server_db::JOB_STATUS_CANCELLED,
      );

      // Enrich with changed value.
      $a_fields = mediamosa_db::db_update_enrich($a_fields);

      // Update.
      mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
        ->fields($a_fields)
        ->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::variable_get('mediamosa_installation_id', 'default'))
        ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
        ->execute();

      // FIXME:
      // Send kill command to specific job.
      // Remove files.
    }

    $jobserver_job_id = $a_jobserver_job[mediamosa_job_server_db::ID];
    $job_type = $a_jobserver_job[mediamosa_job_server_db::JOB_TYPE];

    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_ANALYSE:
        // Remove.
        mediamosa_db::db_delete(mediamosa_job_server_analyse_db::TABLE_NAME)
          ->condition(mediamosa_job_server_analyse_db::ID, $jobserver_job_id)
          ->execute();
        break;
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        // Remove.
        mediamosa_db::db_delete(mediamosa_job_server_transcode_db::TABLE_NAME)
          ->condition(mediamosa_job_server_transcode_db::ID, $jobserver_job_id)
          ->execute();
        break;
      case mediamosa_job_server_db::JOB_TYPE_STILL:
        // Remove.
        mediamosa_db::db_delete(mediamosa_job_server_still_db::TABLE_NAME)
          ->condition(mediamosa_job_server_still_db::ID, $jobserver_job_id)
          ->execute();
        break;
    }

    // Remove.
    mediamosa_db::db_delete(mediamosa_job_server_db::TABLE_NAME)
      ->condition(mediamosa_job_server_db::ID, $jobserver_job_id)
      ->execute();
  }
}
