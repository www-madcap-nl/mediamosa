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
      mediamosa_statistics_file_upload_db::FILE_SIZE => (is_null($file_size)? 0 : $file_size),
      mediamosa_statistics_file_upload_db::TIMESTAMP => mediamosa_datetime::utc_current_timestamp_now(),
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

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->leftJoin(mediamosa_asset_db::TABLE_NAME, 'a', 'a.asset_id = mf.asset_id');
    $query->fields('mf');
    $query->fields('a', array(mediamosa_asset_db::ID));
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
    $mediafile_ext = $query->execute()->fetchAssoc();
    // Not found?
    if ($mediafile_ext === FALSE) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $mediafile_id));
    }

    $asset_id = $mediafile_ext[mediamosa_asset_mediafile_db::ASSET_ID];

    // Create the event log for the request.
    $fields = array(
      mediamosa_statistics_stream_request_db::APP_ID => $mediafile_ext[mediamosa_asset_mediafile_db::APP_ID],
      mediamosa_statistics_stream_request_db::ASSET_ID => $asset_id,
      mediamosa_statistics_stream_request_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_statistics_stream_request_db::OWNER_ID => $mediafile_ext[mediamosa_asset_mediafile_db::OWNER_ID],
      mediamosa_statistics_stream_request_db::GROUP_ID => $mediafile_ext[mediamosa_asset_mediafile_db::GROUP_ID],
      mediamosa_statistics_stream_request_db::FILESIZE => mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile_id, mediamosa_asset_mediafile_metadata::FILESIZE),
      mediamosa_statistics_stream_request_db::CONTAINER_TYPE => mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE),
      mediamosa_statistics_stream_request_db::PLAY_TYPE => $response_type,
      mediamosa_statistics_stream_request_db::PLAYED => mediamosa_datetime::utc_current_timestamp_now(),
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
        mediamosa_statistics_stream_request_collection_db::OWNER_ID => (string)$collection[mediamosa_collection_db::OWNER_ID],
        mediamosa_statistics_stream_request_collection_db::GROUP_ID => (string)$collection[mediamosa_collection_db::GROUP_ID],
        mediamosa_statistics_stream_request_collection_db::TITLE => (string)$collection[mediamosa_collection_db::TITLE],
        mediamosa_statistics_stream_request_collection_db::DESCRIPTION => (string)$collection[mediamosa_collection_db::DESCRIPTION],
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
    // Is we detect an array, then we are not gonna log it. In this case we
    // are in multi App mode, and we only log during single app mode.
    if (is_array($app_id)) {

      $app_id = reset($app_id);

      // If we are as super user, then dont store the stats.
      if (!$app_id) {
        return;
      }
    }

    $keywords = trim(implode(' ', $keywords));
    if (empty($keywords)) {
      return;
    }

    mediamosa_db::db_insert(mediamosa_statistics_search_request_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_search_request_db::APP_ID => $app_id,
        mediamosa_statistics_search_request_db::KEYWORD => $keywords,
        mediamosa_statistics_search_request_db::SEARCHED => mediamosa_datetime::utc_current_timestamp_now(),
      ))
      ->execute();
  }

  /**
   * Log a REST call request.
   *
   * @param string $request_uri
   * @param float $processed_time
   * @param integer $query_count
   */
  static public function log_event_restcall($request, $process_time, $query_count) {

    return mediamosa_db::db_insert(mediamosa_statistics_rest_log_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_rest_log_db::REQUEST => $request,
        mediamosa_statistics_rest_log_db::PROCESS_TIME => $process_time,
        mediamosa_statistics_rest_log_db::QUERY_COUNT => $query_count,
      ))
      ->execute();
  }

  /**
   * Delete all stats related to this asset.
   *
   * @param string $asset_id
   */
  static public function delete_by_asset_id($asset_id) {
    mediamosa_db::db_delete(mediamosa_statistics_stream_request_db::TABLE_NAME)
      ->condition(mediamosa_statistics_stream_request_db::ASSET_ID, $asset_id)
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

    $year = (int) $year;
    $month = (int) $month;

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
      /*
      // Example queries:
      // group_id => group
      SELECT SUM(m.val_int) / 1024 / 1024 AS diskspace_mb, mf.group_id, mf.app_id, mm.val_char AS container_type
      FROM mediamosa_asset_mediafile_metadata AS m
      LEFT JOIN mediamosa_asset_mediafile_metadata AS mm USING(mediafile_id)
      JOIN mediamosa_asset_mediafile AS mf USING(mediafile_id)
      WHERE m.val_int > 0 AND m.prop_id = 13 AND mm.prop_id = 10
      GROUP BY app_id, group_id, mm.val_char
      // owner_id => user
      SELECT SUM(m.val_int) / 1024 / 1024 AS diskspace_mb, mf.owner_id, mf.app_id, mm.val_char AS container_type
      FROM mediamosa_asset_mediafile_metadata AS m
      LEFT JOIN mediamosa_asset_mediafile_metadata AS mm USING(mediafile_id)
      JOIN mediamosa_asset_mediafile AS mf USING(mediafile_id)
      WHERE m.val_int > 0 AND m.prop_id = 13 AND mm.prop_id = 10
      GROUP BY app_id, owner_id, mm.val_char
      // app_id => container
      SELECT SUM(m.val_int) / 1024 / 1024 AS diskspace_mb, mf.app_id, mf.app_id, mm.val_char AS container_type
      FROM mediamosa_asset_mediafile_metadata AS m
      LEFT JOIN mediamosa_asset_mediafile_metadata AS mm USING(mediafile_id)
      JOIN mediamosa_asset_mediafile AS mf USING(mediafile_id)
      WHERE m.val_int > 0 AND m.prop_id = 13 AND mm.prop_id = 10
      GROUP BY app_id, app_id, mm.val_char
       */
      $result = mediamosa_db::db_query(
        'SELECT SUM(m.#filesize) / 1024 / 1024 AS diskspace_mb, mf.#subject, mf.#app_id, mm.#container_type AS container_type
         FROM {#mediamosa_asset_mediafile_metadata} AS m
         LEFT JOIN {#mediamosa_asset_mediafile_metadata} AS mm USING(#mediafile_id)
         JOIN {#mediamosa_asset_mediafile} AS mf USING(#mediafile_id)
         WHERE m.#filesize > 0 AND m.#prop_id = :prop_id_filesize AND mm.#prop_id = :prop_id_container_type
         GROUP BY #app_id, #subject, mm.#container_type',
        array(
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#filesize' => mediamosa_asset_mediafile_metadata_db::VAL_INT,
          '#subject' => $subject,
          '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
          '#container_type' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
          '#mediafile_id' => mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID,
          '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
          ':prop_id_filesize' => mediamosa_asset_mediafile_metadata_property::get_property_id_int(mediamosa_asset_mediafile_metadata::FILESIZE),
          ':prop_id_container_type' => mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::CONTAINER_TYPE),
        )
      );

      // Insert new (or updated) statistics.
      foreach ($result as $data) {
        mediamosa_db::db_query(
          'INSERT INTO {mediamosa_statistics_diskspace_used}
           SET app_id = :app_id, type = :type, keyword = :keyword, container_type = :container_type, diskspace = :diskspace, timestamp = :timestamp',
          array(
            ':app_id' => $data['app_id'],
            ':type' => $name,
            ':keyword' => ($data[$subject] ? $data[$subject] : ''),
            ':container_type' => $data['container_type'],
            ':diskspace' => round($data['diskspace_mb']),
            ':timestamp' => mediamosa_datetime::utc_current_timestamp_now()
          )
        );
      }
    }
  }

  /**
   * Insert job transcode data for the statistics.
   *
   * @param string $mediafile_id
   * @param string $job_id
   */
  static public function insert_job_transcode($mediafile_id, $job_id) {
    // Get data.
    $job = mediamosa_job::get($job_id);
    $job_transcode = mediamosa_job_transcode::get($job_id);
    $profile = mediamosa_transcode_profile::get($job_transcode[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID]);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_statistics_job_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_job_db::ID => NULL,
        mediamosa_statistics_job_db::APP_ID => $job[mediamosa_job_db::APP_ID],
        mediamosa_statistics_job_db::MEDIAFILE_ID => $mediafile_id,
        mediamosa_statistics_job_db::STARTED => $job[mediamosa_job_db::STARTED],
        mediamosa_statistics_job_db::FINISHED => $job[mediamosa_job_db::FINISHED],
        mediamosa_statistics_job_db::TRANSCODE_PROFILE_ID => $job_transcode[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID],
        mediamosa_statistics_job_db::NID => $profile[mediamosa_transcode_profile_db::NID],
        mediamosa_statistics_job_db::VID => $profile[mediamosa_transcode_profile_db::VID],
        mediamosa_statistics_job_db::FILESIZE => NULL,
      ))
      ->execute();
  }

  /**
   * Update job transcode filesize data for the statistics.
   *
   * @param string $mediafile_id
   * @param int $filesize
   */
  static public function update_job_transcode_filesize($mediafile_id, $filesize) {
    mediamosa_db::db_update(mediamosa_statistics_job_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_job_db::FILESIZE => $filesize,
      ))
      ->condition(mediamosa_statistics_job_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();
  }
}
