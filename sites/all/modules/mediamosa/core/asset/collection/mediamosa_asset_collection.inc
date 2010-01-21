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
 * The helper functions of asset_collection module.
 */

class mediamosa_asset_collection {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Remove link between asset and collection.
   *
   * @param string $asset_id
   *  When given, remove all links from any colletion where this asset is in.
   *  In combination with coll_id, only one link might be deleted.
   * @param string $coll_id
   *  When given, remove all links from any asset in this collection.
   *  In combination with asset_id, only one link might be deleted.
   * @return boolean
   */
  static public function delete($asset_id = FALSE, $coll_id = FALSE) {

    if (!$asset_id && !$coll_id) {
      // if none is provided, none will be deleted.
      return FALSE;
    }

    // Do the delete.
    $query = mediamosa_db::db_delete(mediamosa_asset_collection_db::TABLE_NAME);

    if ($asset_id) {
      $query->condition(mediamosa_asset_collection_db::ASSET_ID, $asset_id);
    }

    if ($coll_id) {
      $query->condition(mediamosa_asset_collection_db::COLL_ID, $coll_id);
    }

    return $query->execute() ? TRUE : FALSE;
  }

  /**
   * Delete all relations between asset and his collections.
   *
   * @param $app_id
   *  The application ID.
   * @param $user_id
   *  The possible owner.
   * @param $is_app_admin
   *  Global switch for app admin.
   * @param $asset_id
   *  The asset.
   */
  static public function delete_asset_from_all_collections($app_id, $user_id, $is_app_admin, $asset_id) {

    // Asset must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Check if we can delete the links first.
    $a_asset = mediamosa_asset::get($asset_id);

    // Get all the links of this asset and his collection(s).
    $a_coll_ids = mediamosa_db::db_query(
      'SELECT #coll_id FROM {#mediamosa_asset_collection} WHERE #asset_id = :asset_id',
      array(
        '#coll_id' => mediamosa_asset_collection_db::COLL_ID,
        '#mediamosa_asset_collection' => mediamosa_asset_collection_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_collection_db::ASSET_ID,
        ':asset_id' => $asset_id
      )
    )->fetchCol();

    // Get the collections in one go.
    $a_collections = mediamosa_collection::getAllAssoc($a_coll_ids);

    foreach ($a_collections as $a_collection) {
      // Check the user rights to delete the relation.
      try {
        mediamosa_aut::owner_check_collection_assign($app_id, $user_id, $a_asset, $a_collection, $is_app_admin);
      }
      catch (mediamosa_exception_error_access_denied $e) {
        // There is no access right, so skip it
        continue;
      }

      // Get access, remove relation.
      mediamosa_asset_collection::delete($asset_id, $a_collection[mediamosa_collection_db::ID]);
    }
  }

  /**
   * Get collections based on given asset_ids.
   *
   * @param array $a_asset_ids
   *  The asset ids.
   * @param array $a_fields (collection fields)
   *  [alias] => field
   */
  static public function get_by_asset_ids(array $a_asset_ids, array $a_fields = array()) {

    if (empty($a_asset_ids)) {
      return array();
    }

    $query = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'ac');
    $query->leftJoin(mediamosa_collection_db::TABLE_NAME, 'c', 'ac.asset_id = c.asset_id');
    if (count($a_fields)) {
      foreach ($a_fields as $key => $field) {
        $query->addField('c', $field, (is_numeric($key) ? $field : $key));
      }
    }
    else {
      $query->fields();
    }
    $query->condition(mediamosa_asset_collection_db::ASSET_ID, $a_asset_ids, 'IN');
    return $query->execute();
  }

  /**
   * Get collections based on given collection ids.
   * 1.x: _media_management_return_collection_list
   *
   * 2.x: no longer return response object.
   *
   * @param array $a_ids
   *  The collection ids.
   */
  function get_by_ids(array $a_coll_ids) {
    // Build query.
    $query = mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME);
    $query->fields();
    $query->condition(mediamosa_collection_db::ID, $a_coll_ids, 'IN');
    return $query->execute();
  }

  /**
   *
   * Create the link between asset and collection.
   *
   * @param int $app_id
   *  The application ID.
   * @param string $user_id
   *  The user ID.
   * @param bool $is_app_admin
   *  App admin.
   * @param string $asset_id
   *  The asset ID.
   * @param string $coll_id
   *  The coll ID.
   */
  static public function create($app_id, $user_id, $is_app_admin, $asset_id, $coll_id) {
    // Asset must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Check if relation already exist.
    if (!mediamosa_db::db_exists(
      mediamosa_asset_collection_db::TABLE_NAME,
      array(
        mediamosa_asset_collection_db::ASSET_ID => $asset_id,
        mediamosa_asset_collection_db::COLL_ID => $coll_id
      )
    )) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_COLLECTION_ASSET_RELATION_ALREADY_EXISTS);
    }

    $a_asset = mediamosa_asset::get($asset_id);
    $a_collection = mediamosa_collection::get($coll_id);

    // Check if we are owner to add the asset to the collection.
    mediamosa_aut::owner_check_collection_assign($app_id, $user_id, $a_asset, $a_collection, $is_app_admin);

    // If we get here, we can create the link.
    mediamosa_db::db_query(
      'INSERT INTO {#mediamosa_asset_collection} SET #asset_id = :asset_id, #coll_id = :coll_id',
      array(
        '#mediamosa_asset_collection' => mediamosa_asset_collection_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_collection_db::ASSET_ID,
        ':asset_id' => $asset_id,
        '#coll_id' => mediamosa_asset_collection_db::COLL_ID,
        ':coll_id' => $coll_id,
      )
    );
  }

  /**
   * Count the number of collections of given asset.
   *
   * @param string $asset_id
   *  The asset ID.
   */
  static public function count_collections($asset_id) {

    // Asset must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Return the number of found rows.
    $query = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'mac');
    $query->fields('mac', array(mediamosa_asset_collection_db::ID));
    $query->condition(mediamosa_asset_collection_db::ID, $asset_id);
    return $query->countQuery()->execute()->fetchField();
  }
}
