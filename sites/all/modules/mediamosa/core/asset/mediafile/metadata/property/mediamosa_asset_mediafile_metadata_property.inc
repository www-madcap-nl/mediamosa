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
  * Metadata functions.
  */
class mediamosa_asset_mediafile_metadata_property {
  /**
   * Get the property id.
   * @return int
   */
  static public function get_property_id_cache($prop_name, $type, $allowed_values, $reset = FALSE) {
    static $prop_ids = NULL;

    if (is_null($prop_ids) || $reset) {
      $prop_ids = array();

      $result = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME, 'p')
        ->fields('p', array(
          mediamosa_asset_mediafile_metadata_property_db::ID,
          mediamosa_asset_mediafile_metadata_property_db::NAME,
          mediamosa_asset_mediafile_metadata_property_db::TYPE,
          mediamosa_asset_mediafile_metadata_property_db::ALLOWED_VALUES,
        ))
        ->execute();
      foreach ($result as $row) {
        if ($row[mediamosa_asset_mediafile_metadata_property_db::TYPE] == mediamosa_asset_mediafile_metadata_property_db::TYPE_ENUM) {
          $prop_ids
            [$row[mediamosa_asset_mediafile_metadata_property_db::NAME]]
            [$row[mediamosa_asset_mediafile_metadata_property_db::TYPE]]
            [$row[mediamosa_asset_mediafile_metadata_property_db::ALLOWED_VALUES]] =
            $row[mediamosa_asset_mediafile_metadata_property_db::ID];
        }
        else {
          $prop_ids
            [$row[mediamosa_asset_mediafile_metadata_property_db::NAME]]
            [$row[mediamosa_asset_mediafile_metadata_property_db::TYPE]] =
            $row[mediamosa_asset_mediafile_metadata_property_db::ID];
        }
      }
    }

    if ($type == mediamosa_asset_mediafile_metadata_property_db::TYPE_ENUM) {
      return (!empty($prop_ids[$prop_name][$type][$allowed_values]) ? $prop_ids[$prop_name][$type][$allowed_values] : NULL);
    }
    else {
      return (!empty($prop_ids[$prop_name][$type]) ? $prop_ids[$prop_name][$type] : NULL);
    }
  }

  /**
   * Get the property id. Save the property, if needed.
   * @return int
   */
  static public function get_property_id($prop_name, $type, $allowed_values = NULL) {
    // Get the property id, if exists.
    $prop_id = self::get_property_id_cache($prop_name, $type, $allowed_values);

    if (is_null($prop_id)) {
      // Save the property id.
      $fields = array(
        'prop_name' => $prop_name,
        'type' => $type,
        'allowed_values' => $allowed_values,
        'created' => mediamosa::db_current_timestamp_now(),
        'changed' => mediamosa::db_current_timestamp_now(),
      );
      $prop_id = mediamosa_db::db_insert('mediamosa_asset_mediafile_metadata_property')
        ->fields($fields)
        ->execute();

      // Get the property id. It must exists.
      $prop_id = self::get_property_id_cache($prop_name, $type, $allowed_values, TRUE);
    }

    return $prop_id;
  }
}
