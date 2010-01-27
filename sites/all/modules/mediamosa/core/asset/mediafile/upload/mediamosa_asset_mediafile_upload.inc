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
  static public function handle_upload($ticket_id, $is_method_put, array $a_trancode_profiles, $retranscode, $transcode_inherits_acl, $tag, $create_still, $a_still_parameters, $completed_url, $completed_transcoding_url, $filename_put) {

    // Set to false.
    $is_app_admin = false;

    // Get the ticket.
    $a_ticket = mediamosa_ticket::get_valid_upload($ticket_id);

    if (empty($a_ticket)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_UPLOAD_TICKET);
    }

    // Get the app id from the ticket.
    $app_id = $a_ticket[mediamosa_ticket_db::APP_ID];

    // Remove ticket.
    mediamosa_ticket::delete($ticket_id);

    // Check user quota.
    mediamosa_user::must_have_user_quota($app_id, $a_ticket[mediamosa_ticket_db::OWNER_ID], $a_ticket[mediamosa_ticket_db::GROUP_ID]);

    // Check if mediafile exists.
    $a_asset_mediafile = mediamosa_asset_mediafile::must_exists($a_ticket[mediamosa_ticket_db::MEDIAFILE_ID]);

    // Must be original.
    if (!mediamosa_asset_mediafile::is_original($a_asset_mediafile)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UPLOAD_TARGET_IS_NOT_AN_ORIGINAL_FILE);
    }

    // Must be on.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_UPLOAD, array($app_id));

    // file_size depends on method used.
    // FIXME: $_FILES might be posted as an array in a array (more than one file uploaded).
    $file_size = $is_method_put ? $_SERVER['CONTENT_LENGTH'] : $_FILES['file']['size'];

    // Create job.
    $job_id = mediamosa_job::create_job_upload($app_id,  $a_ticket[mediamosa_ticket_db::OWNER_ID], $a_ticket[mediamosa_ticket_db::GROUP_ID], $is_app_admin, $a_ticket[mediamosa_ticket_db::MEDIAFILE_ID], $file_size, $retranscode, $create_still, $a_still_parameters);

    // Build the filename.
    $filename = mediamosa_configuration_storage::mediafile_id_filename_get($a_ticket[mediamosa_ticket_db::MEDIAFILE_ID]);

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

    // (re)build the completed_url based on success.
    if (!empty($completed_url)) {
      $a_query = array();

      $pos = strpos($completed_url, '?');// Lets see if there is a http query
      if ($pos !== FALSE) {
        if (strlen($completed_url) > $pos + 1) {
          parse_str(substr($completed_url, $pos+1), $a_query);
        }

        // Strip the query_string.
        $completed_url = substr($completed_url, 0, $pos);
      }

      $a_query['upload_ticket'] = $ticket_id;
      $a_query['status_code'] = $success ? mediamosa_error::ERRORCODE_OKAY : mediamosa_error::ERRORCODE_CANNOT_COPY_MEDIAFILE;

      $completed_url .= '?' . http_build_query($a_query, '', '&');
    }

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
    mediamosa_asset_mediafile::update($app_id, $a_ticket[mediamosa_ticket_db::MEDIAFILE_ID], $a_ticket[mediamosa_ticket_db::OWNER_ID], $a_fields);

    // Update the job.
    mediamosa_job::update_progress_upload($job_id, $file_size);

    // Trigger URL.
    self::trigger_completion_url($completed_url);

    // Start transcodes.
    foreach ($a_trancode_profiles as $profile_id) {
      try {
        mediamosa_job::create_job_transcode(
          $app_id,
          $a_ticket[mediamosa_ticket_db::OWNER_ID],
          $a_ticket[mediamosa_ticket_db::GROUP_ID],
          $is_app_admin,
          $a_ticket[mediamosa_ticket_db::MEDIAFILE_ID],
          $create_still,
          $a_still_parameters,
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
  static protected function handle_upload_put($filename, $job_id) {
    $filename_after_upload = NULL;

    // If mediafile exists, rename it.
    if (file_exists($filename)) {
      $filename_after_upload = $filename;
      $filename .= '.upload_temp';
    }

    // Get complete job.
    $a_job_ext = mediamosa_job::get_job_ext($job_id);

    // Change status to inprogress.
    mediamosa_job::update_status($a_job_ext, mediamosa_job_db::JOB_STATUS_INPROGRESS, '0.000');

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

    $i_byte_count = 0;
    $last_update = $i_timestamp = time();

    /// Read a chunk.
    $kb = fread($stream, mediamosa_settings::UPLOAD_PUT_CHUNK_SIZE);

    while ($kb) {
      if (!@fwrite($filename_file, $kb, mediamosa_settings::UPLOAD_PUT_CHUNK_SIZE)) {
        return FALSE;
      }

      $i_byte_count += mediamosa_settings::UPLOAD_PUT_CHUNK_SIZE;

      if ($last_update != time() && ((time() - $i_timestamp) % mediamosa_settings::UPLOAD_PROGRESS_INTERVAL) == 0) {
        $last_update = time();

        // Update the upload job progress.
        mediamosa_job::update_progress_upload($job_id, $i_byte_count);
      }

      // Read a chunk.
      $kb = fread($stream, mediamosa_settings::UPLOAD_PUT_CHUNK_SIZE);

      // Hmm why every loop?
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

/*
function vpx_upload_handle_file($a_args) {
  if ($a_args['get']['still_upload'] == 'TRUE' || $a_args['get']['type'] == 'still') {
    if (!$a_args['get']['mediafile_id']) {
      $a_args['get']['mediafile_id'] = $a_args['uri']['mediafile_id'];
    }
    return vpx_upload_handle_still($a_args);
  }

}
*/
}