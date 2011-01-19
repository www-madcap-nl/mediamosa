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
 * oEmbed rest calls.
 */


/**
 * URI: /services/oembed
 * Method: GET
 */
class mediamosa_rest_call_oembed_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const STILL_ID = 'still_id';
  const MAXWIDTH = 'maxwidth';
  const MAXHEIGHT = 'maxheight';
  const FORMAT = 'format';
  const FORMAT_XML = 'xml';
  const FORMAT_JSON = 'json';
  const PROVIDER_NAME = 'provider_name';
  const PROVIDER_URL = 'provider_url';

  // Types.
  const TYPE_VIDEO = 'video';
  const TYPE_PHOTO = 'photo';
  const TYPE_LINK = 'link';
  const TYPE_RICH = 'rich';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STILL_ID,
          self::VAR_DESCRIPTION => 'Default this specific still.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::MAXWIDTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The maximum width of the embedded resource.',
          self::VAR_RANGE_START => 0,
        ),
        self::MAXHEIGHT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The maximum height of the embedded resource.',
          self::VAR_RANGE_START => 0,
        ),
        self::FORMAT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_DESCRIPTION => 'The required response format.',
          self::VAR_ALLOWED_VALUES => array(
            self::FORMAT_XML,
            self::FORMAT_JSON,
          ),
          self::VAR_DEFAULT_VALUE => self::FORMAT_JSON,
        ),
        self::PROVIDER_NAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Name of the provider.',
        ),
        self::PROVIDER_URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'URL of the provider.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Get the 1st app.
    $app_id = reset($app_ids);

    // Get params.
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $still_id = $this->get_param_value(self::STILL_ID);
    $maxwidth = $this->get_param_value(self::MAXWIDTH);
    $maxheight = $this->get_param_value(self::MAXHEIGHT);
    $format = $this->get_param_value(self::FORMAT);
    $provider_name = $this->get_param_value(self::PROVIDER_NAME);
    $provider_url = $this->get_param_value(self::PROVIDER_URL);

    // Change the input value to the response value.
    $format = ($format == self::FORMAT_JSON ? mediamosa_rest_call::RESPONSE_TYPE_JSON_OEMBED : mediamosa_rest_call::RESPONSE_TYPE_XML_OEMBED);
    $this->set_param_value(self::FORMAT, $format);

    // Variables.
    $user_id = NULL;
    $group_id = NULL;
    $aut_domain = NULL;
    $aut_realm = NULL;
    $is_app_admin = FALSE;
    $play_call = TRUE;

    $output = array();


    if ($mediafile_id) {

      // Mediafile.

      // Variables.
      $response_type = mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_OBJECT;
      $still_id = NULL;

      // Now get the mediafile.
      $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);
      $asset_id = $mediafile[mediamosa_asset_mediafile_db::ASSET_ID];
      $asset_metadata = mediamosa_asset_metadata::metadata_get($asset_id);

      // Get the metadata.
      $mediafile['metadata'] = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id);

      // Test if access on unappropriate flag.
      mediamosa_asset::is_unappropriate($asset_id, $app_ids, $user_id, $is_app_admin, $play_call);

      // Check if allowed to play.
      mediamosa_asset_mediafile_metadata::is_playable($mediafile_id);

      // Check access.
      mediamosa_aut::access_check_on_object(
        mediamosa_aut::AUT_TYPE_MEDIAFILE,
        $asset_id,
        $mediafile_id,
        $app_id,
        $user_id,
        $group_id,
        $aut_domain,
        $aut_realm,
        $is_app_admin
      );

      $type = self::TYPE_VIDEO;
      $title = (empty($asset_metadata['dublin_core']['title']['values']) ? '' : implode(', ', $asset_metadata['dublin_core']['title']['values']));
      $author_name = $mediafile[mediamosa_asset_mediafile_db::OWNER_ID];

      // Create ticket ID.
      $ticket = mediamosa_db::uuid($app_id);

      $filename = mediamosa_configuration_storage::mediafile_filename_get($mediafile);
      if (empty($mediafile[mediamosa_asset_mediafile_db::URI]) && file_exists($filename)) {
        $ticket = mediamosa_ticket::create_play_proxy($ticket, $mediafile, $response_type, $app_id, $user_id, $still_id);

        $response = mediamosa_asset_mediafile_play_proxy::create_response(
          $response_type,
          $app_id,
          $ticket,
          $asset_id,
          TRUE,
          ($maxwidth ? $maxwidth : 0),
          ($maxheight ? $maxheight : 0),
          NULL,
          NULL,
          FALSE,
          $asset_metadata,
          $mediafile,
          array()
        );
        $url = $response['output'];

        $video_size = mediamosa_asset_mediafile_play_proxy::calc_video_size(
          isset($mediafile['metadata']['width']) ? $mediafile['metadata']['width'] : 0,
          isset($mediafile['metadata']['height']) ? $mediafile['metadata']['height'] : 0,
          $maxwidth,
          $maxheight,
          isset($mediafile['metadata']['container_type']) ? $mediafile['metadata']['container_type'] : self::CONTAINER_TYPE_WMV
        );

        $width = ($video_size['width'] ? $video_size['width'] : $maxwidth);
        $height = ($video_size['height'] ? $video_size['height'] : $maxheight);
      }

    }
    elseif ($still_id) {

      // Still.

      // Variables.
      $response_type = mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL;

      // Now get the still.
      $still = mediamosa_asset_mediafile::must_exists($still_id, $app_id);
      $mediafile_id = $still[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE];

      $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);
      $asset_id = $mediafile[mediamosa_asset_mediafile_db::ASSET_ID];
      $asset_metadata = mediamosa_asset_metadata::metadata_get($asset_id);

      // Test if access on unappropriate flag.
      mediamosa_asset::is_unappropriate($asset_id, $app_ids, $user_id, $is_app_admin, $play_call);

      // Check access.
      mediamosa_aut::access_check_on_object(
        mediamosa_aut::AUT_TYPE_MEDIAFILE,
        $asset_id,
        $mediafile_id,
        $app_id,
        $user_id,
        $group_id,
        $aut_domain,
        $aut_realm,
        $is_app_admin
      );

      $type = self::TYPE_PHOTO;
      $title = (empty($asset_metadata['dublin_core']['title']['values']) ? '' : implode(', ', $asset_metadata['dublin_core']['title']['values']));
      $author_name = $mediafile[mediamosa_asset_mediafile_db::OWNER_ID];

      $ticket = mediamosa_db::uuid($app_id);

      // Generate ticket or perm link?
      $ticket = mediamosa_asset_mediafile_still::is_still_permanent($still_id, $is_app_admin) ? $still_id : mediamosa_ticket::TICKET_PATH . mediamosa_ticket::create_play_proxy($ticket, $mediafile, $response_type, $app_id, $user_id, $still_id);

      // Create the response.
      $response = mediamosa_asset_mediafile_play_proxy::create_response(
        $response_type,
        $app_id,
        $ticket,
        $asset_id,
        TRUE,
        ($maxwidth ? $maxwidth : 0),
        ($maxheight ? $maxheight : 0),
        NULL,
        NULL,
        FALSE,
        $asset_metadata,
        $mediafile,
        array()
      );
      $url = $response['output'];

      $width = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($still_id, 'width');
      $height = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($still_id, 'height');

    }


    // Collect the data for output.

    $output += array(
      'type' => $type,
      'version' => '1.0',
      'title' => $title,
      'author_name' => $author_name,
      //'author_url' => NULL,
      //'cache_age' => NULL,
      'provider_name' => ($provider_name ? $provider_name : 'MediaMosa'),
      'provider_url' => ($provider_url ? $provider_url : 'http://mediamosa.org/'),
    );

    if (!empty($thumbnail_url)) {
      $output['thumbnail_url'] = $thumbnail_url;
      $output['thumbnail_width'] = $thumbnail_width;
      $output['thumbnail_height'] = $thumbnail_height;
    }

    if (!empty($html)) {
      $output['html'] = $html;
    }

    if (!empty($url)) {
      $output['url'] = $url;
    }

    if (!empty($width)) {
      $output['width'] = $width;
    }

    if (!empty($height)) {
      $output['height'] = $height;
    }

    $mediamosa->add_item($output);
  }
}
