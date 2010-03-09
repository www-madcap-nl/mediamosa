<?php

/**
 * Met deze functie worden alle aanwezige stills voor een asset verwijderd.
 */

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



