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

    // Mediafile must exist.
    $a_mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Must be owner.
    mediamosa_aut::owner_check($app_id, $user_id, $a_mediafile[mediamosa_asset_mediafile_db::APP_ID], $a_mediafile[mediamosa_asset_mediafile_db::OWNER_ID]);

    // Check quota.
    mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);

    // Create upload ticket.
    $ticket_id = mediamosa_ticket::create($app_id, mediamosa_ticket::TICKET_TYPE_UPLOAD, $user_id, $mediafile_id, NULL, $group_id);

    // Get serverupload.
    $a_server_upload = mediamosa_server_upload::get_random();

    // Set response.
    $o_mediamosa->add_item(
      array(
        'action' => str_replace("{TICKET}", $ticket_id, $a_server_upload[mediamosa_server_upload_db::URI]),
        'uploadprogress_url' => $a_server_upload[mediamosa_server_upload_db::URI_UPLOADPROGRESS],
      )
    );
  }
}




