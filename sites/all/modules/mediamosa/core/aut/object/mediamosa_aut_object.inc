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
  * The aut_object functions.
  */

class mediamosa_aut_object {
  // ------------------------------------------------------------------ Static Function.
  /**
   * Returns the join for joining with aut_object table
   *
   * @param enum $aut_type
   *  See mediamosa_aut::AUT_TYPE_*
   * @return string
   */
  static public function join_aut_object_get($aut_type) {
    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        $on = 'mf.mediafile_id';
        break;
      case mediamosa_aut::AUT_TYPE_COLLECTION:
        $on = 'c.coll_id';
        break;
      case mediamosa_aut::AUT_TYPE_ASSET:
        $on = 'a.asset_id';
        break;
      default:
        throw new mediamosa_exception_program_error('Unknown type used in mediamosa_aut_object::join_aut_object_get');
    }

    return strtr(
      "LEFT JOIN {#aut_object} AS aut_obj ON aut_obj.aut_object_type = '#aut_type' AND #on = aut_obj.aut_object_id",
      array(
        '#aut_object' => mediamosa_aut_object_db::TABLE_NAME,
        '#aut_type' => $aut_type,
        '#on' => $on
      )
    );
  }

  /**
   * Delete all connection based on aut_type and aut_id.
   *
   * @param string $aut_type
   * @param integer $aut_id
   */
  function delete($aut_type, $aut_id) {

    // Delete all connections.
    mediamosa_db::db_delete(mediamosa_aut_object_db::TABLE_NAME)
      ->condition(mediamosa_aut_object_db::AUT_TYPE, $aut_type)
      ->condition(mediamosa_aut_object_db::AUT_ID, $aut_id)
      ->execute();
  }

  /**
   * Creates a link between object and aut_name
   *
   * @param string $aut_object_id
   * @param string $aut_type
   * @param string $aut_object_type
   * @param integer $aut_id
   * @param array $a_rights (reserved)
   */
  function link_to_object($aut_object_id, $aut_type, $aut_object_type, $aut_id, $a_rights = array(mediamosa_aut::RIGHT_ACCESS)) {
    // Here so the var is used and Zend doesn't complain).
    assert(is_array($a_rights));

    // Master-slave safe now.
    try {
      mediamosa_db::db_query("INSERT INTO {mediamosa_aut_object} SET aut_object_type = '%s', aut_object_id = '%s', aut_type = '%s', aut_id= %d", $aut_object_type, $aut_object_id, $aut_type, $aut_id);
    }
    catch (PDOException $e) {
      assert($e);
      // ignore, double row.
    }
  }
}
