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
   * Get collections based on given asset_ids.
   *
   * @param array $a_asset_ids
   * @param array $a_fields (collection fields)
   *  [alias] => field
   */
  static public function get_by_asset_ids(array $a_asset_ids, array $a_fields = array()) {

  	if (empty($a_asset_ids)) {
  	  return array();
  	}

  	$query = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'ac');
  	$query->leftjoin(mediamosa_collection_db::TABLE_NAME, 'c', 'ac.asset_id = c.asset_id');
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
}
