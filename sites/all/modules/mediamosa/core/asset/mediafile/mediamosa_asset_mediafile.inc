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
  * Common function class for mediamosa_asset_mediafile
  */

class mediamosa_asset_mediafile {

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Remove one or more mediafiles.
   *
   * @param mixed $mediafile_id
   */
  static public function delete($mixed_mediafile_id) {
    $a_mediafile_ids = is_array($mixed_mediafile_id) ? $mixed_mediafile_id : array($mixed_mediafile_id);

    // Delete mediafile metadata first.
    $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
    $query->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $a_mediafile_ids, 'IN');
    $query->execute();

    // Delete from SAN/NAS.
    $query = db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->fields('mf', array(mediamosa_asset_mediafile_db::APP_ID, mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT));
    $query->condition(mediamosa_asset_mediafile_db::ID, $a_mediafile_ids, 'IN');

    // Now delete the file of each found mediafile.
    foreach ($query->execute() as $a_mediafile) {
      @unlink(mediamosa_maintenance_san_nas::mediafile_filename_get($a_mediafile));
    }

    // Delete mediafile from DB.
    $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_db::TABLE_NAME);
    $query->condition(mediamosa_asset_mediafile_db::ID, $a_mediafile_ids, 'IN');
    $query->execute();

    // Update the asset information.
    foreach ($a_mediafile_ids as $mediafile_id) {
      mediamosa_asset::update_asset_info($mediafile_id);
    }
  }

  /**
   * Enrich the response with mediafile data. 1.x _media_management_return_mediafile_list()
   *
   * @param object $o_mediamosa
   * @param array $a_mediafile_ids
   * @param integer $app_id
   * @param boolean $show_stills
   */

  static public function enrich_response_mediafile($o_mediamosa, array $a_mediafile_ids, $app_id, $show_stills = TRUE) {

    // Can be empty or null.
    if (empty($a_mediafile_ids) || is_null(reset($a_mediafile_ids))) {
      return;
    }

    $a_columns = mediamosa_db::db_get_columns(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->leftjoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'USING(mediafile_id)');
    $query->leftjoin(mediamosa_asset_db::TABLE_NAME, 'a', 'mf.asset_id = a.asset_id');
    $query->condition(mediamosa_asset_mediafile_db::ID, $a_mediafile_ids, 'IN');
    $query->addField('mf');
    $query->addField('a', mediamosa_asset_db::PARENT_ID);

    // Now select the metadata columns to the query.
    foreach ($a_columns as $column) {
      if (in_array(
        $column,
        array(
          mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID,
          mediamosa_asset_mediafile_metadata_db::ID,
          mediamosa_asset_mediafile_metadata_db::STILL_TIME_CODE,
          mediamosa_asset_mediafile_metadata_db::STILL_ORDER,
          mediamosa_asset_mediafile_metadata_db::STILL_FORMAT,
          mediamosa_asset_mediafile_metadata_db::STILL_TYPE,
          mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT
        )
      )) {
        continue; // Some columns need to be skipped.
      }

      $query->addField('mfm', $column, 'mediafile_metadata_' . $column);
    }

    // Execute query.
    $result = $query->execute();

    $a_items = array();
    foreach ($result as $a_row) {
      $a_items[$a_row[mediamosa_asset_mediafile_db::ID]] = $a_row;
    }

    // Now we know which mediafiles where found.
    $a_mediafile_ids_found = array_keys($a_items);

    // Lets include the still, if we need them.
    if (!empty($a_mediafile_ids_found) && $show_stills) {
      // Get the stills.
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->leftjoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'USING(mediafile_id)');
      $query->condition(mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, $a_mediafile_ids_found, 'IN');
      $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE);
      $query->orderBy('mfm.' . mediamosa_asset_mediafile_metadata_db::STILL_ORDER, 'ASC');
      $result = $query->execute();

      $i = 0;
      foreach ($result as $a_row) {
        $ticket = mediamosa_db::uuid($app_id);

        mediamosa_ticket::create(
          $ticket,
          NULL,
          mediamosa_ticket::RESPONSE_TYPE_STILL,
          $app_id,
          $a_row[mediamosa_asset_mediafile_db::OWNER_ID],
          $a_row[mediamosa_asset_mediafile_db::ID]
        );

        $response = mediamosa_asset_mediafile_play_proxy::create_response_still($s_ticket, $app_id);
        $a_row['still_ticket'] = $response['output'];

        // Add still to mediafile.
        $a_items[$a_row['mediafile_source']]['still']['#'. $i++] = $a_row;
      }
    }

    // Now add to response.
    // Must be in the same order as $a_mediafile_ids.
    foreach ($a_mediafile_ids as $mediafile_id) {
      if (isset($a_items[$mediafile_id])) { // In case the mediafile was not found.
        $o_mediamosa->add_item($a_items[$mediafile_id]);
      }
    }

    return _media_management_process_mediafile_output($o_rest_reponse, $app_id);
  }
}
