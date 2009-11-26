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

  // ------------------------------------------------------------------ Static Functions.
  static public function create_response_still($ticket, $app_id) {
    $uri = get_media_server(self::RESPONSE_TYPE_STILL);

  }

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
}

/*
function _play_proxy_create_response($a_parameters, $a_asset_info, $a_mediafile_ext, $a_asset_metadata, $ticket, $app_id, $mediafile_group_id = '', $stills = array()) {

  $s_content_type = "";
  if (!isset($a_mediafile_ext['uri']) || is_null($a_mediafile_ext['uri'])) {
    $s_streaming_server_uri = _play_proxy_get_media_server($a_parameters, $a_mediafile_ext);
  }
  else {
    $s_streaming_server_uri = $a_mediafile_ext['uri'];
  }

  if (!$s_streaming_server_uri) { // No streaming profile found
    return vpx_return_error(ERRORCODE_STREAMING_PROFILE_NOT_FOUND, array("@video_container" => $a_mediafile_ext['metadata']['container_type']));
  }

  // if app_id in ticket link
  if ($a_parameters['response']['value'] != RESPONSE_TYPE_STILL) {
    //$s_streaming_server_ticket_uri = str_replace("{TICKET}", "{APP_ID}/{TICKET}?group_id={MEDIAFILE_GROUP_ID}&filename={FILENAME}", $s_streaming_server_uri);
    //$s_streaming_server_ticket_uri = str_replace(array("{TICKET}", "{APP_ID}", "{MEDIAFILE_ID}", "{FILENAME}"), array(urlencode($ticket), $app_id, urlencode($mediafile_group_id), urlencode($a_mediafile_ext['filename'])), $s_streaming_server_uri);
    // overrule for now...
    $s_streaming_server_ticket_uri = str_replace("{TICKET}", $app_id .'/'. $ticket, $s_streaming_server_uri);
  }

  // Uri voor niet-tickets (still, download etc)
  if ($stills && is_array($stills) && $stills != array()) {
    foreach ($stills as $key => $value) {
      $stills[$key]['still_ticket'] = str_replace("{TICKET}", urlencode($stills[$key]['ticket']), $s_streaming_server_uri);
    }
  }
  $s_streaming_server_uri = str_replace("{TICKET}", urlencode($ticket), $s_streaming_server_uri);

  // Autostart switch
  $autostart = (isset($a_parameters['autostart']['value']) ? vpx_shared_boolstr2bool($a_parameters['autostart']['value']) : TRUE);

  $a_video_size = _play_proxy_calc_video_size(
    isset($a_mediafile_ext['metadata']['width']) ? $a_mediafile_ext['metadata']['width'] : 0,
    isset($a_mediafile_ext['metadata']['height']) ? $a_mediafile_ext['metadata']['height'] : 0,
    isset($a_parameters['width']['value']) ? $a_parameters['width']['value'] : 0,
    isset($a_parameters['height']['value']) ? $a_parameters['height']['value'] : 0,
    isset($a_mediafile_ext['metadata']['container_type']) ? $a_mediafile_ext['metadata']['container_type'] : CONTAINER_TYPE_WMV
  );

  switch ($a_parameters['response']['value']) {
    case RESPONSE_TYPE_METAFILE:

      switch ($a_mediafile_ext['metadata']['container_type']) {
        case CONTAINER_TYPE_MP4:
          $s_output = '<?xml version="1.0"?>'."\n";
          $s_output .= '<?quicktime type="application/x-quicktime-media-link"?>'."\n";
          $s_output .= sprintf("<embed autoplay=\"%s\" src=\"%s\" />\n", $autostart ? 'true' : 'false', $s_streaming_server_ticket_uri);
          $s_content_type = "application/x-quicktime-media-link";
          break;

        case CONTAINER_TYPE_MP3:
          $s_content_type = "audio/mpeg";
          $s_output = "<asx version=\"3.0\">\n\t<entry>\n";
          $a_subjects = array('TITLE' => 'title', 'COPYRIGHT' => 'rights', 'ABSTRACT' => 'description', 'AUTHOR' => 'creator');
          if (isset($a_asset_info['dublin_core'])) {
            foreach ($a_subjects as $tag => $subject) {
              if (is_array($a_asset_info['dublin_core'][$subject]) && count($a_asset_info['dublin_core'][$subject])) {
                $s_output .= "\t\t<" . $tag .'>'. implode(' ', $a_asset_info['dublin_core'][$subject]) .'</'. $tag. ">\n";
              }
            }
          }

          $s_output .= sprintf("\t\t<ref href=\"%s\" />\n", $s_streaming_server_ticket_uri);
          $s_output .= "\t</entry>\n</asx>\n";

          break;

        case CONTAINER_TYPE_WMV:
          $s_content_type = "video/x-ms-asf";
          $s_output = "<asx version=\"3.0\">\n\t<entry>\n";
          $a_subjects = array('TITLE' => 'title', 'COPYRIGHT' => 'rights', 'ABSTRACT' => 'description', 'AUTHOR' => 'creator');
          if (isset($a_asset_info['dublin_core'])) {
            foreach ($a_subjects as $tag => $subject) {
              if (is_array($a_asset_info['dublin_core'][$subject]) && count($a_asset_info['dublin_core'][$subject])) {
                $s_output .= "\t\t<" . $tag .'>'. implode(' ', $a_asset_info['dublin_core'][$subject]) .'</'. $tag. ">\n";
              }
            }
          }

          // Add start and duration to asx.
          if (!@is_null($a_parameters['start']['value'])) {
            $s_output .= sprintf("\t\t<starttime value=\"%s\" />\n", vpx_convert_msec($a_parameters['start']['value'], CONTAINER_TYPE_WMV));
          }
          if (!@is_null($a_parameters['duration']['value'])) {
            $s_output .= sprintf("\t\t<duration value=\"%s\" />\n", vpx_convert_msec($a_parameters['duration']['value'], CONTAINER_TYPE_WMV));
          }

          $s_output .= sprintf("\t\t<ref href=\"%s\" />\n", $s_streaming_server_ticket_uri);
          $s_output .= "\t</entry>\n</asx>\n";
          break;

        default:
          return vpx_return_error(
            ERRORCODE_NO_METAFILE_AVAILABLE,
            array("@container_type" => $a_mediafile_ext['metadata']['container_type'])
          );
      }
      break;

    case RESPONSE_TYPE_URI:
      switch ($a_mediafile_ext['metadata']['container_type']) {
        case CONTAINER_TYPE_FLV:
          $script = "StroboScope.php?file=";
          $s_output = str_replace("{SCRIPT}", $script, $s_streaming_server_ticket_uri);
          break;
        default:
          $s_output = $s_streaming_server_ticket_uri;
          break;
      }
      break;

    case RESPONSE_TYPE_DOWNLOAD:
      if (isset($a_mediafile_ext['is_downloadable']) && strcasecmp($a_mediafile_ext['is_downloadable'], 'false') === 0 && $a_parameters['is_app_admin']['value'] != 'TRUE' ) {
        return vpx_return_error(ERRORCODE_MEDIAFILE_DOWNLOAD_DISABLED);
      }
      if (!@is_null($a_mediafile_ext['uri'])) {
        return vpx_return_error(STREAM_DOWNLOAD_NOT_ALLOWED);
      }
      if (empty($a_mediafile_$s_streaming_server_uriext['filename'])) {
        $a_mediafile_ext['filename'] = UNKNOWN_FILENAME;
      }
      $s_output = $s_streaming_server_uri ."/". $a_mediafile_ext['filename'];
      break;

    case RESPONSE_TYPE_OBJECT:

      // try first the video_codec
      // Get random embed code for the container
      $s_output = db_result(db_query_range("SELECT objectcode FROM {streaming_server} ss
        LEFT JOIN {streaming_server_container} ssc
          USING(streaming_server_id)
        WHERE ssc.container = '%s'
          AND ss.active = 1
        ORDER BY RAND()",
        $a_mediafile_ext['metadata']['container_type'] .':'. $a_mediafile_ext['metadata']['video_codec'], 0, 1));

      if (!$s_output) {
      $s_output = db_result(db_query_range("SELECT objectcode FROM {streaming_server} ss
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
        'TICKET_URI' => $s_streaming_server_ticket_uri,
        'AUTOPLAY' => $autostart ? 'true' : 'false',
        'AUTOPLAY_NUM' => $autostart ? '1' : '0',
        'AUTOPLAY_TEXT' => $autostart ? 'autoplay' : '',
        'IF_START' => '', '/IF_START' => '',
        'IF_DURATION' => '', '/IF_DURATION' => '',
        'IF_EXTERNAL' => '', '/IF_EXTERNAL' => '',
      );

      // START, END DURATION TIMES variables
      if (isset($a_parameters['start']['value'])) {
        $variables['START_TIME'] = vpx_convert_msec($a_parameters['start']['value'], $a_mediafile_ext['metadata']['container_type']);
        $variables['START_TIME_SECONDS'] = (int)($a_parameters['start']['value']/1000);
      }
      else {
        // Remove code {IF_START} (Exclude from output) //   \{IF_START\}(.*?)\{/IF_START\}/is
        $s_output = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $s_output);
      }

      if (isset($a_parameters['duration']['value'])) {
        $endtime = $a_parameters['start']['value'] + $a_parameters['duration']['value'];
        $variables['DURATION_TIME'] = vpx_convert_msec($a_parameters['duration']['value'], $a_mediafile_ext['metadata']['container_type']);
        $variables['DURATION_TIME_SECONDS'] = (int)($a_parameters['duration']['value']/1000);
        $variables['START_PLUS_DURATION_TIME_SECONDS'] = (int)($a_parameters['start']['value'] + $a_parameters['duration']['value'])/1000;
        $variables['END_TIME'] = vpx_convert_msec($endtime, $a_mediafile_ext['metadata']['container_type']);
      }
      else {
        $s_output = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $s_output);
      }

      $s_content_type = "text/html";

      switch ($a_mediafile_ext['metadata']['container_type']) {
        case CONTAINER_TYPE_FLV:
        case CONTAINER_TYPE_MP4:
        case CONTAINER_TYPE_MP3:
        case CONTAINER_TYPE_OGG:
          break;
        case CONTAINER_TYPE_WMV:
        default:

          $use_external_asx_file = (isset($a_parameters['start']['value']) || isset($a_parameters['duration']['value']));

          // If TRUE, create an asx file and when everything goes as planned, ref to the new file.
          if ($use_external_asx_file) {
            preg_match('@\{IF_EXTERNAL\}(.*?)\{/IF_EXTERNAL\}@is', $s_output, $input);
            if (isset($input[0])) {
              $s_output_asx = $input[0];
            }

            if (isset($a_parameters['start'])) {
              // Remove code {IF_START} (Exclude from output)
              $variables['START_TIME'] = vpx_convert_msec($a_parameters['start']['value'], CONTAINER_TYPE_WMV);
            }
            else {
              $s_output_asx = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $s_output_asx);
            }

            if (isset($a_parameters['duration'])) {
              $variables['DURATION_TIME'] = vpx_convert_msec($a_parameters['duration']['value'], CONTAINER_TYPE_WMV);
            }
            else {
              $s_output_asx = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $s_output_asx);
            }

            // Process all the variables and make replacement in the objectcode
            if (isset($variables)) {
              foreach ($variables as $key => $value) {
                $s_output_asx = str_replace('{'. $key .'}', $value, $s_output_asx);
              }
            }

            // Get the filename for the asx file.
            $file_asx = _play_proxy_asx_file_location($ticket);

            $handle_file_asx = @fopen($file_asx, 'w+');
            if ($handle_file_asx) {
              $bytes_written = @fwrite($handle_file_asx, $s_output_asx);
              fclose($handle_file_asx);

              if ($bytes_written) {
                $uri = db_result(db_query("SELECT uri FROM {still_server} where active = 1 "));
                $s_streaming_server_ticket_uri = str_replace("{TICKET}", urlencode($ticket) . '.asx', $uri);
                $s_content_type = "video/x-ms-asx";
              }
              else {
                watchdog('playproxy', 'Unable to write to .asx file, falling back to normal object code');
              }
            }
            else {
              watchdog('playproxy', 'Unable to create .asx file, falling back to normal object code');
            }
          }
          $variables['WMA_TICKET_URI'] = $s_streaming_server_ticket_uri;

          // Remove code {IF_EXTERNAL} (Exclude from output)
          $s_output = preg_replace('@\{IF_EXTERNAL\}.*?\{/IF_EXTERNAL\}@si', '', $s_output);

          break;
      }
      break;
    case RESPONSE_TYPE_STILL:
      $s_output = $s_streaming_server_uri;
      break;
  }

  // Process all the variables and make replacement in the objectcode
  if (isset($variables)) {
    foreach ($variables as $key => $value) {
      $s_output = str_replace('{'. $key .'}', $value, $s_output);
    }
  }

  $ret = array(
    'output' => $s_output,
    'content_type' => $s_content_type,
  );
  if ($stills && is_array($stills) && $stills != array()) {
    $i = 1;
    foreach ($stills as $key => $value) {
      $ret['stills']['#'. $i] = $value;
      $i++;
    }
  }
  return $ret;
}
*/