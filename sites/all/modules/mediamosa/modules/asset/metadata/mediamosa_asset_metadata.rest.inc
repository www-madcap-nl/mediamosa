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
  *
  */


/**
 * URI: /asset/$asset_id/metadata
 *
 * Create the metadata for the asset.
 *
 * 1.x: media_management_create_metadata
 */
class mediamosa_rest_call_asset_metadata_create extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const ACTION = 'action';

  private $a_metadata_definitions_full = NULL;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to create the metadata for.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID, must be owner of asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::REPLACE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Replace all metadata or append given metadata values. Do not specify both action and replace parameters.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ACTION => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ALPHA,
          self::VAR_DESCRIPTION => "Action to choose for given metadata values, choose either 'replace', 'append' or 'update'. Do not specify both action and replace parameters.",
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_ALLOWED_VALUES => array('replace', 'append', 'update'),
        ),
      ),
    );

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Override process_rest_args.
  protected function process_rest_args(array $a_var_setup) {
    // process current args so we can use them.
    $a_var_setup = parent::process_rest_args($a_var_setup);

    // Now we can add the metadata definition input to our var setup.
    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Asset must exist.
    $a_asset = mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // If the app match, we check ownership or else a master slave record must exist
    if ($app_id == $a_asset[mediamosa_asset_db::APP_ID]) {
      mediamosa_aut::owner_check($app_id, $user_id, $a_asset[mediamosa_asset_db::APP_ID], $a_asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

      // Get full definitions.
      $this->a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id);
    }
    else {
      // Check if there is a master / slave record on the asset / mediafile we are trying to add metadata to.
      mediamosa_aut::read_single_object(mediamosa_aut::AUT_TYPE_ASSET, $asset_id, $app_id);

      // Only allow own definitions, not dc, qdc etc.
      $this->a_metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id, array());
    }

    // We need to add the allowed definitions to the $a_var_setup.
    foreach ($this->a_metadata_definitions_full as $name => $a_definition) {

      switch ($a_definition['propdef_type']) {
        case mediamosa_asset_metadata_property_db::TYPE_INT:
          $type = mediamosa_type::TYPE_INT;
          break;
        case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
          $type = mediamosa_type::TYPE_DATETIME;
          break;
        case mediamosa_asset_metadata_property_db::TYPE_CHAR:
        default:
          // Default is string.
          $type = mediamosa_type::TYPE_STRING;
          break;
      }

      // FIXME Don't like this hack. Rather see extension on metadata property with a allowed values.
      if ($name == 'language') {
        $type = mediamosa_type::TYPE_LANGUAGE_CODE;
      }

      // Do not setup a default value, for any value(!).
      $a_var_setup[self::VARS][$name] = array(
        self::VAR_TYPE => $type,
        self::VAR_DESCRIPTION => 'A metadata property.',
        self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES, // always array.
      );
    }

    // Now again process the args with the metadata properties.
    return parent::process_rest_args($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $asset_id = $this->get_param_value(self::ASSET_ID);

    $a_app_ids = $this->get_param_value_app();;
    $app_id = reset($a_app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Can't be set both.
    if ($this->isset_param(self::ACTION) && $this->isset_param(self::REPLACE)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ACTION_AND_REPLACE);
    }

    // Make sure replace is set to default.
    if (!$this->isset_param(self::REPLACE)) {
      $this->set_param_value(self::REPLACE, true);
    }

    // Get replace value.
    $replace = $this->get_param_value(self::REPLACE);

    // Will use $action, so if not set, use value of replace to get value for it.
    if (!$this->isset_param(self::REPLACE)) {
      $this->set_param_value(self::ACTION, $replace ? 'replace' : 'append');
    }

    $action = $this->get_param_value(self::ACTION);

    // Fill the values.
    $a_params = array();
    foreach ($this->a_metadata_definitions_full as $name => $a_definition) {
      if ($this->isset_given_param($name)) {
        $a_params[$name] = $this->get_param_value($name);
      }
    }

    // Create the metadata.
    $a_value_set = mediamosa_asset_metadata::metadata_create($asset_id, $this->a_metadata_definitions_full, $a_params, $action);

    // Return what has been changed/inserted.
    foreach ($a_value_set as $name => $value) {
      $o_mediamosa->add_item(array($name => $value));
    }
  }
}
