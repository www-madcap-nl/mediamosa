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
  * Metadata functions.
  */
class mediamosa_asset_mediafile_metadata {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the mediafile metadata.
   *
   * @param string $mediafile_id
   */
  static public function get_with_mediafileid($mediafile_id, array $a_fields = array()) {

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm');
    $query->fields('mfm', $a_fields);
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
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfmd')
      ->fields('mfmd')
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id);
    $a_mediafile = $query->execute()->fetchAssoc();

    if ($a_mediafile[mediamosa_asset_mediafile_metadata_db::MIME_TYPE] == mediamosa_lib::MIME_TYPE_APPLICATION_X_EMPTY) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_PLAY_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile has an empty filesize.'));
    }
  }

  /**
   * Test if mediafile can be transcoded.
   * Will throw an exception when failure.
   *
   * 1.x: media_management_is_transcodable.
   *
   * @param string $mediafile_id
   */
  static public function is_transcodable($mediafile_id) {
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mamm');
    $query->fields('mamm');
    $query->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id);
    $a_mediafile = $query->execute()->fetchAssoc();

    if ($a_mediafile[mediamosa_asset_mediafile_metadata_db::MIME_TYPE] == mediamosa_lib::MIME_TYPE_APPLICATION_X_EMPTY) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_TRANSCODE_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile has an empty filesize.'));
    }
  }

  /**
   * Delete all metadata properties of mediafile.
   *
   * @param string $mediafile_id
   */
  static public function delete_by_mediafileid($mediafile_id) {

    return mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();
  }

  /**
   * Calculate the bits per plane.
   *
   * 1.x: _vpx_jobhandler_calculate_bpp
   *
   * @param $width
   * @param $height
   * @param $fps
   * @param $bitrate
   */
  function calculate_bpp($width, $height, $fps, $bitrate) {
    $result = '';
    if ($width != '' && $height != '' && $fps != '' && $bitrate != '') {
      $result = round((($bitrate * 1000) / ($fps * $width * $height)), 2);
    }

    return $result;
  }

  /**
   * Parse the returned string from analyse.
   *
   * 1.x: _vpx_jobhandler_parse_metadata
   *
   * @param string $analyse_result
   */
  static public function parse_metadata($analyse_result) {

    $a_metadata = mediamosa_lib::create_named_array($analyse_result, "\n", ': ');

    // MIME-type.
    $a_fields[mediamosa_asset_mediafile_metadata_db::MIME_TYPE] = isset($a_metadata['MIME-type']) ? $a_metadata['MIME-type'] : '';

    // Video-codec.
    $a_fields[mediamosa_asset_mediafile_metadata_db::VIDEO_CODEC] = isset($a_metadata['Video-codec']) ? $a_metadata['Video-codec'] : '';

    // Video-colorspace.
    $a_fields[mediamosa_asset_mediafile_metadata_db::COLORSPACE] = isset($a_metadata['Video-colorspace']) ? $a_metadata['Video-colorspace'] : '';

    // Video-size.
    list($width, $height) = isset($a_metadata['Video-size']) ? explode('x', $a_metadata['Video-size']) : array(0, 0);
    $a_fields[mediamosa_asset_mediafile_metadata_db::WIDTH] = $width;
    $a_fields[mediamosa_asset_mediafile_metadata_db::HEIGHT] = $height;

    // Video-framespersecond.
    $a_fields[mediamosa_asset_mediafile_metadata_db::FPS] = isset($a_metadata['Video-framespersecond']) ? $a_metadata['Video-framespersecond'] : '';

    // Audio-codec.
    $a_fields[mediamosa_asset_mediafile_metadata_db::AUDIO_CODEC] = isset($a_metadata['Audio-codec']) ? $a_metadata['Audio-codec'] : '';

    // Audio-frequency.
    $a_fields[mediamosa_asset_mediafile_metadata_db::SAMPLE_RATE] = isset($a_metadata['Audio-frequency']) ? $a_metadata['Audio-frequency'] : '0';

    // Audio-channels.
    $a_fields[mediamosa_asset_mediafile_metadata_db::CHANNELS] = isset($a_metadata['Audio-channels']) ? $a_metadata['Audio-channels'] : '';

    // file-duration.
    $a_fields[mediamosa_asset_mediafile_metadata_db::FILE_DURATION] = isset($a_metadata['File-duration']) ? $a_metadata['File-duration'] : '';

    // container.
    $a_fields[mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE] = isset($a_metadata['File-type']) ? $a_metadata['File-type'] : '';

    // file-bitrate.
    $a_fields[mediamosa_asset_mediafile_metadata_db::BITRATE] = isset($a_metadata['File-bitrate']) ? $a_metadata['File-bitrate'] : '0';

    // bpp.
    $a_fields[mediamosa_asset_mediafile_metadata_db::BPP] = (string)self::calculate_bpp($width, $height, $a_fields[mediamosa_asset_mediafile_metadata_db::FPS], $a_fields[mediamosa_asset_mediafile_metadata_db::BITRATE]);

    // is hinted.
    $a_fields[mediamosa_asset_mediafile_metadata_db::IS_HINTED] = isset($a_metadata['Is-hinted']) ? $a_metadata['Is-hinted'] : 'no';
    $a_fields[mediamosa_asset_mediafile_metadata_db::IS_HINTED] = strcasecmp($a_fields[mediamosa_asset_mediafile_metadata_db::IS_HINTED], 'yes') == 0 ? 'TRUE' : 'FALSE';

    // has inserted extra metadata.
    $a_fields[mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD] = isset($a_metadata['Is-inserted-md']) ? $a_metadata['Is-inserted-md'] : 'no';
    $a_fields[mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD] = strcasecmp($a_fields[mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD], 'yes') == 0 ? 'TRUE' : 'FALSE';

    // The output of ffmpeg.
    $a_fields['ffmpeg_output'] = isset($a_metadata['ffmpeg-output']) ? implode('\n', explode('}-{', $a_metadata['ffmpeg-output'])) : '';

    // Done.
    return $a_fields;
  }

  /**
   * Fill the mediafile metadata.
   *
   * @param $still_id
   * @param $width
   * @param $height
   * @param $filesize
   * @param $mime
   * @param $file_type
   */
  static public function create_metadata_still_default($still_id, $width, $height, $filesize, $mime, $file_type) {
    // Create mediafile metadata.
    $a_fields = array(
      mediamosa_asset_mediafile_metadata_db::ID => $still_id,
      mediamosa_asset_mediafile_metadata_db::WIDTH => $width,
      mediamosa_asset_mediafile_metadata_db::HEIGHT => $height,
      mediamosa_asset_mediafile_metadata_db::BPP => '',
      mediamosa_asset_mediafile_metadata_db::FILESIZE => $filesize,
      mediamosa_asset_mediafile_metadata_db::MIME_TYPE => $mime,
      mediamosa_asset_mediafile_metadata_db::STILL_ORDER => 0,
      mediamosa_asset_mediafile_metadata_db::STILL_FORMAT => $file_type,
      mediamosa_asset_mediafile_metadata_db::STILL_TYPE => STILL_TYPE_NONE,
      mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT => STILL_DEFAULT_FALSE,
    );

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Store the analyse result in the mediafile metadata.
   *
   * 1.x: _vpx_jobhandler_update_analyse_job_info_to_mediafile
   *
   * @param $job_id
   * @param $analyse_result
   */
  static public function store_analyse($job_id, $a_analyse_result) {

    // Get the job.
    $a_job = mediamosa_job::get($job_id);

    // Get the filesize.
    $filesize = mediamosa_io::filesize(mediamosa_configuration_storage::mediafile_id_filename_get($a_job[mediamosa_job_db::MEDIAFILE_ID]));

    // Parse the analyse result.
    $a_fields = self::parse_metadata(implode("\n", $a_analyse_result));

    // Remove any metadata first.
    mediamosa_asset_mediafile_metadata::delete_by_mediafileid($a_job[mediamosa_job_db::MEDIAFILE_ID]);

    // No changed/created in metadata.

    // Add these.
    $a_fields[mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID] = $a_job[mediamosa_job_db::MEDIAFILE_ID];
    $a_fields[mediamosa_asset_mediafile_metadata_db::FILESIZE] = $filesize;

    // Copy and remove.
    $ffmpeg_output = $a_fields['ffmpeg_output'];
    unset($a_fields['ffmpeg_output']);

    // Enrich with created/changed.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();

    // Store in error description.
    mediamosa_job::store_error_description($job_id, $ffmpeg_output);

    // Normalize to asset.
    mediamosa_asset::update_asset_info($a_job[mediamosa_job_db::MEDIAFILE_ID]);

    if ($a_job[mediamosa_job_db::CREATE_STILL] == mediamosa_job_db::CREATE_STILL_TRUE) {
      $a_still_parameters = (empty($a_job[mediamosa_job_db::STILL_PARAMETERS]) ? array() : unserialize($a_job[mediamosa_job_db::STILL_PARAMETERS]));

      mediamosa_job::create_job_still($a_job[mediamosa_job_db::APP_ID], $a_job[mediamosa_job_db::OWNER_ID], FALSE, $a_job[mediamosa_job_db::MEDIAFILE_ID], $a_still_parameters);
    }
  }
}
