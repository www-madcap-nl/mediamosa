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
  * Common function class for mediamosa_asset_mediafile
  */
class mediamosa_asset_mediafile {

  // ------------------------------------------------------------------ Consts.
  const UNKNOWN_FILENAME = 'Filename_not_set';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Get the mediafile.
   *
   * @param string $mediafile_id
   */
  static public function get($mediafile_id, $app_id = NULL) {

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->fields('mf');
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
    if (isset($app_id)) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::APP_ID, $app_id);
    }
    return $query->execute()->fetchAssoc();
  }

  /**
   * Will test if mediafile exits, will throw when fail.
   *
   * @param string $mediafile_id
   */
  static public function must_exists($mediafile_id, $app_id = NULL) {
    if (isset($app_id)) {
      return mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::APP_ID => $app_id, mediamosa_asset_mediafile_db::ID => $mediafile_id));
    }

    return mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));
  }

  /**
   * Remove one or more mediafiles.
   *
   * @param mixed $mediafile_id
   */
  static public function delete($mixed_mediafile_id) {
    $mediafile_ids = is_array($mixed_mediafile_id) ? $mixed_mediafile_id : (empty($mixed_mediafile_id) ? array() : array($mixed_mediafile_id));

    // must have something.
    assert(count($mediafile_ids));
    if (!count($mediafile_ids)) {
      return;
    }

    // Delete mediafile metadata first.
    mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_ids, 'IN')
      ->execute();

    // Delete from SAN/NAS.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->fields('mf', array(mediamosa_asset_mediafile_db::APP_ID, mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT, mediamosa_asset_mediafile_db::ID));
    $query->condition(mediamosa_asset_mediafile_db::ID, $mediafile_ids, 'IN');

    // Now delete the file of each found mediafile.
    foreach ($query->execute() as $mediafile) {
      mediamosa_io::unlink(mediamosa_configuration_storage::mediafile_filename_get($mediafile));
    }

    // Delete mediafile from DB.
    $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_db::TABLE_NAME);
    $query->condition(mediamosa_asset_mediafile_db::ID, $mediafile_ids, 'IN');
    $query->execute();

    // Update the asset information.
    foreach ($mediafile_ids as $mediafile_id) {
      mediamosa_asset::update_asset_info($mediafile_id);
    }
  }

  /**
   * Delete unofficial mediafiles.
   *
   * @param string $asset_id
   */
  static public function delete_transcoded_unoriginal_mediafiles($asset_id) {
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->addField('mf', mediamosa_asset_mediafile_db::ID);
    $query->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id);
    $query->condition(mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE);
    $result = $query->execute()->fetchCol();

    // Delete mediafiles.
    if (!empty($result)) {
      self::delete($result);
    }
  }

  /**
   * Enrich the response with mediafile data.
   *
   * 1.x _media_management_return_mediafile_list()
   *
   * Merged with 1.x _media_management_process_mediafile_output.
   *
   * @param array $mediafile_ids
   * @param array $mixed_app_id
   *  Either array or integer application ID.
   * @param boolean $is_oai
   *  Include extended EGA Play Urls. (WAS hack is_oai on _POST/_GET (!))
   * @param boolean $show_stills
   */
  static public function enrich_response_mediafile(array $mediafile_ids, $mixed_app_id, $is_oai, $show_stills = TRUE) {

    // Can be empty or null.
    if (empty($mediafile_ids) || is_null(reset($mediafile_ids))) {
      return;
    }

    // Both are allowed and needed.
    $app_id = is_array($mixed_app_id) ? reset($mixed_app_id) : $mixed_app_id;
    $app_ids = is_array($mixed_app_id) ? $mixed_app_id : array($mixed_app_id);

    // Prefix for metadata in array.
    $prefix = 'mediafile_metadata_';

    // Get the columns from the schema.
    $a_columns = mediamosa_db::db_get_columns(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'mfm.mediafile_id = mf.mediafile_id');
    $query->leftJoin(mediamosa_asset_db::TABLE_NAME, 'a', 'mf.asset_id = a.asset_id');
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_ids, 'IN');
    $query->fields('mf');

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
    $items = array();
    foreach ($result as $a_row) {
      $items[$a_row[mediamosa_asset_mediafile_db::ID]] = $a_row;
    }

    // Now we know which mediafiles where found.
    $mediafile_ids_found = array_keys($items);

    // Lets include the still, if we need them.
    if (!empty($mediafile_ids_found) && $show_stills) {
      // Get the stills.
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->fields('mf');
      $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'mfm.mediafile_id = mf.mediafile_id');
      $query->fields('mfm');
      $query->condition('mf.' . mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, $mediafile_ids_found, 'IN');
      $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE);
      $query->orderBy('mfm.' . mediamosa_asset_mediafile_metadata_db::STILL_ORDER, 'ASC');
      $result = $query->execute();

      $i = 0;
      foreach ($result as $a_row) {
        // Create unique ID.
        $ticket = mediamosa_db::uuid($app_id);

        // Create ticket.
        mediamosa_ticket::create_play_proxy(
          $ticket,
          NULL,
          mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL,
          $app_id,
          $a_row[mediamosa_asset_mediafile_db::OWNER_ID],
          $a_row[mediamosa_asset_mediafile_db::ID]
        );

        // Create response.
        $response = mediamosa_asset_mediafile_play_proxy::create_response_still($ticket, $app_id);
        $a_row['still_ticket'] = $response['output'];

        // Add still to mediafile.
        assert(!empty($a_row[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE])); // Make sure its here.
        $items[$a_row[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE]]['still']['#'. $i++] = $a_row;
      }
    }

    // Get the app.
    $a_app = mediamosa_app::get_by_appid($app_id);

    $patterns = array(
      '/{asset_id}/i',
      '/{mediafile_id}/i',
      '/{mediafile_filename}/i',
    );

    // Result array.
    $items_result = array();

    $i = 1;

    // Now add to result array.
    // Must be in the same order as $mediafile_ids.
    foreach ($mediafile_ids as $mediafile_id) {
      if (!isset($items[$mediafile_id])) { // In case the mediafile was not found.
        continue;
      }

      // Set empty array for metadata.
      $items[$mediafile_id]['metadata'] = array();

      // Test for downloadable.
      $is_downloadable = mediamosa_lib::boolstr2bool($items[$mediafile_id][mediamosa_asset_mediafile_db::IS_DOWNLOADABLE]);

      // Download URI replacements.
      $replacements = array(
        $items[$mediafile_id][mediamosa_asset_db::ID],
        $items[$mediafile_id][mediamosa_asset_mediafile_db::ID],
        $items[$mediafile_id][mediamosa_asset_mediafile_db::FILENAME],
      );

      // Default empty.
      $items[$mediafile_id]['ega_download_url'] = '';

      // If downloadable and application has its download URL set, then include it in our output.
      if ($is_downloadable && !empty($a_app[mediamosa_app_db::DOWNLOAD_URL])) {

        // Set EGA download url.
        $items[$mediafile_id]['ega_download_url'] = preg_replace($patterns, $replacements, $a_app[mediamosa_app_db::DOWNLOAD_URL]);
      }

      // Default empty.
      $items[$mediafile_id]['ega_stream_url'] = '';

      // Stream URL, only if streamable.
      if (mediamosa_server::is_streamable($items[$mediafile_id][$prefix . mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE])) {
        $items[$mediafile_id]['ega_play_url'] = preg_replace($patterns, $replacements, $a_app[mediamosa_app_db::PLAY_PROXY_URL]);

        // Output extra?
        if ($is_oai) {
          // Stream URL(s) for OAI.
          $mediafile_app_id = $items[$mediafile_id][mediamosa_asset_mediafile_db::APP_ID];

          // Check if mediafile APP ID is in the requested app_ids.
          if (in_array($mediafile_app_id, $app_ids)) {
            $items[$mediafile_id]['ega_stream_url'][] = !empty($a_app[mediamosa_app_db::STREAM_URL]) ? preg_replace($patterns, $replacements, $a_app[mediamosa_app_db::STREAM_URL]) : '';
          }

          // Get masters of this mediafile.
          $a_app_id_master = mediamosa_aut_app_master_slave::master_get($mediafile_app_id, mediamosa_aut::AUT_TYPE_MEDIAFILE, $items[$mediafile_id][mediamosa_asset_mediafile_db::ID]);

          foreach ($a_app_id_master as $app_id_master) {
            if (in_array($app_id_master, $app_ids)) {
              // Get the master app.
              $app_tmp = mediamosa_app::get_by_appid($app_id_master);

              // Set the ega_stream_url of the master.
              $items[$mediafile_id]['ega_stream_url'][] = preg_replace($patterns, $replacements, $app_tmp[mediamosa_app_db::STREAM_URL]);
            }
          }
        }
        else {
          $items[$mediafile_id]['ega_stream_url'][] = !empty($a_app[mediamosa_app_db::STREAM_URL]) ? preg_replace($patterns, $replacements, $a_app[mediamosa_app_db::STREAM_URL]) : '';
        }
      }

      // Remove the prefix from the metadata and move into the metadata array of the asset.
      foreach ($items[$mediafile_id] as $key => $value) {
        if (mediamosa_unicode::strpos($key, $prefix) === 0) { // is het een metadata tag?
          $new_key = mediamosa_unicode::substr($key, mediamosa_unicode::strlen($prefix));
          $items[$mediafile_id]['metadata'][$new_key] = $items[$mediafile_id][$key];
          unset($items[$mediafile_id][$key]);
        }
      }

      // Unset these;
      unset(
        $items[$mediafile_id][mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT],
        $items[$mediafile_id][mediamosa_asset_mediafile_db::ASSET_ID_ROOT]
      );

      // Add to result.
      $items_result['mediafile']['#' . serialize(array('id' => $i++))] = $items[$mediafile_id];
    }

    return $items_result;
  }

  /**
   * Search for mediafiles.
   * Normally called _media_management_return_mediafile_list (enrich_response_mediafile) at the end.
   *
   * 1.x _media_management_search_mediafiles
   *
   * @param string $asset_id
   * @param string $tag
   * @param boolean $is_still
   */
  static public function mediafiles_search($asset_id, $tag = NULL, $is_still = FALSE, array $app_ids = array()) {

    assert($asset_id !== FALSE);

      // Asset must exist.
    mediamosa_db::db_must_exists(
      mediamosa_asset_db::TABLE_NAME,
      array(
        mediamosa_asset_db::ID => $asset_id
      )
    );

    // FIXME write me to normal db_query.
    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "mf.mediafile_id";
    $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset} AS a";

    $a_query[mediamosa_db_query::ALLOW_DISTINCT] = FALSE;

    // No left join here, only include assets that have mediafiles
    $a_query[mediamosa_db_query::A_JOIN]["mediafile"][] = "JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id = mf.asset_id_root";

    if (!empty($app_ids)) {
      // Filter mediafiles according to the master-slave aut.
      $a_query[mediamosa_db_query::A_JOIN]["mediafile"][] = sprintf("
        JOIN {mediamosa_aut_app_master_slave} AS ms ON ms.aut_object_type = 'MEDIAFILE'
          AND ms.aut_object_id = mf.mediafile_id
          AND ms.app_id_master IN (%s)", implode(',', $app_ids));
    }

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("a.asset_id = '%s'", mediamosa_db::escape_string($asset_id));
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("mf.is_still = '%s'", $is_still ? mediamosa_asset_mediafile_db::IS_STILL_TRUE :  mediamosa_asset_mediafile_db::IS_STILL_FALSE);

    if ($tag) {
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['tag'] = sprintf("mf.tag = '%s'", mediamosa_db::escape_string($tag));
    }

    // Create the query
    $query = mediamosa_db_query::query_select($a_query);

    // Do the query and return found mediafiles as an array with mediafile_ids.
    return mediamosa_db::db_query($query)->fetchCol();
  }

  /**
   * Get all transcoded mediafiles of given asset ID.
   *
   * @param $asset_id
   */
  static public function get_all_transcoded($asset_id) {
    // FIXME: just wondering if it must be asset_root_id here.
    return mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->condition(mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE)
      ->isNotNull(mediamosa_asset_mediafile_db::TOOL)
      ->isNotNull(mediamosa_asset_mediafile_db::COMMAND)
      ->isNotNull(mediamosa_asset_mediafile_db::FILE_EXTENSION)
      ->execute();
  }

  /**
   * Analyse the mediafile and store it into the database metadata of mediafile.
   *
   * 1.x: vpx_jobs_get_analysis_upload
   *
   * @param $app_id
   * @param $mediafile_id
   */
  static public function analyse($app_id, $mediafile_id) {

    // Remove all metadata.
    // FIXME: why now? (also done when we insert it)
    mediamosa_asset_mediafile_metadata::delete_by_mediafileid($mediafile_id);

    // Get the app.
    $a_mediamosa_app = mediamosa_app::get_by_appid($app_id);

    $options = array();

    if ($a_mediamosa_app[mediamosa_app_db::ALWAYS_HINT_MP4] == mediamosa_app_db::ALWAYS_HINT_MP4_TRUE) {
      $options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
    }

    if ($a_mediamosa_app[mediamosa_app_db::ALWAYS_INSERT_MD] == mediamosa_app_db::ALWAYS_INSERT_MD_TRUE) {
      $options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
    }

    $execution_string = sprintf('%s %s %s',
      mediamosa_settings::analyse_file(),
      mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(),
      $mediafile_id
    );

    $execution_string .= (count($options) ? ' ' . implode(' ', $options) : '');

    $output = array();
    mediamosa_io::exec($execution_string . " 2>&1", $output);

    return $output;
  }

  /**
   * Returns TRUE/FALSE if the mediafile is an original.
   *
   * @param array $mediafile
   */
  static public function is_original($mediafile) {
    // FIXME: will need to convert into mediafile_id_source.
    return ($mediafile[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE);
  }


  /**
   * Update the mediafile.
   *
   * @param integer $app_id
   * @param string $mediafile_id
   * @param string $user_id
   * @param string $type
   * @param array $fields
   */
  static public function update($app_id, $mediafile_id, $user_id, array $fields) {
    // Cant mix input of uri/filename/is_downloadable.
    if (isset($fields[mediamosa_asset_mediafile_db::URI]) && (isset($fields[mediamosa_asset_mediafile_db::FILENAME]) || isset($fields[mediamosa_asset_mediafile_db::IS_DOWNLOADABLE]))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MIX_OF_URI_AND_FILE);
    }

    // Mediafile must exists.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Does the user has access?
    mediamosa_aut::owner_check(
      $app_id,
      $user_id,
      $mediafile[mediamosa_asset_mediafile_db::APP_ID],
      $mediafile[mediamosa_asset_mediafile_db::OWNER_ID]
    );

    // Can't change the type.
    if (!empty($mediafile[mediamosa_asset_mediafile_db::URI]) && ( isset($fields[mediamosa_asset_mediafile_db::FILENAME]) || isset($fields[mediamosa_asset_mediafile_db::IS_DOWNLOADABLE]) ) || (!empty($mediafile[mediamosa_asset_mediafile_db::FILENAME]) && isset($fields[mediamosa_asset_mediafile_db::URI])) ) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CHANGE_URI_AND_FILE);
    }

    // Update the timestamps of the asset
    mediamosa_asset::update_asset_timestamps($mediafile[mediamosa_asset_mediafile_db::ASSET_ID]);

    // Add changed field.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Build the query.
    mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_mediafile_db::ID, $mediafile_id)
      ->execute();

    // Set the external.
    mediamosa_asset::update_asset_info_is_external($mediafile_id);
    // Set the empty asset.
    mediamosa_asset::update_asset_info_is_empty_asset($mediafile_id);

  }

  /**
   * Create the mediafile.
   *
   * @param string $mediafile_id
   *  Provide the new mediafile_id.
   * @param integer $app_id
   * @param string $asset_id
   *  Parent asset.
   * @param string $user_id
   *  The new owner.
   * @param array $fields
   *  Optional fields. See mediamosa_asset_mediafile_db.
   */
  static public function create($mediafile_id, $app_id, $asset_id, $user_id, array $fields = array()) {
    // Is original with mediafile_id_source combo.
    assert(
      (
        isset($fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE]) && $fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE] != ''
        &&
        $fields[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE
      )
      ||
      (
        (!isset($fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE]) || $fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE] == '')
        &&
        $fields[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE
      )
    );

    $fields[mediamosa_asset_mediafile_db::ID] = $mediafile_id;
    $fields[mediamosa_asset_mediafile_db::APP_ID] = $app_id;
    $fields[mediamosa_asset_mediafile_db::ASSET_ID] = $asset_id;
    $fields[mediamosa_asset_mediafile_db::ASSET_ID_ROOT] = $asset_id; // depricated, set to own asset.
    $fields[mediamosa_asset_mediafile_db::OWNER_ID] = $user_id;

    // Enrich for created/changed.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // Set the external.
    mediamosa_asset::update_asset_info_is_external($mediafile_id);
    // Set the empty asset.
    mediamosa_asset::update_asset_info_is_empty_asset($mediafile_id);

    // Return the mediafile ID.
    return $mediafile_id;
  }
}
