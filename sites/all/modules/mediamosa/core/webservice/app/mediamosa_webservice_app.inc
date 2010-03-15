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
  * Webservices management per app.
  */
class mediamosa_webservice_app {
  // ------------------------------------------------------------------ Consts.
  const HANDLE_MEDIA_MANAGEMENT = 'media_management';
  const HANDLE_BATCH_UPLOAD = 'batch_upload';
  const HANDLE_JOBS = 'jobs';
  const HANDLE_MEDIA_UPLOAD = 'media_upload';
  const HANDLE_TRANSCODING = 'transcoding';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Check active status of an webservice.
   *
   * 1.x: vpx_shared_webservice_is_active
   *
   * @param string $handle
   *  Webservice handle.
   * @param integer $app_id
   *  Application ID.
   * @return bool
   */
  static public function webservice_is_active($handle, array $a_app_ids = array()) {
    if (!empty($a_app_ids)) {
      $result = mediamosa_db::db_query(
        'SELECT wa.#wa_status AS wa_status, w.#w_status AS w_status
         FROM {#mediamosa_webservice_app} AS wa
         JOIN {#mediamosa_webservice} AS w ON w.#handle = wa.#webservice_handle
         JOIN {#mediamosa_app} AS ma ON ma.#app_id = wa.#app_id
         WHERE wa.#webservice_handle = :handle AND ma.#app_id IN (:app_id)',
        array(
          '#wa_status' => mediamosa_webservice_app_db::STATUS,
          '#w_status' => mediamosa_webservice_db::STATUS,
          '#mediamosa_webservice_app' => mediamosa_webservice_app_db::TABLE_NAME,
          '#mediamosa_webservice' => mediamosa_webservice_db::TABLE_NAME,
          '#handle' => mediamosa_webservice_db::HANDLE,
          '#webservice_handle' => mediamosa_webservice_app_db::WEBSERVICE_HANDLE,
          '#mediamosa_app' => mediamosa_app_db::TABLE_NAME,
          '#app_id' => mediamosa_app_db::APP_ID,
          ':handle' => $handle,
          ':app_id' => $a_app_ids
        )
      )->fetchAssoc();

      if (!$result) {
        return TRUE; // Not found then ON.
      }

      // Return only false when webservice is specified as OFF.
      return ($result['wa_status'] == mediamosa_webservice_app_db::STATUS_TRUE && $result['w_status'] == mediamosa_webservice_db::STATUS_TRUE) ? TRUE : FALSE;
    }

    $result = mediamosa_db::db_query(
      'SELECT #status FROM {#mediamosa_webservice} WHERE #handle = :handle',
      array(
        '#mediamosa_webservice' => mediamosa_webservice_db::TABLE_NAME,
        '#handle' => mediamosa_webservice_db::HANDLE,
        ':handle' => $handle,
        '#status' => mediamosa_webservice_db::STATUS,
      )
    )->fetchField();

    if (!$result) {
      return TRUE; // Not found then ON.
    }

    // If specified ON, return TRUE.
    return $result == mediamosa_webservice_db::STATUS_TRUE ? TRUE : FALSE;
  }

  /**
   * Same as webservice_is_active, except will throw an exception.
   *
   * @param string $handle
   *  Webservice handle.
   * @param integer $app_id
   *  Application ID.
   */
  static public function webservice_must_be_active($handle, array $a_app_ids = array()) {
    if (!self::webservice_is_active($handle, $a_app_ids)) {
      throw new mediamosa_exception_error_webservice_disabled();
    }
  }

  /**
   * Return enabled/disabled statussem of webservices of each app.
   */
  static public function webservice_get_all_status() {
    $result = mediamosa_db::db_select(mediamosa_webservice_app_db::TABLE_NAME, 'w')
      ->fields('w')
      ->execute();

    $a_webservice_apps = array();
    foreach ($result as $a_webservice_app) {
      $a_webservice_apps[$a_webservice_app[mediamosa_webservice_app_db::APP_ID]][$a_webservice_app[mediamosa_webservice_app_db::WEBSERVICE_HANDLE]] = ($a_webservice_app[mediamosa_webservice_app_db::STATUS] == mediamosa_webservice_app_db::STATUS_TRUE);
    }

    return $a_webservice_apps;
  }

}
