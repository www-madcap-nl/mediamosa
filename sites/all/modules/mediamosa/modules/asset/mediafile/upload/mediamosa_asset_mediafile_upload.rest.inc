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
 * REST functions for upload.
 */

/**
 * URI: /mediafile/$mediafile_id/uploadticket/create
 *      /mediafile/$mediafile_id/upload_ticket (deprecated)
 *
 * Method: GET
 *
 * 1.x: vpx_upload_create_ticket
 */
class mediamosa_rest_call_asset_mediafile_uploadticket_create extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';

  private $a_metadata_definitions_full = NULL;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'ID of an original file (then it is without transcode) or empty mediafile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_db::HASH_ID_LENGTH,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User id, determined to check whether the applicant has the rights and quotas to upload the mediafile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Group id, used for any quota check..',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);

    // Create.
    $ticket_id = mediamosa_ticket::create_for_upload($app_id, $mediafile_id, $user_id, $group_id);

    // Get serverupload.
    $a_server_upload = mediamosa_server::get_random_upload();

    if (!$a_server_upload) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SERVER_UPLOAD_NOT_FOUND);
    }

    // Set response.
    $o_mediamosa->add_item(
      array(
        'action' => str_replace("{TICKET}", $ticket_id, $a_server_upload[mediamosa_server_db::URI]),
        'uploadprogress_url' => $a_server_upload[mediamosa_server_db::URI_UPLOAD_PROGRESS],
      )
    );
  }
}

/**
 * URI: /mediafile/upload
 *
 * Method: POST
 *
 * 1.x: vpx_upload_handle_file
 */
class mediamosa_rest_call_asset_mediafile_upload extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const UPLOAD_TICKET = 'upload_ticket';
  const REDIRECT_URI = 'redirect_uri';
  const TRANSCODE = 'transcode';
  const TRANSCODE_INHERITS_ACL = 'transcode_inherits_acl';
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
  const RETRANSCODE = 'retranscode';
  const FILENAME = 'filename';
  const COMPLETED_URL = 'completed_url';
  const COMPLETED_TRANSCODING_URL = 'completed_transcoding_url';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::UPLOAD_TICKET => array(
          self::VAR_TYPE => mediamosa_type::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The ID that has been given to enable the upload.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_db::HASH_ID_LENGTH,
        ),
        self::REDIRECT_URI => array(
          self::VAR_TYPE => mediamosa_type::TYPE_URL_URI,
          self::VAR_DESCRIPTION => 'The URL to redirect when upload has been successful.',
        ),
        self::TRANSCODE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'If one (or more) transcode profiles is in this parameter than the media file will be transcoded after the upload.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DEFAULT_VALUE => array(),
        ),
        self::TRANSCODE_INHERITS_ACL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => "If 'true'; All the acl media rights will be copied to the new transcoded media file.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::CREATE_STILL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Specify 'true', if you want a still is generated after the upload and analysis\n\nSee: Still creation parameters (POST) on /mediafile/{mediafile_id}/still/create [POST].",
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
        self::RETRANSCODE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => "if 'retranscode' is not put, all existing transcodes of the original media file will be lost. If 'retranscode' is 'true', then new transcode jobs will be created, which replace the existing transcodes..",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The filename of the media file (PUT method only, ignored otherwise).',
        ),
        self::COMPLETED_URL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_URL,
          self::VAR_DESCRIPTION => "This URL gives opportunity to the APP builder for a status raport; \nupload_ticket\nstatus_code\n\nThe upload_ticket is used to identify the upload. The status_code is a Mediamosa code.\nThe type is URL and not URI, because this parameter http:// or https://.",
        ),
        self::COMPLETED_TRANSCODING_URL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_URL,
          self::VAR_DESCRIPTION => "This URL is called after the status of transcoding job changed.",
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get params.
    $ticket_id = $this->get_param_value(self::UPLOAD_TICKET);
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
    $completed_url = $this->get_param_value(self::COMPLETED_URL);
    $completed_transcoding_url = $this->get_param_value(self::COMPLETED_TRANSCODING_URL);
    $transcode_inherits_acl = $this->get_param_value(self::TRANSCODE_INHERITS_ACL);
    $tag = $this->get_param_value(self::TAG);
    $a_trancode_profiles = $this->get_param_value(self::TRANSCODE);
    $retranscode = $this->get_param_value(self::RETRANSCODE);

    $filename_put = NULL;

    if ($this->is_method_put()) {
      $filename_put = $this->get_param_value(self::FILENAME); // With PUT only.
    }

    try {
      mediamosa_asset_mediafile_upload::handle_upload($ticket_id, $this->is_method_put(), $a_trancode_profiles, $retranscode, $transcode_inherits_acl, $tag, $create_still, $a_still_parameters, $completed_url, $completed_transcoding_url, $filename_put);
    }
    catch (Exception $e) {

      if ($this->isset_param(self::REDIRECT_URI)) {
        // Any errors need to be redirected.
        throw new mediamosa_exception_redirect_and_exit($this->get_param_value(self::REDIRECT_URI), $e->getCode(), $e->getMessage());
      }

      // Lets create error output then.
      throw $e;
    }

    // Redirect when needed.
    if ($this->isset_param(self::REDIRECT_URI)) {
      throw new mediamosa_exception_redirect_and_exit($this->get_param_value(self::REDIRECT_URI), mediamosa_error::ERRORCODE_OKAY);
    }

    // Set ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/upload
 *
 * Method: POST
 *
 * 1.x: vpx_upload_handle_file_put_app
 */
class mediamosa_rest_call_asset_mediafile_upload_app extends mediamosa_rest_call_asset_mediafile_upload {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {

    // Call parent.
    $a_var_setup = parent::get_var_setup();

    // Not required.
    $a_var_setup[self::VARS][self::UPLOAD_TICKET][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;

    // Add user_id.
    $a_var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      self::VAR_DESCRIPTION => 'User id, determined to check whether the applicant has the rights and quotas to upload the mediafile.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
    );

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Process Rest Args.
  protected function process_rest_args(array $a_var_setup) {

    $a_var_setup = parent::process_rest_args($a_var_setup);

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Create ticket.
    $ticket_id = mediamosa_ticket::create_for_upload($app_id, $mediafile_id, $user_id);

    return $a_var_setup;
  }
}


/**
 * URI: mediafile/uploadprogress
 *
 * Method: GET
 *
 * 1.x: vpx_upload_uploadprogress_get
 */
class mediamosa_rest_call_asset_mediafile_uploadprogress extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ID = 'id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    // Add ID of the upload.
    $a_var_setup[self::VARS][self::ID] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_ALPHA_NUM,
      self::VAR_DESCRIPTION => 'ID of the upload that has been randomly chosen and provided when the upload was send to the server.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
    );

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $id = $this->get_param_value(self::ID);

    // Add to response.
    $o_mediamosa->add_item(json_encode(mediamosa_asset_mediafile_upload::get_uploadprogress($id)));
  }
}
