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
  * Play Proxy functions
  */

class mediamosa_asset_mediafile_play_proxy {
  // ------------------------------------------------------------------ Consts.
  /*
   * Response types
   */
  const RESPONSE_TYPE_URI = 'plain';
  const RESPONSE_TYPE_STILL = 'still';
  const RESPONSE_TYPE_METAFILE = 'metafile';
  const RESPONSE_TYPE_OBJECT = 'object';
  const RESPONSE_TYPE_DOWNLOAD = 'download';

  // Params in URI replacements.
  const PARAM_URI_TICKET = '{TICKET}';
  const PARAM_URI_SCRIPT = '{SCRIPT}';

  // Default width & height for playproxy flash object HTML.
  CONST PP_FLASH_OBJECT_HEIGHT = 240;
  CONST PP_FLASH_OBJECT_WIDTH = 320;

  /*
   *  Container types
   */
  const CONTAINER_TYPE_WMV = 'asf';
  const CONTAINER_TYPE_MP3 = 'mp3';
  const CONTAINER_TYPE_MP4 = 'mov;mp4;m4a;3gp;3g2;mj2';
  const CONTAINER_TYPE_FLV = 'flv';
  const CONTAINER_TYPE_OGG = 'ogg';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Get the uri of the correct media server.
   *
   * @param string $response_type
   * @param unknown_type $a_mediafile_info
   * @return unknown
   */
  static public function get_media_server($response_type, $a_mediafile_ext = array()) {

    switch ($response_type) {
      case self::RESPONSE_TYPE_STILL:
        return mediamosa_db::db_query(
          "SELECT #uri FROM {#mediamosa_server_still} WHERE #active = 1 ORDER BY rand() LIMIT 1",
          array(
            '#uri' => mediamosa_server_still_db::URI,
            '#mediamosa_server_still' => mediamosa_server_still_db::TABLE_NAME,
            '#active' => mediamosa_server_still_db::ACTIVE,
          )
        )->fetchField();

      case RESPONSE_TYPE_DOWNLOAD:
        return mediamosa_db::db_query(
          "SELECT #uri FROM {#mediamosa_download_still} WHERE #active = 1 ORDER BY rand() LIMIT 1",
          array(
            '#uri' => mediamosa_server_download_db::URI,
            '#mediamosa_download_still' => mediamosa_server_download_db::TABLE_NAME,
            '#active' => mediamosa_server_download_db::ACTIVE,
          )
        )->fetchField();
    }

    // Needed at this point.
    assert(!empty($a_mediafile_ext));

    // Get the URI.
    $uri = mediamosa_server_streaming::get_uri($a_mediafile_ext['metadata']['container_type'], $a_mediafile_ext['metadata']['video_codec']);

    if (!$uri) { // No streaming profile found
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STREAMING_PROFILE_NOT_FOUND, array('@video_container' => $a_mediafile_ext['metadata']['container_type']));
    }

    return $uri;
  }

  /**
   * This function converts miliseconds to the format specified
   *
   * @param integer $msec
   * @param string $container_type
   *  See CONTAINER_TYPE_*
   *
   * @return
   *  requested time format.
   */
  function convert_msec($input, $container_type) {
    $output = FALSE;

    // bereken uur, minuut en seconde
    $second = 1000;
    $minute = $second * 60;
    $hour = $minute * 60;
    $a_output['hours'] = floor($input / $hour);
    $input -= $a_output['hours'] * $hour;
    $a_output['minutes'] = floor($input / $minute);
    $input -= $a_output['minutes'] * $minute;
    $a_output['seconds'] = floor($input / $second);
    $input -= $a_output['seconds'] * $second;
    $msec = $input;

    switch ($container_type) {
      case self::CONTAINER_TYPE_WMV: // hh:mm:ss.fract(=sec/100)
        $fraction = substr($msec, 0, 2);
        foreach ($a_output as $subject => $value) {
          if ($a_output[$subject] < 10) {
            $a_output[$subject] = '0' . $a_output[$subject];
          }
        }
        $output = implode(':', $a_output) . '.' . $fraction;
        break;

      case self::CONTAINER_TYPE_MP4: // hh:mm:ss:frames(=sec/30).
      default:
        $frame = 1 / 30;
        $fraction = round($msec / $frame / 1000, 1);
        if ($fraction == 30) {
          $fraction = 0;
          $a_output['seconds']++;
        }

        foreach ($a_output as $subject => $value) {
          if ($a_output[$subject] < 10) {
            $a_output[$subject] = "0". $a_output[$subject];
          }
        }
        $output = implode(':', $a_output) . ':' . $fraction;
        break;
    }

    return $output;
  }

  /**
   * Calculate the size of the video based on given size and type.
   * See Mantis [0016569] for specification.
   *
   * @param integer v_width
   *  mediafile width
   * @param integer v_height
   *  mediafile height
   * @param integer width
   *  play proxy call parameter width
   * @param integer height
   *  play proxy call parameter height
   *
   * @return array
   *  new width en height.
   *
   */
  function calc_video_size($v_width, $v_height, $width, $height, $container_type) {

    // If video size is known, we can do some calculations on height/weight.
    if ($v_width > 0 && $v_height > 0) {
      if ($width > 0 && ($height == 0 || !isset($height))) {
        $height = (int)round($width * ($v_height / $v_width));
      }
      elseif (($width == 0 || !isset($width)) && $height > 0) {
        $width = (int)round($height * ($v_width / $v_height));
      }
      elseif (($width == 0 || !isset($width)) && ($height == 0 || !isset($height))) {
        // if none is given we use the default size.
        $width = $v_width;
        $height = $v_height;
      }
    }

    if (!($width > 0 && $height > 0)) {
      // no luck, use default values.
      $width = self::PP_FLASH_OBJECT_WIDTH;
      $height = self::PP_FLASH_OBJECT_HEIGHT;
    }

    // add an extra 64 pixels for windows media buttons.
    switch ($container_type) {
      case self::CONTAINER_TYPE_FLV:
      case self::CONTAINER_TYPE_MP4:
      case self::CONTAINER_TYPE_OGG:
        break;

      case self::CONTAINER_TYPE_WMV:
      default:
        if ($height) {
          $height += 64;
        }
        break;
    }

    return array('width' => $width, 'height' => $height);
  }

  /**
   * Wrapper for STILL response type.
   *
   * @param string $ticket
   * @param integer $app_id
   */
  static public function create_response_still($ticket, $app_id) {
    return self::create_response(self::RESPONSE_TYPE_STILL, $ticket, $app_id);
  }

  /**
   * Wrapper for 1.x to 2.x.
   * @param array $a_parameters
   * @param array $a_asset_info
   * @param array $a_mediafile_ext
   * @param array $a_asset_metadata
   * @param string $ticket
   * @param integer $app_id
   * @param string $mediafile_group_id
   * @param array $a_stills
   */
  static public function create_response_1x_to_2x($a_parameters, $a_asset_info, $a_mediafile_ext, $a_asset_metadata, $ticket, $app_id, $mediafile_group_id = '', $a_stills = array()) {
    return self::create_response(
      $a_parameters['response']['value'],
      $app_id,
      $ticket,
      isset($a_parameters['autostart']['value']) ? mediamosa_lib::boolstr2bool($a_parameters['autostart']['value']) : TRUE,
      isset($a_parameters['width']['value']) ? $a_parameters['width']['value'] : 0,
      isset($a_parameters['height']['value']) ? $a_parameters['height']['value'] : 0,
      isset($a_parameters['start']['value']) ? $a_parameters['start']['value'] : NULL,
      isset($a_parameters['duration']['value']) ? $a_parameters['duration']['value'] : NULL,
      isset($a_parameters['is_app_admin']['value']) ? mediamosa_lib::boolstr2bool($a_parameters['is_app_admin']['value']) : FALSE,
      $a_asset_info,
      $a_mediafile_ext,
      $a_asset_metadata,
      $mediafile_group_id,
      $a_stills
    );
  }

  /**
   * Create response for mediafile.
   * 1.x params; $a_parameters, $a_asset_info, $a_mediafile_ext, $a_asset_metadata, $ticket, $app_id, $mediafile_group_id = '', $a_stills = array()) {
   *
   * @param string $response_type
   *  See RESPONSE_TYPE_*.
   * @param integer $app_id
   *  Application ID.
   * @param string $ticket
   *  Precreated ticket.
   * @param integer $autostart
   *  Value for autostart in HTML.
   * @param $width
   *  Value for width in HTML object.
   * @param $height
   *  Value for height in HTML object.
   * @param $start
   *  Start param in HTML object.
   * @param $duration
   *  Duration param in HTML object.
   * @param boolean $is_app_admin
   *  Super admin.
   * @param array $a_asset_info
   *  Asset array.
   * @param array $a_mediafile_ext
   *  Mediafile array.
   * @param array $a_asset_metadata
   *  Asset metadata.
   * @param string $mediafile_group_id
   *  Group ID mediafile.
   * @param $a_stills
   *  Array stills.
   */
  static public function create_response(
    $response_type,
    $app_id,
    $ticket,
    $autostart = TRUE,
    $width = 0,
    $height = 0,
    $start = NULL,
    $duration = NULL,
    $is_app_admin = FALSE,
    $a_asset_info = array(),
    $a_mediafile_ext = array(),
    $a_asset_metadata = array(),
    $mediafile_group_id = '',
    $a_stills = array()
  ) {

  	// Not an array here.
  	assert(!is_array($app_id));

    // Expecting array here, even when empty.
    assert(is_array($a_stills));

    $content_type = "";
    if (isset($a_mediafile_ext['uri'])) {
      $streaming_server_uri = $a_mediafile_ext['uri'];

      if (trim($streaming_server_uri) == '') { // No streaming profile found
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STREAMING_PROFILE_NOT_FOUND, array('@video_container' => $a_mediafile_ext['metadata']['container_type']));
      }
    }
    else {
      $streaming_server_uri = self::get_media_server(self::RESPONSE_TYPE_STILL);
    }

    // Replace {TICKET} with app_id / ticket.
    if ($streaming_server_uri != self::RESPONSE_TYPE_STILL) {
      $streaming_server_ticket_uri = str_replace(self::PARAM_URI_TICKET, $app_id . '/' . $ticket, $streaming_server_uri);
    }

    // For some reason the old code thinks this can be non-array or NULL, lets log this and solve it.
    assert(is_array($a_stills));

    // Uri voor niet-tickets (still, download etc).
    foreach ($a_stills as $key => $value) {
      $a_stills[$key]['still_ticket'] = str_replace(self::PARAM_URI_TICKET, urlencode($a_stills[$key]['ticket']), $streaming_server_uri);
    }

    // Replace ticker param in URI.
    $streaming_server_uri = str_replace(self::PARAM_URI_TICKET, urlencode($ticket), $streaming_server_uri);

    $a_video_size = self::calc_video_size(
      isset($a_mediafile_ext['metadata']['width']) ? $a_mediafile_ext['metadata']['width'] : 0,
      isset($a_mediafile_ext['metadata']['height']) ? $a_mediafile_ext['metadata']['height'] : 0,
      $width,
      $height,
      isset($a_mediafile_ext['metadata']['container_type']) ? $a_mediafile_ext['metadata']['container_type'] : self::CONTAINER_TYPE_WMV
    );

    // So it exists.
    $a_variables = array();

    switch ($response_type) {
      case self::RESPONSE_TYPE_METAFILE:

        switch ($a_mediafile_ext['metadata']['container_type']) {
          case self::CONTAINER_TYPE_MP4:
            $output = '<?xml version="1.0"?>' . "\n";
            $output .= '<?quicktime type="application/x-quicktime-media-link"?>' . "\n";
            $output .= sprintf("<embed autoplay=\"%s\" src=\"%s\" />\n", $autostart ? 'true' : 'false', $streaming_server_ticket_uri);
            $content_type = 'application/x-quicktime-media-link';
            break;

          case self::CONTAINER_TYPE_MP3:
            $content_type = "audio/mpeg";
            $output = "<asx version=\"3.0\">\n\t<entry>\n";
            $a_subjects = array('TITLE' => 'title', 'COPYRIGHT' => 'rights', 'ABSTRACT' => 'description', 'AUTHOR' => 'creator');
            if (isset($a_asset_info['dublin_core'])) {
              foreach ($a_subjects as $tag => $subject) {
                if (is_array($a_asset_info['dublin_core'][$subject]) && count($a_asset_info['dublin_core'][$subject])) {
                  $output .= "\t\t<" . $tag .'>'. implode(' ', $a_asset_info['dublin_core'][$subject]) .'</'. $tag. ">\n";
                }
              }
            }

            $output .= sprintf("\t\t<ref href=\"%s\" />\n", $streaming_server_ticket_uri);
            $output .= "\t</entry>\n</asx>\n";

            break;

          case self::CONTAINER_TYPE_WMV:
            $content_type = "video/x-ms-asf";
            $output = "<asx version=\"3.0\">\n\t<entry>\n";
            $a_subjects = array('TITLE' => 'title', 'COPYRIGHT' => 'rights', 'ABSTRACT' => 'description', 'AUTHOR' => 'creator');
            if (isset($a_asset_info['dublin_core'])) {
              foreach ($a_subjects as $tag => $subject) {
                if (is_array($a_asset_info['dublin_core'][$subject]) && count($a_asset_info['dublin_core'][$subject])) {
                  $output .= "\t\t<" . $tag .'>'. implode(' ', $a_asset_info['dublin_core'][$subject]) .'</'. $tag. ">\n";
                }
              }
            }

            // Add start and duration to asx.
            if (isset($start)) {
              $output .= sprintf("\t\t<starttime value=\"%s\" />\n", self::convert_msec($start, self::CONTAINER_TYPE_WMV));
            }

            if (isset($duration)) {
              $output .= sprintf("\t\t<duration value=\"%s\" />\n", self::convert_msec($duration, self::CONTAINER_TYPE_WMV));
            }

            $output .= sprintf("\t\t<ref href=\"%s\" />\n", $streaming_server_ticket_uri);
            $output .= "\t</entry>\n</asx>\n";
            break;

          default:
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_METAFILE_AVAILABLE, array('@container_type' => $a_mediafile_ext['metadata']['container_type']));
        }
        break;

      case RESPONSE_TYPE_URI:
        switch ($a_mediafile_ext['metadata']['container_type']) {
          case self::CONTAINER_TYPE_FLV:
            $output = str_replace(self::PARAM_URI_SCRIPT, 'StroboScope.php?file=', $streaming_server_ticket_uri);
            break;

          default:
            $output = $streaming_server_ticket_uri;
            break;
        }
        break;

      case RESPONSE_TYPE_DOWNLOAD:
        if (!$is_app_admin) {
          if (isset($a_mediafile_ext['is_downloadable']) && !mediamosa_lib::boolstr2bool($a_mediafile_ext['is_downloadable'])) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_DOWNLOAD_DISABLED);
          }
        }

        if (!@is_null($a_mediafile_ext['uri'])) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STREAM_DOWNLOAD_NOT_ALLOWED);
        }

        if ($a_mediafile_ext['filename'] == '') {
          $a_mediafile_ext['filename'] = mediamosa_asset_mediafile::UNKNOWN_FILENAME;
        }

        $output = $streaming_server_uri . '/' . $a_mediafile_ext['filename'];
        break;

      case RESPONSE_TYPE_OBJECT:

        // Get the Object code.
        $object_code = mediamosa_server_streaming::get_objectcode($a_mediafile_ext['metadata']['container_type'], $a_mediafile_ext['metadata']['video_codec']);

        $a_variables = array(
          'WIDTH' => !$width ? '' : $width,
          'HEIGHT' => !$height ? '' : $height,
          'HEIGHT_PLUS_20' => $height > 0 ? $height + 20 : '',
          'MEDIAFILE_ID' => $a_mediafile_ext['mediafile_id'],
          'TICKET_URI' => $streaming_server_ticket_uri,
          'AUTOPLAY' => $autostart ? 'true' : 'false',
          'AUTOPLAY_NUM' => $autostart ? '1' : '0',
          'AUTOPLAY_TEXT' => $autostart ? 'autoplay' : '',
          'IF_START' => '', '/IF_START' => '',
          'IF_DURATION' => '', '/IF_DURATION' => '',
          'IF_EXTERNAL' => '', '/IF_EXTERNAL' => '',
        );

        // START, END DURATION TIMES variables
        if (isset($start)) {
          $a_variables['START_TIME'] = self::convert_msec($start, $a_mediafile_ext['metadata']['container_type']);
          $a_variables['START_TIME_SECONDS'] = (int)($start / 1000);
        }
        else {
          // Remove code {IF_START} (Exclude from output) //   \{IF_START\}(.*?)\{/IF_START\}/is
          $output = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $output);
        }

        if (isset($duration)) {
          $endtime = $start + $duration;
          $a_variables['DURATION_TIME'] = self::convert_msec($duration, $a_mediafile_ext['metadata']['container_type']);
          $a_variables['DURATION_TIME_SECONDS'] = (int)($duration/1000);
          $a_variables['START_PLUS_DURATION_TIME_SECONDS'] = (int)($start + $duration)/1000;
          $a_variables['END_TIME'] = self::convert_msec($endtime, $a_mediafile_ext['metadata']['container_type']);
        }
        else {
          $output = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $output);
        }

        $content_type = 'text/html';

        switch ($a_mediafile_ext['metadata']['container_type']) {
          case self::CONTAINER_TYPE_FLV:
          case self::CONTAINER_TYPE_MP4:
          case self::CONTAINER_TYPE_MP3:
          case self::CONTAINER_TYPE_OGG:
            break;

          case self::CONTAINER_TYPE_WMV:
          default:

            // Either set, then we must use an external asx file.
            $use_external_asx_file = (isset($start) || isset($duration));

            // If TRUE, create an asx file and when everything goes as planned, ref to the new file.
            if ($use_external_asx_file) {
              preg_match('@\{IF_EXTERNAL\}(.*?)\{/IF_EXTERNAL\}@is', $output, $input);
              if (isset($input[0])) {
                $output_asx = $input[0];
              }

              if (isset($start)) {
                // Remove code {IF_START} (Exclude from output)
                $a_variables['START_TIME'] = self::convert_msec($start, self::CONTAINER_TYPE_WMV);
              }
              else {
                $output_asx = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $output_asx);
              }

              if (isset($duration)) {
                $a_variables['DURATION_TIME'] = self::convert_msec($duration, self::CONTAINER_TYPE_WMV);
              }
              else {
                $output_asx = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $output_asx);
              }

              // Process all the variables and make replacement in the objectcode.
              foreach ($a_variables as $key => $value) {
                $output_asx = str_replace('{'. $key .'}', $value, $output_asx);
              }

              // Get the filename for the asx file.
              $file_asx = mediamosa_configuration_storage::asx_file_location($ticket);

              // Open file for writing.
              $handle_file_asx = @fopen($file_asx, 'w+');

              // Check if file handle is open, else log it.
              if ($handle_file_asx) {
                // Write to file.
                $bytes_written = @fwrite($handle_file_asx, $output_asx);

                // Close file handle.
                fclose($handle_file_asx);

                // Any bytes written.
                if ($bytes_written) {
                  $uri = mediamosa_server_still::get_uri();

                  // If not found, then throw an error.
                  if ($uri == FALSE) {
                    throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SERVER_STILL_NOT_FOUND);
                  }

                  // Create new streaming server URI based on asx file.
                  $streaming_server_ticket_uri = str_replace(self::PARAM_URI_TICKET, urlencode($ticket) . '.asx', $uri);
                  $content_type = 'video/x-ms-asx';
                }
                else {
                  mediamosa::log('Unable to write to .asx file (@filename), falling back to normal object code', array('@filename' => $file_asx), WATCHDOG_CRITICAL, 'playproxy');
                }
              }
              else {
                mediamosa::log('Unable to create .asx file (@filename), falling back to normal object code', array('@filename' => $file_asx), WATCHDOG_CRITICAL, 'playproxy');
              }
            }

            $a_variables['WMA_TICKET_URI'] = $streaming_server_ticket_uri;

            // Remove code {IF_EXTERNAL} (Exclude from output)
            $output = preg_replace('@\{IF_EXTERNAL\}.*?\{/IF_EXTERNAL\}@si', '', $output);
            break;
        }
        break;

      case RESPONSE_TYPE_STILL:
        $output = $streaming_server_uri;
        break;
    }

    // Process all the variables and make replacement in the objectcode
    foreach ($a_variables as $key => $value) {
      $output = str_replace('{' . $key . '}', $value, $output);
    }

    $a_result = array(
      'output' => $output,
      'content_type' => $content_type,
    );

    $i = 1;
    foreach ($a_stills as $key => $value) {
      $a_result['stills']['#'. $i] = $value;
      $i++;
    }
    return $a_result;
  }
}
