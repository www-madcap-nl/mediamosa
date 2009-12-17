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
  * Webservices management per app.
  */
class mediamosa_webservice_app {
  // ------------------------------------------------------------------ Consts.
  const HANDLE_MEDIA_MANAGEMENT = 'media_management';
  const HANDLE_BATCH_UPLOAD = 'batch_upload';

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
        'SELECT 1
         FROM {#mediamosa_webservice_app} AS wa
         INNER JOIN {#mediamosa_webservice} AS w ON w.#handle = wa.#webservice_handle
         INNER JOIN {#mediamosa_app} AS app ON ap.#app_id = wa.#app_id
         WHERE wa.#webservice_handle = :handle AND ap.#app_id IN (:app_id) AND wa.:status = :status_true AND w.:status = :status_true',
        array(
          ':status' => mediamosa_webservice_app_db::STATUS,
          ':status_true' => mediamosa_webservice_app_db::STATUS_TRUE,
          '#mediamosa_webservice_app' => mediamosa_webservice_app_db::TABLE_NAME,
          '#mediamosa_webservice' => mediamosa_webservice_db::TABLE_NAME,
          '#handle' => mediamosa_webservice_db::HANDLE,
          '#webservice_handle' => mediamosa_webservice_app_db::WEBSERVICE_HANDLE,
          '#mediamosa_app' => mediamosa_app_db::TABLE_NAME,
          '#app_id' => mediamosa_app_db::APP_ID,
          ':handle' => $handle,
          ':app_id' => $a_app_ids
        )
      )->fetchField();
    }
    else {
      $result = mediamosa_db::db_query(
        'SELECT 1 FROM {#mediamosa_webservice} WHERE #handle = :handle AND #status = :status_true',
        array(
          '#mediamosa_webservice' => mediamosa_webservice_db::TABLE_NAME,
          '#handle' => mediamosa_webservice_db::HANDLE,
          ':handle' => $handle,
          '#status' => mediamosa_webservice_db::STATUS,
          ':status_true' => mediamosa_webservice_db::STATUS_TRUE
        )
      )->fetchField();
    }

    return $result > 0 ? TRUE : FALSE;
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
}
