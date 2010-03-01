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

    $query = mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c');
    $query->fields('c');
    $query->condition('c.' . mediamosa_collection_db::ID, $coll_id);

    if (isset($app_id)) {
      $query->condition('c.' . mediamosa_collection_db::APP_ID, $app_id);
    }

    return $query->execute()->fetchAssoc();
  }
}
