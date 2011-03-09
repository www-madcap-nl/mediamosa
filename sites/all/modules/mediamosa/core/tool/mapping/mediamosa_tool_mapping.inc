<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * Tool mapping functions.
  */

class mediamosa_tool_mapping {

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Get tool mapping with tool.
   *
   * @param string $tool
   * @param array $fields
   */
  static public function get($tool, array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_tool_mapping_db::TABLE_NAME, 'tm');
    $query
      ->condition('tm.' . mediamosa_tool_mapping_db::TOOL, $tool)
      ->fields('tm', $fields);
    $result = $query->execute();

    return $result;
  }

  /**
   * Get all tool mapping.
   *
   * @param array $fields
   */
  static public function get_all(array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_tool_mapping_db::TABLE_NAME, 'tm');
    $query
      ->fields('tm', $fields);
    $result = $query->execute();

    return $result;
  }

  /**
   * Get asset metadata in special output format for tool mapping.
   */
  static public function get_asset_metadata_display_name($prop_id) {
    $metadata_property = mediamosa_asset_metadata_property::property_get_row($prop_id);
    $metadata_property_group = mediamosa_asset_metadata_property_group::property_group_row_get($metadata_property[mediamosa_asset_metadata_property_db::PROPGROUP_ID]);
    $asset_metadata = $metadata_property[mediamosa_asset_metadata_property_db::NAME] . ' (' . $metadata_property_group[mediamosa_asset_metadata_property_group_db::NAME] . ')';

    return $asset_metadata;
  }
}
