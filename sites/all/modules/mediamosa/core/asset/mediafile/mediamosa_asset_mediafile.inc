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
      @unlink(mediamosa_configuration_storage::mediafile_filename_get($a_mediafile));
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
   * Merged with 1.x _media_management_process_mediafile_output.
   *
   * @param object $o_mediamosa
   *  The mediamosa response object.
   * @param array $a_mediafile_ids
   * @param array $mixed_app_id
   *  Either array or integer application ID.
   * @param boolean $is_oai
   *  Include extended EGA Play Urls.
   * @param boolean $show_stills
   */
  static public function enrich_response_mediafile($o_mediamosa, array $a_mediafile_ids, $mixed_app_id, $is_oai, $show_stills = TRUE) {

    // Can be empty or null.
    if (empty($a_mediafile_ids) || is_null(reset($a_mediafile_ids))) {
      return;
    }

    // Both are allowed and needed.
    $app_id = is_array($mixed_app_id) ? reset($mixed_app_id) : $mixed_app_id;
    $a_app_ids = is_array($mixed_app_id) ? $mixed_app_id : array($mixed_app_id);

    // Prefix for metadata in array.
    $prefix = 'mediafile_metadata_';

    $a_columns = mediamosa_db::db_get_columns(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->leftjoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'mfm.mediafile_id = mf.mediafile_id');
    $query->leftjoin(mediamosa_asset_db::TABLE_NAME, 'a', 'mf.asset_id = a.asset_id');
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $a_mediafile_ids, 'IN');
    $query->fields('mf');
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

      // Prefix the metadata in the result.
      $query->addField('mfm', $column, $prefix . $column);
    }

    // Execute query.
    $result = $query->execute();

    // Add to item.
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
      $query->fields('mf');
      $query->leftjoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'mfm.mediafile_id = mf.mediafile_id');
      $query->fields('mfm');
      $query->condition('mf.' . mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, $a_mediafile_ids_found, 'IN');
      $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE);
      $query->orderBy('mfm.' . mediamosa_asset_mediafile_metadata_db::STILL_ORDER, 'ASC');
      $result = $query->execute();

      $i = 0;
      foreach ($result as $a_row) {
        // Create unique ID.
      	$ticket = mediamosa_db::uuid($app_id);

      	// Create ticket.
        mediamosa_ticket::create(
          $ticket,
          NULL,
          mediamosa_ticket::RESPONSE_TYPE_STILL,
          $app_id,
          $a_row[mediamosa_asset_mediafile_db::OWNER_ID],
          $a_row[mediamosa_asset_mediafile_db::ID]
        );

        // Create response.
        $response = mediamosa_asset_mediafile_play_proxy::create_response_still($ticket, $app_id);
        $a_row['still_ticket'] = $response['output'];

        // Add still to mediafile.
        assert(!empty($a_row[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE])); // Make sure its here.
        $a_items[$a_row[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE]]['still']['#'. $i++] = $a_row;
      }
    }

    // Get the app.
    $a_app = mediamosa_app::get_by_appid($app_id);

    $a_patterns = array(
      '/{asset_id}/i',
      '/{mediafile_id}/i',
      '/{mediafile_filename}/i',
    );

    // Now add to result array.
    // Must be in the same order as $a_mediafile_ids.
    foreach ($a_mediafile_ids as $mediafile_id) {
      if (!isset($a_items[$mediafile_id])) { // In case the mediafile was not found.
        continue;
      }

      // Set empty array for metadata.
      $a_items[$mediafile_id]['metadata'] = array();

      // If its a child, then unparent it.
      if (isset($a_items[$mediafile_id][mediamosa_asset_db::PARENT_ID])) {
      	// Copy parent to main asset_id.
      	$a_items[$mediafile_id][mediamosa_asset_db::ID] = $a_items[$mediafile_id][mediamosa_asset_db::PARENT_ID];

      	// Unset the parent value.
      	unset($a_items[$mediafile_id][mediamosa_asset_db::PARENT_ID]);
      }

      // Test for downloadable.
      $is_downloadable = mediamosa_lib::boolstr2bool($a_items[$mediafile_id][mediamosa_asset_mediafile_db::IS_DOWNLOADABLE]);

      // Download URI replacements.
      $a_replacements = array(
        $a_items[$mediafile_id][mediamosa_asset_db::ID],
        $a_items[$mediafile_id][mediamosa_asset_mediafile_db::ID],
        $a_items[$mediafile_id][mediamosa_asset_mediafile_db::FILENAME],
      );

      // Default empty.
      $a_items[$mediafile_id]['ega_download_url'] = '';

      // If downloadable and application has its download URL set, then include it in our output.
      if ($is_downloadable && !empty($a_app[mediamosa_app_db::DOWNLOAD_URL])) {

	      // Set EGA download url.
	      $a_items[$mediafile_id]['ega_download_url'] = preg_replace($a_patterns, $a_replacements, $a_app[mediamosa_app_db::DOWNLOAD_URL]);
      }

      // Default empty.
      $a_items[$mediafile_id]['ega_stream_url'] = '';

      // Stream URL, only if streamable.
      if (mediamosa_server_streaming::is_streamable($a_items[$mediafile_id][$prefix . mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE])) {
        $a_items[$mediafile_id]['ega_play_url'] = preg_replace($a_patterns, $a_replacements, $a_app[mediamosa_app_db::PLAY_PROXY_URL]);

        // Output extra?
        if ($is_oai) {
        	// Stream URL(s) for OAI.
          $mediafile_app_id = $a_items[$mediafile_id][mediamosa_asset_mediafile_db::APP_ID];

          // Check if mediafile APP ID is in the requested app_ids.
          if (in_array($mediafile_app_id, $a_app_ids)) {
            $a_items[$mediafile_id]['ega_stream_url'][] = !empty($a_app[mediamosa_app_db::STREAM_URL]) ? preg_replace($a_patterns, $a_replacements, $a_app[mediamosa_app_db::STREAM_URL]) : '';
          }

          // Get masters of this mediafile.
          $a_app_id_master = mediamosa_aut_app_master_slave::master_get($mediafile_app_id, mediamosa_aut::AUT_TYPE_MEDIAFILE, $a_items[$mediafile_id][mediamosa_asset_mediafile_db::ID]);

          foreach ($a_app_id_master as $app_id_master) {
            if (in_array($app_id_master, $a_app_ids)) {
            	// Get the master app.
            	$a_app_tmp = mediamosa_app::get_by_appid($app_id_master);

            	// Set the ega_stream_url of the master.
            	$a_items[$mediafile_id]['ega_stream_url'][] = preg_replace($a_patterns, $a_replacements, $a_app_tmp[mediamosa_app_db::STREAM_URL]);
            }
          }
        }
        else {
          $a_items[$mediafile_id]['ega_stream_url'][] = !empty($a_app[mediamosa_app_db::STREAM_URL]) ? preg_replace($a_patterns, $a_replacements, $a_app[mediamosa_app_db::STREAM_URL]) : '';
        }
      }

	    // Remove the prefix from the metadata and move into the metadata array of the asset.
	    foreach ($a_items[$mediafile_id] as $key => $value) {
	      if (mediamosa_unicode::strpos($key, $prefix) === 0) { // is het een metadata tag?
	        $new_key = mediamosa_unicode::substr($key, mediamosa_unicode::strlen($prefix));
	        $a_items[$mediafile_id]['metadata'][$new_key] = $a_items[$mediafile_id][$key];
	        unset($a_items[$mediafile_id][$key]);
	      }
	    }

	    // Unset these;
	    unset($a_items[$mediafile_id][mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]);

	    // Add to result.
	    $o_mediamosa->add_item($a_items[$mediafile_id]);
    }
  }
}
