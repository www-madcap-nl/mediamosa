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
 * The helper functions of asset module.
 */

class mediamosa_asset {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Preprocess the asset output.
   *
   * @param array $a_input
   * @param array $a_metadata_definitions_full
   * @return array
   */
  static public function process_asset_output($a_asset, $a_metadata_definitions_full) {
    foreach ($a_metadata_definitions_full as $key => $value) {
      $a_asset[$value['propgroup_name']][$key] = NULL;
    }

    unset(
      $a_asset['parent_id'],
      $a_asset['asset_property_value'],
      $a_asset['asset_property_name'],
      $a_asset['asset_property_group_name']
    );

    return $a_asset;
  }

  /**
   * Get the asset.
   *
   * @param string $asset_id
   */
  static public function get($asset_id, $app_id = NULL) {

    $query = mediamosa_db::db_select(mediamosa_asset_db::TABLE_NAME, 'a');
    $query->fields('a');
    $query->condition('a.' . mediamosa_asset_db::ID, $asset_id);
    if (isset($app_id)) {
      $query->condition('a.' . mediamosa_asset_db::APP_ID, $app_id);
    }
    return $query->execute()->fetchAssoc();
  }

  /**
   * Get multiple assets within an array, with key
   * as asset_id.
   *
   * @param array $a_asset_ids
   */
  static public function getAllAssoc(array $a_asset_ids) {

    $query = mediamosa_db::db_select(mediamosa_asset_db::ID, 'a');
    $query->fields('a');
    $query->condition('a.' . mediamosa_asset_db::ID, $a_asset_ids, 'IN');
    return $query->execute()->fetchAllAssoc(mediamosa_asset_db::ID);
  }

  /**
   * Delete jobs attached to asset.
   *
   * @param string $asset_id
   */
  static public function delete_jobs($asset_id) {
//@todo:
    return self::delete_job_by_asset($asset_id);
  }

  /**
   * Remove asset from database.
   *
   * @param string $asset_id
   *  The asset to delete.
   * @param bool $cascade
   *  Cascade mode will allow deletion of mediafiles. If FALSE and mediafiles (not counting
   *  stills) are present, then deletion will fail.
   *
   * Note: does not return boolean, will throw exception when failure.
   *
   * Big change from 1.x; it will now remove mediafiles.
   */
  static public function delete($asset_id, $cascade = TRUE) {

    // @todo: start of transaction?

    // Find all my asset children.
    $a_asset_ids = mediamosa_db::db_query(
      'SELECT #asset_id FROM {#mediamosa_asset} WHERE #parent_id = :asset_id',
      array(
        '#asset_id' => mediamosa_asset_db::ID,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#parent_id' => mediamosa_asset_db::PARENT_ID,
        ':asset_id' => $asset_id,
      )
    )->fetchCol();

    // Add me (last).
    $a_asset_ids[] = $asset_id;

    // Find all mediafiles (except stills, they dont count for cascade mode.).
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->addField('mf', mediamosa_asset_mediafile_db::ID);
    $query->condition('mf' . mediamosa_asset_mediafile_db::ASSET_ID, $a_asset_ids, 'IN');
    $query->condition('mf' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
    $mediafile_ids = $query->execute()->fetchCol();

    // Any mediafiles left and casade off?
    if (count($mediafile_ids) && !$cascade) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_EMPTY, array('@mediafile_count' => count($mediafile_ids)));
    }

    // Remove all jobs.
    foreach ($a_asset_ids as $asset_id) {
      if (!mediamosa_asset::delete_jobs($asset_id)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOBS_COULD_NOT_BE_STOPPED);
      }
    }


    // Check the temporary wmv files
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->addField('mf', mediamosa_asset_mediafile_db::ID);
    $query->condition('mf' . mediamosa_asset_mediafile_db::ASSET_ID, $a_asset_ids, 'IN');
    $query->condition('mf' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
    $query->condition('mf' . mediamosa_asset_mediafile_db::ID, '"%.avi"', 'LIKE');
    $mediafile_ids = $query->execute()->fetchCol();
    if (count($mediafile_ids)) {
      // Delete the temporary wmv files
      mediamosa_asset_mediafile::delete($mediafile_ids);
    }
    unset($mediafile_ids);


    // Remove all metadata of the asset.
    $query = mediamosa_db::db_delete(mediamosa_asset_metadata_db::TABLE_NAME);
    $query->condition(mediamosa_asset_metadata_db::ASSET_ID, $asset_id);
    $query->execute();

    // Find all mediafiles.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->addField('mf', mediamosa_asset_mediafile_db::ID);
    $query->condition(mediamosa_asset_mediafile_db::ASSET_ID_ROOT, $asset_id);
    $a_mediafile_ids = $query->execute()->fetchCol();

    // Delete the mediafiles.
    mediamosa_asset_mediafile::delete($a_mediafile_ids);

    // @todo: stats
    //mediamosa_statistics::delete_by_asset_id($asset_id)
    // db_query("DELETE FROM {statistics_stream_request} WHERE asset_id = '%s'", $asset_id);

    // Find my children.
    $result = mediamosa_db::db_query(
      "SELECT asset_id FROM {#mediamosa_asset} WHERE parent_id = :asset_id",
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        ':asset_id' => $asset_id
      )
    );

    // Foreach my children and call myself again.
    foreach ($result as $child_asset_id) {
      mediamosa_asset::delete($child_asset_id);
    }

    // Now we removed our children, lets remove us.
    $app_id = mediamosa_db::db_query(
      "SELECT #app_id FROM {#mediamosa_asset} WHERE #asset_id = :asset_id",
      array(
        '#app_id' => mediamosa_asset_db::APP_ID,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    )->fetchField();

    // Find my collections.
    $result = mediamosa_db::db_query(
      "SELECT #coll_id FROM {#mediamosa_collection} WHERE #asset_id = :asset_id",
      array(
        '#coll_id' => mediamosa_asset_collection_db::COLL_ID,
        '#mediamosa_collection' => mediamosa_asset_collection_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_collection_db::ASSET_ID,
        ':asset_id' => $asset_id
      )
    );
    // Foreach my collection.
    $is_coll = FALSE;
    foreach ($result as $collection_id) {
      $is_coll = TRUE;
      // Create entry in the mediamosa_asset_delete table.
      mediamosa_db::db_query(
        "REPLACE LOW_PRIORITY {#mediamosa_asset_delete} (#asset_id, #app_id, #videotimestampmodified, #coll_id) VALUES (:asset_id, :app_id, NOW(), :coll_id)",
        array(
          '#mediamosa_asset_delete' => mediamosa_asset_delete_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_delete_db::ID,
          '#app_id' => mediamosa_asset_delete_db::APP_ID,
          '#videotimestampmodified' => mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED,
          '#coll_id' => mediamosa_asset_delete_db::COLL_ID,
          ':asset_id' => $asset_id,
          ':app_id' => $app_id,
          ':coll_id' => $collection_id
        )
      );
    }
    if (!$is_coll) {
      // Create entry in the mediamosa_asset_delete table.
      mediamosa_db::db_query(
        "REPLACE LOW_PRIORITY {#mediamosa_asset_delete} (#asset_id, #app_id, #videotimestampmodified, #coll_id) VALUES (:asset_id, :app_id, NOW(), NULL)",
        array(
          '#mediamosa_asset_delete' => mediamosa_asset_delete_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_delete_db::ID,
          '#app_id' => mediamosa_asset_delete_db::APP_ID,
          '#videotimestampmodified' => mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED,
          '#coll_id' => mediamosa_asset_delete_db::COLL_ID,
          ':asset_id' => $asset_id,
          ':app_id' => $app_id
        )
      );
    }

    // Remove the asset from each collection.
    mediamosa_asset_collection::delete($asset_id);

    // Now delete the asset.
    $query = mediamosa_db::db_delete(mediamosa_asset_db::TABLE_NAME);
    $query->condition(mediamosa_asset_db::ID, $asset_id);
    $query->execute();
  }

  /**
   * Update the normalized fields in asset table.
   *
   * @param string $mediafile_id
   */
  static public function update_asset_info($mediafile_id) {

    // Find top asset.
    $asset_id_root = mediamosa_db::db_query(
      "SELECT #asset_id_root FROM {#mediamosa_asset_mediafile} WHERE #mediafile_id = :mediafile_id",
      array(
        '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        ':mediafile_id' => $mediafile_id,
      )
    )->fetchField();

    // Find the length of the first mediafile.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->fields('mfm', array(mediamosa_asset_mediafile_metadata_db::FILE_DURATION, mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE));
    $query->join(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', "mf.mediafile_id = mfm.mediafile_id AND mf.is_original_file = 'TRUE'");
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID_ROOT, $asset_id_root);
    $query->range(0, 1);
    $mediafile_info = $query->execute()->fetchAssoc();

    // Set the length of the 1st original mediafile in the parent asset.
    if ($mediafile_info && $mediafile_info['file_duration'] !== NULL) {
      mediamosa_db::db_query(
        "UPDATE {#mediamosa_asset} SET #mediafile_duration = :mediafile_duration, #mediafile_container_type = :mediafile_container_type WHERE #asset_id = :asset_id",
        array(
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#mediafile_duration' => mediamosa_asset_db::MEDIAFILE_DURATION,
          ':mediafile_duration' => $mediafile_info[mediamosa_asset_mediafile_metadata_db::FILE_DURATION],
          '#mediafile_container_type' => mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE,
          ':mediafile_container_type' => $mediafile_info[mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE],
          '#asset_id' => mediamosa_asset_db::ID,
          ':asset_id' => $asset_id_root,
        )
      );
    }

    // Set the external.
    mediamosa_asset::update_asset_info_is_external($mediafile_id, $asset_id_root);
  }

  /**
   * Update the normalized field in asset for is_external flag.
   *
   * @param string $mediafile_id
   * @param string $asset_id_root
   */
  static public function update_asset_info_is_external($mediafile_id, $asset_id_root = NULL) {
  // zoek de bijbehorende asset_id op
    if (!isset($asset_id_root)) {
      // Find top asset.
      $asset_id_root = mediamosa_db::db_query(
        "SELECT #asset_id_root FROM {#mediamosa_asset_mediafile} WHERE #mediafile_id = :mediafile_id",
        array(
          '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          ':mediafile_id' => $mediafile_id,
        )
      )->fetchField();
    }

    $a_mediafile = mediamosa_db::db_query_range(
      "SELECT #uri, #filename FROM {#mediamosa_asset_mediafile} WHERE #is_original_file = :TRUE AND #asset_id_root = :asset_id_root",
      0, // From.
      1, // Count.
      array(
        '#uri' => mediamosa_asset_mediafile_db::URI,
        '#filename' => mediamosa_asset_mediafile_db::FILENAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#is_original_file' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
        ':TRUE' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
        '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
        ':asset_id_root' => $asset_id_root,
      )
    )->fetchAssoc();

    if ($a_mediafile) {
      $uri = $a_mediafile[mediamosa_asset_mediafile_db::URI];
      $filename = $a_mediafile[mediamosa_asset_mediafile_db::FILENAME];

      // Set is_external && is_empty_asset
      mediamosa_db::db_query(
        "UPDATE {#mediamosa_asset} SET #is_external = :is_external, #is_empty_asset = :is_empty_asset WHERE #asset_id = :asset_id",
        array(
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#is_external' => mediamosa_asset_db::IS_EXTERNAL,
          ':is_external' => trim($uri) != '' ? mediamosa_asset_db::IS_EXTERNAL_TRUE : mediamosa_asset_db::IS_EXTERNAL_FALSE,
          '#is_empty_asset' => mediamosa_asset_db::IS_EMPTY_ASSET,
          ':is_empty_asset' => trim($uri) != '' || trim($filename) != '' ? mediamosa_asset_db::IS_EMPTY_ASSET_TRUE : mediamosa_asset_db::IS_EMPTY_ASSET_FALSE,
          '#asset_id' => mediamosa_asset_db::ID,
          ':asset_id' => $asset_id_root,
        )
      );
    }
  }

  /**
   * Collect the asset information.
   *
   * 1.x: media_management_asset_collect
   *
   * @param array $a_asset_ids
   *  Array asset_ids.
   * @param array $a_app_ids
   *  Application app ids.
   * @param string $user_id
   *  The aut_user ID.
   * @param boolean $granted
   *  Show or hide accessable assets.
   * @param array $a_aut_group_id
   *  Group ID authentication.
   * @param string $aut_domain
   *  Domain authentication.
   * @param string $aut_realm
   *  Realm authentication.
   * @param boolean $is_app_admin
   *  Used for including unappropriate assets.
   * @param $show_stills
   *  Include stills.
   * @param $show_collections
   *  Include collection information.
   */
  static public function asset_collect(
    array $a_asset_ids,
    array $a_app_ids,
    $aut_user_id,
    $granted,
    array $a_aut_group_id,
    $aut_domain,
    $aut_realm,
    $is_app_admin = FALSE,
    $show_stills = TRUE,
    $show_collections = FALSE
  ) {
    $a_prop_ids = array();
    $a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($a_app_ids);
    foreach ($a_metadata_definitions_full as $metadata_definitions_full) {
      $a_prop_ids[] = $metadata_definitions_full['propdef_id'];
    }

    $s_ids = implode("','", $a_asset_ids);

    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "a.*";
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "ap.val_char AS asset_property_value";
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "apd.prop_name AS asset_property_name";
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "apg.propgroup_name AS asset_property_group_name";

    // Main select on asset table.
    $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset} AS a";

    // Joins.
    $a_query[mediamosa_db_query::A_JOIN]['ap'] = "LEFT JOIN {mediamosa_asset_metadata} AS ap ON ap.asset_id = a.asset_id";
    $a_query[mediamosa_db_query::A_JOIN]['apd'] = "LEFT JOIN {mediamosa_asset_metadata_property} AS apd ON ap.prop_id = apd.prop_id";
    $a_query[mediamosa_db_query::A_JOIN]['apg'] = "LEFT JOIN {mediamosa_asset_metadata_property_group} AS apg ON apd.propgroup_id = apg.propgroup_id";

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['ap'][mediamosa_db_query::WHERE_OR]['prop_id'][] = sprintf('apd.prop_id IN(%s)', implode(',', $a_prop_ids));
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['ap'][mediamosa_db_query::WHERE_OR]['prop_id'][] = "apd.prop_id IS NULL"; // in case it doesn't have properties
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['a'][] = sprintf("a.asset_id IN ('%s')", $s_ids);

    $a_query[mediamosa_db_query::A_ORDER_BY][] = "ap.prop_id ASC";

    // Build the query
    $s_query = mediamosa_db_query::query_select($a_query);

    // Do the query.
    $result = mediamosa_db::db_query($s_query);

    $a_items = array();
    foreach ($result as $a_row) {
      if (!empty($a_row['still_id']) && $a_row['still_default'] != 'FALSE') {
        $a_items['stills'][$a_row['asset_id']] = $a_row['still_id'];
      }

      unset(
        $a_row['created'],
        $a_row['changed'],
        $a_row['still_id']
      );

      $a_items['assets'][] = $a_row;
    }

    $result = mediamosa_db::db_query(sprintf("SELECT * FROM {mediamosa_asset_delete} WHERE asset_id IN ('%s')", $s_ids));
    foreach ($result as $a_row) {
      $a_row['status'] = 'deleted';
      $a_items['assets'][] = $a_row;
    }

    $result = mediamosa_app::get_by_appids($a_app_ids,
      array(
        'id' => mediamosa_app_db::APP_ID,
        mediamosa_app_db::PLAY_PROXY_URL,
        mediamosa_app_db::VIEW_ASSET_URL,
        mediamosa_app_db::PREVIEW_PROFILE_ID,
        mediamosa_app_db::STILL_URL
      )
    );
    foreach ($result as $a_app) {
      $a_items['app_info'][$a_app['id']] = $a_app;
    }

    // Get all favorites.
    $a_items['user_favorites'] = array();
    if ($aut_user_id != '') {
      $a_query = array();
      $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "fav_id";
      $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_user_favorite}";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("name = '%s'", mediamosa_db::escape_string($aut_user_id));
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("app_id IN(%s)", implode(',', $a_app_ids));
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("fav_type = '%s'", mediamosa_user_favorite_db::FAV_TYPE_ASSET);
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("fav_id IN ('%s')", $s_ids);
      $s_query = mediamosa_db_query::query_select($a_query);
      $result = mediamosa_db::db_query($s_query);

      foreach ($result as $a_row) {
        $a_items['user_favorites'][] = $a_row[mediamosa_user_favorite_db::FAV_ID];
      }
    }

    // Get all preview mediafiles.
    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'm.mediafile_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'm.filename AS mediafile_filename';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'IFNULL(a.parent_id, a.asset_id) AS asset_id';
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_asset_mediafile} AS m';
    $a_query[mediamosa_db_query::A_JOIN]['asset'] = 'JOIN {mediamosa_asset} AS a USING(asset_id)';
    if (isset($a_items['app_info'])) {
      $a_keys = array_keys($a_items['app_info']);
      foreach ($a_keys as $key) {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['profile'][mediamosa_db_query::WHERE_OR][] = sprintf("(m.transcode_profile_id = %d AND m.app_id = %d)", $a_items['app_info'][$key]['preview_profile_id'], $key);
      }
    }

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("a.app_id IN(%s)", implode(",", $a_app_ids));
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][mediamosa_db_query::WHERE_OR][] = sprintf("a.asset_id IN('%s')", $s_ids);
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][mediamosa_db_query::WHERE_OR][] = sprintf("a.parent_id IN('%s')", $s_ids);

    $s_query = mediamosa_db_query::query_select($a_query);
    $result = mediamosa_db::db_query($s_query);
    foreach ($result as $a_row) {
      $a_items['mediafile_preview'][$a_row['asset_id']] = $a_row;
    }

    // If granted is TRUE, we check access, but allow assets in our result even when we dont have access.
    // If FALSE, we expect search only to supply the assets where we have access on.
    $a_items['granted'] = array(); // must exist, else granted flag is not included on the non-granted

    if ($granted) {
      $a_asset_ids_access = mediamosa_aut::access_check_media($a_asset_ids, $a_app_ids, $aut_user_id, $a_aut_group_id, $aut_domain, $aut_realm, $is_app_admin);

      foreach ($a_asset_ids_access as $asset_id) {
        $a_items['granted'][$asset_id] = $asset_id;
      }
    }
    else {
      // Expect we have access on all...
      foreach ($a_items['assets'] as $a_row) {
        $a_items['granted'][$a_row['asset_id']] = $a_row['asset_id'];
      }
    }

    return mediamosa_asset::enrich_response_asset($a_items, $a_asset_ids, $a_app_ids, $aut_user_id, $show_stills, $show_collections);
  }

  /**
   * Enrich the response object with asset information.
   *
   * @param array $a_items
   * @param array $a_asset_ids
   * @param array $a_app_ids
   * @param string $aut_user_id
   * @param boolean $show_stills
   * @param boolean $show_collections
   *  Include collections (was hack on _GET)
   */
  static public function enrich_response_asset(array $a_items, array $a_asset_ids, array $a_app_ids, $aut_user_id, $show_stills = TRUE, $show_collections = FALSE) {
    $a_collections = array();

    $a_items_pre_sort = array();

    // Need the first one.
    $app_id = reset($a_app_ids);

    // Get the metadata definitions.
    $a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($a_app_ids);

    // Need to get collections?
    if ($show_collections) {
      // Get all collection information in one go.
      $a_asset_ids = array();
      foreach ($a_items['assets'] as $a_asset) {
        $a_asset_ids[] = $a_asset['asset_id'];
      }

      $result = mediamosa_asset_collection::get_by_asset_ids(
        $a_asset_ids,
        array(
          mediamosa_asset_collection_db::ASSET_ID => 'ca.' . mediamosa_asset_collection_db::ASSET_ID,
          mediamosa_collection_db::ID => 'c.' . mediamosa_collection_db::ID,
          mediamosa_collection_db::TITLE => 'c.' . mediamosa_collection_db::TITLE
        )
      );

      // Now store it in the cache.
      foreach ($result as $a_collection) {
        $a_collections[$a_collection[mediamosa_asset_collection_db::ASSET_ID]][$a_collection[mediamosa_asset_collection_db::COLL_ID]] = $a_collection;
      }
    }
    foreach ($a_items['assets'] as $a_asset) {
      // Now only stuff that may be set once per asset.
      if (!isset($a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]])) {
        $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]] = mediamosa_asset::process_asset_output($a_asset, $a_metadata_definitions_full);

        if (isset($a_items['app_info'][$a_asset['app_id']]['play_proxy_url']) && isset($a_items['mediafile_preview'][$a_asset[mediamosa_asset_db::ID]])) {
          $a_patterns = array(
            '/{asset_id}/i',
            '/{preview_profile_id}/i',
            '/{mediafile_id}/i',
            '/{mediafile_filename}/i',
          );

          $a_replacements = array(
            $a_asset[mediamosa_asset_db::ID],
            $a_items['app_info'][$a_asset['app_id']]['preview_profile_id'],
            $a_items['mediafile_preview'][$a_asset[mediamosa_asset_db::ID]]['mediafile_id'],
            $a_items['mediafile_preview'][$a_asset[mediamosa_asset_db::ID]]['mediafile_filename'],
          );

          $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['ega_play_url'] = isset($a_items['app_info'][$a_asset[mediamosa_asset_db::APP_ID]]) ? preg_replace($a_patterns, $a_replacements, $a_items['app_info'][$a_asset[mediamosa_asset_db::APP_ID]]['play_proxy_url']) : '';
        }
        else {
          foreach ($a_app_ids as $a_app_id) {
            if ($a_items['app_info'][$a_app_id]['view_asset_url']) {
              $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['ega_view_url'][] = preg_replace('/{asset_id}/i', $a_asset[mediamosa_asset_db::ID], $a_items['app_info'][$a_app_id]['view_asset_url']);
            }
          }
        }

        // Include the user favorites (if any).
        $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['is_favorite'] = isset($a_items['user_favorites']) && array_search($a_asset[mediamosa_asset_db::ID], $a_items['user_favorites']) !== FALSE ? 'TRUE' : 'FALSE';

        // Insert granted.
        $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['granted'] = isset($a_items['granted']) && array_search($a_asset[mediamosa_asset_db::ID], $a_items['granted']) !== FALSE ? 'TRUE' : 'FALSE';

        // Add ticket.
        $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['vpx_still_url'] = "";
        $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['ega_still_url'] = "";

        if ($show_stills) {
          $ticket = mediamosa_db::uuid($app_id);
          $a_mediafile_ext['asset_id'] = $a_asset[mediamosa_asset_db::ID];
          mediamosa_ticket::create_play_proxy($ticket, $a_mediafile_ext, mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL, $app_id, $aut_user_id, $a_items['stills'][$a_asset[mediamosa_asset_db::ID]]);

          // Create still response.
          $response = mediamosa_asset_mediafile_play_proxy::create_response_still($ticket, $app_id);
          $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['vpx_still_url'] = $response['output'];
          $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['ega_still_url'] = str_replace('{asset_id}', $a_asset[mediamosa_asset_db::ID], $a_items['app_info'][$a_asset['app_id']]['still_url']);
        }
      }

      // Now stuff that can be set more than once (f.e. metadata).
      if (!empty($a_asset['asset_property_group_name'])) {
        $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]][$a_asset['asset_property_group_name']][$a_asset['asset_property_name']][] = $a_asset['asset_property_value'];
      }

      // Add collection data.
      if ($show_collections && isset($a_collections[$a_asset[mediamosa_asset_db::ID]])) {
        foreach ($a_collections[$a_asset[mediamosa_asset_db::ID]] as $coll_id => $a_coll) {
          $a_items_pre_sort[$a_asset[mediamosa_asset_db::ID]]['collection_' . $coll_id] = $a_coll[mediamosa_collection_db::TITLE];
        }
      }
    }

    $a_items = array();
    foreach ($a_asset_ids as $s_id) {
      $a_items[] = $a_items_pre_sort[$s_id];
    }

    return $a_items;
  }

  /**
   * Update the dates for asset.
   *
   * 1.x: _media_management_update_asset_timestamps
   *
   * @param string $asset_id
   *
   */
  static public function update_asset_timestamps($asset_id) {
    $real_asset_id = mediamosa_db::db_query(
      'SELECT #parent_id FROM {#mediamosa_asset} WHERE #asset_id = :asset_id AND NOT #parent_id IS NULL',
      array(
        '#parent_id' => mediamosa_asset_db::PARENT_ID,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
      )
    );

    if ($real_asset_id !== FALSE) {
      $asset_id = $real_asset_id;
    }

    // Get the NOW date.
    $now = mediamosa::db_current_timestamp_now();

    // Update videotimestamp.
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #videotimestamp = :now WHERE #asset_id = :asset_id AND #videotimestamp IS NULL',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#videotimestamp' => mediamosa_asset_db::VIDEOTIMESTAMP,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
        ':now' => $now,
      )
    );

    // Update videotimestampmodified.
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #videotimestampmodified = :now WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#videotimestampmodified' => mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
        ':now' => $now,
      )
    );
  }

  /**
   * Increase viewed.
   *
   * 1.x: _mediafile_management_asset_viewed
   *
   * @param $asset_id
   */
  static public function asset_viewed($asset_id) {
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #viewed = #viewed + 1 WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#viewed' => mediamosa_asset_db::VIEWED,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    );
  }

  /**
   * Increase played.
   *
   * 1.x: _mediafile_management_asset_played
   *
   * @param $asset_id
   */
  static public function asset_played($asset_id) {
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #played = #played + 1 WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#played' => mediamosa_asset_db::PLAYED,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    );
  }

  /**
   * Test access for unappropriate.
   *
   * @param string $asset_id
   * @param array $app_id
   * @param string $user_id
   * @param bool $is_app_admin
   */
  static public function is_unappropriate($asset_id, array $a_app_ids, $user_id, $is_app_admin = FALSE) {

    $a_asset = self::get($asset_id);
    if (!$a_asset) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    // If its not unapp. then leave.
    if ($a_asset[mediamosa_asset_db::IS_UNAPPROPRIATE] != mediamosa_asset_db::IS_UNAPPROPRIATE_TRUE) {
      return;
    }

    // Must be owner or admin.
    try {
      mediamosa_aut::owner_check(reset($a_app_ids), $user_id, $a_asset[mediamosa_asset_db::APP_ID], $a_asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);
    }
    catch (mediamosa_exception_error_access_denied $e) {
      assert($e);
      throw new mediamosa_exception_error(mediamosa_db::ERRORCODE_IS_UNAPPROPRIATE);
    }
  }

  /**
   * @param $app_id
   *  The Application ID.
   * @param $owner_id
   *  The owner ID.
   * @param $group_id
   *  The group ID.
   * @param $reference_id
   *  The reference ID.
   * @param $provider_id
   *  The provider ID
   * @param $parent_id (depricated)
   *  The parent ID.
   */
  static public function create($app_id, $owner_id, $group_id = NULL, $reference_id = NULL, $provider_id = NULL, $parent_id = NULL) {

    // maak een uniek asset_id aan
    $asset_id = mediamosa_db::uuid($app_id);

    $a_fields = array(
      mediamosa_asset_db::ID => $app_id,
      mediamosa_asset_db::OWNER_ID => $owner_id,
    );

    if ($group_id) {
      $a_fields[mediamosa_asset_db::GROUP_ID] = $group_id;
    }
    if ($reference_id) {
      $a_fields[mediamosa_asset_db::REFERENCE_ID] = $reference_id;
    }
    if ($provider_id) {
      $a_fields[mediamosa_asset_db::PROVIDER_ID] = $provider_id;
    }
    if ($parent_id) {
      $a_fields[mediamosa_asset_db::PARENT_ID] = $parent_id;
    }

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_asset_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();

    // Return our ID.
    return $asset_id;
  }

  /**
   * Update a asset.
   *
   * @param string $app_id
   *  The application ID.
   * @param string $user_id
   *  The user id of current user.
   * @param bool $is_app_admin
   *  Is application admin.
   * @param string $asset_id
   *  The asset ID.
   * @param string $owner_id
   *  The new owner ID.
   * @param string $group_id
   *  The new group ID.
   * @param string $play_restriction_start
   *  Start range restriction.
   * @param string $play_restriction_end
   *  End range restriction.
   * @param bool $isprivate
   *  Is private flag.
   * @param bool $is_unappropriate
   *  Is unappropriate flag.
   */
  static public function update($app_id, $user_id, $is_app_admin, $asset_id, $owner_id = NULL, $group_id = NULL, $play_restriction_start = NULL, $play_restriction_end = NULL, $isprivate = NULL, $is_unappropriate = NULL) {

    // Asset must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id, mediamosa_asset_db::PARENT_ID => NULL));

    // Get the asset.
    $a_asset = mediamosa_asset::get($asset_id);

    // Must be owner.
    mediamosa_aut::owner_check($app_id, $user_id, $a_asset[mediamosa_asset_db::APP_ID], $a_asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

    // If not null, then check if this is the ega admin...
    if (!is_null($is_unappropriate)) {
      mediamosa_aut::app_admin_check($app_id, $a_asset[mediamosa_asset_db::APP_ID], $is_app_admin);
    }

    $a_fields = array();
    if (!is_null($play_restriction_start)) {
      $a_fields[mediamosa_asset_db::PLAY_RESTRICTION_START] = $play_restriction_start;
    }
    if (!is_null($play_restriction_end)) {
      $a_fields[mediamosa_asset_db::PLAY_RESTRICTION_END] = $play_restriction_end;
    }
    if (!is_null($isprivate)) {
      $a_fields[mediamosa_asset_db::ISPRIVATE] = $isprivate;
    }
    if (!is_null($is_unappropriate)) {
      $a_fields[mediamosa_asset_db::IS_UNAPPROPRIATE] = $is_unappropriate;
    }

    if ($is_app_admin) {
      if (!is_null($owner_id)) {
        $a_fields[mediamosa_asset_db::OWNER_ID] = $owner_id;
      }
      if (!is_null($group_id)) {
        $a_fields[mediamosa_asset_db::GROUP_ID] = $group_id;
      }
    }

    // retourneer een foutmelding als er geen wijzigingen gestuurd zijn
    if (!count($a_fields)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_CHANGES);
    }

    // Add changed value.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    // Update the asset.
    mediamosa_db::db_update(mediamosa_asset_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_asset_db::ID, $asset_id)
      ->execute();

    // Change the owner_id / group_id of mediafiles and jobs
    if ($is_app_admin) {
      // Collect the data
      $a_fields = array();
      if (!is_null($owner_id)) {
        $a_fields[mediamosa_asset_mediafile_db::OWNER_ID] = $owner_id;
      }
      if (!is_null($group_id)) {
        $a_fields[mediamosa_asset_mediafile_db::GROUP_ID] = $group_id;
      }
      if (count($a_fields)) {
        // Change the mediafile table
        mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
          ->fields($a_fields)
          ->condition(mediamosa_asset_mediafile_db::ASSET_ID_ROOT, $asset_id)
          ->execute();
        if (!is_null($owner_id)) {
          // Change the job table
          $a_fields = array();
          $a_fields[mediamosa_job_db::OWNER_ID] = $owner_id;
          mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
            ->fields($a_fields)
            ->condition(mediamosa_job_db::ASSET_ID, $asset_id)
            ->execute();

        }
      }
    }

  }

  /**
   * Given asset must exist.
   *
   * @param $asset_id
   */
  static public function must_exists($asset_id, $do_parent = FALSE) {
      // Asset must exist to be deleted.
    return mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));
  }

}
