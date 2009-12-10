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
   * 1.x vpx_acl_join_aut_app_master_slave_get
   *
   * @param enum $aut_type
   *  See mediamosa_aut::AUT_TYPE_*
   * @return string
   */
  static public function join_aut_app_master_slave_get($aut_type) {
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
        assert(0);
        return '';
    }

    return strtr(
      "LEFT JOIN {#aut_app_master_slave} AS aut_ms ON aut_ms.aut_object_type = '@aut_object_type' AND @on = aut_ms.aut_object_id",
      array(
        '#aut_app_master_slave' => mediamosa_aut_app_master_slave_db::TABLE_NAME,
        '@aut_object_type' => $aut_type,
        '@on' => $on
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
  function create($aut_object_id, $aut_object_type, $app_id_master, $app_id_slave, array $a_rights = array(VPX_ACL_RIGHT_ACCESS)) {

    // Here to sadisify Zend about unused var.
    assert(is_array($a_rights));

    // Make sure we dont allow that app ids of master and slave are the same.
    if ($app_id_master == $app_id_slave) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MASTERSLAVE_OWN_APP);
    }

    // Master-slave safe now.
    try {
      mediamosa_db::db_query(
        'INSERT INTO {#mediamosa_aut_app_master_slave} SET #aut_object_type= :aut_object_type, #aut_object_id = :aut_object_id, #app_id_master = :app_id_master, #app_id_slave = :app_id_slave',
        array(
	        '#aut_app_master_slave' => mediamosa_aut_app_master_slave_db::TABLE_NAME,
	        '#aut_object_type' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE,
	        ':aut_object_type' => $aut_object_type,
	        '#aut_object_id' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_ID,
	        ':aut_object_id' => $aut_object_id,
	        '#app_id_master' => mediamosa_aut_app_master_slave_db::APP_ID_MASTER,
	        ':app_id_master' => $app_id_master,
          '#app_id_slave' => mediamosa_aut_app_master_slave_db::APP_ID_SLAVE,
          ':app_id_slave' => $app_id_slave
        )
      );
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
   * @param array/integer $mixed_app_id
   *  The app ID array or integer.
   * @param string $aut_type
   *  The type of acl (mediamosa_acl::AUT_TYPE_*)
   * @param string $object_id (optional)
   *  The ID of object you want to know slaves on.
   * @return array
   *  returns array with info about the slaves
   */
  function slave_get($mixed_app_id, $aut_object_type, $object_id = NULL) {
    $a_app_id = is_array($mixed_app_id) ? $mixed_app_id : array($mixed_app_id);

    if (isset($object_id)) {
      $db_result = mediamosa_db::db_query("SELECT app_id_slave FROM {mediamosa_aut_app_master_slave} WHERE aut_object_type='%s' AND aut_object_id = '%s' AND app_id_master IN(%s) GROUP BY app_id_slave", $aut_object_type, $object_id, implode(",", $a_app_id));
    }
    else {
      $db_result = mediamosa_db::db_query("SELECT app_id_slave FROM {mediamosa_aut_app_master_slave} WHERE aut_object_type='%s' AND app_id_master IN(%s) GROUP BY app_id_slave", $aut_object_type, implode(",", $a_app_id));
    }

    $array_result = array();
    foreach ($db_result as $a_row) {
      $array_result[$a_row["app_id_slave"]] = array("app_id" => $a_row["app_id_slave"]);
    }

    return $array_result;
  }

  /**
   * Return the app_id of applications (master) of my supplied application (slave)
   *
   * @param integer $slave_app_id
   *  The slave app ID.
   * @param string $aut_object_type
   *  The type of acl (mediamosa_acl::AUT_TYPE_*)
   * @param string $aut_object_id (optional)
   *  The ID of object you want to know slaves on.
   * @return integer
   *  returns master app_id.
   */
  static public function master_get($app_id_slave, $aut_object_type, $aut_object_id) {

  	static $a_app_id_masters = array();

  	if (isset($a_app_id_masters[$app_id_slave][$aut_object_type][$aut_object_id])) {
  		return $a_app_id_masters[$app_id_slave][$aut_object_type][$aut_object_id];
  	}

    $result = mediamosa_db::db_query(
      'SELECT #app_master_id FROM {#aut_app_master_slave} WHERE #aut_object_type = :aut_object_type AND #aut_object_id = :aut_object_id AND #app_id_master > 0 AND #app_id_slave = :app_id_slave',
      array(
        '#aut_app_master_slave' => mediamosa_aut_app_master_slave_db::TABLE_NAME,
        '#aut_object_type' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE,
        ':aut_object_type' => $aut_object_type,
        '#aut_object_id' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_ID,
        ':aut_object_id' => $aut_object_id,
        '#app_id_master' => mediamosa_aut_app_master_slave_db::APP_ID_MASTER,
        '#app_id_slave' => mediamosa_aut_app_master_slave_db::APP_ID_SLAVE,
        ':app_id_slave' => $app_id_slave
      )
    )->fetchField();

    // Default none.
    $a_app_id_masters[$app_id_slave][$aut_object_type][$aut_object_id] = array();

    // Fill it.
    foreach ($result as $a_row) {
      $a_app_id_masters[$app_id_slave][$aut_object_type][$aut_object_id][] = $a_row[mediamosa_aut_app_master_slave_db::APP_ID_MASTER];
    }

    // Return possible masters.
    return $a_app_id_masters[$app_id_slave][$aut_object_type][$aut_object_id];
  }
}
