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
  * The job server module.
  */


/**
 * URI: /server/joblist
 * Method: GET
 *
 * Internal only.
 *
 * 1.x: vpx_jobserver_get_jobs_status
 *
 */
class mediamosa_rest_call_job_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $a_jobs = mediamosa_job_server::search();

    foreach ($a_jobs as $a_item) {
      $o_mediamosa->add_item($a_item);
    }
  }
}

/**
 * URI: /job/$job_id/status
 * Method: POST
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

    // Job must exist.
    mediamosa_job::must_exists($job_id);

    // Get complete job.
    $a_job_ext = mediamosa_job::get_job_info($job_id);

    /**
     * If JOB is upload and we are setting the status on FINISHED, then start analyse job.
     * FIXME: progress as char is bad idea.
     */
    if ($a_job_ext[mediamosa_job_db::JOB_TYPE] == mediamosa_job_db::JOB_TYPE_UPLOAD &&
        $progress = '1.000' &&
        $a_job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FINISHED) {

      // Create an analyse job.
      mediamosa_job::create_analyse_job($a_job_ext);
    }
  }
}

/*

  // stel de query samen op basis van de opgegeven waarden
  // en wat de waarde was.
  $a_updatevalues = array();

  // zet de progress
  if ($a_parameters['progress']['value'] != 'novalue') {
    if ($jobinfo['progress'] != $a_parameters['progress']['value']) {
      $a_updatevalues[] = sprintf("progress = '%s'", $a_parameters['progress']['value']);
    }
    if (($a_parameters['progress']['value'] == '1.000') &&
        ( ($a_parameters['status']['value'] != JOBSTATUS_FINISHED) &&
          ($a_parameters['status']['value'] != JOBSTATUS_FAILED) &&
          ($jobinfo['status'] != JOBSTATUS_FINISHED) &&
          ($jobinfo['status'] != JOBSTATUS_FAILED))) {
       $a_parameters['status']['value'] = JOBSTATUS_FINISHED;
    }
  }

  // zet de started waarde
  if (($jobinfo['started'] == "") &&
      ($a_parameters['status']['value'] != JOBSTATUS_WAITING)) {
    $a_updatevalues[] = "started = NOW()";
  }

  // zet de finished waarde
  if (($jobinfo['finished'] == "") &&
      (($a_parameters['status']['value'] == JOBSTATUS_FAILED) ||
       ($a_parameters['status']['value'] == JOBSTATUS_FINISHED) )) {
    $a_updatevalues[] = "finished = NOW()";
  }

  // zet de status
  if ($jobinfo['status'] != $a_parameters['status']['value']) {
    $a_updatevalues[] = sprintf("status = '%s'", db_escape_string($a_parameters['status']['value']));
  }

  // zet de error description
  if ($jobinfo['error_description'] != $a_parameters['error_description']['value']) {
    $a_updatevalues[] = sprintf("error_description = '%s'", db_escape_string($a_parameters['error_description']['value']));
  }

  if (count($a_updatevalues)) {
    // voer de query uit op de 'data' database
    db_set_active("data");
    $status = db_result(db_query_range("SELECT status FROM {job} WHERE job_id = %d", $a_parameters['job_id']['value'], 0, 1));
    $db_result = db_query("
      UPDATE {job}
      SET ". implode(',', $a_updatevalues) ."
      WHERE job_id = %d", $a_parameters['job_id']['value']);
    db_set_active();

    vpx_jobs_notify_transcoding($status, $a_parameters['status']['value'], $a_parameters['job_id']['value']);
  }

  // Alles is goed gegaan, geef okay terug
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}
*/