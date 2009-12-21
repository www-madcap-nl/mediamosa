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
  * The APP authentication class.
  */

class mediamosa_app_authentication {
  // ------------------------------------------------------------------ Consts.
  // End User Application authentication settings
  const APP_AUTH_CONTEXT = 'vpx';
  const APP_AUTH_SECRET_PHRASE = '41d0c72bd73afaa2c207064d81d5a3d9';
  const APP_AUTH_SERVER_GUID = 'server_guid';

  // ------------------------------------------------------------------ Functions (static).
  /**
   * Get the shared key for the application.
   *
   * @param string $name
   * @return string
   */
  function get_shared_key($name) {
    return mediamosa_db::db_query(
      "SELECT #shared_key FROM {#table_name} WHERE #name = :name AND #active = 'TRUE'",
      array(
        '#shared_key' => mediamosa_app_db::SHARED_KEY,
        '#table_name' => mediamosa_app_db::TABLE_NAME,
        '#name' => mediamosa_app_db::NAME,
        ':name' => $name,
        '#active' => mediamosa_app_db::ACTIVE,
      )
    )->fetchField();
  }

  /**
   * Test if application is enabled (active).
   *
   * @param string $name
   * @return boolean
   */
  function is_app_active($name) {
    $query = mediamosa_db::db_query(
      "SELECT #active FROM {#table} WHERE #name = :name",
      array(
        '#active' => mediamosa_app_db::ACTIVE,
        '#table' => mediamosa_app_db::TABLE_NAME,
        '#name' => mediamosa_app_db::NAME,
        ':name' => $name,
      )
    )->fetchAssoc();
    return $query[mediamosa_app_db::ACTIVE] == 'TRUE';
  }


  /**
   * Get the app.
   *
   * @param string $name
   * @return array
   */
  function get_app($name) {
    $query = mediamosa_db::db_query(
      "SELECT * FROM {#table_name} WHERE #name = :name",
      array(
        '#name' => mediamosa_app_db::NAME,
        '#table_name' => mediamosa_app_db::TABLE_NAME,
        ':name' => $name,
      )
    )->fetchAssoc();
    return $query;
  }

  /**
   * A (cookie) ID authentication method.
   *
   * @param string $op
   * @param string $param1
   * @param string $param2
   * @return string
   */
  function dbus_cookie_sha1($op, $param1, $param2) {

    switch ($op) {
      case 'AUTH':
        if (!mediamosa_app_authentication::is_app_active($param2)) {
          return 'ERROR invalid account';
        }

        drupal_session_regenerate();
        $_SESSION['mediamosa_dbus']['username'] = $param2;
        $_SESSION['mediamosa_dbus']['cookie_id'] = 0;
        $_SESSION['mediamosa_dbus']['challenge'] = md5(microtime() . self::APP_AUTH_SECRET_PHRASE);

        return
          'DATA '. self::APP_AUTH_CONTEXT . ' ' .
          $_SESSION['mediamosa_dbus']['cookie_id'] . ' ' .
          $_SESSION['mediamosa_dbus']['challenge'];

      case 'DATA':
        $shared_key = self::get_shared_key($_SESSION['mediamosa_dbus']['username']);
        if ($param2 == sha1($_SESSION['mediamosa_dbus']['challenge'] .':'. $param1 .':'. $shared_key)) {
          return 'OK '. self::APP_AUTH_SERVER_GUID;
        }
        else {
          session_destroy();
          return 'ERROR: authentication failure';
        }

      default:
        session_destroy();
        return 'ERROR syntax error';
    }
  }

  /**
   * Process the incoming DBUS line.
   *
   * @param string $dbus_data
   * @return string
   */
  function app_dbus_protocol($dbus_data) {
    $a_dbus_data = explode(' ', $dbus_data, 3);
    list($op, $param1, $param2) = $a_dbus_data;

    switch ($op) {
      case 'AUTH':
        $_SESSION['mediamosa_dbus']['method'] = $param1;
        return self::$_SESSION['mediamosa_dbus']['method']($op, $param1, $param2);
        break;

      case 'DATA':
        if (!isset($_SESSION['mediamosa_dbus']['method'])) {
          unset($_SESSION['mediamosa_dbus']);
          return 'ERROR: no method specified';
        }

        self::$_SESSION['mediamosa_dbus']['method']($op, $param1, $param2);
        break;

      default:
        unset($_SESSION['mediamosa_dbus']);
        return 'ERROR: unknown command';
    }
  }
}
