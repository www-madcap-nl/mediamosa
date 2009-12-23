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
    mediamosa_job::update_status($a_job_ext, $progress, $status, $error_description);

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
    $o_mediamosa = mediamosa::get();

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

    $job_id = $this->get_param_value(self::JOB_ID);
    $uploaded_file_size = $this->get_param_value(self::UPLOADED_FILE_SIZE);

    // Must exist.
    mediamosa_job::must_exists($job_id);

    // Get complete job.
    $a_job_ext = mediamosa_job::get_job_ext($job_id);

    if (!$uploaded_file_size  && $a_job_ext[mediamosa_job_db::JOB_TYPE] == mediamosa_job_db::JOB_TYPE_UPLOAD) {
      // If the filesize is 0, then we are done...
      if (!$a_job_ext[mediamosa_job_upload_db::FILE_SIZE]) {

        // Update the job.
        mediamosa_job::update_status($a_job_ext, '1.000', mediamosa_job_db::JOB_STATUS_INPROGRESS);

        $o_mediamosa->set_result_okay();
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
    $progress = mediamosa_db_query(
      'SELECT uploaded_file_size / file_size FROM {#mediamosa_job_upload} WHERE #job_id = :job_id',
      array(
        '#mediamosa_job_upload' => mediamosa_job_upload_db::TABLE_NAME,
        '#job_id' => mediamosa_job_upload_db::JOB_ID,
        ':job_id' => $job_id,
      )
    )->execute()->fetchField();

    // 1.xxx
    $progress = number_format(floatval($progress), 3, '.', '');

    // FIXME: all rest calls with vpx_jobs_set_job_status are broken, still params not used~!

    // Update the job.
    mediamosa_job::update_status($a_job_ext, $progress, mediamosa_job_db::JOB_STATUS_INPROGRESS);

    $o_mediamosa->set_result_okay();
    return; // done
  }
}
