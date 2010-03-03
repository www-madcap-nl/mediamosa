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
 * The helper functions of collection module.
 */

class mediamosa_collection {
  // ------------------------------------------------------------------ Static Functions.
  /**
   * Remove collection.
   *
   * @param string $coll_id
   * @return boolean
   */
  static public function delete($coll_id) {
    $query = mediamosa_db::db_delete(mediamosa_collection_db::TABLE_NAME);
    $query->condition(mediamosa_collection_db::ID, $coll_id);
    return $query->execute() ? TRUE : FALSE;
  }

  /**
   * Get multiple collections within an array, with key
   * as coll_id.
   *
   * @param array $a_coll_ids
   */
  static public function getAllAssoc(array $a_coll_ids) {

    $query = mediamosa_db::db_select(mediamosa_collection_db::ID, 'c');
    $query->fields('c');
    $query->condition('c.' . mediamosa_collection_db::ID, $a_coll_ids, 'IN');
    return $query->execute()->fetchAllAssoc(mediamosa_collection_db::ID);
  }

  /**
   * Get the collection.
   *
   * @param string $coll_id
   */
  static public function get($coll_id, $app_id = NULL) {

    $query = mediamosa_db::db_select(mediamosa_collection_db::ID, 'c');
    $query->fields('c');
    $query->condition('c.' . mediamosa_collection_db::ID, $coll_id);

    if (isset($app_id)) {
      $query->condition('c.' . mediamosa_collection_db::APP_ID, $app_id);
    }

    return $query->execute()->fetchAssoc();
  }

  /**
   * Create a collection.
   */
  static public function create($app_id, $owner_id, $group_id = NULL, $private = NULL, $public  = NULL, $category = NULL, $isprivate = NULL, $public_assign = NULL, $is_unappropriate = NULL) {

    // maak een uniek asset_id aan
    $coll_id = mediamosa_db::uuid($app_id);

    // Required.
    $a_fields = array();
    $a_fields[mediamosa_collection_db::ID] = $coll_id;
    $a_fields[mediamosa_collection_db::APP_ID] = $app_id;
    $a_fields[mediamosa_collection_db::OWNER_ID] = $owner_id;

    if (!is_null($group_id)) {
      $a_fields[mediamosa_collection_db::GROUP_ID] = $group_id;
    }
    if (!is_null($private)) {
      $a_fields[mediamosa_collection_db::PRIVATE_] = $private ? mediamosa_collection_db::PRIVATE_TRUE : mediamosa_collection_db::PRIVATE_FALSE;
    }
    if (!is_null($public)) {
      $a_fields[mediamosa_collection_db::PUBLIC_] = $public ? mediamosa_collection_db::PUBLIC_TRUE : mediamosa_collection_db::PUBLIC_FALSE;
    }
    if (!is_null($category)) {
      $a_fields[mediamosa_collection_db::CATEGORY] = $category ? mediamosa_collection_db::CATEGORY_TRUE : mediamosa_collection_db::CATEGORY_FALSE;
    }
    if (!is_null($isprivate)) {
      $a_fields[mediamosa_collection_db::ISPRIVATE] = $isprivate ? mediamosa_collection_db::ISPRIVATE_TRUE : mediamosa_collection_db::ISPRIVATE_FALSE;
    }
    if (!is_null($public_assign)) {
      $a_fields[mediamosa_collection_db::PUBLIC_ASSIGN] = $public_assign ? mediamosa_collection_db::PUBLIC_ASSIGN_TRUE : mediamosa_collection_db::PUBLIC_ASSIGN_FALSE;
    }
    if (!is_null($is_unappropriate)) {
      $a_fields[mediamosa_collection_db::IS_UNAPPROPRIATE] = $is_unappropriate ? mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE : mediamosa_collection_db::IS_UNAPPROPRIATE_FALSE;
    }

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_collection_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();

    return $coll_id;
  }


}
