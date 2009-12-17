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
    return mediamosa_db::db_query(
      'SELECT * FROM {#table_name} WHERE #app_id = :app_id',
      array(
        '#app_id' => mediamosa_app_db::APP_ID,
        '#table_name' => mediamosa_app_db::TABLE_NAME,
        ':app_id' => $app_id,
      )
    )->fetchAssoc();
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
      $query->fields();
    }
    $query->condition(mediamosa_app_db::APP_ID, $a_app_id, 'IN');
    return $query->execute();
  }
}
