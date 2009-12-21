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
   * Get the mediafile metadata.
   *
   * @param string $mediafile_id
   */
  static public function get_with_mediafileid($mediafile_id) {

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm');
    $query->fields('mfm');
    $query->condition('mfm.' . mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id);
    return $query->execute()->fetchAssoc();
  }

  /**
   * Test if mediafile can be played.
   * Will throw an exception when failure.
   *
   * @param string $mediafile_id
   */
  static public function is_playable($mediafile_id) { // media_management_is_playable.
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
  static public function is_transcodable($mediafile_id) { // media_management_is_transcodable.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
    $query->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id);
    $a_mediafile = $query->execute()->fetchAssoc();

    if ($a_mediafile[mediamosa_asset_mediafile_metadata_db::MIME_TYPE] == mediamosa_lib::MIME_TYPE_APPLICATION_X_EMPTY) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_TRANSCODE_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile has an empty filesize.'));
    }
  }

  static public function store_analyse($mediafile_id, $a_analyse) {

  }

  /**
 * Verwerk de informatie in de database die terugkomt van een analysejob
 */
function _vpx_jobhandler_update_analyse_job_info_to_mediafile($job_id, $analyse_result) {
  // verwerk de analyse gegevens naar een named array
  $parameters = _vpx_jobhandler_parse_metadata($analyse_result);

  // haal de mediafile_id op
  db_set_active("data");
  $db_result = db_query("SELECT mediafile_id, create_still, still_parameters, owner, app_id FROM {job} WHERE job_id = %s", $job_id);
  $dbrow_job = db_fetch_array($db_result);
  assert($dbrow_job["app_id"]);
  db_set_active();

  // bepaal de bestandsgrootte
  $filesize = filesize(SAN_NAS_BASE_PATH . DS . DATA_LOCATION . DS . $dbrow_job["mediafile_id"]{0} . DS . $dbrow_job["mediafile_id"]);

  // voeg een record toe aan de mediafile metadata tabel.
  db_set_active("data");
  $query_result = db_query(
    "INSERT INTO {mediafile_metadata}
    (mediafile_id, video_codec, colorspace, width, height, fps,
     audio_codec, sample_rate, channels, file_duration,
     container_type, bitrate, bpp, filesize, mime_type, is_hinted, is_inserted_md)
    VALUES
    ('%s', '%s', '%s',
      %d, %d, '%s',
     '%s', %d,
     '%s', '%s',
     '%s', %d,
     '%s', %d, '%s', '%s', '%s')",
      $dbrow_job["mediafile_id"], $parameters["video_codec"], $parameters["colorspace"],
      $parameters["width"], $parameters["height"], $parameters["fps"],
      $parameters["audio_codec"], $parameters["sample_rate"],
      $parameters["channels"], $parameters["file_duration"],
      $parameters["container_type"], $parameters["bitrate"],
      $parameters["bpp"], $filesize, $parameters["mime_type"], (strcasecmp($parameters["is_hinted"], 'yes') == 0 ? 'TRUE' : 'FALSE'), (strcasecmp($parameters["is_inserted_md"], 'yes') == 0 ? 'TRUE' : 'FALSE'));
  db_set_active();

  // Set the ffmpeg log
  db_set_active("data");
  db_query("UPDATE {job} SET error_description='%s' WHERE job_id = %d", $parameters['ffmpeg_output'], $job_id);
  db_set_active();

  // Update the asset data
  _media_management_update_asset_info($dbrow_job["mediafile_id"]); // werk 'length' bij

  if ($dbrow_job["create_still"] == 'TRUE') {
    // If we need to create a still after analyse job, then we must....

    $a_args = array();
    $a_args['uri'] = array();
    $a_args['uri']['mediafile_id'] = $dbrow_job["mediafile_id"];
    $a_args['get'] = array();
    $a_args['get']['user_id'] = $dbrow_job["owner"];
    $a_args['get']['app_id'] = $dbrow_job["app_id"];
    $a_args['get']['test_tag'] = $dbrow_job["test_tag"];
    if ($dbrow_job["still_parameters"]) {
      $a_args['post'] = unserialize($dbrow_job["still_parameters"]);
    }

    vpx_jobscheduler_create_new_still_job($a_args);
  }
}




}
