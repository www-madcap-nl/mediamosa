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
  function delete($asset_id = FALSE, $coll_id = FALSE) {

    if (!$asset_id && !$coll_id) {
      // if none is provided, none will be deleted.
      return FALSE;
    }

    // Do the the delete.
    $query = mediamosa_db::db_delete(mediamosa_asset_collection_db::TABLE_NAME);

    if ($asset_id) {
      $query->condition(mediamosa_asset_collection_db::ASSET_ID, $asset_id);
    }

    if ($coll_id) {
      $query->condition(mediamosa_asset_collection_db::COLL_ID, $coll_id);
    }

    return $query->execute() ? TRUE : FALSE;
  }
}
