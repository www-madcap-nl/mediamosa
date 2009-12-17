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
  * The REST calls of the mediamosa_asset_mediafile module.
  */

/**
 * URI: /mediafile/count
 * Method: GET
 *
 * 1.x: media_management_mediafile_count.
 *
 */
class mediamosa_rest_call_count_asset_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const OWNER_ID = 'owner_id';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID to match.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    $group_id = $this->get_param_value(self::GROUP_ID);
    $owner_id = $this->get_param_value(self::OWNER_ID);

    $o_mediamosa->set_result_okay();

    // Set the total count.
    $o_mediamosa->item_total_count = mediamosa_db::db_count_items(mediamosa_asset_mediafile_db::TABLE_NAME, $app_id, $owner_id, $group_id);
  }
}

/**
 * URI: /mediafile/$mediafile_id (1.x media_management_get_mediafile)
 * Method: GET
 */
class mediamosa_rest_call_get_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const SHOW_STILLS = 'show_stills';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include the stills in the output.',
          self::VAR_DEFAULT_VALUE => 'TRUE'
        )
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $show_stills = $this->get_param_value(self::SHOW_STILLS);

    // Must exists.
    mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));

    // Must have access.
    mediamosa_aut::read_single_object(mediamosa_aut::AUT_TYPE_MEDIAFILE, $mediafile_id, $app_id);

    // Now enrich response with mediafile data.
    mediamosa_asset_mediafile::enrich_response_mediafile($o_mediamosa, array($mediafile_id), $a_app_ids, FALSE, $show_stills);

    // Done.
  }
}

/**
 * URI: /asset/$asset_id/mediafile
 * Method: GET
 *
 * 1.x: media_management_get_asset_mediafiles
 */
class mediamosa_rest_call_get_asset_mediafiles extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const TAG = 'tag';
  const IS_STILL = 'is_still';
  const IS_OAI = 'is_oai';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::IS_STILL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include the stills in the output.',
          self::VAR_DEFAULT_VALUE => 'FALSE'
        ),
        self::IS_OAI => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include extra information required for OAI output.',
          self::VAR_DEFAULT_VALUE => 'FALSE'
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Search on tag.',
        )
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $tag = $this->get_param_value(self::TAG);
    $is_still = $this->get_param_value(self::IS_STILL);
    $is_oai = $this->get_param_value(self::IS_OAI);

    $a_mediafile_ids = mediamosa_asset_mediafile::mediafiles_search($asset_id, $tag, $is_still);

    // Create output.
    mediamosa_asset_mediafile::enrich_response_mediafile($o_mediamosa, $a_mediafile_ids, $a_app_ids, $is_oai);
  }
}

/**
 * URI: /mediafile/$mediafile_id/delete
 * Method: GET
 *
 * 1.x: media_management_delete_mediafile
 */
class mediamosa_rest_call_delete_mediafiles extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';
  const DELETE = 'delete';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile to delete.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::DELETE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_ALLOWED_VALUES => array('cascade', '')
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);
    $delete = $this->get_param_value(self::DELETE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $a_app_ids);

    // Must exists.
    mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));

    // Get the mediafile.
    $a_mediafile = mediamosa_aut::get_data_from_mediafile($mediafile_id);

    // Does the user has access?
    mediamosa_aut::owner_check(
      $app_id,
      self::get_param_value(self::USER_ID),
      $a_mediafile[mediamosa_asset_mediafile_db::APP_ID],
      $a_mediafile[mediamosa_asset_mediafile_db::OWNER_ID]
    );

    // Check if this mediafile is the only one under a sub-asset.
    $query = db_select(mediamosa_asset_db::TABLE_NAME, 'a');
    $query->fields('a', array(mediamosa_asset_db::PARENT_ID, mediamosa_asset_db::ID));
    $query->addJoin('mf', mediamosa_asset_mediafile_db::ID, strtr('a.#asset_id = mf.#asset_id', array('#asset_id' => mediamosa_asset_db::ID)));
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
    $a_asset = $query->execute()->fetchAssoc();

    // Count the number of mediafiles.
    $query = db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->fields('mf', array(mediamosa_asset_mediafile_db::ID));
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID, $a_asset[mediamosa_asset_db::ID]);
    $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
    $count = $query->countQuery()->execute()->fetchField();

    // Moet een subasset zijn en alleen deze mediafile als kind.
    $delete_asset = (!is_null($a_asset['parent_id']) && $count == 1);

    if ($delete == 'cascade') {
      // Deletes the stills.
      $query = db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->fields('mf', array(mediamosa_asset_mediafile_db::ASSET_ID_ROOT));
      $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
      $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
      $asset_id_root = $query->execute()->fetchField();

      mediamosa_asset_mediafile_still::delete($asset_id_root, $mediafile_id);
    }

    // Delete the mediafile.
    mediamosa_asset_mediafile::delete($mediafile_id);

    if ($delete_asset) {
      mediamosa_asset::delete($a_asset[mediamosa_asset_db::ID]);
    }

    $o_mediamosa->set_result_okay();

    // update de timestamps van de asset
    mediamosa_asset::update_asset_timestamps($a_asset[mediamosa_asset_db::ID]);
  }
}


/**
 * URI: /asset/$asset_id/mediafile/create
 * Method: POST
 *
 * 1.x: media_management_create_mediafile
 */
class mediamosa_rest_call_create_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const GROUP_ID = 'group_id';
  const USER_ID = 'user_id';
  const DELETE = 'delete';
  const IS_DOWNLOADABLE = 'is_downloadable';
  const MOUNT_POINT = 'sannas_mount_point';
  const IS_ORIGINAL_FILE = 'is_original_file';
  const FILENAME = 'filename';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID_ID,
          self::VAR_DESCRIPTION => 'The ID of the parent asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID.',
        ),
        self::URI => array(
          self::VAR_TYPE => mediamosa_type::TYPE_URI,
          self::VAR_DESCRIPTION => 'The stream URI.',
        ),
        self::IS_DOWNLOADABLE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Mediafile can be downloaded.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_ORIGINAL_FILE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Mark mediafile as original.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_FILENAME,
          self::VAR_DESCRIPTION => 'Filename of the mediafile, can not be combined with URI.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
        self::MOUNT_POINT => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The location path of the file.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);
    $is_original_file = $this->get_param_value(self::IS_ORIGINAL_FILE);
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Must be enabled.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $a_app_ids);

    if ($this->isset_param(self::URI) && ($this->isset_param(self::FILENAME) || $this->isset_param(self::IS_DOWNLOADABLE))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MIX_OF_URI_AND_FILE);
    }

    // Set mediafile ID to something if not present.
    if (!$this->isset_param(self::MEDIAFILE_ID)) {
      $this->set_param_value(self::MEDIAFILE_ID, mediamosa_db::uuid($app_id));
    }

    if ($is_original_file) {
      mediamosa_db::db_must_exists(
         mediamosa_asset_db::TABLE_NAME,
         array(
           mediamosa_asset_db::ID => $asset_id,
           mediamosa_asset_db::PARENT_ID => NULL
         ),
         mediamosa_error_db::ERRORCODE_ASSET_NOT_FOUND,
         array(
           '@asset_id' => $asset_id,
         )
      );
    }
    else {
      mediamosa_db::db_must_exists(
         mediamosa_asset_db::TABLE_NAME,
         array(
           mediamosa_asset_db::ID => $asset_id,
         ),
         mediamosa_error_db::ERRORCODE_ASSET_NOT_FOUND,
         array(
           '@asset_id' => $asset_id,
         )
      );
    }

    $a_asset = mediamosa_db::db_query(
      'SELECT #app_id, #owner_id, #parent_id FROM {#mediamosa_asset} WHERE #asset_id = :asset_id',
      array(
        '#app_id' => mediamosa_asset_db::APP_ID,
        '#owner_id' => mediamosa_asset_db::OWNER_ID,
        '#parent_id' => mediamosa_asset_db::PARENT_ID,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    )->fetchAssoc();

    $owner_id  = $a_asset[mediamosa_asset_db::OWNER_ID];
    $asset_app_id = $a_asset[mediamosa_asset_db::APP_ID];
    $asset_id_root = is_null($a_asset[mediamosa_asset_db::PARENT_ID]) ? $asset_id : $a_asset[mediamosa_asset_db::PARENT_ID];

    // Make sure we test owner on the asset root.
    if ($asset_id_root != $asset_id) {
      $a_asset = mediamosa_db::db_query(
        'SELECT #app_id, #owner_id FROM {#mediamosa_asset} WHERE #asset_id = :asset_id',
        array(
          '#app_id' => mediamosa_asset_db::APP_ID,
          '#owner_id' => mediamosa_asset_db::OWNER_ID,
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_db::ID,
          ':asset_id' => $asset_id_root
        )
      )->fetchAssoc();

      $owner_id  = $a_asset[mediamosa_asset_db::OWNER_ID];
      $asset_app_id = $a_asset[mediamosa_asset_db::APP_ID];
    }

    // Check owner, need ownership.
    mediamosa_aut::owner_check($app_id, $user_id, $asset_app_id, $owner_id, $is_app_admin);

    // @todo: Make design choice on parent assets.

/*
    // See where mediafile needs to be created.
    if ($is_original_file) {
      // Original file, so lets create new sub-asset.

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
  mediamosa_asset::update_asset_info_is_external($a_parameters['mediafile_id']['value']);

  // retourneer de gegenereerde mediafile_id
  $rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));
  $rest_response->add_item(array(
    "mediafile_id" => $a_parameters['mediafile_id']['value'],
  ));
  return $rest_response;*/
  }
}

/**
 * URI: /mediafile/$mediafile_id
 * Method: POST
 *
 * 1.x: media_management_update_mediafile
 */
class mediamosa_rest_call_update_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';
  const IS_DOWNLOADABLE = 'is_downloadable';
  const MOUNT_POINT = 'sannas_mount_point';
  const IS_ORIGINAL_FILE = 'is_original_file';
  const FILENAME = 'filename';
  const TRANSCODE_INHERITS_ACL = 'transcode_inherits_acl';
  const TAG = 'tag';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_FILENAME,
          self::VAR_DESCRIPTION => 'Mark mediafile as original.',
        ),
        self::URI => array(
          self::VAR_TYPE => mediamosa_type::TYPE_URI,
          self::VAR_DESCRIPTION => 'The stream URI.',
        ),
        self::IS_DOWNLOADABLE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Mediafile can be downloaded.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::TRANSCODE_INHERITS_ACL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Any transcodes will inherit ACL settings.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'User specified tag.',
        ),
        self::MOUNT_POINT => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The location path of the file.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
        self::IS_ORIGINAL_FILE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Mark mediafile as original.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $mediafile_id = $this->get_param_value(mediamosa_asset_mediafile_db::ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Must be enabled.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $a_app_ids);

    // Mediafile must exists.
    mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));

    // Cant mix input of uri/filename/is_downloadable.
    if ($this->isset_param(self::URI) && ($this->isset_param(self::FILENAME) || $this->isset_param(self::IS_DOWNLOADABLE))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MIX_OF_URI_AND_FILE);
    }

    // Get the mediafile.
    $a_mediafile = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_asset_mediafile} WHERE #mediafile_id = :mediafile_id',
      array(
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        ':mediafile_id' => $mediafile_id
      )
    );

    // Does the user has access?
    mediamosa_aut::owner_check(
      $app_id,
      $user_id,
      $a_mediafile[mediamosa_asset_mediafile_db::APP_ID],
      $a_mediafile[mediamosa_asset_mediafile_db::OWNER_ID]
    );

    // Either uri or file type.
    $type = $this->isset_given_param(self::URI) ? 'uri' : 'file';

    // Can't change the type.
    if ( ($a_mediafile[mediamosa_asset_mediafile_db::URI] && $type == 'file') || ($a_mediafile[mediamosa_asset_mediafile_db::FILENAME] && $type == 'uri') ) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CHANGE_URI_AND_FILE);
    }

    // update de timestamps van de asset
    mediamosa_asset::update_asset_timestamps($a_mediafile[mediamosa_asset_mediafile_db::ASSET_ID]);

    // Fields allowed to update.
    $a_params = array(
      mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
      mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
      mediamosa_asset_mediafile_db::FILENAME,
      mediamosa_asset_mediafile_db::URI,
      mediamosa_asset_mediafile_db::IS_DOWNLOADABLE,
      mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL,
      mediamosa_asset_mediafile_db::TAG
    );

    // Build the query.
    $query = mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME);

    $a_fields = array();
    foreach ($a_params as $param) {
      if (!$this->isset_given_param($param)) {
        // If its not supplied, skip it.
        continue;
      }

      $a_fields[$param] = $this->get_param_value($param);
    }

    if (empty($a_fields)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_CHANGES);
    }

    // Add the fields to the query.
    $query->fields($a_fields);
    // Add conditions.
    $query->condition(mediamosa_asset_mediafile_db::ID, $mediafile_id);
    $query->execute();

    // Set all OK.
    $o_mediamosa->set_result_okay();
  }
}
