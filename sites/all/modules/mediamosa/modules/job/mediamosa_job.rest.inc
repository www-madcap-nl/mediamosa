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
  * Jobs Module REST calls.
  */

/**
 * URI: /job/$job_id/status
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_jobs_set_job_status
 *
 */
class mediamosa_rest_call_job_set_status extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const JOB_ID = 'job_id';
  const STATUS = 'status';
  const PROGRESS = 'progress';
  const ERROR_DESCRIPTION = 'error_description';
  const CREATE_STILL = 'create_still';
  const STILL_TYPE = 'still_type';
  const STILL_PER_MEDIAFILE = 'still_per_mediafile';
  const STILL_EVERY_SECOND = 'still_every_second';
  const START_FRAME = 'start_frame';
  const END_FRAME = 'end_frame';
  const SIZE = 'size';
  const H_PADDING = 'h_padding';
  const V_PADDING = 'v_padding';
  const TAG = 'tag';
  const FRAMETIME = 'frametime';
  const WIDTH = 'width';
  const HEIGHT ='height';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'The job ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::STATUS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The status.',
          self::VAR_ALLOWED_VALUES => array(mediamosa_job_db::STATUS_WAITING, mediamosa_job_db::STATUS_INPROGRESS, mediamosa_job_db::STATUS_FINISHED, mediamosa_job_db::STATUS_FAILED),
        ),
        self::PROGRESS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The progress of the job from 0.000 to 1.000.',
        ),
        self::ERROR_DESCRIPTION => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The possible error description log.',
        ),
        self::CREATE_STILL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'The create still switch after upload.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::STILL_TYPE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The still type.',
        ),
        self::STILL_PER_MEDIAFILE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills per mediafile.',
        ),
        self::STILL_EVERY_SECOND => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills every second.',
        ),
        self::START_FRAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still start frame.',
        ),
        self::END_FRAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still end frame.',
        ),
        self::SIZE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Size of the still.',
        ),
        self::H_PADDING => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still horizontal padding.',
        ),
        self::V_PADDING => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still vertical padding.',
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Still tag.',
        ),
        self::FRAMETIME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still frametime.',
        ),
        self::WIDTH => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still width.',
        ),
        self::HEIGHT => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still height.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    // Get params.
    $job_id = $this->get_param_value(self::JOB_ID);
    $progress = $this->get_param_value(self::PROGRESS);
    $status = $this->get_param_value(self::STATUS);
    $error_description = $this->get_param_value(self::ERROR_DESCRIPTION);

    /*
     * The extra params are not used atm, because code has been turned off that
     * uses them.
     * FIXME: need to think about removing redudent params.
     * its an internal, so should not be a problem.
     */

    // Job must exist.
    mediamosa_job::must_exists($job_id);

    // Get complete job.
    $a_job_ext = mediamosa_job::get_job_ext($job_id);

    // Update the job.
    mediamosa_job::update_status($a_job_ext, $status, $progress, $error_description);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/$mediafile_id/analyse
 * Method: POST
 *
 *
 * 1.x: vpx_jobs_analyse_mediafile
 *
 */
class mediamosa_rest_call_job_analyse_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Get params.
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Create it.
    mediamosa_job::create(mediamosa_job_db::JOB_TYPE_ANALYSE, $app_id, $user_id, $is_app_admin, $mediafile_id);
  }
}

/**
 * URI: /job/$job_id/progress
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_jobs_set_upload_progress
 *
 */
class mediamosa_rest_call_job_set_upload_progress extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const JOB_ID = 'job_id';
  const UPLOADED_FILE_SIZE = 'uploaded_file_size';
  const CREATE_STILL = 'create_still';
  const STILL_TYPE = 'still_type';
  const STILL_PER_MEDIAFILE = 'still_per_mediafile';
  const STILL_EVERY_SECOND = 'still_every_second';
  const START_FRAME = 'start_frame';
  const END_FRAME = 'end_frame';
  const SIZE = 'size';
  const H_PADDING = 'h_padding';
  const V_PADDING = 'v_padding';
  const TAG = 'tag';
  const FRAMETIME = 'frametime';
  const WIDTH = 'width';
  const HEIGHT ='height';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'The job ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::UPLOADED_FILE_SIZE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'The uploaded file size.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $job_id = $this->get_param_value(self::JOB_ID);
    $uploaded_file_size = $this->get_param_value(self::UPLOADED_FILE_SIZE);

    // Must exist.
    mediamosa_job::must_exists($job_id);

    // Update it.
    mediamosa_job::update_progress_upload($job_id, $uploaded_file_size);

    // Set Okay.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /job/create
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_jobscheduler_create_new_job
 *
 */
class mediamosa_rest_call_job_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const JOB_TYPE = 'job_type';
  const CREATE_STILL = 'create_still';
  const STILL_TYPE = 'still_type';
  const STILL_PER_MEDIAFILE = 'still_per_mediafile';
  const STILL_EVERY_SECOND = 'still_every_second';
  const START_FRAME = 'start_frame';
  const START_TIME = 'start_time';
  const END_FRAME = 'end_frame';
  const END_TIME = 'end_time';
  const SIZE = 'size';
  const H_PADDING = 'h_padding';
  const V_PADDING = 'v_padding';
  const TAG = 'tag';
  const FRAMETIME = 'frametime';
  const WIDTH = 'width';
  const HEIGHT ='height';
  const COMPLETED_TRANSCODING_URL = 'completed_transcoding_url';

  const PROFILE_ID = 'profile_id';
  const TOOL = 'tool';
  const FILE_EXTENSION = 'file_extension';
  const COMMAND = 'command';
  const PRIORITY = 'priority';
  const FILE_SIZE = 'file_size';
  const RETRANSCODE = 'retranscode';
  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::JOB_TYPE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_JOB,
          self::VAR_DESCRIPTION => 'The job type.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID.',
        ),
        self::CREATE_STILL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Create still after job run.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::STILL_TYPE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The still type.',
        ),
        self::STILL_PER_MEDIAFILE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills per mediafile.',
        ),
        self::STILL_EVERY_SECOND => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills every second.',
        ),
        self::START_FRAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still start frame.',
        ),
        self::START_TIME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still start time.',
        ),
        self::END_FRAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still end frame.',
        ),
        self::END_TIME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still end time.',
        ),
        self::SIZE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Size of the still.',
        ),
        self::H_PADDING => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still horizontal padding.',
        ),
        self::V_PADDING => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still vertical padding.',
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Still tag.',
        ),
        self::FRAMETIME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still frametime.',
        ),
        self::WIDTH => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still width.',
        ),
        self::HEIGHT => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still height.',
        ),
        self::COMPLETED_TRANSCODING_URL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_URL,
          self::VAR_DESCRIPTION => 'Triggered when transcoding is completed.',
        ),
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Profile ID, used with transcode job.',
        ),
        self::TOOL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Tool used for transcode, used with transcode job.',
        ),
        self::FILE_EXTENSION => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'file extension of the file, used with transcode job.',
        ),
        self::COMMAND => array(
          self::VAR_TYPE => mediamosa_type::TYPE_COMMAND,
          self::VAR_DESCRIPTION => 'Command used with transcode, used with transcode job.',
        ),
        self::PRIORITY => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Priority of the job, used with transcode job.',
          self::VAR_RANGE_START => -100,
          self::VAR_RANGE_END => 100,
        ),
        self::FILE_SIZE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'File size.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO, // Only when type is upload.
        ),
        self::RETRANSCODE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Retranscode after upload, user with upload jobs.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Progress Rest Args.
  /**
   * Process the supplied parameters.
   *
   * @param array $a_var_setup
   */
  protected function process_rest_args(array $a_var_setup) {
    $a_var_setup = parent::process_rest_args($a_var_setup);

    $job_type = $this->get_param_value(self::JOB_TYPE);

    // Make File size required when uploading.
    if ($job_type == mediamosa_job_db::JOB_TYPE_UPLOAD) {
      $a_var_setup[self::VARS][self::FILE_SIZE][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $a_var_setup[self::VARS][self::GROUP_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
    }

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $job_type = $this->get_param_value(self::JOB_TYPE);
    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $completed_transcoding_url = $this->get_param_value(self::COMPLETED_TRANSCODING_URL);
    $file_extension = $this->get_param_value(self::FILE_EXTENSION);
    $command = $this->get_param_value(self::COMMAND);
    $tool = $this->get_param_value(self::TOOL);
    $profile_id = $this->get_param_value(self::PROFILE_ID);
    $file_size = $this->get_param_value(self::FILE_SIZE);
    $priority = $this->get_param_value(self::PRIORITY);
    $retranscode = $this->get_param_value(self::RETRANSCODE);

    $create_still = $this->get_param_value(self::CREATE_STILL);
    $a_still_parameters = array(
      'still_type' => $this->get_param_value(self::STILL_TYPE),
      'still_per_mediafile' => $this->get_param_value(self::STILL_PER_MEDIAFILE),
      'still_every_second' => $this->get_param_value(self::STILL_EVERY_SECOND),
      'start_frame' => $this->get_param_value(self::START_FRAME),
      'end_frame' => $this->get_param_value(self::END_FRAME),
      'size' => $this->get_param_value(self::SIZE),
      'h_padding' => $this->get_param_value(self::H_PADDING),
      'v_padding' => $this->get_param_value(self::V_PADDING),
      'tag' => $this->get_param_value(self::TAG),
      'frametime' => $this->get_param_value(self::FRAMETIME),
      'width' => $this->get_param_value(self::WIDTH),
      'height' => $this->get_param_value(self::HEIGHT),
    );

    // Webservice must be enabled.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_JOBS, $a_app_ids);

    // Make sure the mediafile exists, returns the mediafile.
    $a_mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);

    if (in_array($job_type, array(mediamosa_job_db::JOB_TYPE_TRANSCODE, mediamosa_job_db::JOB_TYPE_UPLOAD))) {
    }

    if ($a_mediafile[mediamosa_asset_mediafile_db::URI] == '' || is_null($a_mediafile[mediamosa_asset_mediafile_db::URI])) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CHANGE_URI_AND_FILE, array('@mediafile_id' => $mediafile_id));
    }

    switch ($job_type) {
      case mediamosa_job_db::JOB_TYPE_STILL:
        $job_id = mediamosa_job::create_job_still($app_id, $user_id, $is_app_admin, $mediafile_id, $a_still_parameters);
        break;

      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        $job_id = mediamosa_job::create_job_transcode($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $create_still, $a_still_parameters, $file_extension, $command, $tool, $profile_id, $completed_transcoding_url, $priority);
        break;

      case mediamosa_job_db::JOB_TYPE_UPLOAD:
        $job_id = mediamosa_job::create_job_upload($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $file_size, $retranscode, $create_still, $a_still_parameters);
        break;

      default:
        $job_id = mediamosa_job::create($job_type, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still, $a_still_parameters);
        break;
    }

    // Return the created job_id.
    $o_mediamosa->add_item(array('job_id' => $job_id));
  }
}

/**
 * URI: /user/$user_id/joblist
 * Method: GET
 *
 * Return list of jobs based on user as owner.
 *
 * 1.x: vpx_jobscheduler_get_user_job_list
 *
 */
class mediamosa_rest_call_job_user_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $user_id = $this->get_param_value(self::USER_ID);

    // Get the jobs from the user.
    $result = mediamosa_job::get_job_list($app_id, $user_id, $is_app_admin);

    foreach ($result as $a_row) {
      $started_unix = isset($a_row[mediamosa_job_db::STARTED]) ? mediamosa_lib::iso8601date_to_unix($a_row[mediamosa_job_db::STARTED]) : '';

      $a_job = array();

      $a_job['id'] = $a_row[mediamosa_job_db::ID];
      $a_job['owner'] = $a_row[mediamosa_job_db::OWNER_ID];
      $a_job['status'] = $a_row[mediamosa_job_db::JOB_STATUS];
      $a_job['progress'] = $a_row[mediamosa_job_db::PROGRESS];
      $a_job['priority'] = $a_row[mediamosa_job_db::PRIORITY];
      $a_job['job_type'] = $a_row[mediamosa_job_db::JOB_TYPE];
      $a_job['started'] = $started_unix ? date('d-m-Y H:i', $started_unix) : '';
      $a_job['started_unix'] = $started_unix;
      $a_job['error_description'] = $a_row[mediamosa_job_db::ERROR_DESCRIPTION];

      $o_mediamosa->add_item($a_job);
    }
  }
}

/**
 * URI: /asset/$asset_id/joblist
 * Method: GET
 *
 * Return list of jobs based on asset_id
 *
 * 1.x: vpx_jobscheduler_get_asset_job_list
 *
 */
class mediamosa_rest_call_job_asset_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const ASSET_ID = 'asset_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to search on.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $user_id = $this->get_param_value(self::USER_ID);
    $asset_id = $this->get_param_value(self::ASSET_ID);

    // Must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Get the jobs from the user.
    $result = mediamosa_job::get_job_list($app_id, $user_id, $is_app_admin, FALSE, $asset_id);

    foreach ($result as $a_row) {
      $started_unix = isset($a_row[mediamosa_job_db::STARTED]) ? mediamosa_lib::iso8601date_to_unix($a_row[mediamosa_job_db::STARTED]) : '';

      $a_job = array();

      $a_job['id'] = $a_row[mediamosa_job_db::ID];
      $a_job['owner'] = $a_row[mediamosa_job_db::OWNER_ID];
      $a_job['status'] = $a_row[mediamosa_job_db::JOB_STATUS];
      $a_job['progress'] = $a_row[mediamosa_job_db::PROGRESS];
      $a_job['priority'] = $a_row[mediamosa_job_db::PRIORITY];
      $a_job['job_type'] = $a_row[mediamosa_job_db::JOB_TYPE];
      $a_job['started'] = $started_unix ? date('d-m-Y H:i', $started_unix) : '';
      $a_job['started_unix'] = $started_unix;
      $a_job['error_description'] = $a_row[mediamosa_job_db::ERROR_DESCRIPTION];

      $o_mediamosa->add_item($a_job);
    }
  }
}

/**
 * URI: /mediafile/$mediafile_id/joblist
 * Method: GET
 *
 * Return list of jobs based on mediafile_id
 *
 * 1.x: vpx_jobscheduler_get_mediafile_job_list
 *
 */
class mediamosa_rest_call_job_mediafile_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const MEDIAFILE_ID = 'mediafile_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID to search on.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $user_id = $this->get_param_value(self::USER_ID);
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);

    // Must exists.
    mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Get the jobs from the user.
    $result = mediamosa_job::get_job_list($app_id, $user_id, $is_app_admin, FALSE, FALSE, $mediafile_id);

    // Build result.
    foreach ($result as $a_row) {
      $started_unix = isset($a_row[mediamosa_job_db::STARTED]) ? mediamosa_lib::iso8601date_to_unix($a_row[mediamosa_job_db::STARTED]) : '';

      $a_job = array();

      $a_job['id'] = $a_row[mediamosa_job_db::ID];
      $a_job['owner'] = $a_row[mediamosa_job_db::OWNER_ID];
      $a_job['status'] = $a_row[mediamosa_job_db::JOB_STATUS];
      $a_job['progress'] = $a_row[mediamosa_job_db::PROGRESS];
      $a_job['priority'] = $a_row[mediamosa_job_db::PRIORITY];
      $a_job['job_type'] = $a_row[mediamosa_job_db::JOB_TYPE];
      $a_job['started'] = $started_unix ? date('d-m-Y H:i', $started_unix) : '';
      $a_job['started_unix'] = $started_unix;
      $a_job['error_description'] = $a_row[mediamosa_job_db::ERROR_DESCRIPTION];

      $o_mediamosa->add_item($a_job);
    }
  }
}

/**
 * URI: /job/$job_id/status
 *      /video/transcode/$job_id/status (depricated)
 *      /mediafile/transcode/$job_id/status
 *
 * Method: GET
 *
 * Return list of jobs based on job_id
 *
 * 1.x: vpx_jobscheduler_get_job_details
 *
 */
class mediamosa_rest_call_job_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const JOB_ID = 'job_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_JOB,
          self::VAR_DESCRIPTION => 'The job ID to search on.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $user_id = $this->get_param_value(self::USER_ID);
    $job_id = $this->get_param_value(self::JOB_ID);

    // Must exists.
    mediamosa_job::must_exists($job_id);

    // Get the jobs from the user.
    $result = mediamosa_job::get_job_list($app_id, $user_id, $is_app_admin, $job_id);

    // Build result.
    foreach ($result as $a_row) {
      $started_unix = isset($a_row[mediamosa_job_db::STARTED]) ? mediamosa_lib::iso8601date_to_unix($a_row[mediamosa_job_db::STARTED]) : '';

      $a_job = array();

      $a_job['id'] = $a_row[mediamosa_job_db::ID];
      $a_job['owner'] = $a_row[mediamosa_job_db::OWNER_ID];
      $a_job['status'] = $a_row[mediamosa_job_db::JOB_STATUS];
      $a_job['progress'] = $a_row[mediamosa_job_db::PROGRESS];
      $a_job['priority'] = $a_row[mediamosa_job_db::PRIORITY];
      $a_job['job_type'] = $a_row[mediamosa_job_db::JOB_TYPE];
      $a_job['started'] = $started_unix ? date('d-m-Y H:i', $started_unix) : '';
      $a_job['started_unix'] = $started_unix;
      $a_job['error_description'] = $a_row[mediamosa_job_db::ERROR_DESCRIPTION];

      $o_mediamosa->add_item($a_job);
    }
  }
}

/**
 * URI: /job/$job_id/transcode
 *      /video/transcode/$job_id/transcode (depricated)
 *      /mediafile/transcode/$job_id/transcode
 *
 * Method: POST
 *
 * Create a transcode job.
 *
 * 1.x: vpx_jobscheduler_create_new_transcode_job
 *
 */
class mediamosa_rest_call_job_create_transcode extends mediamosa_rest_call_job_create {
  // ------------------------------------------------------------------ Consts.

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = parent::get_var_setup();

    unset($a_var_setup[self::VARS][parent::JOB_TYPE]);

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Progress Rest Args.
  /**
   * Process the supplied parameters.
   *
   * @param array $a_var_setup
   */
  protected function process_rest_args(array $a_var_setup) {

    // Set job type and give it fixed value.
    $a_var_setup[self::VARS][self::JOB_TYPE] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_JOB,
      self::VAR_DESCRIPTION => 'The job type.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_OVERRIDE_VALUE => mediamosa_job_db::JOB_TYPE_TRANSCODE,
    );

    // GET / POST to array.
    $a_var_setup = parent::process_rest_args($a_var_setup);

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    parent::do_call();
  }
}

/**
 * URI: /asset/$asset_id/still/create
 *
 * Method: POST
 *
 * Create a still job.
 *
 * 1.x: vpx_jobscheduler_create_new_still_job_2
 *
 */
class mediamosa_rest_call_job_create_still extends mediamosa_rest_call_job_create {
  // ------------------------------------------------------------------ Consts.

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = parent::get_var_setup();

    unset($a_var_setup[self::VARS][parent::JOB_TYPE]);

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Progress Rest Args.
  /**
   * Process the supplied parameters.
   *
   * @param array $a_var_setup
   */
  protected function process_rest_args(array $a_var_setup) {

    // Set job type and give it fixed value.
    $a_var_setup[self::VARS][self::JOB_TYPE] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_JOB,
      self::VAR_DESCRIPTION => 'The job type.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_OVERRIDE_VALUE => mediamosa_job_db::JOB_TYPE_STILL,
    );

    // GET / POST to array.
    $a_var_setup = parent::process_rest_args($a_var_setup);

    // Backward compatibility.
    if ($this->isset_given_param(self::STILL_TYPE) || $this->get_param_value(self::STILL_TYPE) == 'NONE') {

      $start_frame = $this->get_param_value(self::START_FRAME);
      $end_frame = $this->get_param_value(self::END_FRAME);
      $frametime = $this->get_param_value(self::FRAMETIME);
      if (!$start_frame && $frametime) {
        $this->set_param_value(self::START_FRAME, $frametime);
      }

      $size = $this->get_param_value(self::SIZE);
      $width = $this->get_param_value(self::WIDTH);
      $height = $this->get_param_value(self::HEIGHT);

      if (!$start_frame) {
        $start_frame = $this->set_param_value(self::START_TIME);
      }
      if (!$end_frame) {
        $end_frame = $this->set_param_value(self::END_TIME);
      }

      if (empty($size) && $width && $height) {
        $this->set_param_value(self::SIZE, $width . 'x' . $height);
      }
    }

    unset($a_var_setup[self::VARS][parent::FRAMETIME]);
    unset($a_var_setup[self::VARS][parent::WIDTH]);
    unset($a_var_setup[self::VARS][parent::HEIGHT]);

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    parent::do_call();
  }
}

/**
 * URI: /mediafile/$mediafile_id/still/create
 *
 * Method: POST
 *
 * Create a still job.
 *
 * This class is more a dummy for mediamosa_rest_call_job_create_still,
 * I could choose to set the URL directly on the class, but for better
 * view on code, lets keep it like own class for now. The only difference
 * is that mediafile_id comes from the URI, which is not a problem for the
 * mediamosa 2 REST interface. Doesn't matter if mediafile_id comes from
 * URI / GET / POST.
 *
 * 1.x: vpx_jobscheduler_create_new_still_job_for_mediafile
 *
 */
class mediamosa_rest_call_job_create_still_for_mediafile extends mediamosa_rest_call_job_create_still {
  // ------------------------------------------------------------------ Consts.

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = parent::get_var_setup();
    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    parent::do_call();
  }
}

/**
 * URI: /mediafile/$mediafile_id/still/create
 *
 * Method: POST
 *
 * Cancel job.
 *
 * 1.x: vpx_jobscheduler_cancel_job
 *
 */
class mediamosa_rest_call_job_cancel extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const JOB_ID = 'job_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_JOB,
          self::VAR_DESCRIPTION => 'The job ID to search on.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $user_id = $this->get_param_value(self::USER_ID);
    $job_id = $this->get_param_value(self::JOB_ID);

    // Cancel it.
    mediamosa_job::cancel_job($app_id, $job_id, $user_id);
  }
}
