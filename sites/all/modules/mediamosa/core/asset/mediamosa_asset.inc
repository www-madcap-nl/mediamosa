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
 * The helper functions of asset module.
 */

class mediamosa_asset {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Preprocess the asset output.
   *
   * @param array $a_input
   * @param array $a_metadata_definitions_full
   * @return array
   */
  static public function process_asset_output($a_asset, $a_metadata_definitions_full) {
    foreach ($a_metadata_definitions_full as $key => $value) {
      $a_asset[$value['propgroup_name']][$key] = NULL;
    }

    unset(
      $a_asset['parent_id'],
      $a_asset['asset_property_value'],
      $a_asset['asset_property_name'],
      $a_asset['asset_property_group_name']
    );

    return $a_asset;
  }

  /**
   * Remove asset from database.
   *
   * @param string $asset_id
   *
   * Note: does not return boolean, will throw exception when failure.
   *
   * Big change from 1.x; it will now remove mediafiles.
   */
  static public function delete($asset_id) {

    // @todo: start of transaction?

    // Remove the asset from each collection.
    mediamosa_asset_collection::delete($asset_id);

    // Remove all metadata of the asset.
    $query = db_delete(mediamosa_asset_metadata_db::TABLE_NAME);
    $query->condition(mediamosa_asset_metadata_db::ASSET_ID, $asset_id);
    $query->execute();

    // Remove our stills.

    // Delete the mediafiles.
    mediamosa_asset_mediafile::delete($asset_id);

    // @todo: stats
    //mediamosa_statistics::delete_by_asset_id($asset_id)
    // db_query("DELETE FROM {statistics_stream_request} WHERE asset_id = '%s'", $asset_id);

    // Find my children.
    $result = mediamosa_db::db_query(
      "SELECT asset_id, app_id FROM {#mediamosa_asset} WHERE parent_id = :asset_id",
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        ':asset_id' => $asset_id
      )
    );

    // Foreach my children and call myself again.
    foreach ($result as $child_asset_id) {
      mediamosa_asset::delete($child_asset_id);
    }

    // Now we removed our children, lets remove us.
    $app_id = mediamosa_db::db_query(
      "SELECT #app_id FROM {#mediamosa_asset} WHERE #asset_id = :asset_id",
      array(
        '#app_id' => mediamosa_asset_db::APP_ID,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    )->fetchField();

    // Create entry in the mediamosa_asset_delete table.
    mediamosa_db::db_query(
      "REPLACE LOW_PRIORITY {#mediamosa_asset_delete} (#asset_id, #app_id, #videotimestampmodified) VALUES (:asset_id, :app_id, NOW())",
      array(
        '#mediamosa_asset_delete' => mediamosa_asset_delete_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_delete_db::ID,
        '#app_id' => mediamosa_asset_delete_db::APP_ID,
        '#videotimestampmodified' => mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED,
        ':asset_id' => $asset_id,
        ':app_id' => $app_id
      )
    );

    $query = mediamosa_db::db_delete(mediamosa_asset_db::TABLE_NAME);
    $query->condition(mediamosa_asset_db::ID, $asset_id);
    $query->execute();
  }

  /**
   * Update the normalized fields in asset table.
   *
   * @param string $mediafile_id
   */
  static public function update_asset_info($mediafile_id) {

    // Find top asset.
    $asset_id_root = mediamosa_db::db_query(
      "SELECT #asset_id_root FROM {#mediamosa_asset_mediafile} WHERE #mediafile_id = :mediafile_id",
      array(
        '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        ':mediafile_id' => $mediafile_id,
      )
    )->fetchField();

    // Find the length of the first mediafile.
    $query = db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->fields('mfm', array(mediamosa_asset_mediafile_metadata_db::FILE_DURATION, mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE));
    $query->join(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', "mf.mediafile_id = mfm.mediafile_id AND mf.is_original_file = 'TRUE'");
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID_ROOT, $asset_id_root);
    $query->range(0, 1);
    $mediafile_info = $query->execute()->fetchAssoc();

    // zet de lengte van de eerste originele mediafile in de parent_asset
    if ($mediafile_info && $mediafile_info['file_duration'] !== NULL) {
      mediamosa_db::db_query(
        "UPDATE {#mediamosa_asset} SET #mediafile_duration = :mediafile_duration, #mediafile_container_type = :mediafile_container_type WHERE #asset_id = :asset_id",
        array(
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#mediafile_duration' => mediamosa_asset_db::MEDIAFILE_DURATION,
          ':mediafile_duration' => $mediafile_info[mediamosa_asset_mediafile_metadata_db::FILE_DURATION],
          '#mediafile_container_type' => mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE,
          ':mediafile_container_type' => $mediafile_info[mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE],
          '#asset_id' => mediamosa_asset_db::ID,
          ':asset_id' => $asset_id_root,
        )
      );
    }

    // Set the external.
    mediamosa_asset::update_asset_info_is_external($mediafile_id, $asset_id_root);
  }

  /**
   * Update the normalized field in asset for is_external flag.
   *
   * @param string $mediafile_id
   * @param string $asset_id_root
   */
  static public function update_asset_info_is_external($mediafile_id, $asset_id_root = NULL) {
  // zoek de bijbehorende asset_id op
    if (!isset($asset_id_root)) {
      // Find top asset.
      $asset_id_root = mediamosa_db::db_query(
        "SELECT #asset_id_root FROM {#mediamosa_asset_mediafile} WHERE #mediafile_id = :mediafile_id",
        array(
          '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          ':mediafile_id' => $mediafile_id,
        )
      )->fetchField();
    }

    $a_mediafile = mediamosa_db::db_query_range(
      "SELECT #uri, #filename FROM {#mediamosa_asset_mediafile} WHERE #is_original_file = :TRUE AND #asset_id_root = :asset_id_root",
      0, // From.
      1, // Count.
      array(
        '#uri' => mediamosa_asset_mediafile_db::URI,
        '#filename' => mediamosa_asset_mediafile_db::FILENAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#is_original_file' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
        ':TRUE' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
        '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
        ':asset_id_root' => $asset_id_root,
      )
    )->fetchAssoc();

    if ($a_mediafile) {
      $uri = $a_mediafile[mediamosa_asset_mediafile_db::URI];
      $filename = $a_mediafile[mediamosa_asset_mediafile_db::FILENAME];

      // Set is_external && is_empty_asset
      mediamosa_db::db_query(
        "UPDATE {#mediamosa_asset} SET #is_external = :is_external, #is_empty_asset = :is_empty_asset WHERE #asset_id = :asset_id",
        array(
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#is_external' => mediamosa_asset_db::IS_EXTERNAL,
          ':is_external' => trim($uri) != '' ? mediamosa_asset_db::IS_EXTERNAL_TRUE : mediamosa_asset_db::IS_EXTERNAL_FALSE,
          '#is_empty_asset' => mediamosa_asset_db::IS_EMPTY_ASSET,
          ':is_empty_asset' => trim($uri) != '' || trim($filename) != '' ? mediamosa_asset_db::IS_EMPTY_ASSET_TRUE : mediamosa_asset_db::IS_EMPTY_ASSET_FALSE,
          '#asset_id' => mediamosa_asset_db::ID,
          ':asset_id' => $asset_id_root,
        )
      );
    }
  }
}
