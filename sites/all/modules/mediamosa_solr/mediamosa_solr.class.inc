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
 * Our extended classes for Solr connections.
 */

class mediamosa_solr {

  // ------------------------------------------------------------------- Consts.

  // ---------------------------------------------------------------- Functions.
  /**
   * Reindex with given asset_ids. Stores it in table, will be reindexed asap.
   *
   * @param string $asset_id
   */
  static public function prepare_reindex(array $asset_ids) {

    foreach ($asset_ids as $asset_id) {
      try {
        // If it fails because of existing primary key, then ignore.
        mediamosa_db::db_insert(mediamosa_solr_index_db::TABLE_NAME)
          ->fields(array(
              mediamosa_solr_index_db::ID => $asset_id,
              mediamosa_solr_index_db::CREATED => mediamosa_datetime::db_current_timestamp_now()
          ))
          ->execute();
      }
      catch (PDOException $e) {
        // Ignore.
      }
    }
  }

  /**
   * Triggered by REST call.
   *
   * Will reindex the solr database as much as its allowed during this call.
   *
   */
  static public function cron_reindex_run() {

    // Number of seconds the cron is running max.
    $set_time_limit = variable('mediamosa_solr_reindex_time_limit', 50);

    // Get possible current running reindex. Make sure its not running too.
    $run_last = variable_get('mediamosa_solr_reindex_cron_last', NULL);

    // Now time.
    $now = round(microtime(TRUE));

    // Make sure we don't run the this twice.
    // We will give it enough time to timeout.
    if ($run_last && round($now - $run_last) < 180) {
      mediamosa_watchdog::log('Unexpected; Solr reindex was still / already running when cron was triggered.');
      return; // Still running.
    }

    // If we get here then we can go.
    variable_set('mediamosa_solr_reindex_cron_last', $now);

    // 10 seconds more than we think we need.
    set_time_limit($set_time_limit + 10);

    // At where
    $end_at = $now + $set_time_limit;

    // Get the now time, as long as we under end_at then continue.
    while (round(microtime(TRUE)) < $end_at) {
      // Do our stuff until we are done or when we ran out of time.

      break; // when done
    }

    // Done.
    variable_set('mediamosa_solr_reindex_cron_last', NULL);
  }
}
