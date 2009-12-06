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
  * The Asset REST calls.
  */

/**
 * URI: /change_ownership
 * Method: POST
 */
class mediamosa_rest_call_change_ownership extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const OLD_GROUP_ID = 'old_group_id';
  const NEW_GROUP_ID = 'new_group_id';
  const OLD_OWNER_ID = 'old_owner_id';
  const NEW_OWNER_ID = 'new_owner_id';
  const OLD_APP_ID = 'old_app_id';
  const NEW_APP_ID = 'new_app_id';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::OLD_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The current group ID to replace.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::NEW_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The new group ID to replace with.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::OLD_OWNER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The current owner to replace.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::NEW_OWNER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The new owner to replace with.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::OLD_APP_ID => array( // Unused.
          self::VAR_TYPE => mediamosa_type::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The current app to replace.',
        ),
        self::NEW_APP_ID => array( // Unused.
          self::VAR_TYPE => mediamosa_type::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The new app to replace with.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    /*
     * Because its to dangerous, new_app_id/old_app_id has been turned off (#774).
     */
    $old_owner_id = $this->get_param_value(self::OLD_OWNER_ID);
    $new_owner_id = $this->get_param_value(self::NEW_OWNER_ID);
    $old_group_id = $this->get_param_value(self::OLD_GROUP_ID);
    $new_group_id = $this->get_param_value(self::NEW_GROUP_ID);
    $old_app_id = reset($a_app_ids);
    $new_app_id = 0;

    // One (or more) of each set, and its ok.
    $set_old = ($old_owner_id == '' && $old_group_id == '' && !$old_app_id);
    $set_new = ($new_owner_id == '' && $new_group_id == '' && !$new_app_id);
    if ($set_old || $set_new) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CHANGE_OWNERSHIP_MISSING_PARAMETERS);
    }

    // All ok.
    $o_mediamosa->set_result_okay();

    // The tables.
    $a_table = array(
      mediamosa_asset_mediafile_db::TABLE_NAME,
      mediamosa_asset_db::TABLE_NAME,
      mediamosa_collection_db::TABLE_NAME,l
    );

    // Go through each table and do our stuff.
    foreach ($a_table as $table) {
      $query = mediamosa_db::db_update($table);

      // Conditions.
      if ($old_owner_id != '') {
        $query->condition('owner_id', $old_owner_id);
      }

      if ($old_group_id != '') {
        $query->condition('group_id', $old_group_id);
      }

      if ($old_app_id) {
        $query->condition('app_id', $old_app_id);
      }

      // Changes.
      $a_set = array();
      if ($new_owner_id != '') {
        $a_set['owner_id'] = $new_owner_id;
      }

      if ($new_group_id != '') {
        $a_set['group_id'] = $new_group_id;
      }

      if ($new_app_id) {
        $a_set['app_id'] = $new_app_id;
      }

      assert(count($a_set));

      // Add sets.
      $query->fields($a_set);

      // Add affected rows to output.
      $o_mediamosa->add_item(array($table => $query->execute()));
    }
  }
}

/**
 * URI: /asset/count
 * Method: GET
 */
class mediamosa_rest_call_count_asset extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const OWNER_ID = 'owner_id';

  // ------------------------------------------------------------------ Public Functions.
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

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    $group_id = $this->get_param_value(self::GROUP_ID);
    $owner_id = $this->get_param_value(self::OWNER_ID);

    $o_mediamosa->set_result_okay();

    // Set the total count.
    $o_mediamosa->item_total_count = mediamosa_db::db_count_items(mediamosa_asset_db::TABLE_NAME, $app_id, $owner_id, $group_id);
  }
}

/**
 * URI: /asset
 * Method: GET
 */
class mediamosa_rest_call_asset_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const RETURN_ASSET_IDS = 'return_asset_ids';
  const USER_ID = 'user_id';
  const AUT_USER_ID = 'aut_user_id'; // Alias for user_id.
  const FAV_USER_ID = 'fav_user_id';
  const OPERATOR = 'operator';
  const HIDE_EMPTY_ASSETS = 'hide_empty_assets';
  const COLL_ID = 'coll_id';
  const GRANTED = 'granted';
  const IS_PUBLIC_LIST = 'is_public_list';
  const AUT_GROUP_ID = 'aut_group_id';
  const AUT_DOMAIN = 'aut_domain';
  const AUT_REALM = 'aut_realm';
  const BATCH_ID = 'batch_id';
  const CQL = 'cql';
  const SHOW_DELETED = 'show_deleted';
  const SHOW_STILLS = 'show_stills';
  const SHOW_COLLECTIONS = 'show_collections';

  // ------------------------------------------------------------------ Public Functions.
  static public function get_search_params() {
    return array(
      'asset_id' => mediamosa_type::TYPE_SEARCH_STRING,
      'owner_id' => mediamosa_type::TYPE_SEARCH_STRING,
      'group_id' => mediamosa_type::TYPE_SEARCH_STRING,
      'provider_id' => mediamosa_type::TYPE_SEARCH_STRING,
      'reference_id' => mediamosa_type::TYPE_SEARCH_STRING,
      'videotimestamp' => mediamosa_type::TYPE_SEARCH_DATETIME,
      'videotimestampmodified' => mediamosa_type::TYPE_SEARCH_DATETIME,
      'mediafile_duration' => mediamosa_type::TYPE_SEARCH_STRING,
      'mediafile_container_type' => mediamosa_type::TYPE_SEARCH_STRING,
      'changed' => mediamosa_type::TYPE_SEARCH_DATETIME,
      'app_id_search' => mediamosa_type::TYPE_SEARCH_INT,
      'mime_type' => mediamosa_type::TYPE_SEARCH_STRING,
      'filename' => mediamosa_type::TYPE_SEARCH_STRING,
      'numofviews' => mediamosa_type::TYPE_SEARCH_INT,
      'numofplays' => mediamosa_type::TYPE_SEARCH_INT,
    );
  }

  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::RETURN_ASSET_IDS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Return only the found asset ids.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::FAV_USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Search within the favorites of this supplied user ID.',
        ),
        self::HIDE_EMPTY_ASSETS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Do not include assets that do not have mediafiles.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'Search within these collections.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::GRANTED => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include a flag on each asset to indicate access. When providing with TRUE, the result will include assets where no access is available.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::IS_PUBLIC_LIST => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include or hide private assets in result. TRUE: exclude private assets, FALSE: include private assets.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_APP_ADMIN => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Allows seeing unappropriate assets.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Alias for aut_user_id parameter, is chosen above aut_user_id.',
        ),
        self::AUT_USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
        ),
        self::AUT_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::AUT_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::AUT_DOMAIN => array(
          self::VAR_TYPE => mediamosa_type::TYPE_DOMAIN,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
        ),
        self::AUT_REALM=> array(
          self::VAR_TYPE => mediamosa_type::TYPE_REALM,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
        ),
        self::BATCH_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BATCH_ID,
          self::VAR_DESCRIPTION => 'Search within batch.',
        ),
        self::CQL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_CQL_ASSET,
          self::VAR_DESCRIPTION => 'The CQL search string, used for extended and complex search options.',
        ),
        self::SHOW_DELETED => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include deleted assets for OAI. The deleted assets are not complete assets, these assets are empty and are only an indication that they are deleted.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include stills in output.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::SHOW_COLLECTIONS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include collection information of each assets, when available.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),

        // Old fasion search params.
        self::OPERATOR => array(
          self::VAR_TYPE => mediamosa_type::TYPE_OPERATOR,
          self::VAR_DESCRIPTION => 'For non-CQL search only; provide either OR or AND for search parameters..',
          self::VAR_DEFAULT_VALUE => 'and',
        )
      )
    );

    // Include limit, offset vars.
    $a_var_setup = self::get_var_setup_range($a_var_setup);

    // Include order by.
    $a_var_setup = self::get_var_setup_order_by($a_var_setup);

    // Include default.
    $a_var_setup = self::get_var_setup_default($a_var_setup);

    // Process what we have.
    $a_var_setup = parent::process_rest_args($a_var_setup);

    // If we return only asset ids, then we differ from limit_max.
    $return_asset_ids = $this->get_param_value(self::RETURN_ASSET_IDS);

    if ($return_asset_ids) {
      $a_var_setup = self::get_var_setup_range($a_var_setup, mediamosa_settings::LIMIT_MAX_IDS_ONLY);
    }

    // Remember, might not be validated.
    $a_app_ids = $this->get_param_value_app();

    // Now include metadata search stuff.
    $a_var_setup = mediamosa_core_cql::get_var_setup_search_asset($a_app_ids, $a_var_setup, self::get_search_params());

    return $a_var_setup;
  }

  // Fix for alias of user_id / aut_user_id.
  protected function process_rest_args(array $a_var_setup) {
    // Process input.
    $a_var_setup = parent::process_rest_args($a_var_setup);

    // If user_id is set, override aut_user_id, dont care if aut_user_id is set.
    $user_id = $this->get_param_value(self::USER_ID);

    if ($user_id != '') {
      $this->set_param_value(self::AUT_USER_ID, $user_id);
    }

    // Return the var_setup.
    return $a_var_setup;
  }

  // Override for checking CQL string.
  protected function validate_rest_args(array $a_var_setup) {

    // Validate first.
    parent::validate_rest_args($a_var_setup);

    $cql = $this->get_param_value(self::CQL);
    $a_app_ids = $this->get_param_value(self::APP_ID);
    $error_text = '';

    if ($cql != '' && !mediamosa_core_cql::verify_asset($cql, $a_app_ids, $error_text)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CQL_ERROR, array('@error' => $error_text));
    }
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Now convert the old search parameters to CQL.
    $a_var_setup = mediamosa_core_cql::get_var_setup_search_asset($a_app_ids, array(), self::get_search_params());

    // Non CQL parameters.
    $order_by = $this->get_param_value(self::ORDER_BY);
    $order_direction = $this->get_param_value(self::ORDER_DIRECTION);

    $a_cql = array();
    $a_translate_to = array();
    foreach ($a_var_setup[mediamosa_rest_call::VARS] as $param => $a_param) {
      if ($a_param[mediamosa_rest_call::VAR_TYPE] == mediamosa_type::TYPE_SEARCH_MATCH) {
        continue;
      }

      // When provided, then convert.
      if (!self::empty_param($param)) {
        $a_value = self::get_param_value($param);
        $match = self::get_param_value($param . '_match');

        $a_cql[] = mediamosa_core_cql::convert_searchparam_to_cql($param, $match, $a_value, $a_param[mediamosa_rest_call::PARAM_TYPE], $a_translate_to);
      }
    }

    $cql = '';
    if (count($a_cql)) {

      // CQL must not be set.
      if (!self::empty_param(self::CQL)) {
        throw new mediamosa_exception_error_cql_exclusive();
      }

      // Get the glue operator.
      $operator = self::get_param_value(self::OPERATOR);

      $cql = implode(' ' . mediamosa_unicode::strtoupper($operator) . ' ', $a_cql);
    }

    // Add possible order by to the CQL.
    if ($order_by != '') {
      if (!self::empty_param(self::CQL)) {
        throw new mediamosa_exception_error_cql_exclusive();
      }

      $cql .= ($cql == '' ? ' ' : '') . 'sortby ' . $order_by;
    }
    else {
      // Now normal search params, then take CQL (if any).
      $cql = self::get_param_value(self::CQL);
    }

    // Now search.
    mediamosa_asset_search::asset_search(
      $o_mediamosa,
      $a_app_ids,
      $cql,
      $this->get_param_value(self::AUT_USER_ID),
      $this->get_param_value(self::AUT_GROUP_ID),
      $this->get_param_value(self::AUT_DOMAIN),
      $this->get_param_value(self::AUT_REALM),
      $this->get_param_value(self::BATCH_ID),
      $this->get_param_value(self::COLL_ID),
      $this->get_param_value(self::FAV_USER_ID),
      $this->get_param_value(self::GRANTED),
      $this->get_param_value(self::IS_PUBLIC_LIST),
      $this->get_param_value(self::IS_APP_ADMIN),
      $this->get_param_value(self::HIDE_EMPTY_ASSETS),
      $this->get_param_value(self::SHOW_DELETED),
      $this->get_param_value(self::RETURN_ASSET_IDS),
      $this->get_param_value(self::SHOW_STILLS),
      $this->get_param_value(self::SHOW_COLLECTIONS),
      $this->get_param_value_limit(),
      $this->get_param_value_offset()
    );
    // @todo;
    //_vpx_statistics_log_search_query($a_args, $app_id);
  }
}
