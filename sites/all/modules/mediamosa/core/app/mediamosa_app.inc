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
 * The MediaMosa App class.
 */
class mediamosa_app {
  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the app using the app_id.
   *
   * @param string $name
   * @return array
   */
  static public function get_by_appid($app_id, $a_fields = array()) {
    return mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'app')
      ->fields('app', $a_fields)
      ->condition(mediamosa_app_db::APP_ID, $app_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the app using the app_id.
   *
   * @param string $name
   * @return array
   */
  static public function get_by_appids(array $a_app_id, array $a_fields = array()) {

    $query = mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'app');

    if (count($a_fields)) {
      foreach ($a_fields as $key => $field) {
        $query->addField('app', $field, (is_numeric($key) ? $field : $key));
      }
    }
    else {
      $query->fields('app');
    }
    $query->condition(mediamosa_app_db::APP_ID, $a_app_id, 'IN');
    return $query->execute()->fetchAll();
  }

  /**
   * Get all apps.
   *
   * @return array
   */
  static public function get_all_apps(array $a_fields = array(), $order_by = FALSE, $order_direction = 'ASC') {
    $query = mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'app');

    if (count($a_fields)) {
      foreach ($a_fields as $key => $field) {
        $query->addField('app', $field, (is_numeric($key) ? $field : $key));
      }
    }
    else {
      $query->fields('app', array(mediamosa_app_db::APP_ID, mediamosa_app_db::NAME));
    }

    // Order the results
    if ($order_by !== FALSE) {
      $query->orderBy($order_by, $order_direction);
    }

    return $query->execute()->fetchAll();
  }

  /**
   * Update an application. Used for internal code.
   */
  static public function update($app_id, $a_fields = array()) {

   // Update.
    return mediamosa_db::db_update(mediamosa_app_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_app_db::APP_ID, $app_id)
      ->execute();
  }


}
