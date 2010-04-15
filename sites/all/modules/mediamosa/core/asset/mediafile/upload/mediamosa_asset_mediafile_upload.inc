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
 * Upload related function for mediafiles.
 */

class mediamosa_asset_mediafile_upload {

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Log for asset_mediafile_uplaod.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa::log($message, $variables, $severity, 'asset_mediafile_upload');
  }

  /**
   * Process the incoming upload. Will throw exceptions,
   * in that case make sure your REST call does not return XML
   * but redirects to some location.
   *
   * 1.x: vpx_upload_handle_file
   *
   */
  static public function handle_upload($ticket_id, $is_method_put, $tag, $completed_url, $filename_put, array $trancode_profiles = array(), $retranscode = FALSE, $transcode_inherits_acl = FALSE, $create_still = FALSE, array $still_parameters = array(), $completed_transcoding_url = '') {

    // Set to false.
    $is_app_admin = false;

    // Get the ticket.
    $ticket = mediamosa_ticket::get_valid_upload($ticket_id);

    if (empty($ticket)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_UPLOAD_TICKET);
    }

    // Get the app id from the ticket.
    $app_id = $ticket[mediamosa_ticket_db::APP_ID];

    // Remove ticket.
    mediamosa_ticket::delete($ticket_id);

    // Check user quota.
    mediamosa_user::must_have_user_quota($app_id, $ticket[mediamosa_ticket_db::OWNER_ID], $ticket[mediamosa_ticket_db::GROUP_ID]);

    // Check if mediafile exists.
    $asset_mediafile = mediamosa_asset_mediafile::must_exists($ticket[mediamosa_ticket_db::MEDIAFILE_ID]);

    // Must be original.
    if (!mediamosa_asset_mediafile::is_original($asset_mediafile)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UPLOAD_TARGET_IS_NOT_AN_ORIGINAL_FILE);
    }

    // Must be on.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_UPLOAD, array($app_id));

    // file_size depends on method used.
    // FIXME: $_FILES might be posted as an array in a array (more than one file uploaded).
    $file_size = $is_method_put ? $_SERVER['CONTENT_LENGTH'] : $_FILES['file']['size'];

    // Create job.
    $job_id = mediamosa_job::create_job_upload($app_id,  $ticket[mediamosa_ticket_db::OWNER_ID], $ticket[mediamosa_ticket_db::GROUP_ID], $is_app_admin, $ticket[mediamosa_ticket_db::MEDIAFILE_ID], $file_size, $retranscode, $create_still, $still_parameters);

    // Build the filename.
    $filename = mediamosa_configuration_storage::mediafile_id_filename_get($ticket[mediamosa_ticket_db::MEDIAFILE_ID]);

    // Create possible missing directories.
    mediamosa_io::mkdir(mediamosa_io::dirname($filename));

    if ($is_method_put) {
      // PUT.
      $success = self::handle_upload_put($filename, $job_id);
    }
    else {
      // Expecting POST here.

      // FIXME: $_FILES might be posted as an array in a array (more than one file uploaded).
      $success = (isset($_FILES['file']) && move_uploaded_file($_FILES['file']['tmp_name'], $filename));
    }

    // Enrich completed URL based on success.
    $completed_url = self::enrich_completed_url($completed_url, $ticket_id, $success);

    // When upload has failed, then set the upload job on 'failed'.
    if (!$success) {
      // Get complete job.
      $a_job_ext = mediamosa_job::get_job_ext($job_id);

      // Change status to failed.
      mediamosa_job::update_status($a_job_ext, mediamosa_job_db::JOB_STATUS_FAILED, '1.000');

      // Trigger URL.
      self::trigger_completion_url($completed_url);

      // Throw exception.
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANNOT_COPY_MEDIAFILE);
    }

    // Update the mediafile.
    $a_fields = array(
      mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => mediamosa_configuration_storage::mount_point_get(),
      mediamosa_asset_mediafile_db::IS_DOWNLOADABLE => 'FALSE',
      mediamosa_asset_mediafile_db::FILENAME => ($is_method_put ? $filename_put : $_FILES['file']['name']),
      mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => 'TRUE',
      mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL => $transcode_inherits_acl ? mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_TRUE : mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_FALSE,
      mediamosa_asset_mediafile_db::TAG => $tag,
    );

    // Update mediafile.
    mediamosa_asset_mediafile::update($app_id, $ticket[mediamosa_ticket_db::MEDIAFILE_ID], $ticket[mediamosa_ticket_db::OWNER_ID], $a_fields);

    // Update the job.
    mediamosa_job::update_progress_upload($job_id, $file_size);

    // Trigger URL.
    self::trigger_completion_url($completed_url);

    // Start transcodes.
    foreach ($trancode_profiles as $profile_id) {
      try {
        mediamosa_job::create_job_transcode(
          $app_id,
          $ticket[mediamosa_ticket_db::OWNER_ID],
          $ticket[mediamosa_ticket_db::GROUP_ID],
          $is_app_admin,
          $ticket[mediamosa_ticket_db::MEDIAFILE_ID],
          $create_still,
          $still_parameters,
          '', '', '', // file_ext, command, tool set by profile ID
          $profile_id,
          $completed_transcoding_url
        );
      }
      catch (mediamosa_exception $e) {
        // ignore.
      }
    }
  }

  /**
   * Upload still
   *
   * @param array $mediafile
   * @param string $ticket_id
   * @param boolean $is_method_put
   * @param string $tag
   * @param string $completed_url
   * @param string $filename_put
   */
  static public function handle_upload_still(array $mediafile, $ticket_id, $is_method_put, $tag, $completed_url, $order, $default, $filename_put) {

    // Set to false.
    $is_app_admin = false;

    // Get the ticket.
    $ticket = mediamosa_ticket::get_valid_upload($ticket_id);

    if (empty($ticket)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_UPLOAD_TICKET);
    }

    // Get the app id from the ticket.
    $app_id = $ticket[mediamosa_ticket_db::APP_ID];

    // Remove ticket.
    mediamosa_ticket::delete($ticket_id);

    // Check user quota.
    mediamosa_user::must_have_user_quota($app_id, $ticket[mediamosa_ticket_db::OWNER_ID], $ticket[mediamosa_ticket_db::GROUP_ID]);

    // Check if mediafile exists.
    mediamosa_asset_mediafile::must_exists($ticket[mediamosa_ticket_db::MEDIAFILE_ID]);

    // Must be on.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_UPLOAD, array($app_id));

    // file_size depends on method used.
    // FIXME: $_FILES might be posted as an array in a array (more than one file uploaded).
    $file_size = $is_method_put ? $_SERVER['CONTENT_LENGTH'] : $_FILES['file']['size'];

    // Check filesize.
    if ($file_size > mediamosa_settings::STILL_FILE_MAXIMUM) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_IMAGE_FILE_TOO_BIG, array('@filesize' => mediamosa_settings::STILL_FILE_MAXIMUM));
    }

    // Generate still_id.
    $still_id = mediamosa_db::uuid($app_id);

    // Get path to still.
    $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);

    // Create possible missing directories.
    mediamosa_io::mkdir(mediamosa_io::dirname($still_filename));

    if ($is_method_put) {
      // PUT.
      $success = self::handle_upload_put($still_filename);
    }
    else {
      // Expecting POST here.

      // FIXME: $_FILES might be posted as an array in a array (more than one file uploaded).
      $success = (isset($_FILES['file']) && move_uploaded_file($_FILES['file']['tmp_name'], $still_filename));
    }

    // Enrich completed URL based on success.
    $completed_url = self::enrich_completed_url($completed_url, $ticket_id, $success);

    // When upload has failed, then set the upload job on 'failed'.
    if (!$success) {
      // Trigger URL.
      self::trigger_completion_url($completed_url);

      // Throw exception.
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANNOT_COPY_MEDIAFILE);
    }

    // Create still.
    mediamosa_asset_mediafile_still::create(
      $mediafile['asset_id'], $mediafile['asset_id']/*no more asset_id_root*/,
      $still_id,
      $ticket[mediamosa_ticket_db::APP_ID],
      $ticket[mediamosa_ticket_db::OWNER_ID],
      $ticket[mediamosa_ticket_db::GROUP_ID],
      $order,
      $default,
      array('still_type' => 'PICTURE'),
      NULL,
      $mediafile[mediamosa_asset_mediafile_db::ID],
      $tag
    );

    // Trigger URL.
    self::trigger_completion_url($completed_url);

    // Done.
    return $still_id;
  }

  /**
   * Enrich completed url based on success.
   */
  static public function enrich_completed_url($completed_url, $ticket_id, $success) {
    // (re)build the completed_url based on success.
    if (!empty($completed_url)) {
      $http_query = array();

      $pos = strpos($completed_url, '?');// Lets see if there is a http query
      if ($pos !== FALSE) {
        if (strlen($completed_url) > $pos + 1) {
          parse_str(substr($completed_url, $pos+1), $http_query);
        }

        // Strip the query_string.
        $completed_url = substr($completed_url, 0, $pos);
      }

      $http_query['upload_ticket'] = $ticket_id;
      $http_query['status_code'] = $success ? mediamosa_error::ERRORCODE_OKAY : mediamosa_error::ERRORCODE_CANNOT_COPY_MEDIAFILE;

      $completed_url .= '?' . http_build_query($http_query, '', '&');
    }

    return $completed_url;
  }

  /**
   * trigger tid itl.
   * @param URL $completed_url
   */
  static function trigger_completion_url($completed_url) {
    // We call compeleted url when the job status has been updated.
    if (!empty($completed_url)) {
      // If it fails, we dont care...
      self::log('Completed URL @completed_url', array('@completed_url' => $completed_url));

      // Do call.
      mediamosa_http::do_head_call($completed_url, array('use_curlproxy' => TRUE));
    }
  }

  /**
   * Specific function for PUT upload
   *
   * 1.x: _vpx_upload_handle_file_put
   *
   * @param string $filename
   *  Destination filename.
   * @param integer $job_id
   *  Job ID.
   */
  static protected function handle_upload_put($filename, $job_id = 0) {
    $filename_after_upload = NULL;

    // If mediafile exists, rename it.
    if (file_exists($filename)) {
      $filename_after_upload = $filename;
      $filename .= '.upload_temp';
    }

    if ($job_id) {
      // Get complete job.
      $job_ext = mediamosa_job::get_job_ext($job_id);

      // Change status to inprogress.
      mediamosa_job::update_status($job_ext, mediamosa_job_db::JOB_STATUS_INPROGRESS, '0.000');
    }

    // Open the new file
    $filename_file = @fopen($filename, 'w');
    if (!$filename_file) {
      throw new mediamosa_exception(0, "Unable to write to '". $filename . "'.", mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
    }

    // Open the stream
    $stream = fopen('php://input', 'r');
    if (!$stream) {
      throw new mediamosa_exception(0, 'Unable to open stream', mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
    }

    $written_bytes = 0;
    $last_update = $timestamp = time();

    /// Read a chunk.
    $kb = fread($stream, mediamosa_settings::UPLOAD_PUT_CHUNK_SIZE);

    while ($kb) {
      $written_bytes += @fwrite($filename_file, $kb);
      if (!$written_bytes) {
        return FALSE;
      }

      if ($last_update != time() && ((time() - $timestamp) % mediamosa_settings::UPLOAD_PROGRESS_INTERVAL) == 0) {
        $last_update = time();

        if ($job_id) {
          // Update the upload job progress.
          mediamosa_job::update_progress_upload($job_id, $written_bytes);
        }
      }

      // Read a chunk.
      $kb = fread($stream, mediamosa_settings::UPLOAD_PUT_CHUNK_SIZE);

      // Set header.
      header('HTTP/1.1 201 Created');
    }

    // Close all open handles.
    @fclose($filename_file);
    @fclose($stream);

    // Ok rename when required.
    if (isset($filename_after_upload)) {
      mediamosa_io::rename($filename, $filename_after_upload);
    }

    return TRUE;
  }

  /**
   * Return the status of the upload using given ID and APC.
   *
   * @param string $id
   */
  static public function get_uploadprogress($id) {

    // Lets make sure APC is enabled.
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
}