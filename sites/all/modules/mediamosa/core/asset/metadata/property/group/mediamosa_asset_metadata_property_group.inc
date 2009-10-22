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
  * The database class of the asset metadata property group module.
  */

class mediamosa_asset_metadata_property_group {

  // ------------------------------------------------------------------ Consts.
  const APP_PREFIX = 'app_';

  // ------------------------------------------------------------------ Functions (public).
  /**
   * Get a property group.
   *
   * @param integer $propgroup_id (unique)
   * @return array
   */
  function property_group_row_get($propgroup_id) {
    return mediamosa_db::db_query(
      'SELECT * FROM #table_name WHERE #propgroup_id = :propgroup_id',
      array(
        '#table_name' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        '#propgroup_id' => mediamosa_asset_metadata_property_group_db::ID,
        ':propgroup_id' => $propgroup_id,
      )
    )->fetchAssoc();
  }

  /**
   * Get a property group by app_id.
   *
   * @param integer $app_id
   * @return array
   */
  function property_group_row_get_with_appid($app_id) {
    assert(is_int($app_id));
    return mediamosa_db::db_query(
      'SELECT * FROM #table_name WHERE #propgroup_id = :propgroup_id',
      array(
        '#table_name' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        '#propgroup_name' => mediamosa_asset_metadata_property_group_db::NAME,
        ':propgroup_name' => self::APP_PREFIX . $app_id,
      )
    )->fetchAssoc();
  }

  /**
   * Create a property group.
   *
   * @param string $name
   *  The name of the property group.
   * @return integer
   *  The new ID of the object.
   */
  function property_group_create($name) {
    return db_insert(mediamosa_asset_metadata_property_group_db::TABLE_NAME)
      ->fields(array(
         mediamosa_asset_metadata_property_group_db::NAME => $name,
         mediamosa_asset_metadata_property_group_db::CREATED => mediamosa::utc_current_timestamp_now(),
      ))
      ->execute();
  }

  /**
   * Create a property group.
   *
   * @param integer $app_id
   *  The name of the property group.
   * @return integer
   *  The new ID of the object.
   */
  function property_group_create_with_appid($app_id) {
    assert(is_int($app_id));
    return property_group_create(self::APP_PREFIX . $app_id);
  }
}
