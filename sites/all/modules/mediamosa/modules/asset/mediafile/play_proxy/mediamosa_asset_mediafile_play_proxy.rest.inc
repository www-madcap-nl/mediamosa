<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * Play Proxy functions
  */



/**
 * URI: /asset/$asset_id/play
 *      /video/$asset_id/play (depricated)
 * Method: GET
 *
 * 1.x: play_proxy_request.
 *
 */
class mediamosa_rest_call_asset_mediafile_play extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const ORIGINAL_MEDIAFILE_ID = 'original_mediafile_id';
  const STILL_ID = 'still_id';
  const RESPONSE = 'response';
  const DOMAIN = 'domain';
  const REALM = 'realm';
  const AUT_DOMAIN = 'aut_domain';
  const AUT_REALM = 'aut_realm';
  const PROFILE_ID = 'profile_id';
  const WIDTH = 'width';
  const HEIGHT = 'height';
  const START = 'start';
  const DURATION = 'duration';
  const AUTOSTART = 'autostart';
  const SIZE = 'size';
  const FORMAT = 'format';
  const RANGE = 'range';
  const TAG = 'tag';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID, is not required when profile_id is used or when response type is still.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,// Is required in some cases.
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Group ID of the person who wants to play the video, can be used for authorization.',
        ),
        self::ORIGINAL_MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The original mediafile ID.',
        ),
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STILL_ID,
          self::VAR_DESCRIPTION => 'The still ID.',
        ),
        self::RESPONSE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The response type.',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_DOWNLOAD,
            mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_METAFILE,
            mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_OBJECT,
            mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL,
            mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_URI
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_URI
        ),
        self::AUT_DOMAIN => array(
          self::VAR_TYPE => mediamosa_type::TYPE_DOMAIN,
          self::VAR_DESCRIPTION => 'The domain for which the application is made. If specified it is used for authorization. The domain is checked against the acl rules of the media files, if the domain appears in the / acl / domain list, the video is played.',
        ),
        self::AUT_REALM => array(
          self::VAR_TYPE => mediamosa_type::TYPE_REALM,
          self::VAR_DESCRIPTION => 'If specified it is used for authorization.',
        ),
        self::DOMAIN => array(
          self::VAR_TYPE => mediamosa_type::TYPE_DOMAIN,
          self::VAR_DESCRIPTION => 'The domain for which the application is made. If specified it is used for authorization. The domain is checked against the acl rules of the media files, if the domain appears in the / acl / domain list, the video is played.',
          self::VAR_IS_HIDDEN => self::VAR_IS_HIDDEN_YES,
        ),
        self::REALM => array(
          self::VAR_TYPE => mediamosa_type::TYPE_REALM,
          self::VAR_DESCRIPTION => 'If specified it is used for authorization.',
          self::VAR_IS_HIDDEN => self::VAR_IS_HIDDEN_YES,
        ),
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => "Play the mediafile which is transcode with {profile_id}.\n\n(new since 1.7.0) If {original_mediafile_id} is also given, plays the transcoded file with profile_id= {profile_id} which was transcoded from source file mediafile_id = {original_mediafile_id}",
        ),
        self::WIDTH => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'The width of the video.',
        ),
        self::HEIGHT => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'The height of the video.',
        ),
        self::START => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Start time of the video clip in milliseconds. Must be less than 24 hours (86,400,000)',
          self::VAR_RANGE_START => 0,
          self::VAR_RANGE_END => 86399999, // (24 uur in msec)-1 msec
        ),
        self::DURATION => array(
          self::VAR_TYPE => mediamosa_type::TYPE_INT,
          self::VAR_DESCRIPTION => 'Playing length of the video clip in milliseconds. Must be less than 24 hours (86,400,000).',
          self::VAR_RANGE_START => 0,
          self::VAR_RANGE_END => 86399999, // (24 uur in msec)-1 msec
        ),
        self::AUTOSTART => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Adjust the value of the autostart in object code.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::SIZE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'If response type = still, then size parameter may filter the response (eg. 150x120).',
        ),
        self::FORMAT => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => "If response type = still, then it is possble to filter the response to format (eg. 'jpeg').",
        ),
        self::RANGE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => "If response type = still, then it is possible to filter the response to range (to the still order value) (eg. '3', or '3,5,6', or '3, 5-8').",
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => "Tag to indentify and search for the mediafile to play.",
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // Override for some cases.
  protected function process_rest_args(array $a_var_setup) {
    $response_type = $this->get_param_value_global(self::RESPONSE);
    $profile_id = $this->get_param_value_global(self::PROFILE_ID);

    // Is reponse is set or response type is still then mediafile_id is not required.
    if (isset($profile_id) || $response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL) {
      $a_var_setup[self::VARS][self::MEDIAFILE_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;
    }

    return parent::process_rest_args($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Fix renames.
    if ($this->isset_given_param(self::DOMAIN)) {
      $this->set_param_value(self::AUT_DOMAIN, $this->get_param_value(self::DOMAIN));
    }
    if ($this->isset_given_param(self::REALM)) {
      $this->set_param_value(self::AUT_REALM, $this->get_param_value(self::REALM));
    }
    // FIXME: hack
    if ($this->is_internal()) {
      $this->set_param_value(self::RESPONSE, mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL);
    }

    $user_id = $this->get_param_value(self::USER_ID);
    $response_type = $this->get_param_value(self::RESPONSE);
    $profile_id = $this->get_param_value(self::PROFILE_ID);
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $original_mediafile_id = $this->get_param_value(self::ORIGINAL_MEDIAFILE_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $aut_domain = $this->get_param_value(self::AUT_DOMAIN);
    $aut_realm = $this->get_param_value(self::AUT_REALM);
    $still_id = $this->get_param_value(self::STILL_ID);

    if ($this->isset_given_param(self::PROFILE_ID)) {
      $mediafile_id = mediamosa_asset_mediafile_play_proxy::get_mediafile_id_on_profile($asset_id, $profile_id, $original_mediafile_id);

      if (!$mediafile_id) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_MEDIAFILE_FOUND_FOR_PROFILE_ID);
      }
    }

    // Still response does not need mediafile_id.
    if ($response_type != mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL && !$mediafile_id) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array('@param' => self::MEDIAFILE_ID, '@type' => mediamosa_type::TYPE_MEDIAFILE_ID));
    }

    $a_asset = array();
    $a_mediafile_ext = array();

    if ($mediafile_id) {
      // Get the asset.
      $a_asset = mediamosa_asset::must_exists($asset_id);

      // Check play restrictions
      if (isset($a_asset['play_restriction_start']) && isset($a_asset['play_restriction_end'])) {
        $result = mediamosa_asset_mediafile_play_proxy::check_time_restrictions(
          strtotime($a_asset['play_restriction_start']),
          strtotime($a_asset['play_restriction_end'])
        );
      }

      // Get the mediafile.
      $a_mediafile_ext = mediamosa_asset_mediafile::must_exists($mediafile_id);

      // Make sure asset_id matches.
      if ($a_mediafile_ext[mediamosa_asset_mediafile_db::ASSET_ID] != $asset_id) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_ASSET_MEDIAFILE_COMBINATION);
      }

      // Get the metadata.
      $a_mediafile_ext['metadata'] = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id);

      // Test if access on unappropriate flag.
      mediamosa_asset::is_unappropriate($asset_id, $a_app_ids, $user_id, $is_app_admin, TRUE);

      // Log stream event.
      mediamosa_statistics::log_event_requested_stream($mediafile_id, $response_type);

      // Check if allowed to play.
      mediamosa_asset_mediafile_metadata::is_playable($mediafile_id);

      // Check access.
      mediamosa_aut::access_check_on_object(
        mediamosa_aut::AUT_TYPE_MEDIAFILE,
        $asset_id,
        $mediafile_id,
        $app_id,
        $user_id,
        is_array($group_id) ? $group_id : array($group_id),
        $aut_domain,
        $aut_realm,
        $is_app_admin
      );
    }
    else {
      $a_mediafile_ext['asset_id'] = $asset_id;
    }

    // Create ticket ID.
    $ticket = mediamosa_db::uuid($app_id);

    // Create ticket.
    if (!isset($a_mediafile_ext[mediamosa_asset_mediafile_db::URI])) {
      $ticket = mediamosa_ticket::create_play_proxy($ticket, $a_mediafile_ext, $response_type, $app_id, $user_id, $still_id);
    }

    // If response type is still, we get all information of all stills with all details.
    $a_stills = array();
    if ($response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL) {
      $width = $this->get_param_value(self::WIDTH);
      $height = $this->get_param_value(self::HEIGHT);
      $size = $this->get_param_value(self::SIZE);
      $format = $this->get_param_value(self::FORMAT);
      $range = $this->get_param_value(self::RANGE);
      $tag = $this->get_param_value(self::TAG);

      if (!empty($size)) {
        list($size_width, $size_height) = explode('x', $size, 2);
      }

      if (!isset($width) && !isset($height) && isset($size_width) && $size_width >= 0 && is_numeric($size_width) && isset($size_height) && is_numeric($size_height) && $size_height >= 0) {
        $width = $size_width;
        $height = $size_height;
      }

      $a_orders = array();
      if (isset($range)) {
        if (is_numeric($range)) {
          $a_orders[] = $range;
        }
        else {
          $a_range = explode(',', $range);
          foreach($a_range as $range_2) {
            if (is_numeric($range_2)) {
              $a_orders[] = $range_2;
            }
            else {
              $pos = strpos($range_2, '-', 1);
              if ($pos !== FALSE) {
                // Think to the negative numbers, so change the separator
                $range_2[$pos] = '!';
                $range_2 = explode('!', $range_2);
                if (is_array($range_2) && isset($range_2[0]) && is_numeric($range_2[0]) && isset($range_2[1]) && is_numeric($range_2[1]) && !isset($range_2[2]) && $range_2[0] <= $range_2[1]) {
                  for ($i = $range_2[0]; $i <= $range_2[1]; $i++) {
                    $a_orders[] = $i;
                  }
                }
              }
            }
          }
        }
      }

      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'm');
      $query->join(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mm', strtr('mm.#mediafile_id = m.#mediafile_id', array('#mediafile_id' => mediamosa_asset_mediafile_db::ID)));
      $query->join(
        mediamosa_asset_mediafile_db::TABLE_NAME,
        'v',
        strtr(
          "v.#mediafile_id = m.#mediafile_id_source AND v.#is_still = '#is_still_false'",
          array(
            '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
            '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
            '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
            '#is_still_false' => mediamosa_asset_mediafile_db::IS_STILL_FALSE,
          )
        )
      );
      $query->addField('m', mediamosa_asset_mediafile_db::ID, 'still_id');
      $query->addField('v', mediamosa_asset_mediafile_db::ID, 'mediafile_id');
      $query->addField('m', mediamosa_asset_mediafile_db::ASSET_ID_ROOT, 'asset_id');
      $query->addField('m', mediamosa_asset_mediafile_db::ASSET_ID, 'orderasset_id');
      $query->fields('m',
        array(
          mediamosa_asset_mediafile_db::APP_ID,
          mediamosa_asset_mediafile_db::OWNER_ID,
          mediamosa_asset_mediafile_db::FILENAME,
          mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          mediamosa_asset_mediafile_db::TAG,
        )
      );
      $query->fields('mm',
        array(
          mediamosa_asset_mediafile_metadata_db::WIDTH,
          mediamosa_asset_mediafile_metadata_db::HEIGHT,
          mediamosa_asset_mediafile_metadata_db::FILESIZE,
          mediamosa_asset_mediafile_metadata_db::MIME_TYPE,
          mediamosa_asset_mediafile_metadata_db::STILL_TIME_CODE,
          mediamosa_asset_mediafile_metadata_db::STILL_ORDER,
          mediamosa_asset_mediafile_metadata_db::STILL_FORMAT,
          mediamosa_asset_mediafile_metadata_db::STILL_TYPE,
          mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT
        )
      );
      $query->condition('m.' . mediamosa_asset_mediafile_db::ASSET_ID_ROOT, $asset_id);
      $query->condition('m.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE);
      $query->condition('m.' . mediamosa_asset_mediafile_db::APP_ID, $app_id);
      if (isset($still_id)) {
        $query->condition('m.' . mediamosa_asset_mediafile_db::ID, $still_id);
      }
      if (isset($mediafile_id)) {
        $query->condition('v.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
      }
      if (isset($width)) {
        $query->condition('mm.' . mediamosa_asset_mediafile_metadata_db::WIDTH, $width);
      }
      if (isset($height)) {
        $query->condition('mm.' . mediamosa_asset_mediafile_metadata_db::HEIGHT, $height);
      }
      if (isset($format)) {
        $query->condition('mm.' . mediamosa_asset_mediafile_metadata_db::STILL_FORMAT, $format);
      }
      if (count($a_orders)) {
        $query->condition('mm.' . mediamosa_asset_mediafile_metadata_db::STILL_ORDER, $a_orders, 'IN');
      }
      if (isset($tag)) {
        $query->condition('m.' . mediamosa_asset_mediafile_db::TAG, $tag);
      }
      $query->orderBy('m.asset_id', 'ASC');
      $query->orderBy('mm.still_order', 'ASC');

      if (isset($still_id)) {
        $result = $query->range(0, 1)->execute()->fetchAssoc();
        $result['ticket'] = $ticket;
        $a_stills[] = $result;
      }
      else {
        $result = $query->execute();

        foreach ($result as $t_stills) {
          $ticket_still = mediamosa_db::uuid($app_id);

          // make a play or download symlink.
          if (!isset($a_mediafile_ext[mediamosa_asset_mediafile_db::URI])) {
            $ticket_still = mediamosa_ticket::create_play_proxy($ticket_still, $a_mediafile_ext, $response_type, $app_id, $user_id, $t_stills['still_id']);
          }

          $t_stills['ticket'] = $ticket_still;
          $a_stills[] = $t_stills;
        }
      }
    }

    // Collect the metadata of the asset.
    $a_asset_metadata = mediamosa_asset_metadata::metadata_get($asset_id);

    // Create the response.
    $response = mediamosa_asset_mediafile_play_proxy::create_response(
      $response_type,
      $app_id,
      $ticket,
      $this->get_param_value(self::AUTOSTART),
      $this->get_param_value(self::WIDTH),
      $this->get_param_value(self::HEIGHT),
      $this->get_param_value(self::START),
      $this->get_param_value(self::DURATION),
      $this->get_param_value(self::IS_APP_ADMIN),
      $a_asset_metadata,
      $a_mediafile_ext,
      $group_id,
      $a_stills
    );

    // All ok, now set played + 1
    mediamosa_asset::asset_played($asset_id);

    // Add response.
    $o_mediamosa->add_item($response);

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}
