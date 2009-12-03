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
  *
  */

function play_proxy_cron() {
  play_proxy_ticket_cleanup();
}

function play_proxy_ticket_cleanup() {
  $i_play_expire_timestamp = time() - PLAY_TICKET_EXPIRATION;
  $i_download_expire_timestamp = time() - DOWNLOAD_TICKET_EXPIRATION;
  $i_still_expire_timestamp = time() - STILL_TICKET_EXPIRATION;
  $s_play_ticket_path = SAN_NAS_BASE_PATH ."/". PLAY_TICKET_LOCATION;
  $s_download_ticket_path = SAN_NAS_BASE_PATH ."/". DOWNLOAD_TICKET_LOCATION;
  $s_still_ticket_path = SAN_NAS_BASE_PATH ."/". STILL_TICKET_LOCATION;

  # FIX ME, make it nice: maak een helper functie aan.
  // ruim de play tickets op
  $a_keep_these_tickets = array();

  // first we remove expired tickets
  db_set_active('data');
  $result = db_query("SELECT ticket_id, app_id FROM {ticket} WHERE ticket_type = '%s' AND issued <= FROM_UNIXTIME(%d)", TICKET_TYPE_PLAY, $i_play_expire_timestamp);
  while ($o_ticket = db_fetch_object($result)) {
    $ticket_file = $s_play_ticket_path . '/' . (int) $o_ticket->app_id  . '/' . $o_ticket->ticket_id;
    //$ticket_file = $s_play_ticket_path . '/' . $o_ticket->ticket_id;
    if (@readlink($ticket_file) !== FALSE) {
      @unlink($ticket_file);
      @unlink($ticket_file . ".asx");
      @unlink($ticket_file . ".mp4");
    }
  }
  db_query("DELETE FROM {ticket} WHERE ticket_type = '%s' AND issued <= FROM_UNIXTIME(%d)", TICKET_TYPE_PLAY, $i_play_expire_timestamp);
  db_set_active();

  // then a "recursive" file directory scan to remove missed files.
  db_set_active('data');
  $result = db_query("SELECT ticket_id FROM {ticket} WHERE ticket_type = '%s' AND issued > FROM_UNIXTIME(%d)", TICKET_TYPE_PLAY, $i_play_expire_timestamp);
  db_set_active();

  while ($o_ticket = db_fetch_object($result)) {
    $a_keep_these_tickets[] = $o_ticket->ticket_id;
  }

  $a_tickets = file_scan_directory($s_play_ticket_path, "^", array('.', '..', 'CVS'), 0, FALSE);
  foreach ($a_tickets as $o_ticket) {

    if (is_dir($o_ticket->filename)) {
      $a_sub_tickets = file_scan_directory($o_ticket->filename, "^", array('.', '..', 'CVS'), 0, FALSE);

      foreach ($a_sub_tickets as $o_ticket) {
        if (!in_array($o_ticket->basename, $a_keep_these_tickets) && @readlink($o_ticket->filename) !== FALSE) {
          unlink($o_ticket->filename);
        }
      }
    }
    elseif (!in_array($o_ticket->basename, $a_keep_these_tickets) && @readlink($o_ticket->filename) !== FALSE) {
      unlink($o_ticket->filename);
    }
  }

  // ruim de download tickets op
  $a_keep_these_tickets = array();

  db_set_active('data');
  $result = db_query("SELECT ticket_id FROM {ticket} WHERE ticket_type = '%s' AND issued > FROM_UNIXTIME(%d)", TICKET_TYPE_DOWNLOAD, $i_download_expire_timestamp);
  while ($o_ticket = db_fetch_object($result)) {
    $a_keep_these_tickets[] = $o_ticket->ticket_id;
  }
  db_set_active();

  db_set_active('data');
  db_query("DELETE FROM {ticket} WHERE ticket_type = '%s' AND issued <= FROM_UNIXTIME(%d)", TICKET_TYPE_DOWNLOAD, $i_download_expire_timestamp);
  db_set_active();

  $a_download_tickets = file_scan_directory($s_download_ticket_path, "^", array('.', '..', 'CVS'), 0, TRUE);
  foreach ($a_download_tickets as $o_ticket) {
    $s_ticket = basename(dirname($o_ticket->filename));
    if (!in_array($s_ticket, $a_keep_these_tickets)) {
      if (is_link($o_ticket->filename)) {
        unlink($o_ticket->filename);
        rmdir(dirname($o_ticket->filename));
      }
    }
  }

  // ruim de still tickets op
  exec('find '. $s_still_ticket_path .'/. -maxdepth 1 -mindepth 1 ! -name "*.asx" -name "????????????????????????" -type l -mmin +'. (int)(STILL_TICKET_EXPIRATION / 60) .' -delete');

  db_set_active('data');
  db_query("DELETE FROM {ticket} WHERE ticket_type = '%s' AND issued <= FROM_UNIXTIME(%d)", TICKET_TYPE_STILL, $i_still_expire_timestamp);
  db_set_active();

  exec('find '. $s_still_ticket_path .'/. -maxdepth 1 -mindepth 1 -name "????????????????????????.asx" ! -type l -mmin +'. (int)(STILL_TICKET_EXPIRATION / 60) .' -delete');
}

/**
 * Return the default path for the ticket
 *
 * @return unknown
 */
function _play_proxy_play_ticket_location($app_id) {
  return SAN_NAS_BASE_PATH ."/". PLAY_TICKET_LOCATION ."/". $app_id ;
}








function _play_proxy_get_mediafile_info($app_id, $mediafile_id) {
  $a_parameters = array(
    'uri' => array(
      'mediafile_id' => $mediafile_id,
    ),
    'get' => array(
      'app_id' => $app_id,
    )
  );
  return media_management_get_mediafile($a_parameters);
}


function _play_proxy_get_asset_info($app_id, $asset_id) {
  $a_parameters = array(
    'uri' => array(
      'asset_id' => $asset_id,
    ),
    'get' => array(
      'app_id' => $app_id,
    )
  );
  return media_management_get_asset($a_parameters);
}

// wrapper functie voor _play_proxy_get_mediafile_info en _play_proxy_get_mediafile_info
function _play_proxy_get_media_info($app_id, $asset_id, $mediafile_id, &$o_asset_info, &$o_mediafile_info) {
  $o_mediafile_info = _play_proxy_get_mediafile_info($app_id, $mediafile_id);
  $o_asset_info = _play_proxy_get_asset_info($app_id, $asset_id);
}


function _play_proxy_check_time_restrictions($time_restriction_start, $time_restriction_end) {
  if (time() < $time_restriction_start and ($time_restriction_start)) {
    return vpx_return_error(ERRORCODE_TIME_RESTRICTION_START, array('@date' => date("Y-m-d G:i:s", $time_restriction_start), '@timestamp' => $time_restriction_start));
  }
  if (time() > $time_restriction_end and ($time_restriction_end)) {
    return vpx_return_error(ERRORCODE_TIME_RESTRICTION_END, array('@date' => date("Y-m-d G:i:s", $time_restriction_end), '@timestamp' => $time_restriction_end));
  }
  return TRUE;
}


function play_proxy_request($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'mediafile_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'original_mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'original_mediafile_id'),
      'type' => 'alphanum',
    ),
    'still_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'still_id'),
      'type' => 'alphanum',
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'response' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'response', 'plain'),
      'type' => 'response_type',
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id'),
      'type' => TYPE_GROUP_ID,
    ),
    'domain' => array(
      'value' => (vpx_get_parameter_2($a_args['get'], 'domain') ? vpx_get_parameter_2($a_args['get'], 'domain') : vpx_get_parameter_2($a_args['get'], 'aut_domain')),
      'type' => 'skip',
    ),
    'realm' => array(
      'value' => (vpx_get_parameter_2($a_args['get'], 'realm') ? vpx_get_parameter_2($a_args['get'], 'realm') : vpx_get_parameter_2($a_args['get'], 'aut_realm')),
      'type' => 'skip',
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_app_admin', 'false'),
      'type' => VPX_TYPE_BOOL,
      'required' => FALSE,
    ),
    'profile_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'profile_id'),
      'type' => 'int',
    ),
    'width' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'width'),
      'type' => 'int',
      'required' => FALSE,
    ),
    'height' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'height'),
      'type' => 'int',
      'required' => FALSE,
    ),
    'start' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'start'),
      'type' => 'int',
      'value_min' => 0,
      'value_max' => 86399999, // (24 uur in msec)-1 msec
      'custom_error' => ERRORCODE_PP_INVALID_TIME
    ),
    'duration' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'duration'),
      'type' => 'int',
      'value_min' => 0,
      'value_max' => 86399999, // (24 uur in msec)-1 msec
      'custom_error' => ERRORCODE_PP_INVALID_TIME
    ),
    'autostart' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'autostart', 'true'),
      'type' => VPX_TYPE_BOOL,
      'required' => FALSE,
    ),
    // See width and height
    'size' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'size', NULL),
      'type' => 'skip',
    ),
    'format' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'format', NULL),
      'type' => 'skip',
    ),
    'range' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'range', NULL),
      'type' => 'skip',
    ),
    'tag' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'tag', NULL),
      'type' => VPX_TYPE_STRING,
    ),
  );
  if (!is_string($a_parameters['user_id']['value'])) {
    $a_parameters['user_id']['value'] = '';
  }

  if (isset($a_args['internal']) && $a_args['internal'] === TRUE) {
    $a_parameters['response']['value'] = "still";
  }

  if ($a_parameters['response']['value'] == "still") {
    if (!isset($a_args['get']['mediafile_id'])) {
      unset($a_parameters['mediafile_id']);
    }

    // valideer alle parameters op aanwezigheid en type
    $result = vpx_validate($a_parameters);
    if (vpx_check_result_for_error($result)) {
      return new rest_response($result);
    }
    /*
    try {
      // Doe autorisatie op de orginele mediafile

      vpx_acl_access_check_on_object(
        VPX_ACL_AUT_TYPE_ASSET,
        $a_parameters['asset_id']['value'],
        $a_parameters['asset_id']['value'],
        $a_parameters['app_id']['value'],
        $a_parameters['user_id']['value'],
        (is_array($a_parameters['group_id']['value']) ? $a_parameters['group_id']['value'] : array($a_parameters['group_id']['value'])),
        $a_parameters['domain']['value'],
        $a_parameters['realm']['value'],
        isset($a_parameters['is_app_admin']['value']) ? vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']) : FALSE
      );
    }
    catch (vpx_exception $e) {
      return $e->vpx_exception_rest_response();
    }
    */

  }

  if (!is_null($a_parameters['profile_id']['value'])) {
  //if (!is_null($a_parameters['profile_id']['value']) && !is_null($a_parameters['original_mediafile_id']['value'])) {
    $a_parameters['mediafile_id']['value'] = _play_proxy_get_mediafile_id_on_profile(
      $a_parameters['asset_id']['value'],
      $a_parameters['profile_id']['value'],
      $a_parameters['original_mediafile_id']['value']
    );

    // Not found?
    if ($a_parameters['mediafile_id']['value'] === FALSE) {
      return new rest_response(vpx_return_error(ERRORCODE_NO_MEDIAFILE_FOUND_FOR_PROFILE_ID));
    }
  }

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // verkrijg info uit media management
  if (isset($a_parameters['mediafile_id'])) {
    _play_proxy_get_media_info(
      $a_parameters['app_id']['value'],
      $a_parameters['asset_id']['value'],
      $a_parameters['mediafile_id']['value'],
      $o_asset_info, // return value (rest_response object)
      $o_mediafile_info // return value (rest_response object)
    );

    if (vpx_check_result_for_error($o_asset_info)) {
      return $o_asset_info; // het is al een rest_response object
    }

    if (vpx_check_result_for_error($o_mediafile_info)) {
      return $o_mediafile_info; // het is al een rest_response object
    }

    $a_asset_info = vpx_get_item_from_rest_response($o_asset_info, 1);
    $a_mediafile_info = vpx_get_item_from_rest_response($o_mediafile_info, 1);

  // controleer asset/mediafile relatie
    if ($a_asset_info['asset_id'] !== $a_mediafile_info['asset_id']) {
      return new rest_response(vpx_return_error(ERRORCODE_INVALID_ASSET_MEDIAFILE_COMBINATION));
    }

    // controleer start- en eindtijd (afspeel restricties)
    if (isset($a_asset_info['time_restriction_start']) && isset($a_asset_info['time_restriction_end'])) {
      $result = _play_proxy_check_time_restrictions(
        $a_asset_info['time_restriction_start'],
        $a_asset_info['time_restriction_end']
      );
      if (vpx_check_result_for_error($result)) {
        return new rest_response($result);
      }
    }

    try {
      _media_management_is_inappropriate($a_asset_info['asset_id'], $a_parameters['app_id']['value'], $a_parameters['user_id']['value'], vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']));
    }
    catch (vpx_exception_error $e) {
      return new rest_response(vpx_return_error(ERRORCODE_IS_UNAPPROPRIATE));
    }

// verstuur de mediafile_id naar de statistics
    _vpx_statistics_log_requested_stream($a_parameters['mediafile_id']['value'], $a_parameters['response']['value']);
  }
  else {
  /*
    if ($a_parameters['response']['value'] != "still") {
      $o_asset_info = _play_proxy_get_asset_info($a_parameters['app_id']['value'], $a_parameters['asset_id']['value']);
      if (vpx_check_result_for_error($o_asset_info)) {
        return $o_asset_info; // het is al een rest_response object
      }
      $a_asset_info = vpx_get_item_from_rest_response($o_asset_info, 1);
      $a_mediafile_info['asset_id'] = $a_asset_info['asset_id'];
    }
    */
    unset($a_asset_info);
    $a_mediafile_info['asset_id'] = $a_parameters['asset_id']['value'];
    unset($a_parameters['mediafile_id']['value']);// = "";
  }

  if (isset($a_parameters['mediafile_id']['value'])) {
    mediamosa_asset_mediafile_metadata::is_playable($a_parameters['mediafile_id']['value']);
  }

  // If for some reason mediafile_id was unset above, we dont test access
  if (isset($a_parameters['mediafile_id']['value'])) {
    try {
      // Doe autorisatie op de orginele mediafile
      vpx_acl_access_check_on_object(
        VPX_ACL_AUT_TYPE_MEDIAFILE,
        $a_parameters['asset_id']['value'],
        $a_parameters['mediafile_id']['value'],
        $a_parameters['app_id']['value'],
        $a_parameters['user_id']['value'],
        (is_array($a_parameters['group_id']['value']) ? $a_parameters['group_id']['value'] : array($a_parameters['group_id']['value'])),
        $a_parameters['domain']['value'],
        $a_parameters['realm']['value'],
        isset($a_parameters['is_app_admin']['value']) ? vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']) : FALSE
      );
    }
    catch (vpx_exception $e) {
      return $e->vpx_exception_rest_response();
    }
  }

  // Generate unique ticket
  $s_ticket = vpx_create_hash($a_parameters['app_id']['value'], $a_parameters['user_id']['value']);
  // make a play or download symlink
  if (!isset($a_mediafile_info['uri'])) {
    $result = _play_proxy_create_ticket(
      $s_ticket,
      $a_mediafile_info,
      $a_parameters['response']['value'],
      $a_parameters['app_id']['value'],
      $a_parameters['user_id']['value'],
      $a_parameters['still_id']['value']
    );
    if (vpx_check_result_for_error($result)) {
      return new rest_response($result);
    }
    $s_ticket = $result;
  }

  // If response type is still, we get all information of all stills with all details
  $stills = array();
  if ($a_parameters['response']['value'] == "still") {
    $width = $a_parameters['width']['value'];
    $height = $a_parameters['height']['value'];
    $size = $a_parameters['size']['value'];
    $format = $a_parameters['format']['value'];
    $range = $a_parameters['range']['value'];
    $tag = $a_parameters['tag']['value'];

    $size = split('x', $size);
    if (!isset($width) && !isset($height) && $size && is_array($size) && isset($size[0]) && $size[0] >= 0 && is_numeric($size[0]) && isset($size[1]) && is_numeric($size[1]) && $size[1] >= 0 && !isset($size[2])) {
      $width = $size[0];
      $height = $size[1];
    }

    $orders = array();
    if (isset($range)) {
      if (is_numeric($range)) {
        $orders[] = $range;
      }
      else {
        $range = split(',', $range);
        foreach($range as $a_range) {
          if (is_numeric($a_range)) {
            $orders[] = $a_range;
          }
          else {
            $pos = strpos($a_range, '-', 1);
            if ($pos !== FALSE) {
              // Think to the negative numbers, so change the separator
              $a_range[$pos] = '!';
              $a_range = split('!', $a_range);
              if (is_array($a_range) && isset($a_range[0]) && is_numeric($a_range[0]) && isset($a_range[1]) && is_numeric($a_range[1]) && !isset($a_range[2]) && $a_range[0] <= $a_range[1]) {
                for ($i = $a_range[0]; $i <= $a_range[1]; $i++) {
                  $orders[] = $i;
                }
              }
            }
          }
        }
      }
    }

    $base_sql = "
        SELECT
          m.mediafile_id AS still_id,
          v.mediafile_id AS mediafile_id,
          m.asset_id_root AS asset_id,
          m.app_id,
          m.owner_id,
          m.filename,
          m.mediafile_source,
          m.tag,
          mm.width,
          mm.height,
          mm.filesize,
          mm.mime_type,
          mm.still_time_code,
          mm.still_order,
          mm.still_format,
          mm.still_type,
          mm.still_default
        FROM {mediafile} AS m
        INNER JOIN {mediafile_metadata} AS mm USING (mediafile_id)
        INNER JOIN {mediafile} AS v ON m.mediafile_source = v.mediafile_id AND v.is_still = 'FALSE'
        WHERE
          m.is_still = 'TRUE' AND ".
          (isset($a_parameters['still_id']['value']) ? sprintf("m.mediafile_id = '%s' AND ", $a_parameters['still_id']['value']) : "").
          (isset($a_parameters['mediafile_id']['value']) ? sprintf("v.mediafile_id = '%s' AND ", $a_parameters['mediafile_id']['value']) : "").
          (isset($width) ? sprintf("mm.width = %d AND ", $width) : "" ).
          (isset($height) ? sprintf("mm.height = %d AND ", $height) : "" ).
          (isset($format) ? sprintf("mm.still_format = '%s' AND ", $format) : "" ).
          (isset($orders) && is_array($orders) && $orders != array() ? sprintf("mm.still_order IN (%s) AND ", join(", ", $orders)) : "" ).
          (isset($tag) ? sprintf("m.tag = '%s' AND ", $tag) : "" ).
          "m.app_id = %d AND
          m.asset_id_root = '%s'
        ORDER BY m.asset_id, mm.still_order
      ";
    if (isset($a_parameters['still_id']['value'])) {
      db_set_active('data');
      $t_stills = db_fetch_array(db_query_range($base_sql,
        $a_parameters['app_id']['value'],
        $a_parameters['asset_id']['value'],
        0, 1));
      db_set_active();
      $t_stills['ticket'] = $s_ticket;
      $stills[] = $t_stills;
    }
    else {
      db_set_active('data');
      $rs = db_query($base_sql,
        $a_parameters['app_id']['value'],
        $a_parameters['asset_id']['value']);
      db_set_active();
      while ($t_stills = db_fetch_array($rs)) {
        // Generate unique ticket
        $s_ticket = vpx_create_hash($a_parameters['app_id']['value'], $a_parameters['user_id']['value']);
        // make a play or download symlink
        if (!isset($a_mediafile_info['uri'])) {
          $result = _play_proxy_create_ticket(
            $s_ticket,
            $a_mediafile_info,
            $a_parameters['response']['value'],
            $a_parameters['app_id']['value'],
            $a_parameters['user_id']['value'],
            $t_stills['still_id']
          );
          if (vpx_check_result_for_error($result)) {
            return new rest_response($result);
          }
          $s_ticket = $result;
        }
        $t_stills['ticket'] = $s_ticket;
        $stills[] = $t_stills;
      }
    }
  }

// maak de play proxy response aan
  $response = _play_proxy_create_response($a_parameters, $a_asset_info, $a_mediafile_info, NULL, $s_ticket, $a_parameters['app_id']['value'], $a_parameters['group_id']['value'], $stills);
  if (vpx_check_result_for_error($response)) {
    return new rest_response($response);
  }

  // All ok, now set played + 1
  _mediafile_management_asset_played($a_parameters['asset_id']['value']);

// retourneer de video
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $rest_response->add_item($response);
  return $rest_response;
}

/**
 * Return a matching mediafile based on the asset_id and a profile_id
 */
function _play_proxy_get_mediafile_id_on_profile($asset_id, $profile_id, $original_mediafile_id = NULL) {
  if (is_null($original_mediafile_id)) {
    $query = sprintf("
      SELECT mediafile_id
      FROM {mediafile}
      WHERE asset_id_root = '%s' AND transcode_profile_id = %d AND is_still = 'FALSE'
      ", $asset_id, $profile_id
    );
  }
  else {
    $query = sprintf("
      SELECT m.mediafile_id
      FROM {mediafile} AS m
      INNER JOIN {mediafile} AS orig USING(asset_id)
      WHERE
        m.asset_id_root = '%s' AND
        m.transcode_profile_id = %d AND
        m.is_still = 'FALSE' AND
        orig.is_original_file = 'TRUE' AND
        orig.is_still = 'FALSE' AND
        orig.asset_id_root = '%s' AND
        orig.mediafile_id = '%s'
      ", $asset_id, $profile_id, $asset_id, $original_mediafile_id
    );
  }
  db_set_active('data');
  $result = db_result(db_query_range($query, 0, 1));
  db_set_active();
  //watchdog('server', $result);
  return $result;
}
