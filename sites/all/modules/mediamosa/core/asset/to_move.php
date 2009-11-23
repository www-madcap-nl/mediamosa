<?php

class mediamosa_asset {

  static public function _media_management_is_streamable($container_type) {
    static $a_streamable_container_types = array();

    if (!isset($a_streamable_container_types[$container_type])) {
      db_query("SELECT COUNT(*) aantal ".
        "FROM {mediamosa_streaming_server} ss ".
        "LEFT JOIN {streaming_server_container} ssc ON ss.streaming_server_id = ssc.streaming_server_id ".
        "WHERE ssc.container = '%s' AND ss.active = 1");




      $sql = sprintf(
        "", $container_type);
      $a_streamable_container_types[$container_type] = (db_result(db_query($sql)) > 0) ? TRUE : FALSE;
    }

    return ($a_streamable_container_types[$container_type]);
  }



}








// @todo: Marker start media_managment.module


function media_management_check_existence($a_args) {
  $a_parameters = array(
    'table' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'table'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'cell' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'cell'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'id'),
      'type' => 'skip',
      'required' => TRUE,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  if (vpx_count_rows($a_parameters['table']['value'], array($a_parameters['cell']['value'], $a_parameters['id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    switch ($a_parameters['table']['value']) {
      case "mediafile": return new rest_response(vpx_return_error(ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $a_parameters['id']['value']))); break;
      case "asset": return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $a_parameters['id']['value']))); break;
      default: return new rest_response(vpx_return_error(ERRORCODE_EMPTY_RESULT));
    }
  }
}

function _media_management_process_asset_output($a_input, $a_metadata_definitions_full) {
  foreach ($a_metadata_definitions_full as $key => $value) {
    $a_input[$value["propgroup_name"]][$key] = NULL;
  }

  unset(
    $a_input['testtag'],
    $a_input['parent_id'],
    $a_input['asset_property_value'],
    $a_input['asset_property_name'],
    $a_input['asset_property_group_name']
  );

  return $a_input;
}

function _media_management_process_mediafile_output($o_rest_reponse, $app_id) {
  $prefix = "mediafile_metadata_";

  for ($i = 1; $i <= $o_rest_reponse->item_count; $i++) {
    if (!isset($o_rest_reponse->response['items'][$i])) {
      continue;
    }
    $o_rest_reponse->response['items'][$i]['metadata'] = array();

    // Get the app_id of the original mediafile. (in case of master / slave)
    $app_info = db_fetch_array(db_query("SELECT download_url, stream_url, play_proxy_url FROM {client_applications} WHERE id = %d", $app_id));

    // indien de mediafile onder een geneste asset valt
    if (isset($o_rest_reponse->response['items'][$i]['parent_id'])) {
      if (!is_null($o_rest_reponse->response['items'][$i]['parent_id'])) {
        $o_rest_reponse->response['items'][$i]['asset_id'] = $o_rest_reponse->response['items'][$i]['parent_id'];
      }
    }
    // voeg de extra url's toe
    $patterns = array(
      '/{asset_id}/i',
      '/{mediafile_id}/i',
      '/{mediafile_filename}/i',
    );

    if (isset($o_rest_reponse->response['items'][$i])) {
      $replacements = array(
        $o_rest_reponse->response['items'][$i]['asset_id'],
        $o_rest_reponse->response['items'][$i]['mediafile_id'],
        $o_rest_reponse->response['items'][$i]['filename'],
      );
    }
    if (isset($o_rest_reponse->response['items'][$i]['is_downloadable'])) {
      $is_downloadable = ($o_rest_reponse->response['items'][$i]['is_downloadable'] == 'TRUE');
    }
    else {
      $is_downloadable = FALSE;
    }
    $o_rest_reponse->response['items'][$i]['ega_download_url'] = ($app_info['download_url'] && $is_downloadable) ? preg_replace($patterns, $replacements, $app_info['download_url']) : "";

    // Show stream_url only if streamable.
    if (_media_management_is_streamable($o_rest_reponse->response['items'][$i]['mediafile_metadata_container_type'])) {
      $o_rest_reponse->response['items'][$i]['ega_play_url'] = preg_replace($patterns, $replacements, $app_info['play_proxy_url']);

      $params = array_merge($_POST, $_GET);
      if ($params['is_oai'] == TRUE) {
        // Stream URL(s) for OAI
/*
        // Old behaviour
        $p_app_id = $o_rest_reponse->response['items'][$i]['app_id'];
        $all = FALSE;
        if ($p_app_id == $params['app_id']) {
          $all = TRUE;
        }
        elseif (is_array($params['app_id']) && $params['app_id'] != array()) {
          $all = in_array($p_app_id, $params['app_id']);
        }
        if ($all) {
          // All
          $o_rest_reponse->response['items'][$i]['ega_stream_url'][] = ($app_info['stream_url']) ? preg_replace($patterns, $replacements, $app_info['stream_url']) : "";
          db_set_active('data');
          $rs = db_query("SELECT app_master_id FROM {aut_app_master_slave} WHERE aut_object_id = '%s' AND aut_object_type = '%s' AND app_slave_id = %d", $o_rest_reponse->response['items'][$i]['mediafile_id'], 'MEDIAFILE', $p_app_id);
          db_set_active();
          while ($rso = db_fetch_object($rs)) {
            $o_rest_reponse->response['items'][$i]['ega_stream_url'][] = preg_replace($patterns, $replacements, db_result(db_query_range("SELECT stream_url FROM {client_applications} WHERE id = %d", $rso->app_master_id, 0, 1)));
          }
        }
        elseif (is_numeric($p_app_id)) {
          db_set_active('data');
          $app_master_id = db_result(db_query_range("SELECT app_master_id FROM {aut_app_master_slave} WHERE aut_object_id = '%s' AND aut_object_type = '%s' AND app_slave_id = %d", $o_rest_reponse->response['items'][$i]['mediafile_id'], 'MEDIAFILE', $p_app_id, 0, 1));
          db_set_active();
          if ($app_master_id && is_numeric($app_master_id)) {
            $o_rest_reponse->response['items'][$i]['ega_stream_url'][] = preg_replace($patterns, $replacements, db_result(db_query_range("SELECT stream_url FROM {client_applications} WHERE id = %d", $app_master_id, 0, 1)));
          }
        }
 */
        $p_app_id = $o_rest_reponse->response['items'][$i]['app_id'];
        // All
        if ($p_app_id == $params['app_id'] || (is_array($params['app_id']) && $params['app_id'] != array() && in_array($p_app_id, $params['app_id']))) {
          $o_rest_reponse->response['items'][$i]['ega_stream_url'][] = ($app_info['stream_url']) ? preg_replace($patterns, $replacements, $app_info['stream_url']) : "";
        }
        db_set_active('data');
        $rs = db_query("SELECT app_master_id FROM {aut_app_master_slave} WHERE aut_object_id = '%s' AND aut_object_type = '%s' AND app_slave_id = %d", $o_rest_reponse->response['items'][$i]['mediafile_id'], 'MEDIAFILE', $p_app_id);
        db_set_active();
        while ($rso = db_fetch_object($rs)) {
          if ($rso->app_master_id == $params['app_id'] || (is_array($params['app_id']) && $params['app_id'] != array() && in_array($rso->app_master_id, $params['app_id']))) {
            $o_rest_reponse->response['items'][$i]['ega_stream_url'][] = preg_replace($patterns, $replacements, db_result(db_query_range("SELECT stream_url FROM {client_applications} WHERE id = %d", $rso->app_master_id, 0, 1)));
          }
        }

      }
      else {
        $o_rest_reponse->response['items'][$i]['ega_stream_url'][] = ($app_info['stream_url']) ? preg_replace($patterns, $replacements, $app_info['stream_url']) : "";
      }
    }
    else {
      $o_rest_reponse->response['items'][$i]['ega_stream_url'] = '';
    }

    // verzamel de metadata onder 'metadata'
    foreach ($o_rest_reponse->response['items'][$i] as $key => $value) {
      if (strpos($key, $prefix) === 0) { // is het een metadata tag?
        $new_key = drupal_substr($key, drupal_strlen($prefix));
        $o_rest_reponse->response['items'][$i]['metadata'][$new_key] = $o_rest_reponse->response['items'][$i][$key];
        unset($o_rest_reponse->response['items'][$i][$key]);
      }
    }

    // geef deze items niet weer in het resultaat
    unset(
      $o_rest_reponse->response['items'][$i]['parent_id'],
      $o_rest_reponse->response['items'][$i]['sannas_mount_point']
    );
  }

  return $o_rest_reponse;
}


function _media_management_delete_collection($s_coll_id) {
  db_set_active('data');

  // verwijder de collection uit alle collecties
  $b_result =  db_query("DELETE FROM {collection} WHERE coll_id = '%s'", $s_coll_id);

  db_set_active();
  return $b_result;
}

function _media_management_delete_asset_collection_relation($s_asset_id = FALSE, $s_coll_id = FALSE) {
  $a_where = array();

  if ($s_coll_id !== FALSE) {
    $a_where[] = sprintf("coll_id='%s'", $s_coll_id);
  }
  if ($s_asset_id !== FALSE) {
    $a_where[] = sprintf("asset_id='%s'", $s_asset_id);
  }

  if (count($a_where) === 0) {
    return FALSE;
  }

  db_set_active('data');
  $result = db_query("DELETE FROM {asset_collection} WHERE ". implode(" AND ", $a_where));
  db_set_active();

  return $result;
}

function _media_management_delete_asset($s_asset_id) {
  // selecteer de 'data' databse
  db_set_active('data');

  // verwijder de asset uit alle collecties
  $b_db_asset_collection = db_query("DELETE FROM {asset_collection} WHERE asset_id = '%s'", $s_asset_id);

  // verwijder alle asset metadata
  $rs = db_query("SELECT id FROM {asset_property} WHERE asset_id = '%s'", $s_asset_id);
  while ($rso = db_fetch_object($rs)) {
    db_query("DELETE FROM {large_asset_property} WHERE aprop_id = %d", $rso->id);
  }
  $b_db_asset_property = db_query("DELETE FROM {asset_property} WHERE asset_id = '%s'", $s_asset_id);

  // verwijder de asset supplement(s)
  $b_db_asset_property = db_query("DELETE FROM {asset_supplement} WHERE asset_id = '%s'", $s_asset_id);

  // verwijder de stills
  $b_db_still = _media_management_delete_still($s_asset_id);

  // verwijder de asset uit stats
  db_set_active('data'); // _media_management_delete_still() zet de database weer op default
  $b_stats = db_query("DELETE FROM {statistics_stream_request} WHERE asset_id = '%s'", $s_asset_id);

  // verwijder alle sub assets
  // First: store the deleted assets for oai
  $rs = db_query("SELECT asset_id, app_id FROM {asset} WHERE parent_id = '%s'", $s_asset_id);
  while ($rso = db_fetch_object($rs)) {
    db_query("REPLACE LOW_PRIORITY {asset_delete} (asset_id, app_id, videotimestampmodified) VALUES ('%s', %d, NOW())", $rso->asset_id, $rso->app_id);
  }
  $b_db_asset_parent = db_query("DELETE FROM {asset} WHERE parent_id = '%s'", $s_asset_id);

  // verwijder de asset
  // First: store the deleted asset for oai
  $rs = db_query("SELECT asset_id, app_id FROM {asset} WHERE asset_id = '%s'", $s_asset_id);
  while ($rso = db_fetch_object($rs)) {
    db_query("REPLACE LOW_PRIORITY {asset_delete} (asset_id, app_id, videotimestampmodified) VALUES ('%s', %d, NOW())", $rso->asset_id, $rso->app_id);
  }
  $b_db_asset = db_query("DELETE FROM {asset} WHERE asset_id = '%s'", $s_asset_id);

  // schakel terug naar de default database
  db_set_active();

  return ($b_stats && $b_db_asset_collection && $b_db_asset_property && $b_db_asset_parent && $b_db_asset && $b_db_still);
}

function _media_management_delete_still($asset_id, $mediafile_id = NULL, $still_id = NULL) {
// haal de still gegeven(s) op en verwijder deze van disk.
  db_set_active('data');
  if ($still_id) {
    $result_query = db_query("SELECT mediafile_id AS still_id, app_id, sannas_mount_point, mediafile_source FROM {mediafile} WHERE mediafile_id = '%s' AND is_still = 'TRUE' AND asset_id_root = '%s'", $still_id, $asset_id);
  }
  elseif ($mediafile_id) {
    $result_query = db_query("
      SELECT s.mediafile_id AS still_id, s.app_id, s.sannas_mount_point, s.mediafile_source
      FROM {mediafile} AS s
      INNER JOIN {mediafile} AS m USING(asset_id)
      WHERE m.mediafile_id = '%s' AND s.is_still = 'TRUE' AND m.asset_id_root = '%s' AND s.asset_id_root = '%s' AND s.mediafile_source = '%s'", $mediafile_id, $asset_id, $asset_id, $mediafile_id);
  }
  else {
    $result_query = db_query("SELECT mediafile_id AS still_id, app_id, sannas_mount_point, mediafile_source FROM {mediafile} WHERE asset_id_root = '%s' AND is_still = 'TRUE'", $asset_id);
  }
  db_set_active();

  $m_id = array();
  while ($query_result_row = db_fetch_object($result_query)) {
    watchdog("server", sprintf("Deleting still: %s/%s/%s/%s", $query_result_row->sannas_mount_point, STILL_LOCATION, $query_result_row->still_id{0}, $query_result_row->still_id));

// fysiek verwijderen
    @unlink(sprintf("%s/%s/%s/%s", $query_result_row->sannas_mount_point, STILL_LOCATION, $query_result_row->still_id{0}, $query_result_row->still_id));

    $m_id[] = $query_result_row->still_id;

    if (!$mediafile_id) {
      $mediafile_id = $query_result_row->mediafile_source;
    }
  }

// verwijder alle stills voor deze asset uit de database
  db_set_active('data');

  if ($m_id && is_array($m_id) && $m_id != array()) {
    $imploded = implode("', '", $m_id);
    db_query(sprintf("DELETE FROM {mediafile_metadata} WHERE mediafile_id IN('%s')", $imploded));
    db_query(sprintf("DELETE FROM {mediafile} WHERE mediafile_id IN('%s')", $imploded));
    unset($imploded);
  }

  // Check is there a default image after the deletion in the media (video) file
  $default = db_result(db_query_range("
    SELECT mm.mediafile_id
    FROM {mediafile_metadata} AS mm
    INNER JOIN {mediafile} AS m USING(mediafile_id)
    WHERE mm.still_default = 'TRUE' AND m.asset_id_root = '%s' AND m.is_still = 'TRUE' AND m.mediafile_source = '%s'
    ", $asset_id, $mediafile_id, 0, 1));
  if (!$default) {
    // There aren't any default image, so creating one
    $default = db_result(db_query_range("
      SELECT mm.mediafile_id
      FROM {mediafile_metadata} AS mm
      INNER JOIN {mediafile} AS m USING(mediafile_id)
      WHERE m.asset_id_root = '%s' AND m.is_still = 'TRUE' AND m.mediafile_source = '%s'
      ORDER BY mm.still_order
      ", $asset_id, $mediafile_id, 0, 1));
    if ($default) {
      db_query("
        UPDATE {mediafile_metadata}
        SET still_default = 'TRUE'
        WHERE mediafile_id = '%s'
        ", $default);
    }
  }
/*
  // Check is there a default image after the deletion in the asset
  $default = db_result(db_query_range("
    SELECT mm.mediafile_id
    FROM {mediafile_metadata} AS mm
    INNER JOIN {mediafile} AS m USING(mediafile_id)
    WHERE mm.still_default = 'TRUE' AND m.asset_id_root = '%s' AND m.is_still = 'TRUE'
    ", $asset_id, 0, 1));
  if (!$default) {
    // There aren't any default image, so creating one
    $default = db_result(db_query_range("
      SELECT mm.mediafile_id
      FROM {mediafile_metadata} AS mm
      INNER JOIN {mediafile} AS m USING(mediafile_id)
      WHERE m.asset_id_root = '%s' AND m.is_still = 'TRUE'
      ORDER BY mm.still_order
      ", $asset_id, 0, 1));
    if ($default) {
      db_query("
        UPDATE {mediafile_metadata}
        SET still_default = 'TRUE'
        WHERE mediafile_id = '%s'
        ", $default);
    }
  }
 */

  db_set_active();

// retourneer een ok
  return TRUE;
}


function _media_management_delete_mediafile($a_mediafiles) {
  if (!is_array($a_mediafiles)) {
    $a_mediafiles = array($a_mediafiles);
  }

  // ruim de eventuele mediafile_metadata eerst op
  db_set_active('data');
  $b_db_mediafile_metadata = db_query("DELETE FROM {mediafile_metadata} WHERE mediafile_id IN ('". implode("', '", $a_mediafiles) ."')");
  db_set_active();

  // verwijder alle mediafiles van SAN/NAS
  foreach ($a_mediafiles as $s_mediafile_id) {
    db_set_active('data');
    $mediafile = db_fetch_array(db_query("SELECT app_id, sannas_mount_point FROM {mediafile} WHERE mediafile_id='%s'", $s_mediafile_id));
    db_set_active();

    $file = $mediafile['sannas_mount_point'] ."/". DATA_LOCATION ."/". $s_mediafile_id{0} ."/". $s_mediafile_id;

    // fysiek verwijderen
    @unlink($file);
  }

  // verwijder de mediafile info
  db_set_active('data');
  $b_db_mediafile = db_query("DELETE FROM {mediafile} WHERE mediafile_id IN ('". implode("', '", $a_mediafiles) ."')");
  db_set_active();

  // update de asset info
  _media_management_update_asset_info($s_mediafile_id);

  return ($b_db_mediafile_metadata && $b_db_mediafile);
}

function media_management_get_current_mount_point($a_args) {
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));

  db_set_active();
  $current_mount_point = db_result(db_query("SELECT current_mount_point FROM {san_nas_settings}"));
  $rest_response->add_item(array("current_mount_point" => $current_mount_point));

  return $rest_response;
}

function _media_management_return_collection_list($a_ids) {
  $query = sprintf(
    "SELECT c.* ".
    "FROM {collection} AS c ".
    "WHERE c.coll_id IN ('%s') ",
    implode("', '", $a_ids)
  );

  db_set_active('data');
  $result = db_query($query);
  db_set_active();

  $a_items_pre_sort = array();
  while ($array = db_fetch_array($result)) {
    unset($array['testtag']);
    $a_items_pre_sort[$array['coll_id']] = $array;
  }

  $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  foreach ($a_ids as $s_id) {
    $o_rest_reponse->add_item($a_items_pre_sort[$s_id]);
  }
  return $o_rest_reponse;
}

function _media_management_return_mediafile_list($a_ids, $app_id, $show_stills = TRUE) {
  db_set_active('data');
  $a_keys = db_fetch_array(db_query("SELECT * FROM mediafile_metadata LIMIT 1"));
  db_set_active();

  $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));

  // als nog geen metadata (bv na migratie)
  if (!isset($a_ids[0]) || ($a_ids[0] == NULL)) {
    return $o_rest_reponse;
  }

  unset($a_keys["mediafile_id"], $a_keys["metadata_id"]); // geef deze niet weer in de metadata
  if (is_array($a_keys)) {
    $a_keys = array_keys($a_keys);
  }
  $s_metadata_select = "";

  if (is_array($a_keys)) {
    foreach ($a_keys as $item) {
      $s_metadata_select .= "mm." . $item . " AS mediafile_metadata_" . $item . ", ";
    }
  }

  $query = sprintf(
    "SELECT %s m.*, a.parent_id ".
    "FROM {mediafile} m ".
    "LEFT JOIN {mediafile_metadata} mm ON m.mediafile_id = mm.mediafile_id ".
    "LEFT JOIN {asset} a ON m.asset_id = a.asset_id ".
    "WHERE m.mediafile_id IN ('%s')",
    $s_metadata_select,
    implode("', '", $a_ids)
  );

  db_set_active('data');
  $result = db_query($query);
  db_set_active();

  $a_items_pre_sort = array();
  $video_id = array();
  $origin = array();
  while ($array = db_fetch_array($result)) {
    unset($array['testtag']);
    unset($array['asset_id_root']);
    unset($array['mediafile_metadata_still_time_code']);
    unset($array['mediafile_metadata_still_order']);
    unset($array['mediafile_metadata_still_format']);
    unset($array['mediafile_metadata_still_type']);
    unset($array['mediafile_metadata_still_default']);
    $a_items_pre_sort[$array['mediafile_id']] = $array;
    //$video_id[$array['asset_id']] = $array['asset_id'];
    $video_id[] = $array['mediafile_id'];
    $origin[$array['asset_id']] = $array['mediafile_id'];
  }
  if ($video_id && $video_id != array() && $show_stills) {
    db_set_active('data');
    $rs = db_query(sprintf("
      SELECT *
      FROM {mediafile} AS m
      LEFT JOIN {mediafile_metadata} AS mm USING(mediafile_id)
      WHERE m.mediafile_source IN ('%s') AND m.is_still = 'TRUE'
      ORDER BY mm.still_order",
      implode("', '", $video_id)));
    db_set_active();
    $i = 0;
    while ($rsa = db_fetch_array($rs)) {
      $s_ticket = vpx_create_hash($rsa['mediafile_id'] . mt_rand(0, 99999), microtime());

      $result = _play_proxy_create_ticket(
        $s_ticket,
        NULL,
        RESPONSE_TYPE_STILL,
        $app_id,
        $rsa['owner_id'],
        $rsa['mediafile_id']
      );

      if ($result !== FALSE) {
        $a_asset_info = array();
        $a_mediafile_info = array();
        $a_asset_metadata = array();
        $a_parameters['response']['value'] = RESPONSE_TYPE_STILL;
        $response = _play_proxy_create_response($a_parameters, array(), array(), array(), $s_ticket, $app_id);
        $rsa['still_ticket'] = $response['output'];
      }

      $a_items_pre_sort[$rsa['mediafile_source']]['still']['#'. $i++] = $rsa;
    }
  }

  foreach ($a_ids as $s_id) {
    if (isset($a_items_pre_sort[$s_id])) {
      $o_rest_reponse->add_item($a_items_pre_sort[$s_id]);
    }
  }

  return _media_management_process_mediafile_output($o_rest_reponse, $app_id);
}

function _media_management_create_asset_response($a_items, $a_metadata_definitions_full, $a_ids = array(), $mixed_app_id, $user_id, $show_stills = TRUE) {
  $colls = array();

  $a_app_ids = $mixed_app_id;
  if (!is_array($a_app_ids)) {
    $a_app_ids = array($a_app_ids);
  }

  $o_rest_reponse = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $s_current_asset = FALSE;
  $a_items_pre_sort = array();

  foreach ($a_items['assets'] as $a_asset) {
    if (!isset($a_items_pre_sort[$a_asset['asset_id']])) {
      $asset_id = $a_asset['asset_id'];

      $a_items_pre_sort[$asset_id] = _media_management_process_asset_output($a_asset, $a_metadata_definitions_full);

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
        $s_ticket = vpx_create_hash(reset($a_app_ids), $user_id);
        $a_mediafile_info['asset_id'] = $asset_id;
        $result = _play_proxy_create_ticket($s_ticket, $a_mediafile_info, "still", reset($a_app_ids), $user_id, $a_items['stills'][$asset_id]);

        // voeg de still toe: still zelf
        if ($result !== FALSE) {
          $a_asset_info = array();
          $a_mediafile_info = array();
          $a_asset_metadata = array();
          $a_parameters['response']['value'] = RESPONSE_TYPE_STILL;
          $response = _play_proxy_create_response($a_parameters, array(), array(), array(), $s_ticket, reset($a_app_ids));
          $a_items_pre_sort[$asset_id]['vpx_still_url'] = $response['output'];
          $a_items_pre_sort[$asset_id]['ega_still_url'] = str_replace('{asset_id}', $a_asset['asset_id'], $a_items['app_info'][$a_asset['app_id']]['still_url']);
        }
      }
    }

    if ($a_asset['asset_property_group_name']) {
      $a_items_pre_sort[$a_asset['asset_id']][$a_asset['asset_property_group_name']][$a_asset['asset_property_name']][] = $a_asset['asset_property_value'];
    }

    if (isset($_GET['show_collections']) && $_GET['show_collections'] == 'TRUE' && !isset($colls[$a_asset['asset_id']])) {
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

  }

  foreach ($a_ids as $s_id) {
    $o_rest_reponse->add_item($a_items_pre_sort[$s_id]);
  }

  // retourneer de rest_response
  return $o_rest_reponse;
}

/**
 * Get definitions from the asset_property_definitions array
 * Extended so all can go into one query
 *
 * @param array/string $mixed_propgroup_name
 * @return array
 */
function _media_management_get_metadata_definitions($mixed_propgroup_name = 'dublin_core') {

  $a_where = array();
  if (is_array($mixed_propgroup_name)) {
    foreach ($mixed_propgroup_name as $s_propgroup_name) {
      $a_where[] = sprintf("apg.name='%s'", db_escape_string($s_propgroup_name));
    }
  }
  else {
    $a_where[] = sprintf("apg.name='%s'", db_escape_string($mixed_propgroup_name));
  }

  db_set_active('data');
  $result = db_query("
    SELECT apg.propgroup_id, apg.name AS propgroup_name, apd.type AS propdef_type, apd.prop_id AS propdef_id, apd.name AS propdef_name
    FROM {assetprop_group} apg
    INNER JOIN {assetprop_definition} apd ON apd.propgroup_id = apg.propgroup_id
    WHERE " . implode(" OR ", $a_where));
  db_set_active();

  $a_output = array();
  while ($array = db_fetch_array($result)) {
    $a_output[$array['propdef_name']] = $array;
  }

  return $a_output;
}

/**
 * Get all definitions, plus our own based on the app_id
 *
 */
function _media_management_get_metadata_definitions_full($mixed_app_id = FALSE, $a_definitions_full = array('dublin_core', 'qualified_dublin_core', 'czp')) {

  if ($mixed_app_id !== FALSE) {
    if (is_array($mixed_app_id)) {
      foreach ($mixed_app_id as $app_id) {
        $a_definitions_full[] = sprintf('app_%d', $app_id);
      }
    }
    else {
      $a_definitions_full[] = sprintf('app_%d', $mixed_app_id);
    }
  }

  return _media_management_get_metadata_definitions($a_definitions_full);
}

function media_management_change_ownership($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'old_group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'old_group_id'),
      'type' => 'alphanum',
    ),
    'new_group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'new_group_id'),
      'type' => 'alphanum',
    ),
    'old_owner_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'old_owner_id'),
      'type' => TYPE_USER_ID,
    ),
    'new_owner_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'new_owner_id'),
      'type' => TYPE_USER_ID,
    ),
/**
 * Omdat het als 'te gevaarlijk' wordt beschouwd, tijdelijk 'new_app_id' uitgezet... (#774)
 *
 * Ook unit test 210 is aangepast op regel 28
 */
    'new_app_id' => array(
      'value' => NULL, // vpx_get_parameter_2($a_args['get'], 'new_app_id'),
      'type' => 'int',
    ),
  );
  $a_parameters['old_app_id'] = array(
    'value' => $a_parameters['app_id']['value'],
    'type' => 'int',
    'required' => TRUE
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// controleer of de nieuwe app_id bestaat (indien gezet)
  if (!is_null($a_parameters['new_app_id']['value'])) {
    if (db_result(db_query("SELECT COUNT(id) FROM {client_applications} WHERE id = %d AND active = 1", $a_parameters['new_app_id']['value'])) != 1) {
      return new rest_response(vpx_return_error(ERRORCODE_INVALID_APP_ID));
    }
  }

// er moet in ieder geval 1 item gewijzigd worden en 1 voorwaarde opgegeven worden
  $b_old = $b_new = FALSE;
  foreach (array('owner_id', 'group_id', 'app_id') as $subject) {
    if (!is_null($a_parameters['old_'. $subject]['value'])) {
      $b_old = TRUE;
    }
    if (!is_null($a_parameters['new_'. $subject]['value'])) {
      $b_new = TRUE;
    }
  }
  if (!$b_old || !$b_new) {
    return new rest_response(vpx_return_error(ERRORCODE_CHANGE_OWNERSHIP_MISSING_PARAMETERS));
  }

// verplaats de items naar de nieuwe eigenaren
  $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  foreach (array('mediafile', 'asset', 'collection') as $table) {
    $a_update = $a_where = array();
    foreach (array('owner_id', 'group_id', 'app_id') as $subject) {
      if (!is_null($a_parameters['old_'. $subject]['value'])) {
        $a_where[] = sprintf("%s = '%s'", $subject, $a_parameters['old_'. $subject]['value']);
      }
      if (!is_null($a_parameters['new_'. $subject]['value'])) {
        $a_update[] = sprintf("%s = '%s'", $subject, db_escape_string($a_parameters['new_'. $subject]['value']));
      }
    }
    $query = sprintf(
      "UPDATE {%s} SET ". implode(", ", $a_update) ." WHERE ". implode(" AND ", $a_where),
      $table
    );
    db_set_active('data');
    db_query($query);
    $o_rest_response->add_item(array($table => db_affected_rows()));
    db_set_active();
  }
  return $o_rest_response;
}


/**
 * Controleer of er lopende jobs zijn voor een asset.
 * Als alle jobs gestopt zijn / of nog niet begonnen, verwijder die jobs dan.
 *
 * Code is verbeterd en verplaatst naar vpx_jobs.
 *
 */
function _media_management_delete_jobs($asset_id) {

  return _vpx_jobs_cancel_job_by_asset($asset_id);
}

/**
 * Helper functie voor het tellen van assets/collections/mediafiles
 */
function _media_management_count_items($subject, $a_args) {
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'owner_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'owner_id'),
      'type' => TYPE_USER_ID,
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id'),
      'type' => TYPE_GROUP_ID,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// stel de where samen
  $where = array(sprintf("app_id = %d", $a_parameters['app_id']['value']));

// voeg de owner_id toe (indien gezet)
  if (!is_null($a_parameters['owner_id']['value'])) {
    $where[] = sprintf("owner_id = '%s'", db_escape_string($a_parameters['owner_id']['value']));
  }

// voeg de group_id toe (indien gezet)
  if (!is_null($a_parameters['group_id']['value'])) {
    $where[] = sprintf("group_id = '%s'", db_escape_string($a_parameters['group_id']['value']));
  }

// query de database
  db_set_active('data');
  $count = (int)db_result(db_query(
    "SELECT COUNT(*) FROM {%s} WHERE ". implode(" AND ", $where),
    $subject
  ));
  db_set_active();

// zet de count in de header
  $rest_response->item_total_count = $count;

// retourneer het resultaat
  return $rest_response;
}

function _media_management_favorites_count($a_parameters) {

  $a_where = array();
  $a_where[] = sprintf("fav_type='%s'", $a_parameters['parameters']['fav_type']['value']);
  $a_where[] = sprintf("fav_id='%s'", $a_parameters['parameters']['fav_id']['value']);

  // query de database
  db_set_active('data');
  $count = (int)db_result(db_query("SELECT COUNT(*) FROM {user_favorites} WHERE ". implode(" AND ", $a_where)));
  db_set_active();

  $o_result = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $o_result->item_total_count = $count; // zelfs al zijn er geen links, we geven altijd het aantal terug.

  return $o_result;
}

// @todo: marker start media_management_still.inc

/**
 * Deze functie creert een still in de database
 */

function media_management_create_still($asset_id, $parent_id, $still_id, $app_id, $owner_id, $group_id, $order, $default = 'FALSE', $still_parameters = NULL, $sec = 0, $mediafile_source = '', $tag = '') {

  $filename = $still_id;
  $destination = vpx_get_san_nas_base_path() . DS . STILL_LOCATION . DS . $filename{0} . DS . $filename;
  // Still size
  $size = getimagesize($destination);
  $width = $size[0];
  $height = $size[1];
  // File type
  $file_type = '';
  $pos = strrpos($size['mime'], '/');
  if ($pos !== FALSE) {
    $file_type = substr($size['mime'], $pos+1);
  }
  $filesize = filesize($destination);

  if ($default) {
    // Clear the earlier default mark on the video (media) file
    db_set_active('data');
    db_query("
      UPDATE {mediafile_metadata} AS mm
      INNER JOIN {mediafile} AS m USING(mediafile_id)
      SET mm.still_default = 'FALSE'
      WHERE m.mediafile_source = '%s' AND m.is_still = 'TRUE'
      ", $mediafile_source);
    db_set_active();
/*
    // Clear the earlier default mark on the asset
    db_set_active('data');
    db_query("
      UPDATE {mediafile_metadata} AS mm
      INNER JOIN {mediafile} AS m USING(mediafile_id)
      SET mm.still_default = 'FALSE'
      WHERE m.asset_id_root = '%s' AND m.is_still = 'TRUE'
      ", $parent_id);
    db_set_active();
 */
  }

  // voeg de still toe aan de database
  db_set_active('data');
  $result = db_query("INSERT INTO {mediafile} (mediafile_id, asset_id, app_id, owner_id, group_id, sannas_mount_point, file_extension, created, changed, asset_id_root, is_still, mediafile_source, tag) VALUES ('%s', '%s', %d, '%s', '%s', '%s', '%s', NOW(), NOW(), '%s', '%s', '%s', '%s')",
    $still_id, $asset_id, $app_id, $owner_id, $group_id, SAN_NAS_BASE_PATH, $file_type, $parent_id, 'TRUE', $mediafile_source, $tag);
  $result = db_query("INSERT INTO {mediafile_metadata} (mediafile_id, width, height, container_type, filesize, mime_type, created, changed, still_time_code, still_order, still_format, still_type, still_default) VALUES ('%s', %d, %d, '%s', %d, '%s', %s, %s, '%s', %d, '%s', '%s', '%s')",
    $still_id, $width, $height, '', $filesize, $size['mime'], 'NOW()', 'NOW()', $sec, $order, $file_type, $still_parameters['still_type'], ($default ? 'TRUE' : 'FALSE'));
  db_set_active();

  return $result;
}

/**
 * Met deze functie worden alle aanwezige stills voor een asset verwijderd.
 */

function media_management_asset_delete_stills($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'mediafile_id'),
      'type' => 'alphanum',
    ),
    'still_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'still_id'),
      'type' => 'alphanum',
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  $asset_id = $a_parameters['asset_id']['value'];
  $mediafile_id = $a_parameters['mediafile_id']['value'];
  $still_id = $a_parameters['still_id']['value'];

// kijk of de asset bestaat en of het geen sub asset is
  if (!vpx_count_rows("asset", array(
    "asset_id", $asset_id,
    "parent_id", NULL
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $asset_id)));
  }
// controleer of de still bestaat
  if (!vpx_count_rows("mediafile", array("asset_id_root", $asset_id))) {
    return new rest_response(vpx_return_error(ERRORCODE_STILL_NOT_FOUND, array("@asset_id" => $asset_id)));
  }

  // controleer of de gebruiker rechten heeft om de still te verwijderen
  // get owner info
  db_set_active('data');
  $asset_app_id = db_result(db_query("SELECT app_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  $asset_owner  = db_result(db_query("SELECT owner_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  db_set_active();

  // controleer of de gebruiker rechten heeft om de metadata aan te passen
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $asset_app_id, $asset_owner);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // verwijder de still
  if (($error = _media_management_delete_still($asset_id, $mediafile_id, $still_id)) === TRUE) {
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    return $error;
  }
}

/**
 * Get stills for mediafile
 * mediafile/$mediafile_id/still
 */
function media_management_get_still_for_mediafile($a_args) {
  if (!is_array($a_args['get'])) {
    $a_args['get'] = array();
  }

  $a_args['get']['mediafile_id'] = $a_args['uri']['mediafile_id'];
  db_set_active('data');
  $a_args['uri']['asset_id'] = db_result(db_query_range("SELECT asset_id_root FROM {mediafile} WHERE mediafile_id = '%s' AND is_still = 'FALSE'", $a_args['get']['mediafile_id'], 0, 1));
  db_set_active();

  return media_management_get_still($a_args);
}

function media_management_get_still($a_args) {
  return play_proxy_request(array_merge($a_args, array("internal" => TRUE)));
}

/**
 * Set a still as a default for an asset.
 */
function media_management_set_still_default($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'mediafile_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'still_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'still_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  $asset_id = $a_parameters['asset_id']['value'];
  $mediafile_id = $a_parameters['mediafile_id']['value'];
  $still_id = $a_parameters['still_id']['value'];

// kijk of de asset bestaat en of het geen sub asset is
  if (!vpx_count_rows("asset", array(
    "asset_id", $asset_id,
    "parent_id", NULL
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $asset_id)));
  }
  // Check the mediafile is exist or not
  if (!vpx_count_rows("mediafile", array(
    "asset_id_root", $asset_id,
    "mediafile_id", $mediafile_id,
    "is_still", 'FALSE'
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_STILL_NOT_FOUND, array("@asset_id" => $asset_id)));
  }
  // Check the still is exist or not
  if (!vpx_count_rows("mediafile", array(
    "asset_id_root", $asset_id,
    "mediafile_id", $still_id,
    "is_still", 'TRUE'
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_STILL_NOT_FOUND, array("@asset_id" => $asset_id)));
  }

  // controleer of de gebruiker rechten heeft om de still te verwijderen
  // get owner info
  db_set_active('data');
  $asset_app_id = db_result(db_query("SELECT app_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  $asset_owner  = db_result(db_query("SELECT owner_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  db_set_active();

  // controleer of de gebruiker rechten heeft om de metadata aan te passen
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $asset_app_id, $asset_owner);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // verwijder de still
  if (($error = _media_management_set_still_default($asset_id, $mediafile_id, $still_id)) === TRUE) {
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    return $error;
  }
}

function _media_management_set_still_default($asset_id, $mediafile_id, $still_id) {
  watchdog("server", sprintf("Default still: asset_id = %s; mediafile_id = %s; still_id = %s", $asset_id, $mediafile_id, $still_id));

  db_set_active('data');
  db_query("
    UPDATE {mediafile_metadata} AS mm
    INNER JOIN {mediafile} AS m USING(mediafile_id)
    SET mm.still_default = IF(mm.mediafile_id = '%s', 'TRUE', 'FALSE')
    WHERE m.asset_id_root = '%s' AND m.is_still = 'TRUE'", $still_id, $asset_id);
  db_set_active();

  return TRUE;
}

/**
 * Set a still order for a mediafile.
 */
function media_management_set_still_order($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'mediafile_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'still_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'still_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'order' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'order'),
      'type' => 'skip',
      'required' => TRUE,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  if (!is_numeric($a_parameters['order']['value'])) {
    return vpx_return_error(ERRORCODE_VALIDATE_FAILED, array("@param" => 'order' ."=". $a_parameters['order']['value'], "@type" => 'int'));
  }

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  $asset_id = $a_parameters['asset_id']['value'];
  $mediafile_id = $a_parameters['mediafile_id']['value'];
  $still_id = $a_parameters['still_id']['value'];
  $order = $a_parameters['order']['value'];

// kijk of de asset bestaat en of het geen sub asset is
  if (!vpx_count_rows("asset", array(
    "asset_id", $asset_id,
    "parent_id", NULL
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $asset_id)));
  }
  // Check the mediafile is exist or not
  if (!vpx_count_rows("mediafile", array(
    "asset_id_root", $asset_id,
    "mediafile_id", $mediafile_id,
    "is_still", 'FALSE'
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_STILL_NOT_FOUND, array("@asset_id" => $asset_id)));
  }
  // Check the still is exist or not
  if (!vpx_count_rows("mediafile", array(
    "asset_id_root", $asset_id,
    "mediafile_id", $still_id,
    "is_still", 'TRUE'
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_STILL_NOT_FOUND, array("@asset_id" => $asset_id)));
  }

  // controleer of de gebruiker rechten heeft om de still te verwijderen
  // get owner info
  db_set_active('data');
  $asset_app_id = db_result(db_query("SELECT app_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  $asset_owner  = db_result(db_query("SELECT owner_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  db_set_active();

  // controleer of de gebruiker rechten heeft om de metadata aan te passen
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $asset_app_id, $asset_owner);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // verwijder de still
  if (($error = _media_management_set_still_order($asset_id, $mediafile_id, $still_id, $order)) === TRUE) {
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    return $error;
  }
}

function _media_management_set_still_order($asset_id, $mediafile_id, $still_id, $order) {
  watchdog("server", sprintf("Still order: asset_id = %s; mediafile_id = %s; still_id = %s; order = %s;", $asset_id, $mediafile_id, $still_id, $order));

  db_set_active('data');
  db_query("
    UPDATE {mediafile_metadata} AS mm
    INNER JOIN {mediafile} AS m USING(mediafile_id)
    SET mm.still_order = %d
    WHERE mm.mediafile_id = '%s' && m.asset_id_root = '%s' AND m.is_still = 'TRUE'
    ", $order, $still_id, $asset_id);
  db_set_active();

  return TRUE;
}

// @todo: marker start media_mangement_seach
// array name for search in where
define("VPX_QUERY_DIM_NAME_SEARCH", "search");
define("VPX_QUERY_DIM_NAME_EGAS", "egas");
define("VPX_QUERY_DIM_NAME_ORDER_BY", "order_by");
define("VPX_QUERY_DIM_NAME_ASSET_COLLECTION", "asset_coll");
define("VPX_QUERY_DIM_NAME_COLLECTION", "collection");
define("VPX_QUERY_DIM_NAME_FTP_BATCH", "ftp_batch");

/**
 * Search on the assets using given search parameters
 *
 * @param array $a_funcparam
 */
function _media_management_search_asset($a_funcparam, $a_metadata_definitions_full) {

  $a_app_ids = vpx_funcparam_get_value($a_funcparam, 'app_id', 0);

  if (!is_array($a_app_ids)) {
    $a_app_ids = array($a_app_ids);
  }

  // Get granted value
  $granted = vpx_funcparam_get_value($a_funcparam, 'granted', 'false');

  $do_master_slave = (drupal_strtolower($granted) == 'false') ? FALSE : TRUE;

  // get aut vars
  $aut_user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
  $a_aut_group_id = vpx_funcparam_get_value($a_funcparam, 'aut_group_id');
  $aut_domain = vpx_funcparam_get_value($a_funcparam, 'aut_domain');
  $aut_realm = vpx_funcparam_get_value($a_funcparam, 'aut_realm');

  // Is private value
  $is_public_list = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_public_list', 'false'));

  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.asset_id";
  $a_query[VPX_DB_QUERY_A_FROM][] = "{asset} AS a";

  $a_query[VPX_DB_QUERYB_ALLOW_DISTINCT] = TRUE;
  $a_query[VPX_DB_QUERY_A_GROUP_BY][] = "a.asset_id";

  $a_slaves = vpx_acl_slave_get($a_app_ids, VPX_ACL_AUT_TYPE_MEDIAFILE);

  $a_app_ids_tmp = $a_app_ids;
  foreach ($a_slaves as $app_id_slave => $a_slave) {
    $a_app_ids_tmp[] = $app_id_slave;
  }

  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = sprintf("a.app_id IN(%s)", implode(",", array_unique($a_app_ids_tmp)));
  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = "a.parent_id IS NULL";

  // ega admin may see unappropiate assets
  $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));

  // isprivate / unappropiate test.
  // Is outside the ACL check, else we would have problems with 'granted'.
  if (!$is_app_admin) {
    if ($is_public_list && $aut_user_id) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_AND][] = sprintf("(a.isprivate = 'FALSE' AND (a.is_unappropiate = 'FALSE' OR a.owner_id='%s'))", db_escape_string($aut_user_id));
    }
    elseif ($is_public_list) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_AND][] = "(a.isprivate = 'FALSE' AND a.is_unappropiate = 'FALSE')"; // Must both be FALSE
    }
    elseif ($aut_user_id) { // if provided, then we only have access to unappropate when owner.
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_AND][] = sprintf("(a.isprivate = 'FALSE' OR a.isprivate = 'TRUE') AND (a.is_unappropiate = 'FALSE' OR a.owner_id='%s')", db_escape_string($aut_user_id));
    }
    else {
      // No public list, no aut_user_id
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_AND][] = "((a.isprivate = 'FALSE' OR a.isprivate = 'TRUE') AND a.is_unappropiate = 'FALSE')"; // Ignore isprivate, is_unappropiate must be TRUE
    }
  }
  else {
    // just to match index for speed
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_AND][] = "(a.isprivate = 'FALSE' OR a.is_unappropiate = 'FALSE' OR a.is_unappropiate = 'TRUE')";// only 3, will match always
  }

  // Switch to hide assets that have no mediafiles
  $hide_empty_assets = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'hide_empty_assets'));
  if ($hide_empty_assets) {
    // exclude empty assets
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_AND][] = "a.is_empty_asset = 'FALSE'";
  }
  else {
    // just to match index for speed
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access'][VPX_DB_WHERE_AND][] = "a.is_empty_asset IS NOT NULL"; // Never NULL, will always match index
  }

  $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by', '');
  $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction', 'ASC');

  $str_cql = vpx_funcparam_get_value($a_funcparam, 'cql', '');

  if ($str_cql != '') {
    assert(!isset($a_funcparam['a_parameters_search']));

    $a_result_cql2sql = vpx_cql_parse_asset($str_cql, $a_app_ids);

    if ($a_result_cql2sql['str_where'] != "") {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_SEARCH] = $a_result_cql2sql['str_where'];
    }

    if (isset($a_result_cql2sql['str_having']) && $a_result_cql2sql['str_having'] != "") {
      $a_query[VPX_DB_QUERY_A_HAVING][] = $a_result_cql2sql['str_having'];
      $a_query[VPX_DB_QUERYB_ALLOW_DISTINCT] = FALSE;
    }

    // Quick hack, or else i really need to extend the VPX_CQL class
    if (isset($a_result_cql2sql['a_joins'])) {
      $key = array_search('MEDIAFILE_METADATA_MIMETYPE', $a_result_cql2sql['a_joins']);
      if ($key) {
        unset($a_result_cql2sql['a_joins'][$key]);

        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile_metadata']['mm_mime_type'] = sprintf("LEFT JOIN {mediafile_metadata} AS mm_mime_type ON mf.mediafile_id=mm_mime_type.mediafile_id");
      }

      $key = array_search('MEDIAFILE_FILENAME', $a_result_cql2sql['a_joins']);
      if ($key) {
        unset($a_result_cql2sql['a_joins'][$key]);

        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
      }

      $key = array_search('AUT_NAME_USER', $a_result_cql2sql['a_joins']);
      if ($key) {
        unset($a_result_cql2sql['a_joins'][$key]);

        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
        $a_query[VPX_DB_QUERY_A_JOIN]["aut_object"] = vpx_acl_join_aut_object_get(VPX_ACL_AUT_TYPE_MEDIAFILE);
        $a_query[VPX_DB_QUERY_A_JOIN]['aut_name_user'] = sprintf("LEFT JOIN {aut_name} AS aut_u ON aut_u.app_id IN (%s) AND aut_u.aut_type = 'USER' AND aut_u.aut_name_id=aut_obj.aut_id" , implode(',', $a_app_ids));
        $do_master_slave = TRUE;
        unset($a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access']);
      }

      $key = array_search('AUT_NAME_USER_GROUP', $a_result_cql2sql['a_joins']);
      if ($key) {
        unset($a_result_cql2sql['a_joins'][$key]);

        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
        $a_query[VPX_DB_QUERY_A_JOIN]["aut_object"] = vpx_acl_join_aut_object_get(VPX_ACL_AUT_TYPE_MEDIAFILE);
        $a_query[VPX_DB_QUERY_A_JOIN]['aut_name_group'] = sprintf("LEFT JOIN {aut_name} AS aut_ug ON aut_ug.app_id IN (%s) AND aut_ug.aut_type = 'USER_GROUP' AND aut_ug.aut_name_id=aut_obj.aut_id" , implode(',', $a_app_ids));
        $do_master_slave = TRUE;
        unset($a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access']);
      }

      $key = array_search('AUT_GROUP_DOMAIN', $a_result_cql2sql['a_joins']);
      if ($key) {
        unset($a_result_cql2sql['a_joins'][$key]);

        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
        $a_query[VPX_DB_QUERY_A_JOIN]["aut_object"] = vpx_acl_join_aut_object_get(VPX_ACL_AUT_TYPE_MEDIAFILE);
        $a_query[VPX_DB_QUERY_A_JOIN]['aut_group_domain'] = sprintf("LEFT JOIN {aut_name} AS aut_gd ON aut_gd.app_id IN (%s) AND aut_gd.aut_type = 'DOMAIN' AND aut_gd.aut_name_id=aut_obj.aut_id" , implode(',', $a_app_ids));
        $do_master_slave = TRUE;
        unset($a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access']);
      }

      $key = array_search('AUT_GROUP_REALM', $a_result_cql2sql['a_joins']);
      if ($key) {
        do {
          unset($a_result_cql2sql['a_joins'][$key]);
          $key = array_search('AUT_GROUP_REALM', $a_result_cql2sql['a_joins']);
        }
        while ($key !== FALSE);

        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
        $a_query[VPX_DB_QUERY_A_JOIN]["aut_object"] = vpx_acl_join_aut_object_get(VPX_ACL_AUT_TYPE_MEDIAFILE);
        $a_query[VPX_DB_QUERY_A_JOIN]['aut_group_realm'] = sprintf("LEFT JOIN {aut_name} AS aut_gr ON aut_gr.app_id IN (%s) AND aut_gr.aut_type = 'REALM' AND aut_gr.aut_name_id=aut_obj.aut_id" , implode(',', $a_app_ids));
        $do_master_slave = TRUE;
        unset($a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access']);
      }

      $key = array_search('AUT_APP_MASTER_SLAVE', $a_result_cql2sql['a_joins']);
      if ($key) {
        do {
          unset($a_result_cql2sql['a_joins'][$key]);
          $key = array_search('AUT_APP_MASTER_SLAVE', $a_result_cql2sql['a_joins']);
        }
        while ($key !== FALSE);

        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";
        $a_query[VPX_DB_QUERY_A_JOIN]['aut_app_master_slave_2'] = sprintf("JOIN {aut_app_master_slave} AS aut_ms ON aut_ms.aut_object_type = 'MEDIAFILE' AND aut_ms.aut_object_id=mf.mediafile_id AND aut_ms.app_slave_id IN (%s)" , implode(',', $a_app_ids));
        $do_master_slave = TRUE;
        unset($a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset']['access']);
        unset($a_query[VPX_DB_QUERY_A_JOIN]['aut_app_master_slave']);
      }

      $a_query[VPX_DB_QUERY_A_JOIN]['cql'] = $a_result_cql2sql['a_joins'];
    }

    if (isset($a_result_cql2sql['a_select_expr'])) {
      $a_query[VPX_DB_QUERY_A_SELECT_EXPR] = array_merge($a_query[VPX_DB_QUERY_A_SELECT_EXPR], $a_result_cql2sql['a_select_expr']);
    }

    if (count($a_result_cql2sql['a_order_by']) > 1) {
      throw new vpx_exception_error_cql_error(array('@error' => 'you can not use \'sortBy\' on multiple columns, only specify one column'));
    }

    $a_order_by = reset($a_result_cql2sql['a_order_by']);
    $order_by = $a_order_by['column'];
    $order_direction = $a_order_by['direction'];
  }

  if ($order_by != "") {
    $a_query[VPX_DB_QUERY_A_ORDER_BY][] = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by) ." ". db_escape_string($order_direction);

    $a_asset_property = _media_management_searchsort_is_type($a_funcparam, $order_by, 'asset_property');

    $order_by_alias = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by, TRUE);
    $order_by_alias_full = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by);
    if ($a_asset_property) {

      // Force it to be first in this array...
      $a_query[VPX_DB_QUERY_A_JOIN] = array('asset_property' => array($order_by_alias => sprintf("JOIN {asset_property} AS %s ON %s.prop_id = %d AND %s.asset_id = a.asset_id", $order_by_alias, $order_by_alias, $a_asset_property['prop_id'], $order_by_alias))) + (isset($a_query[VPX_DB_QUERY_A_JOIN]) ? $a_query[VPX_DB_QUERY_A_JOIN] : array());

      if ($a_asset_property['type'] == VPX_PROP_DEF_TYPE_CHAR) {
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][] = $order_by_alias_full ."<>''";
      }
    }
    else {
      //$a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][] = $order_by_alias_full ." IS NOT NULL";

      if ($a_funcparam['a_sort'][VPX_QUERY_DIM_NAME_ORDER_BY]['type'] == VPX_TYPE_ALPHANUM) {
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][] = $order_by_alias_full ."<>''";
      }
    }
  }

  // If coll_id is given, then search within the given collection(s)
  $coll_id = vpx_funcparam_get_value($a_funcparam, 'coll_id', 0);
  if ($coll_id) {
    $a_query[VPX_DB_QUERY_A_JOIN]["asset_collection"][VPX_QUERY_DIM_NAME_ASSET_COLLECTION] = "JOIN {asset_collection} AS asset_coll USING(asset_id)";

    // is_unappropriate
    if (!$is_app_admin) {
      $tmp = '';
      if ($is_public_list && $aut_user_id) {
        $tmp = sprintf("(c.isprivate = 'FALSE' AND (c.is_unappropriate = 'FALSE' OR c.owner_id='%s'))", db_escape_string($aut_user_id));
      }
      elseif ($is_public_list) {
        $tmp = "c.isprivate = 'FALSE' AND c.is_unappropriate = 'FALSE'"; // is_unappropiate must be TRUE
      }
      elseif ($aut_user_id) { // if provided, then we only have access to unappropate when owner.
        $tmp = sprintf("(c.is_unappropriate = 'FALSE' OR c.owner_id='%s')", db_escape_string($aut_user_id));
      }
      else {
        // No public list, no aut_user_id
        $tmp = "c.is_unappropriate = 'FALSE'"; // is_unappropiate must be TRUE
      }
      $a_query[VPX_DB_QUERY_A_JOIN]["collection"][VPX_QUERY_DIM_NAME_COLLECTION] = "INNER JOIN {collection} AS c ON asset_coll.coll_id = c.coll_id AND ". $tmp;
    }

    if (is_array($coll_id)) {
      foreach ($coll_id as $the_coll_id) {
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ASSET_COLLECTION][VPX_DB_WHERE_OR][] = sprintf("asset_coll.coll_id='%s'", db_escape_string($the_coll_id));
      }
    }
    else {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ASSET_COLLECTION][] = sprintf("asset_coll.coll_id='%s'", db_escape_string($coll_id));
    }
  }

  $batch_id = vpx_funcparam_get_value($a_funcparam, 'batch_id', 0);

  if ($batch_id) {
    // Check if batch exists
    vpx_shared_must_exist("ftp_batch", array("batch_id" => $batch_id));

    $a_query[VPX_DB_QUERY_A_JOIN]['ftp_batch_asset'][VPX_QUERY_DIM_NAME_FTP_BATCH] = "JOIN {ftp_batch_asset} AS ftp_ba USING(asset_id)";

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_FTP_BATCH][] = sprintf("ftp_ba.batch_id='%s'", db_escape_string($batch_id));
  }

// Als fav_user_id is gegeven, dan hebben we alleen de resultaten nodig die alleen fav. zijn voor gegeven gebruiker
  $fav_user_id = vpx_funcparam_get_value($a_funcparam, 'fav_user_id', 0);

  if ($fav_user_id) {
    $a_query[VPX_DB_QUERY_A_JOIN]["user_favorites"]['user_fav'] = "LEFT JOIN {user_favorites} AS user_fav ON user_fav.fav_type='". USER_FAV_TYPE_ASSET ."' AND user_fav.fav_id = a.asset_id\n";

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['user_fav'][] = sprintf("user_fav.name='%s'", db_escape_string($fav_user_id));
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['user_fav'][] = sprintf("user_fav.app_id IN(%s)", implode(",", $a_app_ids));
  }

  // Get operator for where
  $s_operator = " ". drupal_strtoupper(vpx_funcparam_get_value($a_funcparam, 'operator', 'and')) ." ";
  $do_alias = (vpx_funcparam_get_value($a_funcparam, 'operator', 'and') == "and");

  $a_where_tmp = array();
  if (isset($a_funcparam['a_parameters_search'])) {

    // Make sure we use alias when we have more than 1 asset_property prop_ids
    if (!$do_alias) {
      $count_asset_properties = 0;
      foreach ($a_funcparam['a_parameters_search'] as $column => $a_searches) {
        $a_search = reset($a_searches);
        if ($a_search['s_table'] == 'asset_property') {
          $count_asset_properties++;
        }
      }

      if ($count_asset_properties > 1) {
        $do_alias = TRUE;
      }
    }

    foreach ($a_funcparam['a_parameters_search'] as $column => $a_searches) {
      $s_tablename_alias = _media_management_search_asset_get_join_name_for_search($a_funcparam, $column, TRUE, $do_alias);
      $s_tablename_alias_full = _media_management_search_asset_get_join_name_for_search($a_funcparam, $column, FALSE, $do_alias);

      $a_search = reset($a_searches);
      if ($a_search['s_table'] == 'asset_property') {

        $a_where_inner = array();
        foreach ($a_searches as $a_search) {
          $a_where_inner[] = _media_management_search_get_part_where($a_search, $s_tablename_alias_full);
        }

        $a_where_tmp[] = sprintf("(%s)", implode(" OR ", $a_where_inner));

        $a_query[VPX_DB_QUERY_A_JOIN]["asset_property"][$s_tablename_alias] = sprintf("LEFT JOIN {asset_property} AS %s ON %s.prop_id = %d AND %s.asset_id = a.asset_id", $s_tablename_alias, $s_tablename_alias, $a_funcparam['a_search'][$column]['prop_id'], $s_tablename_alias);
      }
      elseif ($a_search['s_table'] == 'mediafile') {
        foreach ($a_searches as $a_search) {
          $a_where_tmp[] = _media_management_search_get_part_where($a_search, $s_tablename_alias_full);
        }

        // Need this one...
        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = sprintf("LEFT JOIN {mediafile} AS %s ON a.asset_id=%s.asset_id_root AND %s.is_still = 'FALSE'", $s_tablename_alias, $s_tablename_alias, $s_tablename_alias);
      }
      elseif ($a_search['s_table'] == 'mediafile_metadata') {
        foreach ($a_searches as $a_search) {
          $a_where_tmp[] = _media_management_search_get_part_where($a_search, $s_tablename_alias_full);
        }

        // Need this one too...
        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile'] = "LEFT JOIN {mediafile} AS mf ON a.asset_id=mf.asset_id_root AND mf.is_still = 'FALSE'";

        // Expensive call, oh well...
        $a_query[VPX_DB_QUERY_A_JOIN]['mediafile_metadata'][$s_tablename_alias] = sprintf("LEFT JOIN {mediafile_metadata} AS %s ON mf.mediafile_id=%s.mediafile_id", $s_tablename_alias, $s_tablename_alias);
      }
      else {
        foreach ($a_searches as $a_search) {
          $a_where_tmp[] = _media_management_search_get_part_where($a_search, $s_tablename_alias_full);
        }
      }

      // We moeten minimaal 1 join hebben op asset_property
//      if (!isset($a_query[VPX_DB_QUERY_A_JOIN]["asset_property"]) || !count($a_query[VPX_DB_QUERY_A_JOIN]["asset_property"]) && count($a_searches)) {
//       $a_query[VPX_DB_QUERY_A_JOIN]["asset_property"]["asset_property_default"] = "LEFT JOIN {asset_property} AS asset_property_default USING(asset_id)";
//      }
    }

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_SEARCH] = "\n(". implode("\n". $s_operator, $a_where_tmp) .")";
  }

  // Authentication check
  // All authentication is done as last part in the where to make the selection to authenticate as small as possible
  // Also we only do authication here when we need to filter out the results (granted == FALSE)
  // Except we need to do master/slave check, so we set switch only to include master/slave...
  // Add to the where the access check for our original app_id
  vpx_acl_build_access_where($a_query, VPX_ACL_AUT_TYPE_MEDIAFILE, NULL, $a_app_ids, $aut_user_id, $a_aut_group_id, $aut_domain, $aut_realm, $a_slaves, FALSE, $do_master_slave);

  // Limit, offset stuff
  $a_query[VPX_DB_QUERY_I_LIMIT] = vpx_funcparam_get_value($a_funcparam, 'limit', 10);
  $a_query[VPX_DB_QUERY_I_OFFSET] = vpx_funcparam_get_value($a_funcparam, 'offset', 0);

  // HACK: Unset the aut_app_master_slave when aut_app_master_slave_2 is set
  if (isset($a_query[VPX_DB_QUERY_A_JOIN]['aut_app_master_slave_2'])) {
    unset($a_query[VPX_DB_QUERY_A_JOIN]['aut_app_master_slave']);
  }

/*
  // Do not delete this code! We may have to implement an easier solution without correct ordering, and it is a good start

  // Do the query
  $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => ($a_query[VPX_DB_QUERY_I_LIMIT] < 7 ? FALSE : TRUE)));

  global $db_url;
  if (array_key_exists('data2', $db_url)) {
    db_set_active('data2');
  }
  else {
    db_set_active('data');
  }
  $db_result = vpx_db_query($s_query);
  $s_query_2 = "SELECT found_rows()";
  $db_result_rows = db_query($s_query_2);
  db_set_active();

  $a_ids = array();
  while ($a_id = db_fetch_array($db_result)) {
    $a_ids[] = $a_id['asset_id'];
  }

  $i_found_rows = db_result($db_result_rows);

  // Deleted assets
  $show_deleted = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'show_deleted', 'false'));
  if ($show_deleted) {
    $d_ordby = $a_query[VPX_DB_QUERY_A_ORDER_BY][0];
    $d_limit = $a_query[VPX_DB_QUERY_I_LIMIT];
    $d_offset = $a_query[VPX_DB_QUERY_I_OFFSET];
    $d_search = $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['search'];
    $d_where = sprintf("a.app_id IN(%s)", implode(",", array_unique($a_app_ids_tmp))) . ($d_search ? " AND ". $d_search : "");
    $d_query = "SELECT a.asset_id FROM {asset_delete} AS a". ($d_where ? " WHERE ". $d_where : "") . ($d_ordby ? " ORDER BY ". $d_ordby : "") . ($d_limit ? ($d_offset ? " LIMIT ". $d_offset .",". $d_limit : " LIMIT ". $d_limit) : "");

    db_set_active('data');
    $rs = db_query($d_query);
    db_set_active();

    while ($rso = db_fetch_object($rs)) {
      $a_ids[] = $rso->asset_id;
      $i_found_rows++;
    }
  }
*/

  //print_r($a_query);
  //print_r($s_query);
  //echo "\n\n";
  //print_r($d_query);

  // Deleted assets
  $show_deleted = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'show_deleted', 'false'));
  if (!$show_deleted || isset($coll_id)) {
    // Normal behaviour
    // Do the query
    $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => ($a_query[VPX_DB_QUERY_I_LIMIT] < 7 ? FALSE : TRUE)));

    global $db_url;
    if (array_key_exists('data2', $db_url)) {
      db_set_active('data2');
    }
    else {
      db_set_active('data');
    }
    $db_result = vpx_db_query($s_query);
    $s_query_2 = "SELECT found_rows()";
    $db_result_rows = db_query($s_query_2);
    db_set_active();

    $a_ids = array();
    while ($a_id = db_fetch_array($db_result)) {
      $a_ids[] = $a_id['asset_id'];
    }

    $i_found_rows = db_result($db_result_rows);
  }
  else {
    // Oai behaviour with deleted assets
/*
 * How it is working:
CREATE TEMPORARY TABLE assets_all (asset_id VARCHAR(32) NOT NULL, videotimestampmodified TIMESTAMP NULL DEFAULT NULL, PRIMARY KEY  (asset_id)) ENGINE=MEMORY;

INSERT INTO assets_all SELECT DISTINCT a.asset_id, a.videotimestampmodified FROM asset AS a LEFT JOIN mediafile AS mf ON a.asset_id=mf.asset_id_root
WHERE a.app_id IN(14) AND
a.parent_id IS NULL AND
(((a.isprivate = 'FALSE' OR a.isprivate = 'TRUE') AND a.is_unappropiate = 'FALSE') AND
a.is_empty_asset IS NOT NULL) AND
((a.videotimestampmodified >= '2009-06-12 00:00:00' AND a.videotimestampmodified < '2009-08-15 00:00:00')) AND
(mf.asset_id IS NULL OR (mf.is_original_file='TRUE' AND
mf.app_id IN(14)));

INSERT INTO assets_all SELECT a.asset_id, a.videotimestampmodified FROM asset_delete AS a WHERE a.app_id IN(14) AND
((a.videotimestampmodified >= '2009-06-12 00:00:00' AND a.videotimestampmodified < '2009-08-15 00:00:00'));

SELECT SQL_CALC_FOUND_ROWS DISTINCT a.asset_id FROM assets_all AS a
ORDER BY a.videotimestampmodified ASC LIMIT 20;

DROP TABLE assets_all;
 */

    // Collecting the data
    $d_ordby = $a_query[VPX_DB_QUERY_A_ORDER_BY][0];
    $d_limit = $a_query[VPX_DB_QUERY_I_LIMIT];
    $d_offset = $a_query[VPX_DB_QUERY_I_OFFSET];
    $d_search = $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['search'];
    $d_where = sprintf("a.app_id IN(%s)", implode(",", array_unique($a_app_ids_tmp))) . ($d_search ? " AND ". $d_search : "");

    global $db_url;
    if (array_key_exists('data2', $db_url)) {
      db_set_active('data2');
    }
    else {
      db_set_active('data');
    }

    // Temporary table
    db_query("
      CREATE TEMPORARY TABLE {assets_all} (
        asset_id VARCHAR(32) NOT NULL,
        videotimestampmodified TIMESTAMP NULL DEFAULT NULL,
        PRIMARY KEY  (asset_id)
      ) ENGINE=MEMORY");

    // Modified original query
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = 'a.videotimestampmodified';
    unset($a_query[VPX_DB_QUERY_A_ORDER_BY]);
    unset($a_query[VPX_DB_QUERY_I_LIMIT]);
    unset($a_query[VPX_DB_QUERY_I_OFFSET]);
    $s_query = vpx_db_query_select($a_query);
    $s_query = str_replace('SQL_CALC_FOUND_ROWS', '', $s_query);
    $s_query = "INSERT INTO {assets_all} ". $s_query;
    //$s_query = "INSERT INTO {assets_all} ". $s_query ." LIMIT ". ($d_offset + $d_limit + 1);
    $db_result = vpx_db_query($s_query);

    // Deleted asset query
    db_query("INSERT INTO {assets_all} SELECT a.asset_id, a.videotimestampmodified FROM {asset_delete} AS a". ($d_where ? " WHERE ". $d_where : ""));
    //db_query("INSERT INTO {assets_all} SELECT a.asset_id, a.videotimestampmodified FROM {asset_delete} AS a". ($d_where ? " WHERE ". $d_where : "") ." LIMIT ". ($d_offset + $d_limit + 1));

    // Harvesting the data
    $rs = db_query("SELECT DISTINCT a.asset_id FROM {assets_all} AS a". ($d_ordby ? " ORDER BY ". $d_ordby : "") . ($d_limit ? ($d_offset ? " LIMIT ". $d_offset .",". $d_limit : " LIMIT ". $d_limit) : ""));
    $a_ids = array();
    while ($rso = db_fetch_object($rs)) {
      $a_ids[] = $rso->asset_id;
    }

    // How much assets we have globally
    $s_query_2 = "SELECT COUNT(*) FROM {assets_all}";
    $db_result_rows = db_query($s_query_2);
    $i_found_rows = db_result($db_result_rows);

    // Drop temorary table
    db_query("DROP TABLE {assets_all}");

    db_set_active();
  }


  // Now create the query that will include all other results that are not included here
  // but only when a order by was present

  if (($order_by != "") && ($a_query[VPX_DB_QUERY_I_LIMIT] > 6)) {
    // Special case. If we have an order by, we need to attach the NULL values to the end result
    // Even if we had enough of the other query, we still need to figure out the total result
    // of rows of the 1st query AND the total of the NULL values.
    // Lucky for us, the NULL value query does not require an order by and can match our index 1 on 1

    $a_query_2 = $a_query; // copy 1st query

    // Remove the order by from the query
    unset($a_query_2[VPX_DB_QUERY_A_ORDER_BY]);

    $a_asset_property = _media_management_searchsort_is_type($a_funcparam, $order_by);
    $order_by_alias = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by, TRUE);
    $order_by_alias_full = _media_management_search_asset_get_join_name_for_sort($a_funcparam, $order_by);

    // Remove the where for order by
    unset($a_query_2[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY]);

    if ($a_asset_property) {
      unset($a_query_2[VPX_DB_QUERY_A_JOIN]["asset_property"][$order_by_alias]);// so its inserted as last in array(!)

      // Note; the force index is kinda hack, mysql is stupid, it uses the index for datetime by default, make sure the index exists(!)
 //     $a_query_2[VPX_DB_QUERY_A_JOIN]["asset_property"][$order_by_alias] = "LEFT JOIN {asset_property} AS ". $order_by_alias . (substr($order_by_alias_full, -strlen(".val_char_sort")) == ".val_char_sort" ? " FORCE INDEX (". VPX_IDX_KEY_ASSET_PROP_VAL_CHAR .")" : "") ." ON ". $order_by_alias .".asset_id=a.asset_id AND ". $order_by_alias .".prop_id=". $a_asset_property['prop_id'];
      $a_query_2[VPX_DB_QUERY_A_JOIN]["asset_property"][$order_by_alias] = sprintf("LEFT JOIN {asset_property} AS %s ON %s.prop_id = %d AND %s.asset_id = a.asset_id", $order_by_alias, $order_by_alias, $a_asset_property['prop_id'], $order_by_alias);

      // add to where to include only descriptions from this property for ordering (if property)
      $a_query_2[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][VPX_DB_WHERE_OR][] = $order_by_alias_full ." IS NULL";

      if ($a_asset_property['type'] == VPX_PROP_DEF_TYPE_CHAR) {
        $a_query_2[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][VPX_DB_WHERE_OR][] = $order_by_alias_full ."=''";
      }
    }
    else {
      $a_query_2[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][VPX_DB_WHERE_OR][] = $order_by_alias_full ." IS NULL";

      if ($a_funcparam['a_sort'][VPX_QUERY_DIM_NAME_ORDER_BY]['type'] == VPX_TYPE_ALPHANUM) {
        $a_query_2[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][VPX_DB_WHERE_OR][] = $order_by_alias_full ."=''";
      }
    }

    if ($i_found_rows < $a_query[VPX_DB_QUERY_I_OFFSET]) {
      // Offset starts beyond the end the 1st query.
      $a_query_2[VPX_DB_QUERY_I_OFFSET] = $a_query[VPX_DB_QUERY_I_OFFSET] - $i_found_rows; // correct position
      // limit stays the same.
    }
    else {
      // offset is inside the 1st query
      $a_query_2[VPX_DB_QUERY_I_OFFSET] = 0;
      $a_query_2[VPX_DB_QUERY_I_LIMIT] = max(($a_query[VPX_DB_QUERY_I_LIMIT] - count($a_ids)), 1); // make sure its at least 1
    }

    // Create the query
    $s_query = vpx_db_query_select($a_query_2, array(SQL_CALC_FOUND_ROWS => TRUE));

    // Ok, fill it up
    if (array_key_exists('data2', $db_url)) {
      db_set_active('data2');
    }
    else {
      db_set_active('data');
    }
    $db_result = vpx_db_query($s_query);
    $s_query = "SELECT found_rows()";
    $db_result_rows = db_query($s_query);
    db_set_active();

    // Count the total of the NULL query
    $i_found_rows += db_result($db_result_rows);

    while (count($a_ids) < $a_query[VPX_DB_QUERY_I_LIMIT] && ($a_row = db_fetch_array($db_result))) {
      $a_ids[] = $a_row["asset_id"];
    }
  }

  if (!count($a_ids)) {
    return new rest_response(vpx_return_error(ERRORCODE_EMPTY_RESULT));
  }

  $return_asset_ids = vpx_funcparam_get_value($a_funcparam, 'return_asset_ids', FALSE);

  if (vpx_shared_boolstr2bool($return_asset_ids)) {
    $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
    $o_rest_response->item_total_count = $i_found_rows;
    $o_rest_response->item_offset = vpx_funcparam_get_value($a_funcparam, 'offset', 0);

    foreach ($a_ids as $asset_id) {
      $o_rest_response->add_item(array("asset_id" => $asset_id));
    }

    return $o_rest_response;
  }

  $aut_user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');

  $show_stills = drupal_strtoupper(vpx_funcparam_get_value($a_funcparam, 'show_stills')) == 'TRUE';
  $o_rest_response = media_management_asset_collect($a_ids, $a_metadata_definitions_full, $a_app_ids, $aut_user_id, $granted, $a_aut_group_id, $aut_domain, $aut_realm, $is_app_admin, $show_stills);
  $o_rest_response->item_total_count = $i_found_rows;
  $o_rest_response->item_offset = vpx_funcparam_get_value($a_funcparam, 'offset', 0);
  return $o_rest_response;
}

/**
 * Build the expression for SQL
 *
 * @param array $a_search
 * @param string $s_tablename_alias_full
 * @return string
 */
function _media_management_search_get_part_where($a_search, $s_tablename_alias_full) {

  if (!is_array($a_search['s_value']) && $a_search['s_value'] == "") {
    return sprintf("(%s = '' || %s IS NULL)", $s_tablename_alias_full, $s_tablename_alias_full);
  }

  switch ($a_search['s_type']) {
    case VPX_MATCH_TYPE_BOOLEAN:
      $a_search['s_value'] = (drupal_strtolower($a_search['s_value']) == "true" ? "TRUE" : "FALSE");
    case VPX_MATCH_TYPE_EXACT:
      return sprintf("(%s = '%s')", $s_tablename_alias_full, vpx_db_query_escape($a_search['s_value']));

    case VPX_MATCH_TYPE_CONTAINS:
      return sprintf("(%s LIKE '%%%s%%')", $s_tablename_alias_full, vpx_db_query_escape_like($a_search['s_value']));

    case VPX_MATCH_TYPE_BEGIN:
      return sprintf("(%s LIKE '%s%%')", $s_tablename_alias_full, vpx_db_query_escape_like($a_search['s_value']));

    case VPX_MATCH_TYPE_PERIOD:
      assert(isset($a_search['s_value'][VPX_MATCH_TYPE_PERIOD_FROM]) && isset($a_search['s_value'][VPX_MATCH_TYPE_PERIOD_TO]));
      return sprintf("(%s >= '%s' AND %s < '%s')", $s_tablename_alias_full, vpx_db_query_escape($a_search['s_value'][VPX_MATCH_TYPE_PERIOD_FROM]['s_value']), $s_tablename_alias_full, vpx_db_query_escape($a_search['s_value'][VPX_MATCH_TYPE_PERIOD_TO]['s_value']));

    case VPX_MATCH_TYPE_RANGE:
      assert(isset($a_search['s_value'][VPX_MATCH_TYPE_RANGE_FROM]) && isset($a_search['s_value'][VPX_MATCH_TYPE_RANGE_TO]));
      return sprintf("(%s >= %d AND %s < %d)", $s_tablename_alias_full, vpx_db_query_escape($a_search['s_value'][VPX_MATCH_TYPE_RANGE_FROM]['s_value']), $s_tablename_alias_full, vpx_db_query_escape($a_search['s_value'][VPX_MATCH_TYPE_RANGE_TO]['s_value']));
  }

  // We should not get here...
  throw new vpx_exception_error_unexpected_error();
}

/**
 * Simple test if column is certain table
 *
 * @param array $a_funcparam
 * @param string $column
 * @return array or FALSE
 */
function _media_management_searchsort_is_type($a_funcparam, $column, $type = 'asset_property') {
  if (isset($a_funcparam['a_sort'][$column]) && $a_funcparam['a_sort'][$column]['table'] == $type) {
    return $a_funcparam['a_sort'][$column];
  }
  if (isset($a_funcparam['a_search'][$column]) && $a_funcparam['a_search'][$column]['table'] == $type) {
    return $a_funcparam['a_search'][$column];
  }

  return FALSE;
}

function _media_management_search_asset_get_join_name_for_sort($a_funcparam, $column, $table_alias_only = FALSE, $s_concat_column = "_sort") {
  if (isset($a_funcparam['a_sort'][$column]) && $a_funcparam['a_sort'][$column]['table'] == "asset") {
    $s_concat_column = "";
  }

  return _media_management_search_asset_get_join_name($a_funcparam['a_sort'], $column, $table_alias_only, TRUE, $s_concat_column, TRUE);
}

function _media_management_search_asset_get_join_name_for_search($a_funcparam, $column, $table_alias_only = FALSE, $do_alias = TRUE, $s_concat_column = "") {
  return _media_management_search_asset_get_join_name($a_funcparam['a_search'], $column, $table_alias_only, $do_alias, $s_concat_column);
}

function _media_management_search_asset_get_join_name($a_column_specs, $column, $table_alias_only = FALSE, $do_alias = TRUE, $s_concat_column = "", $is_sort = FALSE) {
  $a_column = FALSE;

  if (isset($a_column_specs[$column])) {
    $a_column = $a_column_specs[$column];
  }
  else {
    assert(0);
  }

  switch ($a_column['table']) {
    case 'asset':
      return ($table_alias_only ? "a". $s_concat_column  : "a". $s_concat_column .".". $a_column['column']);

    case 'mediafile_metadata':
      return ($table_alias_only ? "mm_". $column . $s_concat_column : "mm_". $column . $s_concat_column .".". $column);

    case 'mediafile':
      return ($table_alias_only ? "mf"  : "mf.". $column);
//      return ($table_alias_only ? "mf_". $column . $s_concat_column : "mf_". $column . $s_concat_column .".". $column);

    case 'asset_property':
      switch ($a_column['type']) {
        case VPX_PROP_DEF_TYPE_DATETIME:
          $s_column_name = "val_datetime";
          break;
        case VPX_PROP_DEF_TYPE_INT:
          $s_column_name = "val_int";
          break;
        default:
          assert(0);// new type ?
        case VPX_PROP_DEF_TYPE_CHAR:
          $s_column_name = ($is_sort ? "val_char_sort" : "val_char");
          break;
      }

      if ($do_alias) {
        return ($table_alias_only ? "asset_property_". $column . $s_concat_column : "asset_property_". $column . $s_concat_column .".". $s_column_name);
      }

      return (($table_alias_only ? "asset_property_default" : "asset_property_default.". $s_column_name));
    default:
      break;
  }

  assert(0); // hmmm
  return $column;
}

function _media_management_search_collection($a_funcparam) {
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "c.*";
  $a_query[VPX_DB_QUERY_A_FROM][] = "{collection} AS c";

  $a_query[VPX_DB_QUERYB_ALLOW_DISTINCT] = FALSE;
  $a_query[VPX_DB_QUERY_A_GROUP_BY][] = "c.coll_id";

  // Moved here so numofvideos is always in the result
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "COUNT(DISTINCT asset_coll.asset_id) AS numofvideos";
  $a_query[VPX_DB_QUERY_A_JOIN]["asset_collection"][] = "LEFT JOIN {asset_collection} AS asset_coll USING(coll_id)";

  $a_app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
  $a_slaves = vpx_acl_slave_get($a_app_id, VPX_ACL_AUT_TYPE_COLLECTION);

  $asset_id = vpx_funcparam_get_value($a_funcparam, 'asset_id', 0);
  if ($asset_id) {
    // Check if asset exists
    vpx_shared_must_exist("asset", array("asset_id" => $asset_id));
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset_coll'][] = sprintf("asset_coll.asset_id = '%s'", db_escape_string($asset_id));
  }

  // ega admin may see private assets
  $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));

  $aut_user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');

  // Is private value
  $is_public_list = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_public_list', 'false'));


  // isprivate / unappropiate test.
  // Is outside the ACL check, else we would have problems with 'granted'.
  if (!$is_app_admin) {
    if ($is_public_list && $aut_user_id) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['collection']['access'][VPX_DB_WHERE_AND][] = sprintf("(c.isprivate = 'FALSE' AND (c.is_unappropriate = 'FALSE' OR c.owner_id='%s'))", db_escape_string($aut_user_id));
    }
    elseif ($is_public_list) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['collection']['access'][VPX_DB_WHERE_AND][] = "c.isprivate = 'FALSE' AND c.is_unappropriate = 'FALSE'"; // is_unappropiate must be TRUE
    }
    elseif ($aut_user_id) { // if provided, then we only have access to unappropate when owner.
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['collection']['access'][VPX_DB_WHERE_AND][] = sprintf("(c.is_unappropriate = 'FALSE' OR c.owner_id='%s')", db_escape_string($aut_user_id));
    }
    else {
      // No public list, no aut_user_id
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['collection']['access'][VPX_DB_WHERE_AND][] = "c.is_unappropriate = 'FALSE'"; // is_unappropiate must be TRUE
    }
  }

  // Access selection is done here;
  vpx_acl_build_access_where($a_query, VPX_ACL_AUT_TYPE_COLLECTION, NULL, $a_app_id, $aut_user_id, NULL, NULL, NULL, NULL, $is_app_admin);

  $str_cql = vpx_funcparam_get_value($a_funcparam, 'cql', '');

  if ($str_cql != '') {
    assert(!isset($a_funcparam['a_parameters_search']));

    $a_result_cql2sql = vpx_cql_parse_collection($str_cql, $a_app_id);

    if ($a_result_cql2sql['str_where'] != "") {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_SEARCH] = $a_result_cql2sql['str_where'];
    }

    if (isset($a_result_cql2sql['str_having']) && $a_result_cql2sql['str_having'] != "") {
      $a_query[VPX_DB_QUERY_A_HAVING][] = $a_result_cql2sql['str_having'];
    }

    $a_query[VPX_DB_QUERY_A_JOIN]['cql'] = $a_result_cql2sql['a_joins'];

    if (count($a_result_cql2sql['a_order_by']) > 1) {
      throw new vpx_exception_error_cql_error(array('@error' => 'you can not use \'sortBy\' on multiple columns, only specify one column'));
    }

    $a_order_by = reset($a_result_cql2sql['a_order_by']);

    $order_by = $a_order_by['column'];
    $order_direction = $a_order_by['direction'];
  }
  else {
    $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction', 'ASC');
    $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by', "");
    if ($order_by == "numofvideos") {
      $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction', 'DESC');
    }
  }

  if ($order_by != "") {

    if ($order_by == "numofvideos") {
      // Sort on numofvideos is a special case
      // Again but with DESC as default
      $a_query[VPX_DB_QUERY_A_ORDER_BY][] = "numofvideos ". db_escape_string($order_direction);
    }
    else {

      $a_query[VPX_DB_QUERY_A_ORDER_BY][] = "ISNULL(". _media_management_search_collection_get_join_name($order_by) .") ". db_escape_string($order_direction);
      $a_query[VPX_DB_QUERY_A_ORDER_BY][] = _media_management_search_collection_get_join_name($order_by) ." ". db_escape_string($order_direction);
/*
      note:
      Hier boven doen we ISNULL met order by. Hierdoor doen we alles 1 query met sort van de NULL waarde onder.
      Dit kan een probleem worden met performance, maar dan praten we zeker over een collection tabel met meer dan 250k rows.
      We zitten nu op 629 rows (0.01 sec query). Indien dit een probleem wordt, de query aanpassen en in 2en hakken zoals met
      asset.

      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][] = _media_management_search_collection_get_join_name($order_by) . " IS NOT NULL";

      if ($a_funcparam['a_sort'][VPX_QUERY_DIM_NAME_ORDER_BY]['type'] == VPX_TYPE_ALPHANUM) {
        $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][VPX_QUERY_DIM_NAME_ORDER_BY][] = _media_management_search_collection_get_join_name($order_by) . "<>''";
      }
 */
    }
  }

// Als fav_user_id is gegeven, dan hebben we alleen de resultaten nodig die alleen fav. zijn voor gegeven gebruiker
  $fav_user_id = vpx_funcparam_get_value($a_funcparam, 'fav_user_id', 0);

  if ($fav_user_id) {
    $a_query[VPX_DB_QUERY_A_JOIN]["user_favorites"]['user_fav'] = "LEFT JOIN {user_favorites} AS user_fav ON user_fav.fav_type = '". USER_FAV_TYPE_COLLECTION."' AND user_fav.fav_id = c.coll_id\n";

    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['user_fav'][] = sprintf("user_fav.name = '%s'", db_escape_string($fav_user_id));
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['user_fav'][] = sprintf("user_fav.app_id IN(%s)", implode(",", $a_app_id));
  }

  $a_where_tmp = array();
  if (isset($a_funcparam['a_parameters_search'])) {
    foreach ($a_funcparam['a_parameters_search'] as $column => $a_searches) {
      foreach ($a_searches as $a_search) {
        // expect collection table
        assert($a_search['s_table'] == 'collection');

        $s_tablename_alias_full = _media_management_search_collection_get_join_name($column);
        $s_value = $a_search['s_value'];

        switch ($a_search['s_type']) {
          case VPX_MATCH_TYPE_BOOLEAN:
            $s_value = (drupal_strtolower($s_value) == "true" ? "TRUE" : "FALSE");
          case VPX_MATCH_TYPE_EXACT:
            $a_where_tmp[] = sprintf("(%s = '%s')", $s_tablename_alias_full, db_escape_string($s_value));
            break;
          case VPX_MATCH_TYPE_CONTAINS:
            $a_where_tmp[] = sprintf("(%s LIKE '%%%s%%')", $s_tablename_alias_full, vpx_db_query_escape_like($s_value));
            break;
          case VPX_MATCH_TYPE_BEGIN:
            $a_where_tmp[] = sprintf("(%s LIKE '%s%%')", $s_tablename_alias_full, vpx_db_query_escape_like($s_value));
            break;
          case VPX_MATCH_TYPE_PERIOD:
            assert(isset($a_search['s_value'][VPX_MATCH_TYPE_PERIOD_FROM]) && isset($a_search['s_value'][VPX_MATCH_TYPE_PERIOD_TO]));
            $a_where_tmp[] = sprintf("(%s >= '%s' AND %s < '%s')", $s_tablename_alias_full, db_escape_string($a_search['s_value'][VPX_MATCH_TYPE_PERIOD_FROM]['s_value']), $s_tablename_alias_full, db_escape_string($a_search['s_value'][VPX_MATCH_TYPE_PERIOD_TO]['s_value']));
            break;
          case VPX_MATCH_TYPE_RANGE:
            assert(isset($a_search['s_value'][VPX_MATCH_TYPE_RANGE_FROM]) && isset($a_search['s_value'][VPX_MATCH_TYPE_RANGE_TO]));
            $a_where_tmp[] = sprintf("(%s >= %d AND %s < %d)", $s_tablename_alias_full, db_escape_string($a_search['s_value'][VPX_MATCH_TYPE_RANGE_FROM]['s_value']), $s_tablename_alias_full, db_escape_string($a_search['s_value'][VPX_MATCH_TYPE_RANGE_TO]['s_value']));
            break;
          default:
            assert(0);// should not happen because value has been checked
            return;
        }
      }
    }
  }

  if (count($a_where_tmp)) {
    // Get operator for where
    $s_operator = " " . drupal_strtoupper(vpx_funcparam_get_value($a_funcparam, 'operator', 'and')) . " ";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['search_join'] = "\n(" . implode("\n". $s_operator, $a_where_tmp) .")";
  }

  $a_query[VPX_DB_QUERY_I_LIMIT] = vpx_funcparam_get_value($a_funcparam, 'limit', 10);
  $a_query[VPX_DB_QUERY_I_OFFSET] = vpx_funcparam_get_value($a_funcparam, 'offset', 0);

  // Do the query
  $s_query = vpx_db_query_select($a_query, array(SQL_CALC_FOUND_ROWS => TRUE));

  //_df($s_query);

  db_set_active('data');
  $db_result = vpx_db_query($s_query);
  $s_query = "SELECT found_rows()";
  $db_result_rows = db_query($s_query);
  db_set_active();

  $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  while ($a_collection = db_fetch_array($db_result)) {
    unset($a_collection['testtag']);
    $o_rest_response->add_item($a_collection);
  }

  $o_rest_response->item_total_count = db_result($db_result_rows);
  $o_rest_response->item_offset = vpx_funcparam_get_value($a_funcparam, 'offset', 0);
  return $o_rest_response;
}

function _media_management_search_collection_get_join_name($s_column_name) {
  return "c." . $s_column_name;
}

function _media_management_search_mediafiles($a_funcparam) {
  $asset_id = vpx_funcparam_get_value($a_funcparam, 'asset_id', FALSE);
  $tag = vpx_funcparam_get_value($a_funcparam, 'tag', '');
  $is_still = vpx_funcparam_get_value($a_funcparam, 'is_still', 'FALSE');
  $show_stills = drupal_strtoupper(vpx_funcparam_get_value($a_funcparam, 'show_stills')) == 'TRUE';

  assert($asset_id !== FALSE);
  // Check if asset exists
  vpx_shared_must_exist("asset", array("asset_id" => $asset_id));

  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "mf.mediafile_id";
  $a_query[VPX_DB_QUERY_A_FROM][] = "{asset} AS a";

  $a_query[VPX_DB_QUERYB_ALLOW_DISTINCT] = FALSE;

  // No left join here, only include assets that have mediafiles
  $a_query[VPX_DB_QUERY_A_JOIN]["mediafile"][] = "JOIN {mediafile} AS mf ON a.asset_id = mf.asset_id_root";

  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = sprintf("(a.asset_id = '%s' OR a.parent_id = '%s')", $asset_id, $asset_id);
  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][] = sprintf("mf.is_still = '%s'", $is_still);

  if ($tag) {
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['tag'] = sprintf("mf.tag = '%s'", $tag);
  }

  // Do the query
  $s_query = vpx_db_query_select($a_query);

  db_set_active('data');
  $db_result = vpx_db_query($s_query);
  db_set_active();
  $a_ids = array();

  while ($id = db_fetch_array($db_result)) {
    $a_ids[] = $id['mediafile_id'];
  }

  //  $i_found_rows = db_result($db_result_rows); unused hmmm

  return _media_management_return_mediafile_list($a_ids, vpx_funcparam_get_value($a_funcparam, 'app_id', 0), $show_stills);
}

// @todo: marker start media_management_mediafile

/**
 * This function lists all mediafiles of the given asset id
 *
 * @param string $app_id
 * @param string $asset_id
 * @return: array
 */
function media_management_get_asset_mediafiles($a_args) {
  try {
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'tag', VPX_TYPE_STRING, FALSE, '');
    vpx_funcparam_add($a_funcparam, $a_args, 'is_still', VPX_TYPE_BOOL, FALSE, 'FALSE');

    return _media_management_search_mediafiles($a_funcparam);
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * This function retrieves all info about the given mediafile id
 *
 * @param string $app_id
 * @param string $asset_id
 * @return: array
 */
function media_management_get_mediafile($a_args) {

  try {
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'mediafile_id', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'show_stills', VPX_TYPE_BOOL, FALSE, 'TRUE');

    $mediafile_id = vpx_funcparam_get_value($a_funcparam, 'mediafile_id', "");
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $show_stills = drupal_strtoupper(vpx_funcparam_get_value($a_funcparam, 'show_stills')) == 'TRUE';

    vpx_shared_must_exist("mediafile", array("mediafile_id" => $mediafile_id));

    // ACL check
    vpx_acl_read_single_object(VPX_ACL_AUT_TYPE_MEDIAFILE, $mediafile_id, $app_id);

    $mediafiles = array($mediafile_id);

    // get the media file
    return _media_management_return_mediafile_list($mediafiles, $app_id, $show_stills);
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

function media_management_delete_mediafile($a_args) {

  try {
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'mediafile_id', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL);
    vpx_funcparam_add($a_funcparam, $a_args, 'delete', VPX_TYPE_IGNORE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $mediafile_id = vpx_funcparam_get_value($a_funcparam, 'mediafile_id', "");
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));
    $delete = vpx_funcparam_get_value($a_funcparam, 'delete', "");

    // controleer of de webservice aan staat
    vpx_shared_webservice_must_be_active('media_management', $app_id);

    // Mediafile moet bestaan
    vpx_shared_must_exist('mediafile', array("mediafile_id" => $mediafile_id));

    $a_mediafile = vpx_acl_get_data_from_mediafile($mediafile_id);

    // Check rechten
    vpx_acl_owner_check($app_id, $user_id, $a_mediafile["app_id"], $a_mediafile["owner_id"], $is_app_admin);

    // controleer of deze mediafile als enige onder een sub-asset hangt
    db_set_active('data');
    $a_asset = db_fetch_array(
      db_query(
        "SELECT a.parent_id, a.asset_id FROM {asset} AS a ".
        "JOIN {mediafile} AS mf ON a.asset_id = mf.asset_id ".
        "WHERE mf.mediafile_id = '%s'",
        $mediafile_id
      )
    );

    // Tel de aantal mediafiles onder mijn asset.
    $count = (int)db_result(db_query("SELECT COUNT(*) FROM {mediafile} WHERE asset_id = '%s' AND is_still = 'FALSE'", $a_asset['asset_id']));
    db_set_active();

    // Moet een subasset zijn en alleen deze mediafile als kind.
    $b_delete_asset = (!is_null($a_asset['parent_id']) && $count == 1);

    if ($delete == 'cascade') {
      // Deletes the stills
      db_set_active('data');
      $asset_id_root = db_result(db_query("SELECT asset_id_root FROM {mediafile} WHERE mediafile_id = '%s' AND is_still = 'FALSE'", $mediafile_id));
      db_set_active();
      _media_management_delete_still($asset_id_root, $mediafile_id);
    }

    // verwijder de mediafile
    if (!_media_management_delete_mediafile($mediafile_id)) {
      throw new vpx_exception_error_unexpected_error();
    }

    if ($b_delete_asset) {
      if (!_media_management_delete_asset($a_asset['asset_id'])) {
        throw new vpx_exception_error_unexpected_error();
      }
    }

    // update de timestamps van de asset
    _media_management_update_asset_timestamps($a_asset['asset_id']);

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

function media_management_create_mediafile_wrapper($a_args) {
  if (isset($a_args['post']['asset_id'])) {
    $a_args['uri']['asset_id'] = $a_args['get']['asset_id'];
  }

  return media_management_create_mediafile($a_args);
}

function media_management_create_mediafile($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id'),
      'type' => TYPE_GROUP_ID,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'uri' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'uri'),
      'type' => 'stream_uri',
    ),
    'is_downloadable' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_downloadable'),
      'type' => 'bool',
    ),
    'sannas_mount_point' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'sannas_mount_point', SAN_NAS_BASE_PATH),
      'type' => 'skip',
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'mediafile_id'),
      'type' => 'alphanum',
    ),
    'is_original_file' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'is_original_file', 'true'),
      'type' => 'bool',
    ),
    'filename' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'filename'),
      'type' => 'filename',
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_app_admin', 'FALSE'),
      'type' => 'bool',
    ),
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // kijk of is_app_admin gezet is
  $is_app_admin = vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']);

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  // uri en (filename of is_downloadable) kunnen niet samen worden gebruikt
  if (!is_null($a_parameters['uri']['value']) && (!is_null($a_parameters['filename']['value']) || !is_null($a_parameters['is_downloadable']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_MIX_OF_URI_AND_FILE));
  }

  // maak een uniek mediafile_id aan indien niet opgegeven
  if (is_null($a_parameters['mediafile_id']['value'])) {
    $a_parameters['mediafile_id']['value'] = vpx_create_hash($a_parameters['app_id']['value'], $a_parameters['user_id']['value']);
  }

  // kijk of de asset bestaat
  if ($a_parameters['is_original_file']['value'] == 'true') { // bij originele files mag de asset geen sub-asset zijn...
    if (!vpx_count_rows("asset", array("asset_id", $a_parameters['asset_id']['value'], "parent_id", NULL))) {
      return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $a_parameters['asset_id']['value'])));
    }
  }
  else {
    if (!vpx_count_rows("asset", array("asset_id", $a_parameters['asset_id']['value']))) {
      return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $a_parameters['asset_id']['value'])));
    }
  }

  // acl check op het asset
  db_set_active('data');
  $dbrow_asset = db_fetch_array(db_query("SELECT app_id, owner_id, parent_id FROM {asset} where asset_id  = '%s' ", $a_parameters['asset_id']['value']));
  $asset_owner  = $dbrow_asset["owner_id"];
  $asset_app_id = $dbrow_asset["app_id"];
  $asset_id_root = is_null($dbrow_asset["parent_id"]) ? $a_parameters['asset_id']['value'] : $dbrow_asset["parent_id"];

  // Make sure we test owner on the asset root!
  if ($asset_id_root != $a_parameters['asset_id']['value']) {
    $dbrow_asset = db_fetch_array(db_query("SELECT app_id, owner_id FROM {asset} where asset_id  = '%s' ", $asset_id_root));
    $asset_owner  = $dbrow_asset["owner_id"];
    $asset_app_id = $dbrow_asset["app_id"];
  }

  db_set_active();

  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $asset_app_id, $asset_owner, $is_app_admin);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // kijk waar de nieuwe mediafile ondergebracht moet worden

  if ($a_parameters['is_original_file']['value'] == 'true') {
    // originele file, dus een nieuwe sub-asset aanmaken

    // Indien er al een sub-asset bestaat, maak dan een nieuwe aan.
    if (vpx_count_rows("asset", array("parent_id", $a_parameters['asset_id']['value']))) {
      // opgegeven asset is reeds een 'parent asset'
      $a_args = array(
        'internal' => array(
          'parent_id' => $a_parameters['asset_id']['value']
        ),
        'get' => array(
          'app_id' => $a_parameters['app_id']['value'],
          'user_id' => $a_parameters['user_id']['value'],
          'group_id' => $a_parameters['group_id']['value'],
        ),
      );
      $result = media_management_create_asset($a_args);
      $a_parameters['asset_id']['value'] = $result->response['items'][1]['asset_id'];
    }
    elseif (vpx_count_rows("mediafile", array("asset_id", $a_parameters['asset_id']['value'], "is_still", 'FALSE'))) {

      // er bestaat al een mediafile onder de asset. deze moet verplaatst worden naar een niewe 'sub asset'
      for ($i = 0; $i < 2; $i++) { // maak 2 nieuwe sub assets aan
        $a_args = array(
          'internal' => array(
            'parent_id' => $a_parameters['asset_id']['value']
          ),
          'get' => array(
            'app_id' => $a_parameters['app_id']['value'],
            'user_id' => $a_parameters['user_id']['value'],
            'group_id' => $a_parameters['group_id']['value'],
          ),
        );
        $result = media_management_create_asset($a_args);
        $a_sub_asset[$i] = $result->response['items'][1]['asset_id'];
      }

      // verplaats de bestaande mediafile naar een nieuwe asset_id
      db_set_active('data');
      db_query("UPDATE {mediafile} SET asset_id = '%s' WHERE asset_id = '%s'", $a_sub_asset[0], $a_parameters['asset_id']['value']);
      db_set_active();

      // maak een nieuwe mediafile aan op de volgende asset_id
      $a_parameters['asset_id']['value'] = $a_sub_asset[1];
    }
  }
  // Niet orginele file mag alleen onder een hoofdasset worden geplaatst
  elseif (vpx_count_rows("asset", array("parent_id", $a_parameters['asset_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR));
  }

  // stel de query samen
  $a_set = array();
  foreach (
    array(
      'asset_id' => 'asset_id',
      'app_id' => 'app_id',
      'group_id' => 'group_id',
      'owner_id' => 'user_id',
      'is_downloadable' => 'is_downloadable',
      'is_original_file' => 'is_original_file',
      'sannas_mount_point' => 'sannas_mount_point',
      'mediafile_id' => 'mediafile_id',
      'filename' => 'filename',
      'uri' => 'uri') as $column => $param_name) {

    if (isset($a_parameters[$param_name]['value'])) {
      if (!is_null($a_parameters[$param_name]['value'])) {
        $a_set[] = sprintf("%s='%s'", $column, db_escape_string($a_parameters[$param_name]['value']));
      }
    }
  }
  assert(!empty($a_set));
  // Add the root asset_id, even if asset_id already points to the root.
  $a_set[] = sprintf("asset_id_root='%s'", db_escape_string($asset_id_root));
  $query = "INSERT INTO {mediafile} SET ". implode(", ", $a_set);

  // voer het unieke id in de database in
  db_set_active('data');
  db_query($query);
  db_set_active();

  // Set the external
  _media_management_update_is_external($a_parameters['mediafile_id']['value']);

  // retourneer de gegenereerde mediafile_id
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $rest_response->add_item(array(
    "mediafile_id" => $a_parameters['mediafile_id']['value'],
  ));
  return $rest_response;
}


function media_management_update_mediafile($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'mediafile_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'mediafile_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'filename' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'filename'),
      'type' => 'filename',
    ),
    'uri' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'uri'),
      'type' => 'stream_uri',
    ),
    'is_downloadable' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'is_downloadable'),
      'type' => 'bool',
    ),
    'transcode_inherits_acl' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'transcode_inherits_acl', 'FALSE'),
      'type' => 'bool',
    ),
    'tag' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'tag'),
      'type' => VPX_TYPE_IGNORE,
    ),
  );
  if (isset($a_args['internal']) && count($a_args['internal'])) {
    $a_parameters = array_merge($a_parameters, $a_args['internal']);
  }

  // uri en (filename of is_downloadable) kunnen niet samen worden gebruikt
  if (!is_null($a_parameters['uri']['value']) && (!is_null($a_parameters['filename']['value']) || !is_null($a_parameters['is_downloadable']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_MIX_OF_URI_AND_FILE));
  }

  // uri en (filename of is_downloadable) sluiten elkaar uit
  if (!is_null($a_parameters['uri']['value'])) {
    $type = 'uri';
  }
  else {
    $type = 'file';
  }

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  // kijk of de mediafile bestaat
  if (!vpx_count_rows("mediafile", array("mediafile_id", $a_parameters['mediafile_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_MEDIAFILE_NOT_FOUND, array("@mediafile_id" => $a_parameters['mediafile_id']['value'])));
  }

  // controleer de rechten van de gebruiker op de mediafile
  db_set_active('data');
  $mf_app_id = db_result(db_query("SELECT app_id FROM {mediafile} where mediafile_id  = '%s' ", $a_parameters['mediafile_id']['value']));
  $mf_owner  = db_result(db_query("SELECT owner_id FROM {mediafile} where mediafile_id  = '%s' ", $a_parameters['mediafile_id']['value']));
  db_set_active();

  // controleer de gebruiker rechten
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $mf_app_id, $mf_owner);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // controleer of het huidige mediafile type (video of uri) niet veranderd
  db_set_active('data');
  $a_mediafile = db_fetch_array(db_query("SELECT uri, filename, asset_id FROM {mediafile} WHERE mediafile_id = '%s'", $a_parameters['mediafile_id']['value']));
  db_set_active();
  if ((!is_null($a_mediafile['uri']) && $type === 'file') || (!is_null($a_mediafile['filename']) && $type === 'uri')) {
    return new rest_response(vpx_return_error(ERRORCODE_CHANGE_URI_AND_FILE)); // verbouwing van type mediafile
  }

  // update de timestamps van de asset
  _media_management_update_asset_timestamps($a_mediafile['asset_id']);

  // stel de query samen
  $a_update = array();
  foreach (array('sannas_mount_point', 'is_original_file', 'filename', 'uri', 'is_downloadable', 'transcode_inherits_acl', 'tag') as $subject) {
    if (isset($a_parameters[$subject]['value'])) {
      if (!is_null($a_parameters[$subject]['value'])) {
        $a_update[] = sprintf("%s = '%s'", $subject, db_escape_string($a_parameters[$subject]['value']));
      }
    }
  }

  // controleer of er wel iets veranderd wordt
  if (count($a_update) == 0) {
    return new rest_response(vpx_return_error(ERRORCODE_NO_CHANGES));
  }

  $query = sprintf("UPDATE {mediafile} SET %s WHERE mediafile_id = '%s'", implode(", ", $a_update), $a_parameters['mediafile_id']['value']);

  // voer de wijzigingen door in de database
  db_set_active('data');
  $result = db_query($query);
  db_set_active();

  if (!$result) {
    return new rest_response(vpx_return_error(2346)); // unknown error
  }

  // retourneer een ok
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}


function media_management_internal_update_mediafile($a_args) {
  $a_internal = array(
    'internal' => array(
      'sannas_mount_point' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'sannas_mount_point'),
        'type' => 'skip',
        'required' => FALSE,
      ),
      'is_original_file' => array(
        'value' => vpx_get_parameter_2($a_args['post'], 'is_original_file'),
        'type' => 'bool',
        'required' => TRUE,
      ),
    )
  );

  // retourneer een ok
  return media_management_update_mediafile(array_merge($a_args, $a_internal));
}

function media_management_is_playable($mediafile_id) {
  // Check if the file can be transcoded, might be type 'application/x-empty'
  db_set_active("data");
  $is_unplayable = db_result(db_query("SELECT COUNT(*) FROM {mediafile_metadata} WHERE mediafile_id = '%s' AND mime_type = '%s'",
    $mediafile_id,
    VPX_MIME_TYPE_APPLICATION_X_EMPTY
  ));
  db_set_active();

  return ($is_unplayable ? "This mediafile has an empty filesize." : NULL);
}

function media_management_is_transcodable($mediafile_id) {
  // Check if the file can be played, might be type 'application/x-empty'
  db_set_active("data");
  $is_untranscodeable = db_result(db_query("SELECT COUNT(*) FROM {mediafile_metadata} WHERE mediafile_id = '%s' AND mime_type = '%s'",
    $mediafile_id,
    VPX_MIME_TYPE_APPLICATION_X_EMPTY
  ));
  db_set_active();

  return ($is_untranscodeable ? "This mediafile has an empty filesize." : NULL);
}

/**
 * count mediafiles
 */
function media_management_mediafile_count($a_args) {
  return _media_management_count_items('mediafile', $a_args);
}

/**
 * Delete watermark image
 * Not implemented yet!
 */
function media_management_delete_watermark() {
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));

  $a_parameters = array(
    'watermark_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'watermark_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
  );

  // Validate all of the parameters
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // Check whether is the webservice on
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  $watermark_id = $a_parameters['watermark_id']['value'];

// kijk of de asset bestaat en of het geen sub asset is
  if (!vpx_count_rows("asset", array(
    "asset_id", $asset_id,
    "parent_id", NULL
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $asset_id)));
  }
// controleer of de still bestaat
  if (!vpx_count_rows("mediafile", array("asset_id_root", $asset_id))) {
    return new rest_response(vpx_return_error(ERRORCODE_STILL_NOT_FOUND, array("@asset_id" => $asset_id)));
  }

  // controleer of de gebruiker rechten heeft om de still te verwijderen
  // get owner info
  db_set_active('data');
  $asset_app_id = db_result(db_query("SELECT app_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  $asset_owner  = db_result(db_query("SELECT owner_id FROM {asset} where asset_id  = '%s' ", $asset_id));
  db_set_active();

  // controleer of de gebruiker rechten heeft om de metadata aan te passen
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $asset_app_id, $asset_owner);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // verwijder de still
  if (($error = _media_management_delete_still($asset_id, $mediafile_id, $still_id)) === TRUE) {
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    return $error;
  }
}

// @todo: marker for media_management_favorites

/**
 * @file
 *
 * Media Management favorites include
 */

function media_management_favorites_count_asset($a_args) {
  $a_parameters = array(
    'parameters' => array(
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => 'int',
        'required' => TRUE,
      ),
      'asset_id' => array(
        'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
        'type' => 'alphanum',
        'required' => TRUE,
      ),
      'fav_type' => array(
        'value' => USER_FAV_TYPE_ASSET,
        'type' => 'alphanum',
        'required' => TRUE,
      ),
    ),
  );

  $result = vpx_validate($a_parameters['parameters']);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // copy app_id to fav_id
  $a_parameters['parameters']['fav_id'] = $a_parameters['parameters']['asset_id'];
  return _media_management_favorites_count($a_parameters);
}

function media_management_favorites_count_collection($a_args) {
  $a_parameters = array(
    'parameters' => array(
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => 'int',
        'required' => TRUE,
      ),
      'coll_id' => array(
        'value' => vpx_get_parameter_2($a_args['uri'], 'coll_id'),
        'type' => 'alphanum',
        'required' => TRUE,
      ),
      'fav_type' => array(
        'value' => USER_FAV_TYPE_COLLECTION,
        'type' => 'alphanum',
        'required' => TRUE,
      ),
    ),
  );

  $result = vpx_validate($a_parameters['parameters']);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  // copy coll_id to fav_id
  $a_parameters['parameters']['fav_id'] = $a_parameters['parameters']['coll_id'];
  return _media_management_favorites_count($a_parameters);
}

// @todo: marker start media_management_collection

/**
 * REST call
 * collection/$coll_id
 */
function media_management_get_collection($a_args) {

  try {
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'coll_id', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL, FALSE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', VPX_TYPE_IGNORE, '');

    $coll_id = vpx_funcparam_get_value($a_funcparam, 'coll_id');
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');

    // Check if collection exists
    vpx_shared_must_exist("collection", array("coll_id" => $coll_id));

    // get owner info
    db_set_active('data');
    $dbrow_result = db_fetch_array(db_query("SELECT app_id, owner_id, is_unappropriate FROM {collection} WHERE coll_id  = '%s' ", $coll_id));
    assert($dbrow_result);
    $coll_app_id = $dbrow_result["app_id"];
    $coll_owner  = $dbrow_result["owner_id"];
    $coll_is_unappropriate = vpx_shared_boolstr2bool($dbrow_result["is_unappropriate"]);
    db_set_active();

    if ($coll_is_unappropriate) {
      // Must be owner or admin.
      try {
        // Check the rights
        vpx_acl_owner_check($app_id, $user_id, $coll_app_id, $coll_owner, $is_app_admin);
      }
      catch (vpx_exception_error_access_denied $e) {
        throw new vpx_exception_error(ERRORCODE_IS_UNAPPROPRIATE);
      }
    }

    return _media_management_return_collection_list(array($coll_id));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

/**
 * REST call
 * asset/$asset_id/collection
 * collection
 */
function media_management_get_collection_search($a_args, $b_fav_user_id_required = FALSE) {
  $a_funcparam = array(
    'check_existence' => array(),
    'a_sort' => array(
      'title' => array('table' => 'collection', 'column' => 'title', 'type' => VPX_TYPE_ALPHANUM),
      'description' => array('table' => 'collection', 'column' => 'description', 'type' => VPX_TYPE_ALPHANUM),
      'owner_id' => array('table' => 'collection', 'column' => 'owner_id', 'type' => VPX_TYPE_ALPHANUM),
      'group_id' => array('table' => 'collection', 'column' => 'group_id', 'type' => VPX_TYPE_ALPHANUM),
      'created' => array('table' => 'collection', 'column' => 'created', 'type' => VPX_TYPE_TIMESTAMP),
      'changed' => array('table' => 'collection', 'column' => 'changed', 'type' => VPX_TYPE_TIMESTAMP),
      'private' => array('table' => 'collection', 'column' => 'private', 'type' => VPX_TYPE_BOOL),
      'public' => array('table' => 'collection', 'column' => 'public', 'type' => VPX_TYPE_BOOL),
      'category' => array('table' => 'collection', 'column' => 'category', 'type' => VPX_TYPE_BOOL),
      'numofvideos' => array(), // special case; count the # of videos in the collection
      'app_id_search' => array('table' => 'collection', 'column' => 'app_id', 'type' => VPX_TYPE_INT),
      'app_id' => array('table' => 'collection', 'column' => 'app_id', 'type' => VPX_TYPE_INT),
    ),
  );
  $a_funcparam['a_search'] = $a_funcparam['a_sort']; // gebruik dezelfde parameters voor search & sort
  unset($a_funcparam['a_search']['numofvideos']); // ...op deze na
  unset($a_funcparam['a_search']['app_id']); // op deze na
  unset($a_funcparam['a_search']['app_id_search']); // op deze na

  try {
    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'fav_user_id', TYPE_USER_ID, $b_fav_user_id_required);
    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, TRUE, 10, 1, MAX_RESULT_COUNT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE);
    vpx_funcparam_add($a_funcparam, $a_args, 'operator', VPX_TYPE_SEARCH_OPERATOR, FALSE, "and");
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add($a_funcparam, $a_args, 'private', VPX_TYPE_BOOL);
    vpx_funcparam_add($a_funcparam, $a_args, 'public', VPX_TYPE_BOOL);
    vpx_funcparam_add($a_funcparam, $a_args, 'category', VPX_TYPE_BOOL);
    vpx_funcparam_add($a_funcparam, $a_args, 'public_assign', VPX_TYPE_BOOL);

    // CQL string
    vpx_funcparam_add($a_funcparam, $a_args, 'cql', VPX_TYPE_CQL_COLLECTION);
    $cql = vpx_funcparam_get_value($a_funcparam, 'cql');

    // is_app_admin, allows seeing unappropiate collections
    vpx_funcparam_add($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL, FALSE, 'false');

    // User id for including the collection that are private
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID);
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');

    // is_public_list, switch, TRUE = hide private assets, FALSE = show private assets (default)
    vpx_funcparam_add($a_funcparam, $a_args, 'is_public_list', VPX_TYPE_BOOL, FALSE, 'false');

    if (!is_null($user_id)) {
      vpx_funcparam_set($a_funcparam, 'aut_user_id', $user_id, TYPE_USER_ID);
    }

    if (is_null(vpx_funcparam_get_value($a_funcparam, 'asset_id'))) {
      vpx_funcparam_add($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM);
    }

    $asset_id = vpx_funcparam_get_value($a_funcparam, 'asset_id');
    if (!is_null($asset_id)) {
      vpx_shared_must_exist("asset", array("asset_id" => $asset_id));
    }

// [name]_match=[contains|exact|begin]
// [name]=value

    // Nu de zoek parameters (default)
    foreach ($a_funcparam['a_search'] as $s_key => $a_search) {
      if (!vpx_isset_parameter($a_args, $s_key)) {
        continue;
      }

      // if CQL is set, you specify any search or order parameter
      if ($cql != '') {
        throw new vpx_exception_error_cql_exclusive();
      }

      // Voeg de zoek parameter toe
      vpx_funcparam_search_add($a_funcparam, $a_args, $s_key);
    }

    if (vpx_funcparam_isset($a_funcparam, 'order_by')) {
      $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by', "");
      if ($order_by != "") {

        // if CQL is set, you specify any search or order parameter
        if ($cql != '') {
          throw new vpx_exception_error_cql_exclusive();
        }

        if (array_search($order_by, array_keys($a_funcparam['a_sort'])) === FALSE) {
          throw new vpx_exception_error_sort_field_error(array('@field' => $order_by));
        }
      }
    }

    return _media_management_search_collection($a_funcparam);
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

function media_management_create_collection($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id'),
      'type' => TYPE_GROUP_ID,
    ),
    'private' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'private'),
      'type' => VPX_TYPE_BOOL,
    ),
    'public' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'public'),
      'type' => VPX_TYPE_BOOL,
    ),
    'category' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'category'),
      'type' => VPX_TYPE_BOOL,
    ),
    'isprivate' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'isprivate'),
      'type' => VPX_TYPE_BOOL,
    ),
    'public_assign' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'public_assign'),
      'type' => VPX_TYPE_BOOL,
    ),
    'is_unappropriate' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'is_unappropriate'),
      'type' => VPX_TYPE_BOOL,
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'is_app_admin', 'false'),
      'type' => VPX_TYPE_BOOL,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  if (!vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value'])) {
    unset($a_parameters['is_unappropriate']);
  }

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

// maak een uniek coll_id aan
  $coll_id = vpx_create_hash($a_parameters['app_id']['value'], $a_parameters['user_id']['value']);

// stel de query samen
  $a_insert_columns = array('coll_id');
  $a_insert_values = array($coll_id);
  foreach (array('app_id', 'group_id', 'user_id', 'private', 'public', 'category', 'isprivate', 'public_assign', 'is_unappropriate') as $subject) {
    if (isset($a_parameters[$subject]['value'])) {
      if (!is_null($a_parameters[$subject]['value'])) {
        $a_insert_columns[] = ($subject != 'user_id') ? $subject : "owner_id";
        $a_insert_values[] = db_escape_string($a_parameters[$subject]['value']);
      }
    }
  }
  $query = sprintf("INSERT INTO {collection} (". implode(", ", $a_insert_columns) .") VALUES ('%s')", implode("', '", $a_insert_values));

// voer het unieke id in de database in
  db_set_active('data');
  $result = db_query($query);
  db_set_active();

  if (!$result) {
    return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR)); // unknown error?
  }

// retourneer de gegenereerde asset_id
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $rest_response->add_item(array("coll_id" => $coll_id));
  return $rest_response;
}


function media_management_update_collection($a_args) {
  $a_parameters = array(
    'coll_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'coll_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'title' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'title'),
      'type' => 'skip',
    ),
    'description' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'description'),
      'type' => 'skip',
    ),
    'private' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'private'),
      'type' => VPX_TYPE_BOOL,
    ),
    'public' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'public'),
      'type' => VPX_TYPE_BOOL,
    ),
    'category' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'category'),
      'type' => VPX_TYPE_BOOL,
    ),
    'isprivate' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'isprivate'),
      'type' => VPX_TYPE_BOOL,
    ),
    'public_assign' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'public_assign'),
      'type' => VPX_TYPE_BOOL,
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'is_app_admin', 'false'),
      'type' => VPX_TYPE_BOOL,
    ),
    'owner_id' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'owner_id'),
      'type' => VPX_TYPE_USER_ID,
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'group_id'),
      'type' => VPX_TYPE_GROUP_ID,
    ),
    'is_unappropriate' => array(
      'value' => vpx_get_parameter_2($a_args['post'], 'is_unappropriate'),
      'type' => VPX_TYPE_BOOL,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  if (!vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value'])) {
    unset($a_parameters['is_unappropriate']);
  }

// kijk of is_app_admin gezet is
  $is_app_admin = vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']);

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

// kijk of de collection bestaat
  if (!vpx_count_rows("collection", array("coll_id", $a_parameters['coll_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_COLLECTION_NOT_FOUND, array("@coll_id" => $a_parameters['coll_id']['value'])));
  }

  // get owner info
  db_set_active('data');
  $coll_app_id = db_result(db_query("SELECT app_id FROM {collection} where coll_id  = '%s' ", $a_parameters['coll_id']['value']));
  $coll_owner  = db_result(db_query("SELECT owner_id FROM {collection} where coll_id  = '%s' ", $a_parameters['coll_id']['value']));
  db_set_active();

  // controleer of de gebruiker rechten heeft om de metadata aan te passen
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $coll_app_id, $coll_owner, $is_app_admin);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

// stel de query samen
  $a_update = array();
  foreach (array('title', 'description', 'private', 'public', 'category', 'isprivate', 'public_assign', 'is_unappropriate') as $subject) {
    if (isset($a_parameters[$subject]['value'])) {
      if (!is_null($a_parameters[$subject]['value'])) {
        $a_update[] = sprintf("%s = '%s'", $subject, db_escape_string($a_parameters[$subject]['value']));
      }
    }
  }
  // indien is_app_admin mag de owner_id en group_id aangepast worden
  if (vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value'])) {
    foreach (array('owner_id', 'group_id') as $var_name) {
      if (!is_null($a_parameters[$var_name]['value'])) {
        $a_update[] = sprintf("%s = '%s'", $var_name, db_escape_string($a_parameters[$var_name]['value']));
      }
    }
  }

  // retourneer een foutmelding als er geen wijzigingen gestuurd zijn
  if (!count($a_update)) {
    return new rest_response(vpx_return_error(ERRORCODE_NO_CHANGES));
  }

// voer de wijzigingen door
  db_set_active('data');
  db_query("UPDATE {collection} SET ". implode(", ", $a_update) ." WHERE coll_id = '%s'", $a_parameters['coll_id']['value']);
  db_set_active();

// retourneer een ok
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}


function media_management_delete_collection($a_args) {
  $a_parameters = array(
    'coll_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'coll_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'delete' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'delete'),
      'type' => 'collection_delete',
      'required' => FALSE,
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_app_admin', 'FALSE'),
      'type' => 'bool',
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of is_app_admin gezet is
  $is_app_admin = vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']);

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

// kijk of de collection bestaat
  if (!vpx_count_rows("collection", array("coll_id", $a_parameters['coll_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_COLLECTION_NOT_FOUND, array("@coll_id" => $a_parameters['coll_id']['value'])));
  }

  // get owner info
  db_set_active('data');
  $db_result = db_query("SELECT app_id, owner_id FROM {collection} where coll_id  = '%s' ", $a_parameters['coll_id']['value']);
  db_set_active();

  $db_row = db_fetch_array($db_result);
  $coll_app_id = $db_row["app_id"];
  $coll_owner = $db_row["owner_id"];

  // controleer of de gebruiker rechten heeft om de collection te verwijderen
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $coll_app_id, $coll_owner, $is_app_admin);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

  // kijk of de collection nog assets bevat
  $i_asset_count = vpx_count_rows("asset_collection", array("coll_id", $a_parameters['coll_id']['value']));
  if ($i_asset_count) {
// indien er nog assets in zitten en delete != cascade
    if ($a_parameters['delete']['value'] != "cascade") {
      return new rest_response(vpx_return_error(ERRORCODE_COLLECTION_NOT_EMPTY, array("@asset_count" => $i_asset_count)));
    }
// indien er nog assets in zitten en delete == cascade
    else {
      $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
      db_set_active('data');
      $result = db_query("SELECT asset_id FROM {asset_collection} WHERE coll_id = '%s'", $a_parameters['coll_id']['value']);
      db_set_active();
      while ($asset_id = db_result($result)) {
        $a_args = array(
          'uri' => array(
            'asset_id' => $asset_id,
          ),
          'get' => array(
            'user_id' => $a_parameters['user_id']['value'],
            'app_id' => $a_parameters['app_id']['value'],
            'group_id' => (isset($a_parameters['group_id']['value']) ? $a_parameters['group_id']['value'] : 0),
            'delete' => 'cascade',
          )
        );
        $delete_result = media_management_delete_asset($a_args);
        $o_rest_response->add_item(array(
          'asset_id' => $asset_id,
          'result' => $delete_result->response['header']['request_result'],
          'result_id' => $delete_result->response['header']['request_result_id'],
          'result_description' => $delete_result->response['header']['request_result_description'],
        ));
      }
      $i_asset_count = vpx_count_rows("asset_collection", array("coll_id", $a_parameters['coll_id']['value']));
      if ($i_asset_count) {
        $o_rest_response->set_result(vpx_return_error(ERRORCODE_COLLECTION_NOT_EMPTY, array("@asset_count" => $i_asset_count)));
      }
      else {
        if (!_media_management_delete_collection($a_parameters['coll_id']['value'])) {
          $o_rest_response->set_result(vpx_return_error(UNKNOWN_ERROR));
        }
      }
      return $o_rest_response;
    }
  }
// indien de collection leeg is
  else {
    if (_media_management_delete_collection($a_parameters['coll_id']['value'])) {
      return new rest_response(vpx_return_error(ERRORCODE_OKAY));
    }
    else {
      return new rest_response(vpx_return_error(UNKNOWN_ERROR));
    }
  }
}


function media_management_delete_collection_relation($a_args) {
  $a_parameters = array(
    'coll_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'coll_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'asset_id'),
      'type' => 'skip',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_app_admin', 'FALSE'),
      'type' => 'bool',
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of is_app_admin gezet is
  $is_app_admin = vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']);

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

// kijk of de collection bestaat
  if (!vpx_count_rows('collection', array('coll_id', $a_parameters['coll_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_COLLECTION_NOT_FOUND, array('@coll_id' => $a_parameters['coll_id']['value'])));
  }

// indien er maar 1 asset_id opgegeven is, maak er dan toch een array van
  if (!is_array($a_parameters['asset_id']['value'])) {
    $a_parameters['asset_id']['value'] = array($a_parameters['asset_id']['value']);
  }

  foreach ($a_parameters['asset_id']['value'] as $asset_id) {
    // controleer de asset_id op validiteit
    $a_asset = array(
      'asset_id' => array(
        'value' => $asset_id,
        'type' => 'alphanum'
      )
    );
    $result = vpx_validate($a_asset);
    if (vpx_check_result_for_error($result)) {
      return new rest_response($result);
    }

    // kijk of de asset bestaat
    if (!vpx_count_rows('asset', array('asset_id', $asset_id))) {
      return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $asset_id)));
    }

    // kijk of de asset/collection relatie bestaat
    if (!vpx_count_rows('asset_collection', array('coll_id', $a_parameters['coll_id']['value'], 'asset_id', $asset_id))) {
      return new rest_response(vpx_return_error(ERRORCODE_COLLECTION_ASSET_RELATION_NOT_FOUND, array('@coll_id' => $a_parameters['coll_id']['value'])));
    }

    // get owner info
    db_set_active('data');
    $db_result  = db_query("SELECT * FROM {collection} where coll_id  = '%s' ", $a_parameters['coll_id']['value']);
    $dbrow_collection = db_fetch_array($db_result);
    $db_result  = db_query("SELECT * FROM {asset} where asset_id  = '%s' ", $asset_id);
    $dbrow_asset = db_fetch_array($db_result);
    db_set_active();


    // controleer of de gebruiker rechten heeft om de collection te verwijderen
    try {
      vpx_acl_owner_check_collection_assign($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $dbrow_asset, $dbrow_collection, $is_app_admin);
    }
    catch (vpx_exception_error_access_denied $e) {
      return $e->vpx_exception_rest_response();
    }

  // delete de asset_collection relatie
    if (!_media_management_delete_asset_collection_relation($asset_id, $a_parameters['coll_id']['value'])) {
      return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR)); // unknown error
    }
  }
  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}


function media_management_create_collection_relation($a_args) {
  $a_parameters = array(
    'coll_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'coll_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'asset_id'),
      'type' => 'skip',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

// kijk of de collection bestaat
  if (!vpx_count_rows("collection", array("coll_id", $a_parameters['coll_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_COLLECTION_NOT_FOUND, array("@coll_id" => $a_parameters['coll_id']['value'])));
  }

// indien er maar 1 asset_id opgegeven is, maak er dan toch een array van
  if (!is_array($a_parameters['asset_id']['value'])) {
    $a_parameters['asset_id']['value'] = array($a_parameters['asset_id']['value']);
  }

  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  foreach ($a_parameters['asset_id']['value'] as $asset_id) {
// controleer de asset_id op validiteit
    $a_asset = array(
      'asset_id' => array(
        'value' => $asset_id,
        'type' => 'alphanum'
      )
    );
    $result_object = vpx_validate($a_asset);
    if (vpx_check_result_for_error($result_object)) {
      $result = array(
        'status' => $result_object->response['header']['request_result'],
        'id' => $result_object->response['header']['request_result_id'],
        'description' => $result_object->response['header']['request_result_description'],
      );
      $rest_response->add_result_item(array(
        'coll_id' => $a_parameters['coll_id']['value'],
        'asset_id' => $asset_id,
        'result' => $result['status'],
        'result_id' => $result['id'],
        'result_description' => $result['description'],
      ));
      continue;
    }
  // kijk of de asset bestaat
    if (!vpx_count_rows("asset", array("asset_id", $asset_id))) {
      $result = vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $asset_id));
      $rest_response->add_result_item(array(
        'coll_id' => $a_parameters['coll_id']['value'],
        'asset_id' => $asset_id,
        'result' => $result['status'],
        'result_id' => $result['id'],
        'result_description' => $result['description'],
      ));
      continue;
    }

  // kijk of de relatie al bestaat
    if (vpx_count_rows("asset_collection", array("coll_id", $a_parameters['coll_id']['value'], "asset_id", $asset_id))) {
      $result = vpx_return_error(ERRORCODE_COLLECTION_ASSET_RELATION_ALREADY_EXISTS);
      $rest_response->add_result_item(array(
        'coll_id' => $a_parameters['coll_id']['value'],
        'asset_id' => $asset_id,
        'result' => $result['status'],
        'result_id' => $result['id'],
        'result_description' => $result['description'],
      ));
      continue;
    }

    // get owner info
    db_set_active('data');
    $db_result  = db_query("SELECT * FROM {collection} where coll_id  = '%s' ", $a_parameters['coll_id']['value']);
    $dbrow_collection = db_fetch_array($db_result);
    $db_result  = db_query("SELECT * FROM {asset} where asset_id  = '%s' ", $asset_id);
    $dbrow_asset = db_fetch_array($db_result);
    db_set_active();

    // controleer of de gebruiker rechten heeft om de asset aan de collectie toe te voegen
    try {
      vpx_acl_owner_check_collection_assign($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $dbrow_asset, $dbrow_collection);
    }
    catch (vpx_exception_error_access_denied $e) {
      $result = $e->vpx_exception_error_array_get();

      $rest_response->add_result_item(array(
        'coll_id' => $a_parameters['coll_id']['value'],
        'asset_id' => $asset_id,
        'result' => $result['status'],
        'result_id' => $result['id'],
        'result_description' => $result['description'],
      ));
      continue;
    }

  // zet de asset/collection in de database
    db_set_active('data');
    $result = db_query(
      "INSERT INTO {asset_collection} (asset_id, coll_id) VALUES ('%s','%s')",
      $asset_id,
      $a_parameters['coll_id']['value']
    );
    db_set_active();

    $result = vpx_return_error(ERRORCODE_OKAY);
    $rest_response->add_result_item(array(
      'coll_id' => $a_parameters['coll_id']['value'],
      'asset_id' => $asset_id,
      'result' => $result['status'],
      'result_id' => $result['id'],
      'result_description' => $result['description'],
    ));
  }

// retourneer de response
  return $rest_response;
}


/**
 * count collections
 */
function media_management_collection_count($a_args) {
  return _media_management_count_items('collection', $a_args);
}

/**
 * Delete an asset from all of collection relation
 */
function media_management_delete_asset_from_all_collections($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_app_admin', 'FALSE'),
      'type' => 'bool',
    ),
  );

  // Validate the parameters
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  $asset_id = $a_parameters['asset_id']['value'];
  $app_id = $a_parameters['app_id']['value'];
  $user_id = $a_parameters['user_id']['value'];

  // Check if is_app_admin is set
  $is_app_admin = vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']);

  // Check whether the webservice is active
  if (!vpx_shared_webservice_is_active('media_management', $app_id)) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

  // Check if the asset exists
  if (!vpx_count_rows('asset', array('asset_id', $asset_id))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $asset_id)));
  }

  // Get asset info
  db_set_active('data');
  $db_result  = db_query("SELECT * FROM {asset} where asset_id  = '%s' ", $asset_id);
  $dbrow_asset = db_fetch_array($db_result);
  db_set_active();

  //
  // Get all the asset-collection relation and delete them
  //
  db_set_active('data');
  $rs = db_query("SELECT * FROM {asset_collection} WHERE asset_id = '%s'", $asset_id);
  db_set_active();
  while ($rsa = db_fetch_array($rs)) {

    // get collection info
    db_set_active('data');
    $db_result  = db_query("SELECT * FROM {collection} where coll_id  = '%s' ", $rsa['coll_id']);
    $dbrow_collection = db_fetch_array($db_result);
    db_set_active();

    // Check the user rights to delete the relation
    try {
      vpx_acl_owner_check_collection_assign($app_id, $user_id, $dbrow_asset, $dbrow_collection, $is_app_admin);
    }
    catch (vpx_exception_error_access_denied $e) {
      // There is no access right, so skip it
      continue;
    }

    // Delete de asset_collection relation
    if (!_media_management_delete_asset_collection_relation($asset_id, $rsa['coll_id'])) {
      return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR)); // unknown error
    }

  }

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

// @todo: marker start media_management_asset
/**
 * Search and return found assets
 *
 * @param array $a_args
 * @return array
 */
function media_management_get_asset_search($a_args, $b_fav_user_id_required = FALSE) {
  $a_funcparam = array(
    'a_sort' => array(
      'asset_id' => array('table' => 'asset', 'column' => 'asset_id', 'type' => VPX_TYPE_ALPHANUM),
      'owner_id' => array('table' => 'asset', 'column' => 'owner_id', 'type' => VPX_TYPE_ALPHANUM),
      'group_id' => array('table' => 'asset', 'column' => 'group_id', 'type' => VPX_TYPE_STRING),
      'provider_id' => array('table' => 'asset', 'column' => 'provider_id', 'type' => VPX_TYPE_ALPHANUM),
      'reference_id' => array('table' => 'asset', 'column' => 'reference_id', 'type' => VPX_TYPE_ALPHANUM),
      'videotimestamp' => array('table' => 'asset', 'column' => 'videotimestamp', 'type' => VPX_TYPE_TIMESTAMP),
      'videotimestampmodified' => array('table' => 'asset', 'column' => 'videotimestampmodified', 'type' => VPX_TYPE_TIMESTAMP),
      'mediafile_duration' => array('table' => 'asset', 'column' => 'mediafile_duration', 'type' => VPX_TYPE_ALPHANUM),
      'mediafile_container_type' => array('table' => 'asset', 'column' => 'mediafile_container_type', 'type' => VPX_TYPE_ALPHANUM),
      'changed' => array('table' => 'asset', 'column' => 'changed', 'type' => VPX_TYPE_TIMESTAMP),
      'app_id_search' => array('table' => 'asset', 'column' => 'app_id', 'type' => VPX_TYPE_INT),
      'app_id' => array('table' => 'asset', 'column' => 'app_id', 'type' => VPX_TYPE_INT),
      'mime_type' => array('table' => 'mediafile_metadata', 'column' => 'mime_type', 'type' => VPX_TYPE_STRING),
      'filename' => array('table' => 'mediafile', 'column' => 'filename', 'type' => VPX_TYPE_STRING),
      'numofviews' => array('table' => 'asset', 'column' => 'viewed', 'type' => VPX_TYPE_INT_RANGE),
      'numofplays' => array('table' => 'asset', 'column' => 'played', 'type' => VPX_TYPE_INT_RANGE),
      ),
  );
  vpx_funcparam_add($a_funcparam, $a_args, 'show_stills', VPX_TYPE_BOOL, FALSE, 'TRUE');

  $a_funcparam['a_search'] = $a_funcparam['a_sort']; // gebruik dezelfde parameters voor search & sort
  unset($a_funcparam['a_search']['mediafile_duration']); // op deze na
  unset($a_funcparam['a_search']['app_id']); // op deze na
  unset($a_funcparam['a_sort']['app_id_search']); // op deze na
  unset($a_funcparam['a_sort']['mime_type']); // op deze na
  unset($a_funcparam['a_sort']['filename']); // op deze na

  try {
    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');

    // Alleen asset_ids terug geven...
    vpx_funcparam_add($a_funcparam, $a_args, 'return_asset_ids', VPX_TYPE_BOOL, FALSE, 'false');
    $return_asset_ids = vpx_shared_boolstr2bool($a_funcparam['a_parameters']['return_asset_ids']['value']);

    // stuur de 'rest' parameters naar statistics
    _vpx_statistics_log_search_query($a_args, $app_id);

    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID);
    vpx_funcparam_add($a_funcparam, $a_args, 'fav_user_id', TYPE_USER_ID, $b_fav_user_id_required);

    vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, TRUE, 10, 1, ($return_asset_ids) ? MAX_RESULT_COUNT_IDS_ONLY : MAX_RESULT_COUNT);
    vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM);
    vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION, FALSE);
    vpx_funcparam_add($a_funcparam, $a_args, 'operator', VPX_TYPE_SEARCH_OPERATOR, FALSE, "and");
    vpx_funcparam_add($a_funcparam, $a_args, 'hide_empty_assets', VPX_TYPE_BOOL, FALSE, 'false');
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'coll_id', VPX_TYPE_ALPHANUM);

    if (is_null(vpx_funcparam_get_value($a_funcparam, 'coll_id'))) {
      vpx_funcparam_add_array($a_funcparam, $a_args, 'coll_id', VPX_TYPE_ALPHANUM);// array allowed
    }

    // granted == TRUE, return all results, even on ones we dont have access on
    vpx_funcparam_add($a_funcparam, $a_args, 'granted', VPX_TYPE_BOOL, FALSE, 'true');

    // is_public_list, switch, TRUE = hide private assets, FALSE = show private assets (default)
    vpx_funcparam_add($a_funcparam, $a_args, 'is_public_list', VPX_TYPE_BOOL, FALSE, 'false');

    // is_app_admin, allows seeing unappropiate assets
    vpx_funcparam_add($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL, FALSE, 'false');

    // autorisatie parameters (to be used with acl_check)
    vpx_funcparam_add_array($a_funcparam, $a_args, 'aut_group_id', VPX_TYPE_STRING);
    vpx_funcparam_add($a_funcparam, $a_args, 'aut_domain', VPX_TYPE_STRING);
    vpx_funcparam_add($a_funcparam, $a_args, 'aut_realm', VPX_TYPE_STRING);

    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    if (is_null($user_id)) {
      // Hmmm no user_id, then try aut_user_id and set user_id if we find one
      vpx_funcparam_add($a_funcparam, $a_args, 'aut_user_id', TYPE_USER_ID);
      $aut_user_id = vpx_funcparam_get_value($a_funcparam, 'aut_user_id');
      vpx_funcparam_set($a_funcparam, 'user_id', $aut_user_id, TYPE_USER_ID);
    }
    else {
      vpx_funcparam_set($a_funcparam, 'aut_user_id', $user_id, TYPE_USER_ID);
    }

    // Batch_id
    vpx_funcparam_add($a_funcparam, $a_args, 'batch_id', VPX_TYPE_STRING);

     // CQL string
    vpx_funcparam_add($a_funcparam, $a_args, 'cql', VPX_TYPE_CQL_ASSET);
    $cql = vpx_funcparam_get_value($a_funcparam, 'cql');

    // Voeg toe met de definities van assets
    $a_metadata_definitions_full = _media_management_get_metadata_definitions_full($app_id);

    foreach ($a_metadata_definitions_full as $s_key => $a_metadata) {
      $array = array(
        'table' => 'asset_property',
        'prop_id' => $a_metadata['propdef_id'],
        'type' => $a_metadata['propdef_type'],
        'column' => $a_metadata['propdef_name'],
      );

      $a_funcparam['a_search'][$s_key] = $array;
      $a_funcparam['a_sort'][$s_key] = $array;
    }

// [name]_match=[contains|exact|begin|period]
// [name]=value or YYYY-MM-DD HH:MM:SS|YYYY-MM-DD HH:MM:SS

    // Nu de zoek parameters (default)
    foreach ($a_funcparam['a_search'] as $s_key => $a_search) {
      if (!vpx_isset_parameter($a_args, $s_key)) {
        continue;
      }

      // if CQL is set, you specify any search or order parameter
      if ($cql != '') {
        throw new vpx_exception_error_cql_exclusive();
      }

      // Voeg de zoek parameter toe
      vpx_funcparam_search_add($a_funcparam, $a_args, $s_key);
    }

    if (vpx_funcparam_isset($a_funcparam, 'order_by')) {
      $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by', "");
      if ($order_by != "") {
        // if CQL is set, you specify any search or order parameter
        if ($cql != '') {
          throw new vpx_exception_error_cql_exclusive();
        }

        if (array_search($order_by, array_keys($a_funcparam['a_sort'])) === FALSE) {
          throw new vpx_exception_error_sort_field_error(array('@field' => $order_by));
        }
      }
    }

    vpx_funcparam_add($a_funcparam, $a_args, 'show_deleted', VPX_TYPE_BOOL, FALSE, 'false');

    return _media_management_search_asset($a_funcparam, $a_metadata_definitions_full);
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * This function retrieves all info about the given asset id
 *
 * @param string $app_id
 * @param string $asset_id
 * @return: array
 */
function media_management_get_asset($a_args) {

  try {
    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID);
    vpx_funcparam_add_array($a_funcparam, $a_args, 'aut_group_id', VPX_TYPE_STRING);
    vpx_funcparam_add($a_funcparam, $a_args, 'aut_domain', VPX_TYPE_STRING);
    vpx_funcparam_add($a_funcparam, $a_args, 'aut_realm', VPX_TYPE_STRING);
    vpx_funcparam_add($a_funcparam, $a_args, 'granted', VPX_TYPE_BOOL, FALSE, 'false');
    vpx_funcparam_add($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL);
    vpx_funcparam_add($a_funcparam, $a_args, 'show_stills', VPX_TYPE_BOOL, FALSE, 'TRUE');

    $asset_id = vpx_funcparam_get_value($a_funcparam, 'asset_id');
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');

    $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));

    $aut_user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    if (is_null($aut_user_id)) {
      // Hmmm no user_id, then try aut_user_id
      vpx_funcparam_add($a_funcparam, $a_args, 'aut_user_id', TYPE_USER_ID);
      $aut_user_id = vpx_funcparam_get_value($a_funcparam, 'aut_user_id');
    }

    // Set them both...
    vpx_funcparam_set($a_funcparam, 'user_id', $aut_user_id, TYPE_USER_ID);
    vpx_funcparam_set($a_funcparam, 'aut_user_id', $aut_user_id, TYPE_USER_ID);

    // Hotfix, we dont provide this, we always return the asset, so always supply the granted flag
    $granted = 'true'; //vpx_funcparam_get_value($a_funcparam, 'granted', FALSE, 'TRUE');

    $a_group_id = vpx_funcparam_get_value($a_funcparam, 'aut_group_id');
    $s_aut_domain = vpx_funcparam_get_value($a_funcparam, 'aut_domain');
    $s_aut_realm = vpx_funcparam_get_value($a_funcparam, 'aut_realm');

    // Check if asset exists
    // App_id verwijderd uit must_exist
    vpx_shared_must_exist("asset", array("asset_id" => $asset_id, "parent_id" => NULL));//, "app_id" => $app_id));

    // ACL check.
    vpx_acl_read_single_object(VPX_ACL_AUT_TYPE_ASSET, $asset_id, $app_id);

    // Check inappropriate flag
    _media_management_is_inappropriate($asset_id, $app_id, $aut_user_id, $is_app_admin);

    $a_metadata_definitions_full = _media_management_get_metadata_definitions_full($app_id);

    $results = media_management_asset_collect(
      array($asset_id),
      $a_metadata_definitions_full,
      $app_id,
      $aut_user_id,
      $granted,
      $a_group_id,
      $s_aut_domain,
      $s_aut_realm,
      $is_app_admin
    );

    // toevoegen van mediafile informatie:
    $res = _media_management_search_mediafiles($a_funcparam);
    if ($res) {
      foreach ($res->response['items'] as $r_key => $r_value) {
        $results->response['items']['1']['mediafiles']['mediafile_'.$r_key] = $r_value;
      }
    }

    if (isset($a_args['show_collections']) && $a_args['show_collections'] == 'TRUE') {
      // toevoegen van collection informatie:
      db_set_active('data');
      $result = db_query("SELECT c.coll_id, c.title FROM {asset_collection} ac
        LEFT JOIN {collection} c USING(coll_id)
        WHERE ac.asset_id = '%s'", $asset_id);

      while ($rs = db_fetch_object($result)) {
        $results->response['items']['collection']['#' . serialize( array('id' => $rs->coll_id))] = $rs->title;
      }
      db_set_active();
    }

    // Set viewed + 1
    _mediafile_management_asset_viewed($asset_id);

    return $results;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


/**
 * This function retrieves all info about the given asset id (Version 1.5.0)
 *
 * @param string $app_id
 * @param string $asset_id
 * @return: array
 */
function media_management_get_asset_1_6_0($a_args) {

  try {
    vpx_funcparam_add_array($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID);
    vpx_funcparam_add_array($a_funcparam, $a_args, 'aut_group_id', VPX_TYPE_STRING);
    vpx_funcparam_add($a_funcparam, $a_args, 'aut_domain', VPX_TYPE_STRING);
    vpx_funcparam_add($a_funcparam, $a_args, 'aut_realm', VPX_TYPE_STRING);
    vpx_funcparam_add($a_funcparam, $a_args, 'granted', VPX_TYPE_BOOL, FALSE, 'false');
    vpx_funcparam_add($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL);
    vpx_funcparam_add($a_funcparam, $a_args, 'show_stills', VPX_TYPE_BOOL, FALSE, 'TRUE');

    $asset_id = vpx_funcparam_get_value($a_funcparam, 'asset_id');
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');

    $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));

    $aut_user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    if (is_null($aut_user_id)) {
      // Hmmm no user_id, then try aut_user_id
      vpx_funcparam_add($a_funcparam, $a_args, 'aut_user_id', TYPE_USER_ID);
      $aut_user_id = vpx_funcparam_get_value($a_funcparam, 'aut_user_id');
    }

    // Set them both...
    vpx_funcparam_set($a_funcparam, 'user_id', $aut_user_id, TYPE_USER_ID);
    vpx_funcparam_set($a_funcparam, 'aut_user_id', $aut_user_id, TYPE_USER_ID);

    // Hotfix, we dont provide this, we always return the asset, so always supply the granted flag
    $granted = 'true'; //vpx_funcparam_get_value($a_funcparam, 'granted', FALSE, 'TRUE');

    $a_group_id = vpx_funcparam_get_value($a_funcparam, 'aut_group_id');
    $s_aut_domain = vpx_funcparam_get_value($a_funcparam, 'aut_domain');
    $s_aut_realm = vpx_funcparam_get_value($a_funcparam, 'aut_realm');

    vpx_funcparam_add($a_funcparam, $a_args, 'show_deleted', VPX_TYPE_BOOL, FALSE, 'false');

    if (vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'show_deleted', 'false'))) {
      if (vpx_shared_must_exist("asset_delete", array("asset_id" => $asset_id)) !== FALSE) {
        $a_metadata_definitions_full = _media_management_get_metadata_definitions_full($app_id);

        $a_app_ids = $app_id;
        if (!is_array($a_app_ids)) {
          $a_app_ids = array($a_app_ids);
        }

        $a_output = array();
        db_set_active('data');
        $rs = db_query("SELECT * FROM {asset_delete} WHERE asset_id = '%s'", $asset_id);
        while ($rsa = db_fetch_array($rs)) {
          $rsa['status'] = 'deleted';
          $a_output['assets'][] = $rsa;
        }
        db_set_active();

        return _media_management_create_asset_response($a_output, $a_metadata_definitions_full, array($asset_id), $a_app_ids, $aut_user_id);
      }
    }

    // Check if asset exists
    // App_id verwijderd uit must_exist
    vpx_shared_must_exist("asset", array("asset_id" => $asset_id, "parent_id" => NULL));//, "app_id" => $app_id));

    // ACL check
    vpx_acl_read_single_object(VPX_ACL_AUT_TYPE_ASSET, $asset_id, $app_id);

    // Check inappropriate flag
    _media_management_is_inappropriate($asset_id, $app_id, $aut_user_id, $is_app_admin);

    $a_metadata_definitions_full = _media_management_get_metadata_definitions_full($app_id);

    $results = media_management_asset_collect(
      array($asset_id),
      $a_metadata_definitions_full,
      $app_id,
      $aut_user_id,
      $granted,
      $a_group_id,
      $s_aut_domain,
      $s_aut_realm,
      $is_app_admin
    );

    // toevoegen van mediafile informatie:
    $res = _media_management_search_mediafiles($a_funcparam);
    if ($res) {
      foreach ($res->response['items'] as $r_key => $r_value) {
        $results->response['items']['1']['mediafiles']['mediafile']['#' . serialize( array('id' => $r_key))] = $r_value;
      }
    }

    // Set viewed + 1
    _mediafile_management_asset_viewed($asset_id);

    return $results;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}


function _media_management_update_asset_timestamps($asset_id) {
  db_set_active('data');
  $real_asset_id = db_result(db_query("SELECT parent_id FROM {asset} WHERE asset_id = '%s' AND NOT parent_id IS NULL", $asset_id));
  if ($real_asset_id !== FALSE) {
    $asset_id = $real_asset_id;
  }
  db_query("UPDATE {asset} SET videotimestamp = NOW() WHERE asset_id = '%s' AND videotimestamp IS NULL", $asset_id);
  db_query("UPDATE {asset} SET videotimestampmodified = NOW() WHERE asset_id = '%s'", $asset_id);
  db_set_active();
}

function _mediafile_management_asset_viewed($asset_id) {
  db_set_active('data');
  db_query("UPDATE {asset} SET viewed = viewed + 1 WHERE asset_id = '%s'", $asset_id);
  db_set_active();
}

function _mediafile_management_asset_played($asset_id) {
  db_set_active('data');
  db_query("UPDATE {asset} SET played = played + 1 WHERE asset_id = '%s'", $asset_id);
  db_set_active();
}


function _media_management_update_asset_info($mediafile_id) {
  db_set_active('data');

// zoek de bijbehorende asset_id op
  $asset_id_root = db_result(db_query(
    "SELECT asset_id_root FROM {mediafile} WHERE mediafile_id = '%s'",
    $mediafile_id
  ));

// zoek de lengte van de eerste originele mediafile op
  $mediafile_info = db_fetch_array(db_query(
    "SELECT mm.file_duration, mm.container_type FROM {mediafile} AS m ".
    "JOIN {mediafile_metadata} AS mm ON m.mediafile_id = mm.mediafile_id AND m.is_original_file = 'TRUE' ".
    "WHERE m.asset_id_root = '%s' LIMIT 1",
    $asset_id_root
  ));

  // zet de lengte van de eerste originele mediafile in de parent_asset
  if ($mediafile_info && $mediafile_info['file_duration'] !== NULL) {
    db_query(
      "UPDATE {asset} SET mediafile_duration = '%s', mediafile_container_type = '%s' WHERE asset_id = '%s'",
      $mediafile_info['file_duration'],
      $mediafile_info['container_type'],
      $asset_id_root
    );
  }

  db_set_active();

  // Set the external
  _media_management_update_is_external($mediafile_id, $asset_id_root);
}

function _media_management_update_is_external($mediafile_id, $asset_id_root = NULL) {
  db_set_active('data');

// zoek de bijbehorende asset_id op
  if (!isset($asset_id_root)) {
    $asset_id_root = db_result(db_query(
      "SELECT asset_id_root FROM {mediafile} WHERE mediafile_id = '%s'",
      $mediafile_id
    ));
  }

  if ($asset_id_root !== FALSE) {
    $dbrow = db_fetch_array(
      db_query(
        "SELECT uri, filename FROM {mediafile} WHERE is_original_file = 'TRUE' AND asset_id_root = '%s' LIMIT 1",
        $asset_id_root
      ));

    if ($dbrow) {
      $uri = $dbrow['uri'];
      $filename = $dbrow['filename'];

      // Set is_external && is_empty_asset
      db_query(
        "UPDATE {asset} SET is_external = '%s', is_empty_asset = '%s' WHERE asset_id = '%s'",
        trim($uri) != '' ? 'TRUE' : 'FALSE',
        trim($uri) != '' || trim($filename) != '' ? 'FALSE' : 'TRUE',
        $asset_id_root
      );
    }
  }

  db_set_active();
}


function query_die($query, $die = TRUE) {
  $query = str_replace("{", "", $query);
  $query = str_replace("}", "", $query);
  foreach (array("%d", "%s", "%f", "%b") as $search) {
    $query = str_replace("%". $search, $search, $query);
  }
  if ($die) {
    die("<pre>\n". $query ."\n</pre>\n");
  }
  else {
    echo "<pre>\n". $query ."\n</pre>\n";
  }
}


function media_management_multi_delete_asset($a_args) {
  $a_asset_ids = vpx_get_parameter_2($a_args['get'], 'asset_id');
  if (!is_array($a_asset_ids)) {
    return new rest_response(
      vpx_return_error(ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array("@param" => 'asset_id', "@type" => VPX_TYPE_ALPHANUM))
    );
  }

  $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  foreach ($a_asset_ids as $asset_id) {
    $a_args['uri']['asset_id'] = $asset_id;

    try {
      // Catch any error, because one failure does not fail all.
      $result = media_management_delete_asset($a_args);
    }
    catch (vpx_exception $e) {
      $result = $e->vpx_exception_rest_response();
    }

    $o_rest_response->add_item(array(
      'asset_id' => $asset_id,
      'result' => $result->response['header']['request_result'],
      'result_id' => $result->response['header']['request_result_id'],
      'result_description' => $result->response['header']['request_result_description']
    ));
  }
  return $o_rest_response;
}


function media_management_delete_asset($a_args) {
  $a_parameters = array(
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => VPX_TYPE_ALPHANUM,
      'required' => TRUE,
    ),
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => VPX_TYPE_INT,
      'required' => TRUE,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'delete' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'delete'),
      'type' => 'collection_delete',
      'required' => FALSE,
    ),
    'is_app_admin' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_app_admin', 'FALSE'),
      'type' => 'bool',
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of is_app_admin gezet is
  $is_app_admin = vpx_shared_boolstr2bool($a_parameters['is_app_admin']['value']);

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

// kijk of de asset bestaat
  if (!vpx_count_rows("asset", array("asset_id", $a_parameters['asset_id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $a_parameters['asset_id']['value'])));
  }

  // get owner info
  db_set_active('data');
  $db_result = db_query("SELECT app_id, owner_id FROM {asset} where asset_id  = '%s' ", $a_parameters['asset_id']['value']);
  db_set_active();

  $db_row = db_fetch_array($db_result);
  assert($db_row);
  $asset_app_id = $db_row['app_id'];
  $asset_owner = $db_row['owner_id'];

  // controleer of de gebruiker rechten heeft om de asset te verwijderen
  try {
    vpx_acl_owner_check($a_parameters['app_id']['value'], $a_parameters['user_id']['value'], $asset_app_id, $asset_owner, $is_app_admin);
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }

// kijk of de asset en/of child assets nog mediafiles bevatten
  $a_check_assets = array($a_parameters['asset_id']['value']);

  db_set_active('data');
  $result = db_query("SELECT asset_id FROM {asset} WHERE parent_id = '%s'", $a_parameters['asset_id']['value']);
  db_set_active();
  while ($string = db_result($result)) {
    $a_check_assets[] = $string;
  }

  // verwijder alle jobs van de asset en alle child assets
  foreach ($a_check_assets as $asset_id) {
    if (_media_management_delete_jobs($asset_id) === FALSE) {
      return new rest_response(vpx_return_error(ERRORCODE_JOBS_COULD_NOT_BE_STOPPED));
    }
  }

  // verzamel alle mediafiles
  db_set_active('data');
  $result = db_query("SELECT mediafile_id FROM {mediafile} WHERE asset_id IN ('". implode("', '", $a_check_assets) ."') AND is_still = 'FALSE'");
  db_set_active();
  $a_child_mediafiles = array();
  while ($string = db_result($result)) {
    $a_child_mediafiles[] = $string;
  }

  if (count($a_child_mediafiles)) {
// indien er nog mediafiles in zitten en delete != cascade
    if ($a_parameters['delete']['value'] != "cascade") {
      return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_EMPTY, array("@mediafile_count" => count($a_child_mediafiles))));
    }
// indien er nog mediafiles in zitten en delete == cascade
    else {
      if (!_media_management_delete_mediafile($a_child_mediafiles)) {
        watchdog('mediafile_delete', implode(" ", $a_child_mediafiles), array(), WATCHDOG_ALERT);
        return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR)); // unknown error
      }
      if (!_media_management_delete_asset($a_parameters['asset_id']['value'])) {
        watchdog('asset_delete', $a_parameters['asset_id']['value'], array(), WATCHDOG_ALERT);
        return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR)); // unknown error
      }
    }
  }
// indien de asset 'leeg' is
  else {
    if (!_media_management_delete_asset($a_parameters['asset_id']['value'])) {
      return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR)); // unknown error
    }
  }

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

function media_management_create_asset($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => VPX_TYPE_INT,
      'required' => TRUE,
    ),
    'group_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'group_id'),
      'type' => TYPE_GROUP_ID,
    ),
    'user_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
      'type' => TYPE_USER_ID,
      'required' => TRUE,
    ),
    'reference_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'reference_id'),
      'type' => 'skip',
    ),
    'provider_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'provider_id'),
      'type' => VPX_TYPE_ALPHANUM,
    ),
    'parent_id' => array(
      'value' => vpx_get_parameter_2((isset($a_args['internal']) ? $a_args['internal'] : array()), 'parent_id'),
      'type' => VPX_TYPE_ALPHANUM,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);

  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// controleer of de webservice aan staat
  if (!vpx_shared_webservice_is_active('media_management', $a_parameters['app_id']['value'])) {
    return new rest_response(vpx_return_error(ERRORCODE_WEBSERVICE_DISABLED));
  }

// maak een uniek asset_id aan
  $asset_id = vpx_create_hash($a_parameters['app_id']['value'], $a_parameters['user_id']['value']);

  $a_parameters['owner_id']['value'] = $a_parameters['user_id']['value']; //request speaks of user_id, but table of owner_id

// stel de query samen
  $a_insert_columns = array('asset_id');
  $a_insert_values = array($asset_id);
  foreach (array('app_id', 'group_id', 'owner_id', 'reference_id', 'provider_id', 'parent_id') as $subject) {
    if (isset($a_parameters[$subject]['value'])) {
      if (!is_null($a_parameters[$subject]['value'])) {
        $a_insert_columns[] = $subject;
        $a_insert_values[] = db_escape_string($a_parameters[$subject]['value']);
      }
    }
  }
  $query = sprintf("INSERT INTO {asset} (". implode(", ", $a_insert_columns) .") VALUES ('%s')", implode("', '", $a_insert_values));

// voer het unieke id in de database in
  db_set_active('data');
  db_query($query);
  db_set_active();

// retourneer de gegenereerde asset_id
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $rest_response->add_item(array("asset_id" => $asset_id));
  return $rest_response;
}

function media_management_update_asset($a_args) {

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', VPX_TYPE_USER_ID, TRUE);

    vpx_funcparam_add_post($a_funcparam, $a_args, 'play_restriction_start', VPX_TYPE_DATETIME);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'play_restriction_end', VPX_TYPE_DATETIME);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'isprivate', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'is_unappropiate', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'is_unappropriate', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'is_inappropriate', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'owner_id', VPX_TYPE_USER_ID);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'group_id', VPX_TYPE_GROUP_ID);

    $asset_id = vpx_funcparam_get_value($a_funcparam, 'asset_id');
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));
    $is_unappropiate = vpx_funcparam_get_value($a_funcparam, 'is_unappropiate');
    if (is_null($is_unappropiate)) {
      $is_unappropiate = vpx_funcparam_get_value($a_funcparam, 'is_unappropriate');
    }
    if (is_null($is_unappropiate)) {
      $is_unappropiate = vpx_funcparam_get_value($a_funcparam, 'is_inappropriate');
    }

    $play_restriction_start = vpx_funcparam_get_value($a_funcparam, 'play_restriction_start');
    $play_restriction_end = vpx_funcparam_get_value($a_funcparam, 'play_restriction_end');
    $isprivate = vpx_funcparam_get_value($a_funcparam, 'isprivate');

    // owner_id en group_id worden gebruikt voor het wijzigen van ownership
    $owner_id = vpx_funcparam_get_value($a_funcparam, 'owner_id');
    $group_id = vpx_funcparam_get_value($a_funcparam, 'group_id');

    // controleer of de webservice aan staat
    vpx_shared_webservice_must_be_active('media_management', $app_id);

    // kijk of de asset bestaat en of het geen sub asset is
    vpx_shared_must_exist("asset",
      array(
        "asset_id" => $asset_id,
        "parent_id" => NULL
      )
    );

    // get owner info
    db_set_active('data');
    $dbrow_result = db_fetch_array(db_query("SELECT app_id, owner_id FROM {asset} WHERE asset_id  = '%s' ", $asset_id));
    assert($dbrow_result);
    $asset_app_id = $dbrow_result["app_id"];
    $asset_owner  = $dbrow_result["owner_id"];
    db_set_active();

    // controleer of de gebruiker rechten heeft om de asset aan te passen
    vpx_acl_owner_check($app_id, $user_id, $asset_app_id, $asset_owner, $is_app_admin);

    // If not null, then check if this is the ega admin...
    if (!is_null($is_unappropiate)) {
      vpx_acl_ega_admin_check($app_id, $asset_app_id, $is_app_admin);
    }

    // stel de query samen
    $a_set = array();
    foreach (array("play_restriction_start", "play_restriction_end", "isprivate", "is_unappropiate") as $var_name) {
      if (!is_null($$var_name)) {
        $a_set[] = sprintf("%s = '%s'", $var_name, db_escape_string($$var_name));
      }
    }
    // indien app_admin mag de owner_id en group_id aangepast worden
    if ($is_app_admin) {
      foreach (array('owner_id', 'group_id') as $var_name) {
        if (!is_null($$var_name)) {
          $a_set[] = sprintf("%s = '%s'", $var_name, db_escape_string($$var_name));
        }
      }
    }

    // retourneer een foutmelding als er geen wijzigingen gestuurd zijn
    if (!count($a_set)) {
      throw new vpx_exception_error(ERRORCODE_NO_CHANGES);
    }

    // voer de wijzigingen door in de database
    db_set_active('data');
    vpx_db_query(sprintf("UPDATE {asset}%s WHERE asset_id = '%s'", vpx_db_simple_set($a_set), db_escape_string($asset_id)));
    db_set_active();

    // retourneer een ok
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception_error_access_denied $e) {
    return $e->vpx_exception_rest_response();
  }
}

/* Cron function to delete empty assets
 */
function media_management_cron() {
  if (($run = variable_get("asset_garbage_collector_run", 0)) < ASSET_GARBAGE_COLLECTOR_CRON_INTERVAL) {
    variable_set("asset_garbage_collector_run", $run + 1);
    return;
  }
  variable_set("asset_garbage_collector_run", 0);

  db_set_active("data");
  if (($resource = db_query("select asset_id, unix_timestamp(created) as created from {asset} a where (select count(*) from mediafile where asset_id=a.asset_id)=0 and (select count(*) from asset where parent_id=a.asset_id)=0 and now()>date_add(created, interval %d second)", ASSET_GARBAGE_COLLECTOR_ASSET_TIMEOUT)) != FALSE) {
    while (($data = db_fetch_array($resource)) != FALSE) {
      _media_management_delete_asset($data["asset_id"]);
    }
  }
  db_set_active();
}


/**
 * count assets
 */
function media_management_asset_count($a_args) {
  return _media_management_count_items('asset', $a_args);
}


/**
 * count collection/asset relations
 */
function media_management_asset_count_collections($a_args) {
  $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => VPX_TYPE_INT,
      'required' => TRUE,
    ),
    'asset_id' => array(
      'value' => vpx_get_parameter_2($a_args['uri'], 'asset_id'),
      'type' => VPX_TYPE_ALPHANUM,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

// kijk of de asset bestaat en of het geen sub asset is
  if (!vpx_count_rows("asset", array(
    "asset_id", $a_parameters['asset_id']['value'],
    "parent_id", NULL
  ))) {
    return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $a_parameters['asset_id']['value'])));
  }

// maak een nieuwe rest response aan
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));

// tel de asset/collectie relaties
  db_set_active('data');
  $count = (int)db_result(db_query("SELECT COUNT(asset_id) FROM {asset_collection} WHERE asset_id = '%s'", $a_parameters['asset_id']['value']));
  db_set_active();

// zet de count in de header
  $rest_response->item_total_count = $count;

// retourneer het resultaat
  return $rest_response;
}

// Rewrite of _media_management_add_asset_properties
function media_management_asset_collect($a_asset_ids, $a_metadata_definitions_full, $mixed_app_id, $user_id, $granted, $a_aut_group_id, $s_aut_domain, $s_aut_realm, $is_app_admin = FALSE, $show_stills = TRUE) {
  $a_app_ids = $mixed_app_id;
  if (!is_array($a_app_ids)) {
    $a_app_ids = array($a_app_ids);
  }

  $a_prop_ids = array();
  foreach ($a_metadata_definitions_full as $metadata_definitions_full) {
    $a_prop_ids[] = $metadata_definitions_full['propdef_id'];
  }

  $s_ids = implode("','", $a_asset_ids);

  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "a.*";
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "ap.val_char AS asset_property_value";
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "apd.name AS asset_property_name";
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "apg.name AS asset_property_group_name";

  $a_query[VPX_DB_QUERY_A_FROM][] = "{asset} AS a";

// Koppel de tijdelijke tabel met asset_id's aan de metadata properties
  $a_query[VPX_DB_QUERY_A_JOIN]['ap'] = "LEFT JOIN {v_asset_property} AS ap USING(asset_id)";

// Koppel de definitie van de metadata property aan de prop_id
  $a_query[VPX_DB_QUERY_A_JOIN]['apd'] = "LEFT JOIN {assetprop_definition} AS apd ON ap.prop_id = apd.prop_id";

// Koppel de definitie van de metadata property aan de prop_id
  $a_query[VPX_DB_QUERY_A_JOIN]['apg'] = "LEFT JOIN {assetprop_group} AS apg ON apd.propgroup_id = apg.propgroup_id";

  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['ap'][VPX_DB_WHERE_OR]['prop_id'][] = sprintf("apd.prop_id IN(%s)", implode(",", $a_prop_ids));
  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['ap'][VPX_DB_WHERE_OR]['prop_id'][] = "apd.prop_id IS NULL"; // in case it doesn't have properties
  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['a'][] = sprintf("a.asset_id IN ('%s')", $s_ids);

  $a_query[VPX_DB_QUERY_A_ORDER_BY][] = "ap.id ASC";

  // Do the query
  $s_query = vpx_db_query_select($a_query);

  db_set_active('data');
  $db_result = db_query($s_query);

  $a_output = array();
  while ($a_row = db_fetch_array($db_result)) {
    if (!empty($a_row['still_id']) && $a_row['still_default'] != 'FALSE') {
      $a_output['stills'][$a_row['asset_id']] = $a_row['still_id'];
    }
    unset($a_row['created'], $a_row['changed'], $a_row['still_id']);
    $a_output['assets'][] = $a_row;
  }

  $rs = db_query(sprintf("SELECT * FROM {asset_delete} WHERE asset_id IN ('%s')", $s_ids));
  while ($rsa = db_fetch_array($rs)) {
    $rsa['status'] = 'deleted';
    $a_output['assets'][] = $rsa;
  }

  // Hmmm always take the 1st app_id for now...
  db_set_active();
  $rs = db_query("SELECT id, play_proxy_url, view_asset_url, preview_profile_id, still_url FROM {client_applications} WHERE id IN (%s)", implode(",", $a_app_ids));
  while ($rsa = db_fetch_array($rs)) {
    $a_output['app_info'][$rsa['id']] = $rsa;
  }
  db_set_active('data');

// zoek alle favorites op binnen de assets en van de opgegeven user
  $a_output['user_favorites'] = array();
  if (!is_null($user_id)) {
    $a_query = array();
    $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "fav_id";
    $a_query[VPX_DB_QUERY_A_FROM][] = "{user_favorites}";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("name='%s'", db_escape_string($user_id));
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("app_id IN(%s)", implode(",", $a_app_ids));
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = "fav_type='ASSET'";
    $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("fav_id IN ('%s')", $s_ids);
    $s_query = vpx_db_query_select($a_query);
    $db_result = db_query($s_query);

    while ($a_row = db_fetch_array($db_result)) {
      $a_output['user_favorites'][] = $a_row['fav_id'];
    }
  }

// zoek alle preview mediafiles op
  $a_query = array();
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "m.mediafile_id";
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "m.filename AS mediafile_filename";
  $a_query[VPX_DB_QUERY_A_SELECT_EXPR][] = "IFNULL(a.parent_id, a.asset_id) AS asset_id";
  $a_query[VPX_DB_QUERY_A_FROM][] = "{mediafile} AS m";
  $a_query[VPX_DB_QUERY_A_JOIN]['asset'] = "JOIN {asset} AS a USING(asset_id)";
  if (is_array($a_output['app_info'])) {
    foreach ($a_output['app_info'] as $key => $value) {
      $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['profile'][VPX_DB_WHERE_OR][] = sprintf("(m.transcode_profile_id = %d AND m.app_id = %d)", $a_output['app_info'][$key]['preview_profile_id'], $key);
    }
  }

  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND][] = sprintf("a.app_id IN(%s)", implode(",", $a_app_ids));
  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][VPX_DB_WHERE_OR][] = sprintf("a.asset_id IN('%s')", $s_ids);
  $a_query[VPX_DB_QUERY_A_WHERE][VPX_DB_WHERE_AND]['asset'][VPX_DB_WHERE_OR][] = sprintf("a.parent_id IN('%s')", $s_ids);

  $s_query = vpx_db_query_select($a_query);
  $db_result = db_query($s_query);
  while ($a_row = db_fetch_array($db_result)) {
    $a_output['mediafile_preview'][$a_row['asset_id']] = $a_row;
  }

  // If granted is TRUE, we check access, but allow assets in our result even when we dont have access.
  // If FALSE, we expect search only to supply the assets where we have access on.
  $a_output['granted'] = array(); // must exist, else granted flag is not included on the non-granted

  if (drupal_strtolower($granted) == 'true') {
    $a_asset_ids_access = vpx_acl_access_check_mediafiles($a_asset_ids, $a_app_ids, $user_id, $a_aut_group_id, $s_aut_domain, $s_aut_realm, $is_app_admin);

    foreach ($a_asset_ids_access as $asset_id) {
      $a_output['granted'][$asset_id] = $asset_id;
    }
  }
  else {
    // Expect we have access on all...
    foreach ($a_output['assets'] as $a_row) {
      $a_output['granted'][$a_row['asset_id']] = $a_row['asset_id'];
    }
  }

  db_set_active();
  return _media_management_create_asset_response($a_output, $a_metadata_definitions_full, $a_asset_ids, $a_app_ids, $user_id, $show_stills);
}

function _media_management_is_inappropriate($asset_id, $app_id, $user_id, $is_app_admin = FALSE) {
    db_set_active('data');
    $dbrow_result = db_fetch_array(db_query("SELECT app_id, owner_id, is_unappropiate FROM {asset} WHERE asset_id = '%s' ", $asset_id));
    db_set_active();

    if (!$dbrow_result) {
      return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $asset_id)));
    }

    // Check if its on.
    if (!vpx_shared_boolstr2bool($dbrow_result['is_unappropiate'])) {
      return;
    }

    $asset_app_id = $dbrow_result['app_id'];
    $asset_owner_id  = $dbrow_result['owner_id'];

    if (is_array($app_id)) {
      $app_id = reset($app_id);
    }

    // Must be owner or admin.
    try {
      vpx_acl_owner_check($app_id, $user_id, $asset_app_id, $asset_owner_id, $is_app_admin);
    }
    catch (vpx_exception_error_access_denied $e) {
      throw new vpx_exception_error(ERRORCODE_IS_UNAPPROPRIATE);
    }
}
/**
 * This function retrieves all info about the given asset id
 *
 * @param string $app_id
 * @param string $property_id
 * @return: array
 */
function media_management_asset_tagcount($a_args) {
   // Haal de parameters op ..
  $parameters = array(
      'prop_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'prop_id'),
        'type' => 'int',
        'required' => TRUE
      ),
      'limit' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'limit'),
        'type' => 'int',
        'required' => TRUE
      ),
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => 'int',
        'required' => TRUE
      )
  );

  // valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  $rest_assetprop = new rest_response;

  // voer de query uit op de 'data' database
  db_set_active('data');
  $db_result = db_query("
      SELECT COUNT(val_char) cnt, val_char FROM {asset}
        USE INDEX FOR JOIN (asset_id)
        INNER JOIN {asset_property} USING(asset_id)
        WHERE prop_id = %d AND app_id = %d
        GROUP BY val_char
        ORDER BY cnt DESC
        LIMIT %d",
      $parameters['prop_id']['value'], $parameters['app_id']['value'], $parameters['limit']['value']);

//  print sprintf("
//      SELECT COUNT(val_char) cnt, val_char FROM {asset}
//        INNER JOIN {asset_property} USING(asset_id)
//        WHERE prop_id = %d AND app_id = %d
//        GROUP BY val_char
//        ORDER BY cnt DESC
//        LIMIT %d",
//      $parameters['prop_id']['value'], $parameters['app_id']['value'], $parameters['limit']['value']);

  while ($db_result_row = db_fetch_object($db_result)) {
    $assetprop = array();
    $assetprop["count"] = $db_result_row->cnt;
    $assetprop["tag"] = $db_result_row->val_char;
    $rest_assetprop->add_item($assetprop);
  }
  db_set_active();
  // Voeg de status van de request samen met de lijst van jobs
  $rest_assetprop->set_result(
    vpx_return_error(ERRORCODE_OKAY)
  );
  return $rest_assetprop;
}

// @todo: marker start media_management_asset_supplement (not migrated)

// @todo: marker start media_management_asset_metadata

function media_management_create_metadata($a_args) {

  try {
    // Init the parameters
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add_uri($a_funcparam, $a_args, 'asset_id', VPX_TYPE_ALPHANUM, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'is_app_admin', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'replace', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'action', VPX_TYPE_ALPHA);

    // Copy them into vars
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $asset_id = vpx_funcparam_get_value($a_funcparam, 'asset_id');
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));
    $action = vpx_funcparam_get_value($a_funcparam, 'action');
    $b_replace = vpx_funcparam_get_value($a_funcparam, 'replace', NULL);

    // Both set is not ok..
    if (!is_null($b_replace) && !is_null($action)) {
      throw new vpx_exception_error(ERRORCODE_ACTION_AND_REPLACE);
    }

    if ((is_null($b_replace) && is_null($action)) || is_null($b_replace)) {
      $b_replace = 'true';
    }

    // make it boolean for real
    $b_replace = vpx_shared_boolstr2bool($b_replace);
    // $action will replace $b_replace, but we still need to support, but only 1 can be used.
    if (is_null($action)) {
      $action = $b_replace ? 'replace' : 'append';
    }

    if (!in_array($action, array('replace', 'append', 'update'))) {
      throw new vpx_exception_error(ERRORCODE_VALIDATE_FAILED, array("@param" => $s_key, "@type" => $vpx_type));
    }

    // Check rights first.
    db_set_active('data');
    $dbrow_result = db_fetch_array(db_query("SELECT app_id, owner_id FROM {asset} WHERE asset_id = '%s' ", $asset_id));
    db_set_active();

    if (!$dbrow_result) {
      return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $asset_id)));
    }

    $asset_app_id = $dbrow_result['app_id'];
    $asset_owner  = $dbrow_result['owner_id'];

    // If the app match, we check ownership or else a master slave record must exist
    if ($app_id == $asset_app_id) {
      vpx_acl_owner_check($app_id, $user_id, $asset_app_id, $asset_owner, $is_app_admin);

      // Get full definitions
      $a_metadata_definitions_full = _media_management_get_metadata_definitions_full($app_id);
    }
    else {
      // Check if there is a master / slave record on the asset / mediafile we are trying to add metadata to
      vpx_acl_read_single_object(VPX_ACL_AUT_TYPE_ASSET, $asset_id, $app_id);

      // Only allow own definitions, not dc, qdc etc
      $a_metadata_definitions_full = _media_management_get_metadata_definitions_full($app_id, array());
    }

    // Collect the prop_ids of the allowed properties that we may alter/create
    $a_metadata_allowed = array();
    foreach ($a_metadata_definitions_full as $name => $a_definition) {
      $a_metadata_allowed[$a_definition['propdef_id']] = $a_definition;
    }

    db_set_active('data');
    $db_result_asset_properties = db_query("SELECT id, prop_id, val_char FROM {asset_property} WHERE asset_id = '%s'", $asset_id);
    db_set_active();

    $array_asset_properties = array();

    while ($dbrow_asset_property = db_fetch_array($db_result_asset_properties)) {
      if (isset($a_metadata_allowed[$dbrow_asset_property['prop_id']])) {
        // Store it, so we know which ones are set in the db
        $array_asset_properties[$dbrow_asset_property['prop_id']][$dbrow_asset_property['id']] = $dbrow_asset_property;
      }
    }

    // First check input
    $a_parameters = array();
    foreach ($a_metadata_definitions_full as $name => $a_definition) {
      vpx_funcparam_add_post_array($a_funcparam, $a_args, $name, VPX_TYPE_IGNORE, FALSE, NULL);
      $a_values = vpx_funcparam_get_value($a_funcparam, $name);

      if (is_null($a_values)) {
        continue;
      }

      $a_parameters[$name] = array(
        'value' => (count($a_values) > 1 ? $a_values : reset($a_values)),
        'required' => FALSE,
        'type' => ($name == 'language' ? 'language_code' : ($a_definition['propdef_type'] == VPX_TYPE_DATETIME ? VPX_TYPE_DATETIME : VPX_TYPE_IGNORE))
      );
    }

    // Now do check
    if (count($a_parameters)) {
      $result = vpx_validate($a_parameters);
      if (vpx_check_result_for_error($result)) {
        return new rest_response($result);
      }
    }

    $a_value_set = array();

    db_set_active('data');
    db_query('START TRANSACTION');

    try {
      foreach ($a_metadata_definitions_full as $name => $a_definition) {
        $do_insert = $do_delete = FALSE;
        switch ($action) {
          case 'replace':
            $do_delete = TRUE;
            $do_insert = TRUE;
            break;
          case 'append':
            // Only append when when value was specified as input.
            if (isset($a_parameters[$name])) {
              $do_insert = TRUE;
              $do_delete = FALSE; // appending...
            }
            break;
          case 'update':
            if (isset($a_parameters[$name])) {
              $do_delete = TRUE;
              $do_insert = TRUE;
            }
            break;
        }

        if ($do_delete) {
          $rs = db_query("SELECT id FROM {asset_property} WHERE asset_id = '%s' AND prop_id = %d", $asset_id, $a_definition['propdef_id']);
          while ($rso = db_fetch_object($rs)) {
            db_query("DELETE FROM {large_asset_property} WHERE aprop_id = %d", $rso->id);
          }
          db_query("DELETE FROM {asset_property} WHERE asset_id = '%s' AND prop_id = %d", $asset_id, $a_definition['propdef_id']);
        }

        if (!$do_insert || !isset($a_parameters[$name])) {
          continue;
        }

        $a_parameters[$name]['value'] = is_array($a_parameters[$name]['value']) ? $a_parameters[$name]['value'] : array($a_parameters[$name]['value']);
        foreach ($a_parameters[$name]['value'] as $s_item_value) {
          if ($s_item_value == "") {
            continue;
          }

          switch (drupal_strtoupper($a_definition['propdef_type'])) {
            case 'DATETIME':
              $result = db_query(
                "CALL sp_set_asset_property('%s', %d, '%s', '%s', null)", // sp_set_asset_property(assetid, propid, valchar, valdatetime, valint)
                $asset_id,
                $a_definition['propdef_id'],
                $s_item_value,
                $s_item_value
              );
              break;
            case 'INT':
              $result = db_query(
                "CALL sp_set_asset_property('%s', %d, '%d', null, %d)", // sp_set_asset_property(assetid, propid, valchar, valdatetime, valint)
                $asset_id,
                $a_definition['propdef_id'],
                $s_item_value,
                $s_item_value
              );
              break;
            default:
              $result = db_query(
                "CALL sp_set_asset_property('%s', %d, '%s', null, null)", // sp_set_asset_property(assetid, propid, valchar, valdatetime, valint)
                $asset_id,
                $a_definition['propdef_id'],
                $s_item_value
              );
          }

          $a_value_set[$name] = $s_item_value;
        }
      }
    }
    catch (vpx_exception $e) {
      db_query('ROLLBACK');
      db_set_active();
      throw $e; // Rethrow
    }

    db_query("COMMIT");
    db_set_active();

    // update de timestamps van de asset
    _media_management_update_asset_timestamps($asset_id);

    $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    // Toon wat veranderd is.
    foreach ($a_value_set as $name => $value) {
      $o_rest_response->add_item(array($name => $value));
    }

    return $o_rest_response;
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

