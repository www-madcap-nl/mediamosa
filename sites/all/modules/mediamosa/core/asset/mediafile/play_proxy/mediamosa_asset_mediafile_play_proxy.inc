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

  const UNKNOWN_FILENAME = 'Filename_not_set';

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

    // Choose a random available streaming server.
    // try to match container:codec first.
    $uri = mediamosa_db::db_query(
      "SELECT #uri FROM {#mediamosa_server_streaming} AS mss WHERE
      JOIN {#mediamosa_server_streaming_container} AS mssc USING(#streaming_server_id)
      WHERE mssc.#container = :container AND mss.#active = 1 ORDER BY rand() LIMIT 1",
      array(
        '#uri' => mediamosa_server_download_db::URI,
        '#mediamosa_server_streaming' => mediamosa_server_streaming_db::TABLE_NAME,
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
        '#streaming_server_id' => mediamosa_server_streaming_db::ID,
        '#container' => mediamosa_server_streaming_container_db::CONTAINER,
        ':container' => $a_mediafile_ext['metadata']['container_type'] .':'. $a_mediafile_ext['metadata']['video_codec'],
        '#active' => mediamosa_server_streaming_db::ACTIVE,
      )
    )->fetchField();

    if ($uri) {
      return $uri;
    }

    $uri = mediamosa_db::db_query(
      "SELECT #uri FROM {#mediamosa_server_streaming} AS mss WHERE
      JOIN {#mediamosa_server_streaming_container} AS mssc USING(#streaming_server_id)
      WHERE mssc.#container = :container AND mss.#active = 1 ORDER BY rand() LIMIT 1",
      array(
        '#uri' => mediamosa_server_download_db::URI,
        '#mediamosa_server_streaming' => mediamosa_server_streaming_db::TABLE_NAME,
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
        '#streaming_server_id' => mediamosa_server_streaming_db::ID,
        '#container' => mediamosa_server_streaming_container_db::CONTAINER,
        ':container' => $a_mediafile_ext['metadata']['container_type'],
        '#active' => mediamosa_server_streaming_db::ACTIVE,
      )
    )->fetchField();

    if (!$uri) { // No streaming profile found
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STREAMING_PROFILE_NOT_FOUND, array('@video_container' => $a_mediafile_ext['metadata']['container_type']));
    }

    return $uri;
  }

  /**
   * This function converts miliseconds to the format specified
   *
   * @param integer $msec
   * @param string $style
   *
   * @return: requested time format
   */
  function convert_msec($input, $style) {
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

    switch ($style) {
      case self::CONTAINER_TYPE_WMV: // hh:mm:ss.fract(=sec/100)
        $fraction = substr($msec, 0, 2);
        foreach ($a_output as $subject => $value) {
          if ($a_output[$subject] < 10) {
            $a_output[$subject] = "0". $a_output[$subject];
          }
        }
        $output = implode(":", $a_output) . '.' . $fraction;
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
        $output = implode(":", $a_output) . ':' . $fraction;
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
  static public function create_response_2x_to_1x($a_parameters, $a_asset_info, $a_mediafile_ext, $a_asset_metadata, $ticket, $app_id, $mediafile_group_id = '', $a_stills = array()) {
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
      $streaming_server_ticket_uri = str_replace(self::PARAM_URI_TICKET, $app_id .'/'. $ticket, $streaming_server_uri);
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
          throw new mediamosa_exception_error(mediamosa_error::STREAM_DOWNLOAD_NOT_ALLOWED);
        }

        if ($a_mediafile_ext['filename'] == '') {
          $a_mediafile_ext['filename'] = self::UNKNOWN_FILENAME;
        }

        $output = $streaming_server_uri . '/' . $a_mediafile_ext['filename'];
        break;
// @todo done tot hier
      case RESPONSE_TYPE_OBJECT:

        // try first the video_codec
        // Get random embed code for the container
        $output = db_result(db_query_range("SELECT objectcode FROM {streaming_server} ss
          LEFT JOIN {streaming_server_container} ssc
            USING(streaming_server_id)
          WHERE ssc.container = '%s'
            AND ss.active = 1
          ORDER BY RAND()",
          $a_mediafile_ext['metadata']['container_type'] .':'. $a_mediafile_ext['metadata']['video_codec'], 0, 1));

        if (!$output) {
        $output = db_result(db_query_range("SELECT objectcode FROM {streaming_server} ss
          LEFT JOIN {streaming_server_container} ssc
            USING(streaming_server_id)
          WHERE ssc.container = '%s'
            AND ss.active = 1
          ORDER BY RAND()",
          $a_mediafile_ext['metadata']['container_type'], 0, 1));
        }

        $variables = array(
          'WIDTH' => $a_video_size['width'] == 0 ? '' : $a_video_size['width'],
          'HEIGHT' => $a_video_size['height'] == 0 ? '' : $a_video_size['height'],
          'HEIGHT_PLUS_20' => $a_video_size['height'] > 0 ? $a_video_size['height']+20 : '',
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
        if (isset($a_parameters['start']['value'])) {
          $variables['START_TIME'] = self::convert_msec($a_parameters['start']['value'], $a_mediafile_ext['metadata']['container_type']);
          $variables['START_TIME_SECONDS'] = (int)($a_parameters['start']['value']/1000);
        }
        else {
          // Remove code {IF_START} (Exclude from output) //   \{IF_START\}(.*?)\{/IF_START\}/is
          $output = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $output);
        }

        if (isset($a_parameters['duration']['value'])) {
          $endtime = $a_parameters['start']['value'] + $a_parameters['duration']['value'];
          $variables['DURATION_TIME'] = self::convert_msec($a_parameters['duration']['value'], $a_mediafile_ext['metadata']['container_type']);
          $variables['DURATION_TIME_SECONDS'] = (int)($a_parameters['duration']['value']/1000);
          $variables['START_PLUS_DURATION_TIME_SECONDS'] = (int)($a_parameters['start']['value'] + $a_parameters['duration']['value'])/1000;
          $variables['END_TIME'] = self::convert_msec($endtime, $a_mediafile_ext['metadata']['container_type']);
        }
        else {
          $output = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $output);
        }

        $content_type = "text/html";

        switch ($a_mediafile_ext['metadata']['container_type']) {
          case self::CONTAINER_TYPE_FLV:
          case self::CONTAINER_TYPE_MP4:
          case self::CONTAINER_TYPE_MP3:
          case self::CONTAINER_TYPE_OGG:
            break;
          case self::CONTAINER_TYPE_WMV:
          default:

            $use_external_asx_file = (isset($a_parameters['start']['value']) || isset($a_parameters['duration']['value']));

            // If TRUE, create an asx file and when everything goes as planned, ref to the new file.
            if ($use_external_asx_file) {
              preg_match('@\{IF_EXTERNAL\}(.*?)\{/IF_EXTERNAL\}@is', $output, $input);
              if (isset($input[0])) {
                $output_asx = $input[0];
              }

              if (isset($a_parameters['start'])) {
                // Remove code {IF_START} (Exclude from output)
                $variables['START_TIME'] = self::convert_msec($a_parameters['start']['value'], self::CONTAINER_TYPE_WMV);
              }
              else {
                $output_asx = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $output_asx);
              }

              if (isset($a_parameters['duration'])) {
                $variables['DURATION_TIME'] = self::convert_msec($a_parameters['duration']['value'], self::CONTAINER_TYPE_WMV);
              }
              else {
                $output_asx = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $output_asx);
              }

              // Process all the variables and make replacement in the objectcode
              if (isset($variables)) {
                foreach ($variables as $key => $value) {
                  $output_asx = str_replace('{'. $key .'}', $value, $output_asx);
                }
              }

              // Get the filename for the asx file.
              $file_asx = _play_proxy_asx_file_location($ticket);

              $handle_file_asx = @fopen($file_asx, 'w+');
              if ($handle_file_asx) {
                $bytes_written = @fwrite($handle_file_asx, $output_asx);
                fclose($handle_file_asx);

                if ($bytes_written) {
                  $uri = db_result(db_query("SELECT uri FROM {still_server} where active = 1 "));
                  $streaming_server_ticket_uri = str_replace(self::PARAM_URI_TICKET, urlencode($ticket) . '.asx', $uri);
                  $content_type = "video/x-ms-asx";
                }
                else {
                  watchdog('playproxy', 'Unable to write to .asx file, falling back to normal object code');
                }
              }
              else {
                watchdog('playproxy', 'Unable to create .asx file, falling back to normal object code');
              }
            }
            $variables['WMA_TICKET_URI'] = $streaming_server_ticket_uri;

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
    if (isset($variables)) {
      foreach ($variables as $key => $value) {
        $output = str_replace('{'. $key .'}', $value, $output);
      }
    }

    $ret = array(
      'output' => $output,
      'content_type' => $content_type,
    );
    if ($a_stills && is_array($a_stills) && $a_stills != array()) {
      $i = 1;
      foreach ($a_stills as $key => $value) {
        $ret['stills']['#'. $i] = $value;
        $i++;
      }
    }
    return $ret;
  }
}