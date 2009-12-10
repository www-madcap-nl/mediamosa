<?php









// @todo: marker start media_management_still.inc


/**
 * Met deze functie worden alle aanwezige stills voor een asset verwijderd.
 */


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





// @todo: marker start media_mangement_seach



// @todo: marker start media_management_mediafile





// @todo: marker for media_management_favorites

/**
 * @file
 *
 * Media Management favorites include
 */



// @todo: marker start media_management_collection











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
    if (!mediamosa_asset_collection::delete($asset_id, $rsa['coll_id'])) {
      return new rest_response(vpx_return_error(ERRORCODE_UNEXPECTED_ERROR)); // unknown error
    }

  }

  return new rest_response(vpx_return_error(ERRORCODE_OKAY));
}

// @todo: marker start media_management_asset




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
        $a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id);

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

    $a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id);

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
    vpx_funcparam_add_post($a_funcparam, $a_args, 'is_unappropriate', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'is_unappropriate', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'is_inappropriate', VPX_TYPE_BOOL);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'owner_id', VPX_TYPE_USER_ID);
    vpx_funcparam_add_post($a_funcparam, $a_args, 'group_id', VPX_TYPE_GROUP_ID);

    $asset_id = vpx_funcparam_get_value($a_funcparam, 'asset_id');
    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');
    $is_app_admin = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'is_app_admin', 'false'));
    $is_unappropriate = vpx_funcparam_get_value($a_funcparam, 'is_unappropriate');
    if (is_null($is_unappropriate)) {
      $is_unappropriate = vpx_funcparam_get_value($a_funcparam, 'is_unappropriate');
    }
    if (is_null($is_unappropriate)) {
      $is_unappropriate = vpx_funcparam_get_value($a_funcparam, 'is_inappropriate');
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
    if (!is_null($is_unappropriate)) {
      vpx_acl_ega_admin_check($app_id, $asset_app_id, $is_app_admin);
    }

    // stel de query samen
    $a_set = array();
    foreach (array("play_restriction_start", "play_restriction_end", "isprivate", "is_unappropriate") as $var_name) {
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
      mediamosa_asset::delete($data["asset_id"]);
    }
  }
  db_set_active();
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
      $a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id);
    }
    else {
      // Check if there is a master / slave record on the asset / mediafile we are trying to add metadata to
      vpx_acl_read_single_object(VPX_ACL_AUT_TYPE_ASSET, $asset_id, $app_id);

      // Only allow own definitions, not dc, qdc etc
      $a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id, array());
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

