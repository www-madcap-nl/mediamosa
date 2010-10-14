<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  static public function get_by_appid($app_id) {
    static $apps = array();

    // Any cache?
    if (!empty($apps[$app_id])) {
      return $apps[$app_id];
    }

    // Store in cache.
    $apps[$app_id] = mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'app')
      ->fields('app')
      ->condition(mediamosa_app_db::APP_ID, $app_id)
      ->execute()
      ->fetchAssoc();

    return $apps[$app_id];
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
   * Get the app using the node ID.
   *
   * @param string $name
   * @return array
   */
  static public function get_by_nid($nid, $a_fields = array()) {
    return mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'app')
      ->fields('app', $a_fields)
      ->condition(mediamosa_app_db::NID, $nid)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get all apps.
   *
   * @return array
   */
  static public function get_all_apps(array $fields = array(), $order_by = NULL, $order_direction = 'ASC') {
    $query = mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'app');

    if (count($fields)) {
      foreach ($fields as $key => $field) {
        $query->addField('app', $field, (is_numeric($key) ? $field : $key));
      }
    }
    else {
      $query->fields('app', array(mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME));
    }

    // Order the results
    if (isset($order_by)) {
      $query->orderBy($order_by, $order_direction);
    }

    // Dont change with ->fetchAll(), some callers expect the object here.
    return $query->execute();
  }

  /**
   * Get the app.
   *
   * @param string $name
   * @return array
   */
  static public function get_by_name($name) {
    return mediamosa_db::db_query(
      "SELECT * FROM {#table_name} WHERE #name = :name",
      array(
        '#name' => mediamosa_app_db::APP_NAME,
        '#table_name' => mediamosa_app_db::TABLE_NAME,
        ':name' => $name,
      )
    )->fetchAssoc();
  }

  /**
   * Update an application. Used for internal code.
   */
  static public function update($app_id, array $fields = array()) {

   // Update.
    return mediamosa_db::db_update(mediamosa_app_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_app_db::APP_ID, $app_id)
      ->execute();
  }

  /**
   * Returns all owners as app_id => owner.
   */
  static public function collect_app_owners() {
    $result = mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'app')
      ->fields('app', array(mediamosa_app_db::OWNER, mediamosa_app_db::APP_ID))
      ->groupBy(mediamosa_app_db::OWNER, mediamosa_app_db::APP_ID)
      ->condition(mediamosa_app_db::OWNER, '', '!=')
      ->execute()
      ->fetchAll();

    $owners = array();
    foreach ($result as $app) {
      $owners[$app['owner']][] = $app['app_id'];
    }

    return $owners;
  }

  /**
   * return an array of all apps that are allowed to master/slave this app.
   *
   * @param integer $app_id
   */
  static public function get_allowed_master_slave_apps($app_id) {
    $allowed_apps = mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'app')
      ->fields('app', array(mediamosa_app_db::ALLOW_MASTERSLAVE_APPS))
      ->condition(mediamosa_app_db::APP_ID, $app_id)
      ->execute()
      ->fetchField();

    if (empty($allowed_apps)) {
      return array();
    }

    // Return stored array.
    return unserialize($allowed_apps);
  }

  /**
   * return the default value of the still, depend of the application id.
   *
   * @param integer $app_id
   */
  static public function get_still_default_value($app_id) {
    if ($app_id) {
      // Get the default size of still of application.
      $application = self::get_by_appid($app_id);
      $still_default_size = $application[mediamosa_app_db::STILL_DEFAULT_SIZE];
      return ($still_default_size ? $still_default_size : NULL);
    }
    else {
      // There isn't app id, we support the default size of still.
      return mediamosa_settings::STILL_DEFAULT_SIZE;
    }
  }

  /**
   * return the still padding value, depend of the application id.
   *
   * @param integer $app_id
   */
  static public function get_still_padding_value($app_id) {
    if ($app_id) {
      $application = self::get_by_appid($app_id);
      return $application[mediamosa_app_db::STILL_PADDING];
    }
    else {
      return mediamosa_app_db::STILL_PADDING_YES;
    }
  }

  /**
   * return the still maintain aspect ratio value, depend of the application id.
   *
   * @param integer $app_id
   */
  static public function get_still_maintain_aspect_ratio_value($app_id) {
    if ($app_id) {
      $application = self::get_by_appid($app_id);
      return $application[mediamosa_app_db::STILL_MAINTAIN_ASPECT_RATIO];
    }
    else {
      return mediamosa_app_db::STILL_MAINTAIN_ASPECT_RATIO_YES;
    }
  }
}
