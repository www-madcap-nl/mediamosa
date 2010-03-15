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
 * functions class for statistics.
 *
 * Functions not converted as they are obsolete or empty;
 * _vpx_statistics_log_search_query

 * Opvragen van een overzicht van aangemaakte streams en collecties
 * STATS:5
  function vpx_statistics_get_created_metadata_and_collections() {
  }

 * Opvragen van een overzicht van zoek en bladeracties.
 * STATS:6
 function vpx_statistics_get_search_queries() {
 }
*/

class mediamosa_statistics {

  /**
   * Create log entry for file upload event.
   *
   * 1.x _vpx_statistics_log_file_upload
   *
   * @param $app_id
   * @param $owner_id
   * @param $group_id
   * @param $file_size
   */
  static public function log_event_file_upload($app_id, $owner_id, $group_id, $file_size) {
    // Data to store.
    $fields = array(
      mediamosa_statistics_file_upload_db::APP_ID => $app_id,
      mediamosa_statistics_file_upload_db::OWNER_ID => $owner_id,
      mediamosa_statistics_file_upload_db::GROUP_ID => $group_id,
      mediamosa_statistics_file_upload_db::FILE_SIZE => $file_size,
      mediamosa_statistics_file_upload_db::TIMESTAMP => mediamosa::db_current_timestamp_now()
    );

    // Do the insert.
    return mediamosa_db::db_insert(mediamosa_statistics_file_upload_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Log request stream event (when mediafile is streamed).
   *
   * @param string $mediafile_id
   * @param string $response_type
   */
  static public function log_event_requested_stream($mediafile_id, $response_type) {

    $mediafile_ext = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'mfm.mediafile_id = mf.mediafile_id')
      ->leftJoin(mediamosa_asset_db::TABLE_NAME, 'a', 'a.asset_id = mf.asset_id')
      ->fields('mf')
      ->fields('a', array(mediamosa_asset_db::ID)) // 2.x allows us to use the asset_id instead of the asset_id_root.
      ->fields('mfm', array(mediamosa_asset_mediafile_metadata_db::FILESIZE, mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE))
      ->execute();

    // Not found?
    if ($mediafile_ext === FALSE) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $mediafile_id));
    }

    // Get the asset_id. Not using asset_id_root anymore.
    $asset_id = $mediafile_ext[mediamosa_asset_mediafile_db::ID];

    // Create the event log for the request.
    $fields = array(
      mediamosa_statistics_stream_request_db::APP_ID => $mediafile_ext[mediamosa_asset_mediafile_db::APP_ID],
      mediamosa_statistics_stream_request_db::ASSET_ID => $asset_id,
      mediamosa_statistics_stream_request_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_statistics_stream_request_db::OWNER_ID => $mediafile_ext[mediamosa_asset_mediafile_db::OWNER_ID],
      mediamosa_statistics_stream_request_db::GROUP_ID => $mediafile_ext[mediamosa_asset_mediafile_db::GROUP_ID],
      mediamosa_statistics_stream_request_db::FILESIZE => $mediafile_ext[mediamosa_asset_mediafile_metadata_db::FILESIZE],
      mediamosa_statistics_stream_request_db::CONTAINER_TYPE => $mediafile_ext[mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE],
      mediamosa_statistics_stream_request_db::PLAY_TYPE => $response_type,
      mediamosa_statistics_stream_request_db::PLAYED => mediamosa::db_current_timestamp_now(),
    );

    // Insert it.
    $request_id = mediamosa_db::db_insert(mediamosa_statistics_stream_request_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // If in any collection, we need to record that as well.
    $collections = mediamosa_asset_collection::find($asset_id);

    // Loop through the results.
    foreach ($collections as $collection) {
      $fields = array(
        mediamosa_statistics_stream_request_collection_db::REQUEST_ID => $request_id,
        mediamosa_statistics_stream_request_collection_db::APP_ID => $collection[mediamosa_collection_db::APP_ID],
        mediamosa_statistics_stream_request_collection_db::COLL_ID => $collection[mediamosa_collection_db::ID],
        mediamosa_statistics_stream_request_collection_db::OWNER_ID => $collection[mediamosa_collection_db::OWNER_ID],
        mediamosa_statistics_stream_request_collection_db::GROUP_ID => $collection[mediamosa_collection_db::GROUP_ID],
        mediamosa_statistics_stream_request_collection_db::TITLE => $collection[mediamosa_collection_db::TITLE],
        mediamosa_statistics_stream_request_collection_db::DESCRIPTION => $collection[mediamosa_collection_db::DESCRIPTION],
      );

      // Insert the collection ID as request.
      mediamosa_db::db_insert(mediamosa_statistics_stream_request_collection_db::TABLE_NAME)
        ->fields($fields)
        ->execute();
    }
  }

  /**
   * Log search query keywords.
   *
   * @param array $keywords
   * @param integer $app_id
   */
  static public function log_event_search_query(array $keywords, $app_id) {

    $keywords = trim(implode(' ', $keywords));
    if (empty($keywords)) {
      return;
    }

    mediamosa_db::db_insert(mediamosa_statistics_search_request_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_search_request_db::APP_ID => $app_id,
        mediamosa_statistics_search_request_db::KEYWORD => $keywords,
        mediamosa_statistics_search_request_db::SEARCHED => mediamosa::db_current_timestamp_now(),
      ))
      ->execute();
  }


  /**
   * Calculate and store used diskspace.
   *
   * 1.x: vpx_statistics_calculate_used_diskspace
   *
   * @param $year
   * @param $month
   */
  static public function calculate_used_diskspace($year, $month) {

    $year = (int)$year;
    $month = (int)$month;

    // Make sure its provide ok.
    if ($year < 2000 || $year > 2099 || $month < 1 || $month > 12) {
      throw new mediamosa_exception_program_error('Invalid input @calculate_used_diskspace');
    }

    mediamosa_db::db_query(
      'DELETE FROM {mediamosa_statistics_diskspace_used} WHERE YEAR(timestamp) = :year AND MONTH(timestamp) = :month',
      array(
        ':year' => $year,
        ':month' => $month,
      )
    );

    // Per usergroup, user and container
    foreach (array('group_id' => 'group', 'owner_id' => 'user', 'app_id' => 'container') as $subject => $name) {
      $result = db_query(
        'SELECT SUM(m.filesize) / 1024 / 1024 AS diskspace_mb, mf.#subject, mf.app_id, m.container_type FROM {mediamosa_asset_mediafile_metadata} AS m
         JOIN {mediamosa_asset_mediafile} AS mf ON m.mediafile_id = mf.mediafile_id
         WHERE filesize > 0
         GROUP BY app_id, #subject, container_type',
        array(
          '#subject' => $subject,
        )
      );

      // Insert new (or updated) statistics.
      foreach ($result as $data) {
        db_query(
          'INSERT INTO {mediamosa_statistics_diskspace_used}
           SET app_id = :app_id, type = :type, keyword = :keyword, container_type = :container_type, diskspace = :diskspace, timestamp = :timestamp',
          array(
            ':app_id' => $data['app_id'],
            ':type' => $name,
            ':keyword' => $data[$subject],
            ':container_type' => $data['container_type'],
            ':diskspace' => round($data['diskspace_mb']),
            ':timestamp' => mediamosa::db_current_timestamp_now()
          )
        );
      }
    }
  }
}
