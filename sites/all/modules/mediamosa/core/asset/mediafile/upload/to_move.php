<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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







function vpx_upload_handle_file_put_app($a_args) {
  $result = vpx_upload_create_ticket($a_args);
  if (vpx_check_result_for_error($result)) {
    return $result;
  }

  $offset = strpos($result->response['items'][1]['action'], 'upload_ticket');
  $upload_ticket = substr($result->response['items'][1]['action'], $offset + 14);

  $a_args['get']['upload_ticket'] = $upload_ticket; //$result->response['items'][1]['upload_ticket'];
  return vpx_upload_handle_file_put($a_args);
}


function vpx_upload_handle_file_put($a_args) {
  $a_args['internal']['put'] = 'TRUE';
  return vpx_upload_handle_file($a_args);
}



function vpx_upload_uploadprogress_get($a_args) {

  // Get the id of the upload (is not ticket)
  vpx_funcparam_add($a_funcparam, $a_args, 'id', VPX_TYPE_ALPHANUM);

  // Get id
  $id = vpx_funcparam_get_value($a_funcparam, 'id');

  // Create output
  $o_response = new rest_response();

  // Add it
  $o_response->add_item(json_encode(vpx_upload_uploadprogress($id)));

  return $o_response;
}

function vpx_upload_uploadprogress($id) {

  if (!function_exists('apc_fetch')) {
    return array(
      'message' => 'Uploading (No Progress Information Available)',
      'percentage' => -1,
      'status' => 1,
    );
  }

  $a_status = apc_fetch('upload_' . $id);

  if (!$a_status['total']) {
    return array(
      'status' => 1,
      'percentage' => -1,
      'message' => 'Uploading',
    );
  }

  $a_status['status'] = 1;
  $a_status['percentage'] = round($a_status['current'] / $a_status['total'] * 100, 0);
  $a_status['message'] = "--:-- left (at --/sec)";

  $a_status['speed_average'] = 0;
  $a_status['est_sec'] = 0;

  $a_status['elapsed'] = time() - $a_status['start_time'];

  if ($a_status['elapsed'] > 0) {
    $a_status['speed_average'] = $a_status['current'] / $a_status['elapsed'];

    if ($a_status['speed_average'] > 0) {
      $a_status['est_sec'] = ($a_status['total'] - $a_status['current']) / $a_status['speed_average'];
      $a_status['message'] = sprintf("%02d:%02d left (at %s/sec)", $a_status['est_sec'] / 60, $a_status['est_sec'] % 60, format_size($a_status['speed_average']));
    }
  }

  return $a_status;
}

/**
 * Still upload as an image
 */
function vpx_upload_handle_still($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => 'alphanum',
    ),
    'upload_ticket' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'upload_ticket'),
      'type' => VPX_TYPE_ALPHANUM,
      'required' => TRUE
    ),
    'redirect_uri' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'redirect_uri'),
      'type' => 'skip',
    ),
    'put' => array(
      'value' => vpx_get_parameter_2($a_args['internal'], 'put', 'FALSE'),
      'type' => VPX_TYPE_BOOL,
    ),
    'filename' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'filename'),
      'type' => 'skip',
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'mediafile_id'),
      'type' => 'alphanum',
      'required' => TRUE
    ),
    'order' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'order'),
      'type' => 'skip',
    ),
    'default' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'default'),
      'type' => 'skip',
    ),
    'tag' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'tag'),
      'type' => VPX_TYPE_STRING,
    ),
  );

  // Validate all parameters
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // PUT or POST?
  $put_upload = vpx_shared_boolstr2bool($a_parameters['put']['value']);
  $a_parameters['filename']['required'] = $put_upload;

  // Check ticket
  db_set_active('data');
  $i_upload_expire_timestamp = time() - UPLOAD_TICKET_EXPIRATION;
  $a_ticket = db_fetch_array(db_query(
    "SELECT * FROM {ticket} WHERE ticket_id = '%s' AND issued > FROM_UNIXTIME(%d)",
    $a_parameters['upload_ticket']['value'],
    $i_upload_expire_timestamp
  ));
  db_set_active();
  if (!is_array($a_ticket)) {
    if (!is_null($a_parameters['redirect_uri']['value'])) {
      header(sprintf('Location: %s', $a_parameters['redirect_uri']['value']));
      exit();
    }
    return new rest_response(vpx_return_error(ERRORCODE_INVALID_UPLOAD_TICKET));
  }

  // Delete ticket
  db_set_active('data');
  db_query("DELETE FROM {ticket} WHERE ticket_id = '%s'", $a_parameters['upload_ticket']['value']);
  db_set_active();

  // Check user quota
  $result = _user_management_check_user_quota($a_ticket['app_id'], $a_ticket['user_id'], $a_ticket['group_id']);
  if (vpx_check_result_for_error($result)) {
    return new rest_response(vpx_return_error(ERRORCODE_NOT_ENOUGH_FREE_QUOTA));
  }

  // Check mediafile
  if ($a_parameters['asset_id']['value']) {
    $mcnt = vpx_count_rows("mediafile", array("mediafile_id", $a_ticket['mediafile_id'], "asset_id_root", $a_parameters['asset_id']['value']));
  }
  else {
    $mcnt = vpx_count_rows("mediafile", array("mediafile_id", $a_ticket['mediafile_id']));
  }
  if (!$mcnt) {
    if (!is_null($a_parameters['redirect_uri']['value'])) {
      header(sprintf('Location: %s', $a_parameters['redirect_uri']['value']));
      exit();
    }
    return new rest_response(vpx_return_error(ERRORCODE_MEDIAFILE_NOT_FOUND, array("@mediafile_id" => $a_ticket['mediafile_id'])));
  }

  // Is webservice active?
  if (!vpx_shared_webservice_is_active('media_upload', $a_ticket['app_id'])) {
    if (!is_null($a_parameters['redirect_uri']['value'])) {
      header(sprintf('Location: %s', $a_parameters['redirect_uri']['value']));
      exit();
    }
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  // Create hash
  $still_id = vpx_create_hash($a_ticket['app_id'], $a_ticket['user_id']);

  // Destination
  $destination = SAN_NAS_BASE_PATH .'/'. STILL_LOCATION .'/'. $still_id[0] .'/'. $still_id;

  if ($put_upload) {

    //
    // PUT upload
    //

    $success = TRUE;

    // if media file already exists, rename the new one
    if (file_exists($destination)) {
      $destination_filename_after_upload = $destination;
      $destination .= '.upload_temp';
    }

    // Open the new file
    $destination_file = @fopen($destination, 'w');
    if (!$destination_file) {
      throw new vpx_exception(0, "Unable to write to '". $destination ."'.", VPX_EXCEPTION_SEVERITY_HIGH);
    }

    // Open the stream
    $stream = fopen('php://input', 'r');
    if ($stream === FALSE) {
      throw new vpx_exception(0, 'Unable to open stream', VPX_EXCEPTION_SEVERITY_HIGH);
    }

    $b_continue = TRUE;

    $i_byte_count = 0;
    $last_update = $i_timestamp = time();
    while (($kb = fread($stream, CHUNK_SIZE)) && $b_continue) {
      $b_continue = (@fwrite($destination_file, $kb, CHUNK_SIZE));
      $i_byte_count += CHUNK_SIZE;
      if ($b_continue) {
        $b_continue = ($i_byte_count < VPX_STILL_FILE_MAXIMUM);
      }

      if (!$b_continue) {
        $success = FALSE;
        break;
      }
      header('HTTP/1.1 201 Created');
    }

    // Close the stream and the file
    @fclose($destination_file);
    @fclose($stream);

    if (!$b_continue) {
      @unlink($destination_file);
    }

    // If the mediafile is exists, then rename the new one
    if ($success && isset($destination_filename_after_upload)) {
      rename($destination, $destination_filename_after_upload);
    }

    //
    // PUT upload - end
    //

  }
  else { // POST upload
    // Move file to the appropriate place
    $success = (isset($_FILES['file']) && $_FILES['file']['size'] < VPX_STILL_FILE_MAXIMUM && move_uploaded_file($_FILES['file']['tmp_name'], $destination));
  }

  if (!$success) {
    // Upload failed
    if (!is_null($a_parameters['redirect_uri']['value'])) {
      header(sprintf('Location: %s', $a_parameters['redirect_uri']['value']));
      exit();
    }
    if ($put_upload) {
      if ($i_byte_count >= VPX_STILL_FILE_MAXIMUM) {
        return new rest_response(vpx_return_error(ERRORCODE_IMAGE_FILE_TOO_BIG, array("@filesize" => VPX_STILL_FILE_MAXIMUM)));
      }
    }
    else {
      if ($_FILES['file']['size'] >= VPX_STILL_FILE_MAXIMUM) {
        return new rest_response(vpx_return_error(ERRORCODE_IMAGE_FILE_TOO_BIG, array("@filesize" => VPX_STILL_FILE_MAXIMUM)));
      }
    }
    return new rest_response(vpx_return_error(ERRORCODE_CANNOT_COPY_MEDIAFILE));
  }

  watchdog('server', t('Still as picture is uploaded. Path: @path', array('@path' => $destination)));

  // Video details
  db_set_active('data');
  $video = db_fetch_array(db_query_range("SELECT * FROM {mediafile} WHERE mediafile_id = '%s'", $a_ticket['mediafile_id'], 0, 1));
  db_set_active();
  // Still size
  $size = getimagesize($destination);
  $width = $size[0];
  $height = $size[1];
  // File type
  $file_type = '';
  $pos = strrpos($size['mime'], '/');
  if ($pos !== FALSE) {
    $file_type = substr($size['mime'], $pos+1);
  }
  $filesize = filesize($destination);
  // Default
  $default = ( isset($a_parameters['default']['value']) && $a_parameters['default']['value'] == 1 ? 'TRUE' : 'FALSE' );
  // Order
  $order = ( is_numeric($a_parameters['order']['value']) ? $a_parameters['order']['value'] : 0 );
  $tag = $a_parameters['tag']['value'];
  $filename = ($put_upload ? $a_parameters['filename']['value'] : $_FILES['file']['name']);

  // Insert still into mediafile / mediafile_metadata
  db_set_active('data');
  if ($default == 'TRUE') {
    // Clear the earlier default mark on the video (media) file
    db_query("
      UPDATE {mediafile_metadata} AS mm
      INNER JOIN {mediafile} AS m USING(mediafile_id)
      SET mm.still_default = 'FALSE'
      WHERE m.mediafile_source = '%s' AND m.is_still = 'TRUE'", $video['mediafile_id']);
/*
    // Clear the earlier default mark on the asset
    db_query("
      UPDATE {mediafile_metadata} AS mm
      INNER JOIN {mediafile} AS m USING(mediafile_id)
      SET mm.still_default = 'FALSE'
      WHERE m.asset_id_root = '%s' AND m.is_still = 'TRUE'", $video['asset_id_root']);
 */
  }
  db_query("INSERT INTO {mediafile}
    (mediafile_id, asset_id, app_id, owner_id, group_id, is_original_file, is_downloadable, filename, uri, sannas_mount_point, transcode_profile_id, tool, command, file_extension, testtag, is_protected, created, changed, asset_id_root, transcode_inherits_acl, is_still, mediafile_source, tag)
    VALUES ('%s', '%s', %d, '%s', '%s', 'FALSE', 'FALSE', '%s', NULL, '%s', NULL, NULL, NULL, '%s', 'FALSE', 'FALSE', NOW(), NOW(), '%s', 'FALSE', 'TRUE', '%s', '%s')",
    $still_id, $video['asset_id'], $a_ticket['app_id'], $a_ticket['user_id'], $a_ticket['group_id'], $filename, SAN_NAS_BASE_PATH, $file_type, $video['asset_id_root'], $video['mediafile_id'], $tag);
  db_query("INSERT INTO {mediafile_metadata}
    (metadata_id, mediafile_id, video_codec, colorspace, width, height, fps, audio_codec, sample_rate, channels, file_duration, container_type, bitrate, bpp, filesize, mime_type, created, changed, is_hinted, is_inserted_md, still_time_code, still_order, still_format, still_type, still_default)
    VALUES (NULL, '%s', NULL, NULL, %d, %d, NULL, NULL, NULL, NULL, NULL, '', NULL, '', %d, '%s', NOW(), NOW(), 'FALSE', 'FALSE', NULL, %d, '%s', 'PICTURE', '%s')",
    $still_id, $width, $height, $filesize, $size['mime'], $order, $file_type, $default);
  db_set_active();

  if (!is_null($a_parameters['redirect_uri']['value'])) {
    header(sprintf('Location: %s', $a_parameters['redirect_uri']['value']));
    exit();
  }
  // Return the generated still_id
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $rest_response->add_item(array(
    "still_id" => $still_id,
  ));
  return $rest_response;
}
