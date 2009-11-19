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
  * The aut_app_master_slave functions.
  */
class mediamosa_aut_app_master_slave {
  /**
   * Returns the join for joining with aut_app_master slave table
   *
   * @param enum $aut_type
   *  See mediamosa_aut::AUT_TYPE_*
   * @return string
   */
  static public function join_aut_app_master_slave_get($aut_type) {
    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIUM:
        $on = 'mf.mediafile_id';
        break;
      case mediamosa_aut::AUT_TYPE_COLLECTION:
        $on = 'c.coll_id';
        break;
      case mediamosa_aut::AUT_TYPE_ASSET:
        $on = 'a.asset_id';
        break;
      default:
        assert(0);
        return '';
    }

    return strtr(
      "LEFT JOIN {#aut_app_master_slave} AS aut_ms ON aut_ms.aut_object_type='%s' AND %s = aut_ms.aut_object_id",
      array(
        '#aut_app_master_slave' => mediamosa_aut_app_master_slave_db::TABLE_NAME,
        '#aut_object_type' => $aut_type,
        '#on' => $on
      )
    );
  }

  /**
   * Create a master - slave record.
   *
   * @param string $aut_object_id
   * @param string $aut_object_type
   * @param integer $app_id
   * @param integer $app_id_slave
   * @param array $a_rights (reserved)
   */
  function create($aut_object_id, $aut_object_type, $app_id, $app_id_slave, $a_rights = array(VPX_ACL_RIGHT_ACCESS)) {

    // Here to sadisify Zend about unused var.
    assert(is_array($a_rights));

    // Make sure we dont allow that app ids of master and slave are the same.
    if ($app_id == $app_id_slave) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MASTERSLAVE_OWN_APP);
    }

    // Master-slave safe now.
    try {
      mediamosa_db::db_query("INSERT INTO {mediamosa_aut_app_master_slave} SET aut_object_type='%s', aut_object_id='%s', app_master_id=%d, app_slave_id=%d", $aut_object_type, $aut_object_id, $app_id, $app_id_slave);
    }
    catch (PDOException $e) {
      assert($e);
      // ignore, double row.
    }
  }

  /**
   * Return the app_id(s) of EGAs that are slaves of my supplied EGA (master)
   *
   *
   * @param integer $app_id
   *  The app ID.
   * @param string $aut_type
   *  The type of acl (mediamosa_acl::AUT_TYPE_*)
   * @param string $object_id (optional)
   *  The ID of object you want to know slaves on.
   * @return array
   *  returns array with info about the slaves
   */
  function slave_get($mixed_app_id, $aut_type, $object_id = NULL) {
    $a_app_id = $mixed_app_id;
    if (!is_array($a_app_id)) {
      $a_app_id = array($a_app_id);
    }

    if (isset($object_id)) {
      $db_result = mediamosa_db::db_query("SELECT app_slave_id FROM {mediamosa_aut_app_master_slave} WHERE aut_object_type='%s' AND aut_object_id = '%s' AND app_master_id IN(%s) GROUP BY app_slave_id", $aut_type, $object_id, implode(",", $a_app_id));
    }
    else {
      $db_result = mediamosa_db::db_query("SELECT app_slave_id FROM {mediamosa_aut_app_master_slave} WHERE aut_object_type='%s' AND app_master_id IN(%s) GROUP BY app_slave_id", $aut_type, implode(",", $a_app_id));
    }

    $array_result = array();
    foreach ($db_result as $a_row) {
      $array_result[$a_row["app_slave_id"]] = array("app_id" => $a_row["app_slave_id"]);
    }

    return $array_result;
  }


}
