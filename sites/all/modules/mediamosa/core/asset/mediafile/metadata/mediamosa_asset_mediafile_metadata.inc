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
  * Metadata functions.
  */
class mediamosa_asset_mediafile_metadata {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Test if mediafile can be played.
   * Will throw an exception when failure.
   *
   * @param string $mediafile_id
   */
  function is_playable($mediafile_id) { // media_management_is_playable.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
    $query->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id);
    $a_mediafile = $query->execute()->fetchAssoc();

    if ($a_mediafile[mediamosa_asset_mediafile_metadata_db::MIME_TYPE] == mediamosa_lib::MIME_TYPE_APPLICATION_X_EMPTY) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_PLAY_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile has an empty filesize.'));
    }
  }

  /**
   * Test if mediafile can be transcoded.
   * Will throw an exception when failure.
   *
   * @param string $mediafile_id
   */
  function is_transcodable($mediafile_id) { // media_management_is_transcodable.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
    $query->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id);
    $a_mediafile = $query->execute()->fetchAssoc();

    if ($a_mediafile[mediamosa_asset_mediafile_metadata_db::MIME_TYPE] == mediamosa_lib::MIME_TYPE_APPLICATION_X_EMPTY) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_TRANSCODE_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile has an empty filesize.'));
    }
  }
}
