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
  * The REST calls of the mediamosa_asset_mediafile module.
  */

/**
 * URI: /mediafile/count
 * Method: GET
 */
class mediamosa_rest_call_count_asset_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const OWNER_ID = 'owner_id';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::DESCRIPTION => 'The group ID to match.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::DESCRIPTION => 'The owner ID to match.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    $group_id = $this->get_param_value(self::GROUP_ID);
    $owner_id = $this->get_param_value(self::OWNER_ID);

    $o_mediamosa->set_result_okay();

    // Set the total count.
    $o_mediamosa->item_total_count = mediamosa_db::db_count_items(mediamosa_asset_mediafile_db::TABLE_NAME, $app_id, $owner_id, $group_id);
  }
}

// @todo: UNFINISHED CODE BELOW

/**
 * URI: /mediafile/$mediafile_id (1.x media_management_get_mediafile)
 * Method: GET
 */
class mediamosa_rest_call_get_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const SHOW_STILLS = 'show_stills';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::DESCRIPTION => 'The ID of the mediafile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::DESCRIPTION => 'Include the stills in the output.',
          self::VAR_DEFAULT_VALUE => 'TRUE'
        )
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $show_stills = $this->get_param_value(self::SHOW_STILLS);

    // Must exists.
    mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));

    // Must have access.
    mediamosa_aut::read_single_object(mediamosa_aut::AUT_TYPE_MEDIAFILE, $mediafile_id, $app_id);

    // Now enrich response with mediafile data.
    mediamosa_asset_mediafile::enrich_response_mediafile($o_mediamosa, array($mediafile_id), $a_app_ids, FALSE, $show_stills);

    // Done.
  }
}
