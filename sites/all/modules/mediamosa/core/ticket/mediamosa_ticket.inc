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
  * The ticket class.
  */

class mediamosa_ticket {

  // ------------------------------------------------------------------ Consts.
  /**
   *  Ticket types
   */
  const TICKET_TYPE_PLAY = 'PLAY';
  const TICKET_TYPE_DOWNLOAD = 'DOWNLOAD';
  const TICKET_TYPE_STILL = 'STILL';
  const TICKET_TYPE_UPLOAD = 'UPLOAD';

  // ------------------------------------------------------------------ Consts.
  /**
   * Return the path for the play ticket.
   * This path is relative, not based on absolute, this is by design.
   *
   * @return string
   */
  static public function play_ticket_target_get() {
    return '..' . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get();
  }

  /**
   * Return the path for the download ticket.
   * This path is relative, not based on absolute, this is by design.
   *
   * @return string
   */
  static public function download_ticket_target_get() {
    return '..' . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get();
  }

  /**
   * Return the path for the still ticket.
   * This path is relative, not based on absolute, this is by design.
   *
   * @return string
   */
  static public function still_ticket_target_get() {
    return '..' . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::still_location_get();
  }

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Create ticket.
   * 1.x: _play_proxy_create_ticket
   *
   * @param string $ticket
   *  A unique ID (HASH).
   * @param array $a_mediafile_ext
   *  Is mediafile array with mediafile_metadata array.
   * @param string $response_type
   *  See mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_*
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   *  Owner of ticket.
   * @param string $still_id
   *  Optional still_id.
   */
  static public function create($ticket, $a_mediafile_ext, $response_type = mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_URI, $app_id, $user_id, $still_id = FALSE) {

    if ($response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL) {
      // Find default or at least one still.
      if (!$still_id) {
        assert(is_array($a_mediafile_ext)); // lets make sure its here.

        // Find default still and make sure its ok.
        $still_id = mediamosa_asset_mediafile_still::find_default($a_mediafile_ext[mediamosa_asset_mediafile_db::ASSET_ID]);
      }
      else {
        // When we provide the still ID, we need to check it.
        mediamosa_asset_mediafile_still::check($still_id);
      }
    }
    else {
      // Check if the mediafile exists.
      $filename = mediamosa_configuration_storage::mediafile_filename_get($a_mediafile_ext);
      if (!file_exists($filename)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FILE_NOT_FOUND, array('@filename' => $filename));
      }
    }

    // Based on type, set the parameters.
    switch ($response_type) {
      case mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL:
        $ticket_type = self::TICKET_TYPE_STILL;
        $ticket_target = self::still_ticket_target_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::objectid_to_location($still_id);
        $ticket_path = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::STILL_TICKET_LOCATION;
        $ticket_symlink = $ticket_path . DIRECTORY_SEPARATOR . $ticket;
        break;

      case mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_DOWNLOAD:
        $ticket_type = self::TICKET_TYPE_DOWNLOAD;
        $ticket_target = self::download_ticket_target_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::objectid_to_location($a_mediafile_ext[mediamosa_asset_mediafile_db::ID]);
        $ticket_path = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::DOWNLOAD_TICKET_LOCATION . DIRECTORY_SEPARATOR . $ticket;

        // Set default filename.
        if (empty($a_mediafile_ext['filename'])) {
          $a_mediafile_ext['filename'] = mediamosa_asset_mediafile::UNKNOWN_FILENAME;
        }

        $ticket_symlink = $ticket_path . DIRECTORY_SEPARATOR . $a_mediafile_ext['filename'];
        break;

      default:
        $ticket_type = self::TICKET_TYPE_PLAY;
        $ticket_target = self::play_ticket_target_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::objectid_to_location($a_mediafile_ext['mediafile_id']);
        $ticket_path = mediamosa_configuration_storage::ticket_location_get($app_id);
        switch ($a_mediafile_ext['metadata']['container_type']) {
          case CONTAINER_TYPE_FLV:
            $ticket .= '.flv';
            break;
          case CONTAINER_TYPE_MP4:
            $ticket .= '.mp4';
            break;
          case CONTAINER_TYPE_MP3:
            $ticket .= '.mp3';
            break;
          case CONTAINER_TYPE_OGG:
            $ticket .= '.ogg';
            break;
        }

        $ticket_symlink = $ticket_path . DIRECTORY_SEPARATOR . $ticket;
        break;
    }

    // Create directory.
    mediamosa_io::mkdir($ticket_path, TRUE);

    // Make sure the location of the ticket is a directory.
    if (!is_dir($ticket_path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_FOUND, array('@location' => $ticket_path));
    }

    // Must be able to write there.
    if (!is_writeable($ticket_path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_WRITABLE, array('@location' => $ticket_path));
    }

    // Insert Ticket.
    mediamosa_db::db_query(
      'INSERT INTO {#mediamosa_ticket} #ticket_id = :ticket_id, #ticket_type = :ticket_type, #issued = :issued, #app_id = :app_id, #user_id = :user_id, #mediafile_id = :mediafile_id',
      array(
        '#mediamosa_ticket' => mediamosa_ticket_db::TABLE_NAME,
        '#ticket_id' => mediamosa_ticket_db::ID,
        ':ticket_id' => $ticket,
        '#ticket_type' => mediamosa_ticket_db::TICKET_TYPE,
        ':ticket_type' => $ticket_type,
        '#app_id' => mediamosa_ticket_db::APP_ID,
        ':app_id' => $app_id,
        '#user_id' => mediamosa_ticket_db::OWNER_ID,
        ':user_id' => $user_id,
        '#mediafile_id' => mediamosa_ticket_db::MEDIAFILE_ID,
        ':mediafile_id' => ($response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL && $still_id ? $still_id : isset($a_mediafile_ext['mediafile_id']) ? $a_mediafile_ext['mediafile_id'] : '')
      )
    );

    // Create the ticket.
    $ticket_target_tmp = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_unicode::substr($ticket_target, 3);
    if (!is_dir($ticket_target_tmp)) {
      if (file_exists($ticket_symlink) || !symlink($ticket_target, $ticket_symlink)) {
       throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNABLE_TO_CREATE_SYMLINK, array('@location' => $ticket_target));
      }
    }

    // Return the ticket, we might have changed it.
    return $ticket;
  }
}
