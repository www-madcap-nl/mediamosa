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
   * Remove asset from database.
   *
   * @param string $asset_id
   *
   * Note: does not return boolean, will throw exception when failure.
   *
   * Big change from 1.x; it will now remove mediafiles.
   */
  static public function delete($asset_id) {

    // @todo: start of transaction?

    // Remove the asset from each collection.
    mediamosa_asset_collection::delete($asset_id);

    // Remove all metadata of the asset.
    $query = db_delete(mediamosa_asset_metadata_db::TABLE_NAME);
    $query->condition(mediamosa_asset_metadata_db::ASSET_ID, $asset_id);
    $query->execute();

    // Remove our stills.

    // Delete the mediafiles.
    mediamosa_asset_mediafile::delete($asset_id);

    // @todo: stats
    //mediamosa_statistics::delete_by_asset_id($asset_id)
    // db_query("DELETE FROM {statistics_stream_request} WHERE asset_id = '%s'", $asset_id);

    // Find my children.
    $result = mediamosa_db::db_query(
      "SELECT asset_id, app_id FROM {#mediamosa_asset} WHERE parent_id = :asset_id",
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

    // Create entry in the mediamosa_asset_delete table.
    mediamosa_db::db_query(
      "REPLACE LOW_PRIORITY {#mediamosa_asset_delete} (#asset_id, #app_id, #videotimestampmodified) VALUES (:asset_id, :app_id, NOW())",
      array(
        '#mediamosa_asset_delete' => mediamosa_asset_delete_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_delete_db::ID,
        '#app_id' => mediamosa_asset_delete_db::APP_ID,
        '#videotimestampmodified' => mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED,
        ':asset_id' => $asset_id,
        ':app_id' => $app_id
      )
    );

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
    $query = db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
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

  // media_management_asset_collect
  static public function asset_collect(
	  mediamosa_response $o_mediafile,
	  array $a_asset_ids,
	  array $a_app_ids,
	  $user_id,
	  $granted,
	  array $a_aut_group_id,
	  $s_aut_domain,
	  $s_aut_realm,
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
	  $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "apd.name AS asset_property_name";
	  $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "apg.name AS asset_property_group_name";

	  // Main select on asset table.
	  $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset} AS a";

	  // Joins.
	  $a_query[mediamosa_db_query::A_JOIN]['ap'] = "LEFT JOIN {mediamosa_asset_metadata} AS ap ON ap.asset_id = a.asset_id";
	  $a_query[mediamosa_db_query::A_JOIN]['apd'] = "LEFT JOIN {mediamosa_asset_metadata_property} AS apd ON ap.prop_id = apd.prop_id";
	  $a_query[mediamosa_db_query::A_JOIN]['apg'] = "LEFT JOIN {mediamosa_asset_metadata_property_group} AS apg ON apd.propgroup_id = apg.propgroup_id";

	  $a_query[mediamosa_db_query::A_WHERE][VPX_DB_WHERE_AND]['ap'][mediamosa_db_query::WHERE_OR]['prop_id'][] = sprintf('apd.prop_id IN(%s)', implode(',', $a_prop_ids));
	  $a_query[mediamosa_db_query::A_WHERE][VPX_DB_WHERE_AND]['ap'][mediamosa_db_query::WHERE_OR]['prop_id'][] = "apd.prop_id IS NULL"; // in case it doesn't have properties
	  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['a'][] = sprintf("a.asset_id IN ('%s')", $s_ids);

    $a_query[mediamosa_db_query::A_ORDER_BY][] = "ap.id ASC";

	  // Do the query
	  $s_query = mediamosa_db_query::query_select($a_query);
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
    if ($user_id != '') {
	    $a_query = array();
	    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "fav_id";
	    $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_user_favorite}";
	    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("name = '%s'", mediamosa_db::escape_string($user_id));
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
	  $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "m.mediafile_id";
	  $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "m.filename AS mediafile_filename";
	  $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "IFNULL(a.parent_id, a.asset_id) AS asset_id";
	  $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset_mediafile} AS m";
	  $a_query[mediamosa_db_query::A_JOIN]['asset'] = "JOIN {mediamosa_asset} AS a USING(asset_id)";
	  if (isset($a_items['app_info'])) {
	    foreach ($a_items['app_info'] as $key => $value) {
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
      $a_asset_ids_access = mediamosa_aut::access_check_media($a_asset_ids, $a_app_ids, $user_id, $a_aut_group_id, $s_aut_domain, $s_aut_realm, $is_app_admin);

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

	  mediamosa_asset::enrich_response_asset($o_mediafile, $a_items, $a_asset_ids, $a_app_ids, $user_id, $show_stills, $show_collections);
  }

  /**
   *
   * @param mediamosa_response $o_mediafile
   * @param array $a_items
   * @param array $a_asset_ids
   * @param array $a_app_ids
   * @param string $user_id
   * @param boolean $show_stills
   * @param boolean $show_collections
   *  Include collections (was hack on _GET)
   */
  static public function enrich_response_asset(mediamosa_response $o_mediafile, array $a_items, array $a_asset_ids, array $a_app_ids, $user_id, $show_stills = TRUE, $show_collections = FALSE) {
	  $colls = array();

	  $s_current_asset = FALSE;
	  $a_items_pre_sort = array();

	  // Need the first one.
	  $app_id = reset($a_app_ids);

	  // Get the metadata definitions.
	  $a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($a_app_ids);

	  foreach ($a_items['assets'] as $a_asset) {
	    if (!isset($a_items_pre_sort[$a_asset['asset_id']])) {
	      $asset_id = $a_asset['asset_id'];

	      $a_items_pre_sort[$asset_id] = mediamosa_asset::process_asset_output($a_asset, $a_metadata_definitions_full);

	      if ($a_items['app_info'][$a_asset['app_id']]['play_proxy_url'] && isset($a_items['mediafile_preview'][$asset_id])) {
	        $a_patterns = array(
	          '/{asset_id}/i',
	          '/{preview_profile_id}/i',
	          '/{mediafile_id}/i',
	          '/{mediafile_filename}/i',
	        );

	        $a_replacements = array(
	          $asset_id,
	          $a_items['app_info'][$a_asset['app_id']]['preview_profile_id'],
	          $a_items['mediafile_preview'][$asset_id]['mediafile_id'],
	          $a_items['mediafile_preview'][$asset_id]['mediafile_filename'],
	        );

	        $a_items_pre_sort[$asset_id]['ega_play_url'] = preg_replace($a_patterns, $a_replacements, $a_items['app_info'][$a_asset['app_id']]['play_proxy_url']);
	      }
	      else {
	        $a_items_pre_sort[$asset_id]['ega_view_url'] = preg_replace('/{asset_id}/i', $asset_id, $a_items['app_info'][$a_asset['app_id']]['view_asset_url']);
	      }

	      // voeg is_favorite toe
	      if (!isset($a_items['user_favorites'])) {
	        $a_items['user_favorites'] = array();
	      }
	      $a_items_pre_sort[$asset_id]['is_favorite'] = (array_search($asset_id, $a_items['user_favorites']) !== FALSE) ? 'TRUE' : 'FALSE';

	      // voeg granted toe
	      if (!isset($a_items['granted'])) {
	        $a_items['granted'] = array();
	      }
	      $a_items_pre_sort[$asset_id]['granted'] = (array_search($asset_id, $a_items['granted']) !== FALSE) ? 'TRUE' : 'FALSE';

	      // voeg de still toe: ticket
	      $a_items_pre_sort[$asset_id]['vpx_still_url'] = "";
	      $a_items_pre_sort[$asset_id]['ega_still_url'] = "";

	      if ($show_stills) {
	        $ticket = mediamosa_db::uuid($app_id);
	        $a_mediafile_ext['asset_id'] = $asset_id;
	        $result = mediamosa_ticket::create($ticket, $a_mediafile_ext, mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL, $app_id, $user_id, $a_items['stills'][$asset_id]);

	        // voeg de still toe: still zelf
	        if ($result !== FALSE) {
	          $response = mediamosa_asset_mediafile_play_proxy::create_response_still($ticket, $app_id);
	          $a_items_pre_sort[$asset_id]['vpx_still_url'] = $response['output'];
	          $a_items_pre_sort[$asset_id]['ega_still_url'] = str_replace('{asset_id}', $a_asset['asset_id'], $a_items['app_info'][$a_asset['app_id']]['still_url']);
	        }
	      }
	    }

	    if ($a_asset['asset_property_group_name']) {
	      $a_items_pre_sort[$a_asset['asset_id']][$a_asset['asset_property_group_name']][$a_asset['asset_property_name']][] = $a_asset['asset_property_value'];
	    }
/*
    if ($show_collections && !isset($colls[$a_asset['asset_id']])) {
      // toevoegen van collection informatie:
      db_set_active('data');
      $result = db_query("SELECT c.coll_id, c.title FROM {asset_collection} ac
        LEFT JOIN {collection} c USING(coll_id)
        WHERE ac.asset_id = '%s'", $a_asset['asset_id']);

      while ($rs = db_fetch_object($result)) {
        $a_items_pre_sort[$a_asset['asset_id']]['collection_'.$rs->coll_id] = $rs->title;
      }
      db_set_active();
      $colls[$a_asset['asset_id']] = TRUE;
    }
*/
    }
/*
  foreach ($a_asset_ids as $s_id) {
    $o_rest_reponse->add_item($a_items_pre_sort[$s_id]);
  }

  // retourneer de rest_response
  return $o_rest_reponse;*/
  }
}
