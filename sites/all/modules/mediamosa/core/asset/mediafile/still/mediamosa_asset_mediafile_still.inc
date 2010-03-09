<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
  * Stills are now mediafiles, so code for still have been moved
  * under mediafile.
  */

class mediamosa_asset_mediafile_still {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Create a still
   *
   * @param string $asset_id
   * @param string $asset_parent_id
   * @param string $still_id
   * @param integer $app_id
   * @param string $owner_id
   * @param string $group_id
   * @param integer $order
   * @param boolean $still_default
   * @param array $still_parameters
   * @param integer $sec
   * @param string $mediafile_id_source
   * @param string $tag
   */
  static public function create($asset_id, $asset_parent_id, $still_id, $app_id, $owner_id, $group_id, $order, $still_default = TRUE, $a_still_parameters = NULL, $sec = 0, $mediafile_id_source = '', $tag = '') {

    // Get path to still.
    $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);

    // Check if still exists.
    if (!file_exists($still_filename)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    $a_size = getimagesize($still_filename);

    // Set width, height.
    $width = $a_size[0];
    $height = $a_size[1];

    // Mime type.
    $mime_type = $a_size['mime'];

    // Set file_type of still based on the mime type of the image.
    $pos = strrpos($mime_type, '/');
    $file_extension = ($pos === FALSE ? '' : substr($mime_type, $pos + 1));

    // Get file size of still.
    $file_size = filesize($still_filename);

    if ($still_default) {
      // Clear the earlier default mark on the video (media) file.
      mediamosa_db::db_query(
        'UPDATE {#mediamosa_asset_mediafile_metadata} AS mm
         JOIN {#mediamosa_asset_mediafile} AS m ON m.#mediafile_id = mm.#mediafile_id
         SET mm.#still_default = :still_default_false
         WHERE m.#mediafile_id_source = :mediafile_id_source AND m.#is_still = :is_still_true',
        array(
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#still_default' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT,
          ':still_default_false' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_FALSE,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          ':mediafile_id_source' => $mediafile_id_source,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE
        )
      );
    }

    // Insert into Mediafile.
    $a_fields = array(
        mediamosa_asset_mediafile_db::ID => $still_id,
        mediamosa_asset_mediafile_db::ASSET_ID => $asset_id,
        mediamosa_asset_mediafile_db::APP_ID => $app_id,
        mediamosa_asset_mediafile_db::OWNER_ID => $owner_id,
        mediamosa_asset_mediafile_db::GROUP_ID => $group_id,
        mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => mediamosa_configuration_storage::mount_point_get(),
        mediamosa_asset_mediafile_db::FILE_EXTENSION => $file_extension,
        mediamosa_asset_mediafile_db::ASSET_ID_ROOT => $asset_parent_id,
        mediamosa_asset_mediafile_db::IS_STILL => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => $mediafile_id_source,
        mediamosa_asset_mediafile_db::TAG => $tag
    );

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();

    // Insert into Mediafile.
    $a_fields = array(
      mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => $still_id,
      mediamosa_asset_mediafile_metadata_db::WIDTH => $width,
      mediamosa_asset_mediafile_metadata_db::HEIGHT => $height,
      mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE => '',
      mediamosa_asset_mediafile_metadata_db::FILESIZE => $file_size,
      mediamosa_asset_mediafile_metadata_db::MIME_TYPE => $mime_type,
      mediamosa_asset_mediafile_metadata_db::STILL_TIME_CODE => $sec,
      mediamosa_asset_mediafile_metadata_db::STILL_ORDER => $order,
      mediamosa_asset_mediafile_metadata_db::STILL_FORMAT => $file_extension,
      mediamosa_asset_mediafile_metadata_db::STILL_TYPE => (empty($a_still_parameters['still_type']) ? mediamosa_asset_mediafile_metadata_db::STILL_TYPE_NORMAL : $a_still_parameters['still_type']),
      mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT => $still_default ? mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_TRUE : mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_FALSE,
    );

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert in metadata.
    mediamosa_db::db_insert(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Change the order of an still
   *
   * @param string $asset_id
   * @param string $mediafile_id
   * @param string $still_id
   * @param integer $order
   */
  static public function set_still_order($asset_id, $mediafile_id, $still_id, $order) {
    mediamosa::log(
      "Still order: asset_id = @asset_id; mediafile_id = @mediafile_id; still_id = @still_id; order = @order;",
      array(
        '@asset_id' => $asset_id,
        '@mediafile_id' => $mediafile_id,
        '@still_id' => $still_id,
        '@order' => $order
      )
    );

    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset_mediafile_metadata} AS mfmd
       INNER JOIN {#mediamosa_asset_mediafile} AS m ON m.#mediafile_id = mfdm.mediafile_id
       SET mfmd.#still_order = :still_order
       WHERE mfmd.#mediafile_id = :mediafile_id AND m.#asset_id_root = :asset_id_root AND m.#is_still = :is_still_true',
      array(
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#still_order' => mediamosa_asset_mediafile_metadata_db::STILL_ORDER,
        ':still_order' => $order,
        ':mediafile_id' => $still_id,
        '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
        ':asset_id_root' => $asset_id,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE
      )
    );
  }

  /**
   * Find default still or at least the first one you find.
   *
   * @param string $asset_id
   * @return string
   */
  static public function find_default($asset_id) {

    // Try default 1st.
    $still_id = mediamosa_db::db_query_range(
      'SELECT m.#mediafile_id FROM {#mediamosa_asset_mediafile} AS m
      JOIN {#mediamosa_asset_mediafile_metadata} AS mm USING(#mediafile_id)
      WHERE m.#asset_id_root = :asset_id_root AND m.#is_still = :is_still_true AND mm.#still_default = :still_default_true
      ORDER BY mm.#metadata_id DESC',
      0,
      1,
      array(
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
        ':asset_id_root' => $asset_id,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        '#still_default' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT,
        ':still_default_true' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_TRUE,
        '#metadata_id' => mediamosa_asset_mediafile_metadata_db::ID
      )
    )->fetchField();

    // Not found? Try any.
    if (!$still_id) {
      $still_id = mediamosa_db::db_query_range(
        "SELECT m.#mediafile_id FROM {#mediamosa_asset_mediafile} AS m
        JOIN {#mediamosa_asset_mediafile_metadata} AS mm USING(#mediafile_id)
        WHERE m.#asset_id_root = :asset_id_root AND m.#is_still = :is_still_true
        ORDER BY mm.#metadata_id DESC",
        0,
        1,
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
          ':asset_id_root' => $asset_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
          '#metadata_id' => mediamosa_asset_mediafile_metadata_db::ID
        )
      )->fetchField();
    }

    // Check if the still id is in the database
    if (!$still_id) {
      // There isn't any still
      return;
    }

    // Check if the still is ok.
    self::check($still_id);

    // Return our ID.
    return $still_id;
  }

  /**
   * Check if still file is available.
   *
   * @param string $still_id
   */
  static public function check($still_id) {
    // controleer of de still op het filesystem staat
    $filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);
    if (!file_exists($filename)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FILE_NOT_FOUND, array('@filename' => $filename));
    }
  }

  /**
   * Delete a still from database and file.
   * 1.x _media_management_delete_still
   *
   * @param $asset_id
   *  The asset_id. (will remove all stills from this asset_id).
   * @param $mediafile_id
   *  Remove specific from Mediafile parent.
   * @param $still_id
   *  Remove specific still.
   */
  static public function delete($asset_id, $mediafile_id = '', $still_id = '') {

    // Can not be set both.
    assert($still_id != '' && $mediafile_id != '');

    // Get the mediafile stills we need to remove.
    if ($still_id != '') {
      // Get specific still.
      $result = mediamosa_db::db_query(
        'SELECT #mediafile_id AS still_id, #app_id, #sannas_mount_point, #mediafile_id_source FROM {#mediamosa_asset_mediafile} WHERE #mediafile_id = :mediafile_id AND #is_still = :is_still_true AND #asset_id_root = :asset_id_root',
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
          '#sannas_mount_point' => mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          ':mediafile_id' => $still_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
          '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
          ':asset_id_root' => $asset_id,
        )
      );
    }
    elseif ($mediafile_id != '') {
      // Get all stills of mediafile.
      $result = mediamosa_db::db_query(
        'SELECT s.#mediafile_id AS still_id, s.#app_id, s.#sannas_mount_point, s.#mediafile_id_source FROM {#mediamosa_asset_mediafile} AS s
         INNER JOIN {#mediamosa_asset_mediafile} AS m ON m.#asset_id = s.#asset_id
         WHERE m.#mediafile_id = :mediafile_id AND m.#asset_id_root = :asset_id_root AND s.#asset_id_root = :asset_id_root AND s.#mediafile_id_source = :mediafile_id_source AND s.#is_still = :is_still_true',
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
          '#sannas_mount_point' => mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
          ':mediafile_id' => $mediafile_id,
          '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
          ':asset_id_root' => $asset_id,
          ':mediafile_id_source' => $mediafile_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        )
      );
    }
    else {
      // Get all stills from asset.
      $result = mediamosa::db_query(
        'SELECT #mediafile_id AS still_id, #app_id, #sannas_mount_point, #mediafile_id_source FROM {#mediamosa_asset_mediafile} WHERE #asset_id_root = :asset_id_root AND #is_still = :is_still_true',
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
          '#sannas_mount_point' => mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
          ':asset_id_root' => $asset_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        )
      );
    }

    // Collect the mediafiles we need to delete.
    $a_mediafile_ids = array();
    foreach ($result as $a_row) {
      // Build the path and filename of the still.
      $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($a_row['still_id'], $a_row[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]);
      mediamosa::log('Deleting still: @location', array('@location' => $still_filename));

      // Remove the file.
      mediamosa_io::unlink($still_filename);

      // Collect the mediafile id of the still.
      $a_mediafile_ids[] = $a_row['still_id'];

      if ($mediafile_id == '') {
        $mediafile_id = $a_row[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE];
      }
    }

    // Remove the stills from the database.
    if (!empty($a_mediafile_ids)) {
      // Remove our metadata.
      $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
      $query->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $a_mediafile_ids, 'IN');
      $query->execute();

      // Remove the mediafiles.
      $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_db::TABLE_NAME);
      $query->condition(mediamosa_asset_mediafile_db::ID, $a_mediafile_ids, 'IN');
      $query->execute();
    }

    // Happens when there where no stills to delete. (check added 2.x)
    if (!isset($mediafile_id)) {
      return;
    }

    // Is there a default still ?
    $mediafile_id_default = mediamosa_db::db_query_range(
      'SELECT mf.#mediafile_id FROM {#mediamosa_asset_mediafile_metadata} AS mfmd
       INNER JOIN {#mediamosa_asset_mediafile} AS mf ON mf.#mediafile_id = mfmd.#mediafile_id
       WHERE mfmd.#still_default = :still_default_true AND mf.#asset_id_root = :asset_id_root AND mf.#is_still = :is_still_true AND mf.#mediafile_id_source = :mediafile_id_source',
      0,
      1,
      array(
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
        ':asset_id_root' => $asset_id,
        '#still_default' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT,
        ':still_default_true' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_TRUE,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
        ':mediafile_id_source' => $mediafile_id,
      )
    )->fetchField();

    // No default, create one.
    if (!$mediafile_id_default) {
      $mediafile_id_default = mediamosa_db::db_query_range(
        'SELECT mf.#mediafile_id FROM {#mediamosa_asset_mediafile_metadata} AS mfmd
         INNER JOIN {#mediamosa_asset_mediafile} AS mf ON mf.#mediafile_id = mfmd.#mediafile_id
         WHERE mf.#asset_id_root = :asset_id_root AND mf.#is_still = :is_still_true AND mf.#mediafile_id_source = :mediafile_id_source
         ORDER BY mfmd.#still_order ASC',
        0,
        1,
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
          ':asset_id_root' => $asset_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          ':mediafile_id_source' => $mediafile_id,
          '#still_order' => mediamosa_asset_mediafile_metadata_db::STILL_ORDER,
        )
      )->fetchField();

      if ($mediafile_id_default) {
        // Found new default? Then set it as default.
        mediamosa_db::db_query(
          'UPDATE {#mediamosa_asset_mediafile_metadata} SET #still_default = #still_default_true WHERE #mediafile_id = :mediafile_id',
          array(
            '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
            '#still_default' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT,
            ':still_default_true' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_TRUE,
            '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
            ':mediafile_id' => $mediafile_id_default
          )
        );
      }
    }
  }

  /**
   * Set the default still.
   * 1.x: _media_management_set_still_default
   * FIXME: the mediafile id unused.
   * 2.x: no longer returns TRUE by default.
   *
   * @param string $asset_id
   *  The asset ID.
   * @param string $mediafile_id (unused!)
   *  The mediafile ID.
   * @param string $still_id
   *  The still ID.
   */
  static public function set_still_default($asset_id, $mediafile_id, $still_id) {
    mediamosa::log(
      'Setting default still: asset_id = @asset_id; mediafile_id = @mediafile_id; still_id = @still_id',
      array(
        '@asset_id' => $asset_id,
        '@mediafile_id' => $mediafile_id,
        '@still_id' => $still_id,
      )
    );

    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset_mediafile_metadata} AS mfmd
       INNER JOIN {#mediamosa_asset_mediafile} AS mf ON mf.#mediafile_id = mfmd.#mediafile_id
       SET mfmd.#still_default = IF(mfmd.#mediafile_id = :mediafile_id, #still_default_true, #still_default_false)
       WHERE mf.#asset_id_root = :asset_id_root AND m.#is_still = #is_still_true',
      array(
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        ':mediafile_id' => $still_id,
        '#still_default' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT,
        '#still_default_true' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_TRUE,
        '#still_default_false' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_FALSE,
        '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
        ':asset_id_root' => $asset_id,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
      )
    );
  }
}
