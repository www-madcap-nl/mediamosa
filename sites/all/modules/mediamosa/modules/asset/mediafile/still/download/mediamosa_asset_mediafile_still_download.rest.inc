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
 * REST call for downloading mediafiles created by tickets.
 */

/**
 * URI: /still/$mediafile_id
 * URI: /still/ticket/$ticket_id
 * Method: GET / POST
 *
 * replacement for still/index.php.
 */
class mediamosa_rest_call_download_mediafile_still extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const STILL_ID = 'still_id';
  const TICKET_ID = 'ticket_id';

    // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STILL_ID,
          self::VAR_DESCRIPTION => 'The still ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::TICKET_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The ticket ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
      )
    );

    // All we need.
    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the ticket ID.
    $still_id = $this->get_param_value(self::STILL_ID);
    $ticket_id = $this->get_param_value(self::TICKET_ID);

    if ($ticket_id) {

      //
      // Temporary link.
      //

      // Get filename.
      $file = mediamosa_configuration_storage::ticket_still_location_get() . $ticket_id;

      if (!file_exists($file)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_STILL_TICKET, array('@ticket_id' => $ticket_id));
      }

      // Set headers.
      header('Content-Type: image/jpeg');

      // Make sure we send the header now.
      ob_clean();
      flush();

      // Download the image and delete the ticket.
      if (@readfile($file) !== FALSE) {
        mediamosa_io::unlink($file);
      }

      // Set the result, reponse object will know what to do.
      $o_mediamosa->set_result(mediamosa_response::SUCCESS, 200, $file);
    }
    elseif ($still_id) {

      //
      // Permanent link.
      //

      // Get the mediafile_id, asset_id.
      $response = mediamosa_asset_mediafile::get($still_id, NULL, array(mediamosa_asset_mediafile_db::ASSET_ID, mediamosa_asset_mediafile_db::IS_PROTECTED));
      if ($response) {
        // The mediafile exists. So get the parameters.
        $asset_id = $response[mediamosa_asset_mediafile_db::ASSET_ID];
        $still_is_protected = $response[mediamosa_asset_mediafile_db::IS_PROTECTED];

        // Check the asset protection.
        $asset_is_protected = mediamosa_asset_db::IS_PROTECTED_FALSE;
        if (mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE == $still_is_protected) {
          $response = mediamosa_asset::get($asset_id, NULL);
          if ($response) {
            // Asset is exists.
            $asset_is_protected = $response[mediamosa_asset_db::IS_PROTECTED];
          }
        }

        if ((mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE == $still_is_protected && mediamosa_asset_db::IS_PROTECTED_FALSE == $asset_is_protected)) {
          // Get filename.
          $file = mediamosa_configuration_storage::ticket_still_location_get() . mediamosa_configuration_storage::objectid_to_location($still_id);

          if (!file_exists($file)) {

            // Create the permanent link.
            $path = mediamosa_configuration_storage::ticket_still_location_get() . $still_id[0];

            // Create directory.
            mediamosa_io::mkdir($path, TRUE);

            // Make sure the location is a directory.
            if (!is_dir($path)) {
              throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_FOUND, array('@location' => $path));
            }

            // Must be able to write there.
            if (!mediamosa_io::is_writable($path)) {
              throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_WRITABLE, array('@location' => $path));
            }

            $target = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::still_location_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::objectid_to_location($still_id);

            // Create the symlink ticket.
            if (!is_dir($target)) {
              if (!mediamosa_io::symlink($target, $file)) {
                throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNABLE_TO_CREATE_SYMLINK, array('@location' => $target));
              }
            }

          }

          // Generate the still.

          // Set headers.
          header('Content-Type: image/jpeg');

          // Make sure we send the header now.
          ob_clean();
          flush();

          // Download the image and delete the ticket.
          if (@readfile($file) !== FALSE) {
            // Don't delete permanent stills.
            //mediamosa_io::unlink($file);
          }

          // Set the result, reponse object will know what to do.
          $o_mediamosa->set_result(mediamosa_response::SUCCESS, 200, $file);
        }
        else {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_STILL_PERMANENT, array('@still_id' => $still_id));
        }
      }
      else {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_STILL_PERMANENT, array('@still_id' => $still_id));
      }

    }
    else {
      // Nor ticket_id, nor still_id.
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_STILL_TICKET, array('@ticket_id' => $ticket_id));
    }
  }
}
