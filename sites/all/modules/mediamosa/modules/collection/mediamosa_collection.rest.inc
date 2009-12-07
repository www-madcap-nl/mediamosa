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
  * The Collection REST calls.
  */

/**
 * URI: /collection/count
 * Method: GET
 *
 * 1.x: media_management_get_collection_search
 */
class mediamosa_rest_call_count_collection extends mediamosa_rest_call {
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
          self::DESCRIPTION => 'The group ID to match.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::DESCRIPTION => 'The owner ID to match.',
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
    $o_mediamosa->item_total_count = mediamosa_db::db_count_items(mediamosa_collection_db::TABLE_NAME, $app_id, $owner_id, $group_id);
  }
}

/**
 * URI: /asset/$asset_id/collection
 * Method: GET
 */
class mediamosa_rest_call_collection_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const FAV_USER_ID = 'fav_user_id';
  const ASSET_ID = 'asset_id';
  const PRIVATE_ = 'private';
  const PUBLIC_ = 'public';
  const CATEGORY = 'category';
  const PUBLIC_ASSIGN = 'public_assign';
  const CQL = 'cql';
  const USER_ID = 'user_id'; // alias of aut_user_id.
  const AUT_USER_ID = 'aut_user_id';
  const IS_PUBLIC_LIST = 'is_public_list';
  const NUMOFVIDEOS = 'numofvideos';
  const OPERATOR = 'operator';

  // ------------------------------------------------------------------ Public Functions.
  static public function get_search_params() {
    return array(
      'title' => mediamosa_type::TYPE_SEARCH_STRING,
      'description' => mediamosa_type::TYPE_SEARCH_STRING,
      'owner_id' => mediamosa_type::TYPE_SEARCH_STRING,
      'group_id' => mediamosa_type::TYPE_SEARCH_STRING,
      'created' => mediamosa_type::TYPE_SEARCH_DATETIME,
      'changed' => mediamosa_type::TYPE_SEARCH_DATETIME,
      'private' => mediamosa_type::TYPE_SEARCH_BOOL,
      'public' => mediamosa_type::TYPE_SEARCH_BOOL,
      'category' => mediamosa_type::TYPE_SEARCH_BOOL,
    );
  }

  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::CQL => array(
          self::VAR_TYPE => mediamosa_type::TYPE_CQL_COLLECTION,
          self::VAR_DESCRIPTION => 'The CQL search string, used for extended and complex search options.',
        ),
        self::FAV_USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Search within the favorites of this supplied user ID.',
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User ID for including the collections that are private.',
        ),
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'Search within the collection of this supplied asset ID.',
        ),
        self::PRIVATE_ => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Search on the private flag status of the collection.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::PUBLIC_ => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Search on the public flag status of the collection.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::CATEGORY => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Search on the category flag status of the collection.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::PUBLIC_ASSIGN => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Search on the public assign flag status of the collection.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_APP_ADMIN => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Allows seeing unappropriate assets.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_PUBLIC_LIST => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'TRUE = include private assets, FALSE = exclude private assets (default),',
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

    // Now include metadata search stuff.
    $a_var_setup = mediamosa_core_cql::get_var_setup_search($a_var_setup, self::get_search_params());

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

    if ($cql != '' && !mediamosa_core_cql::verify_collection($cql, $a_app_ids, $error_text)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CQL_ERROR, array('@error' => $error_text));
    }
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Now convert the old search parameters to CQL.
    $a_var_setup = mediamosa_core_cql::get_var_setup_search(array(), self::get_search_params());

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

    // Build either the CQL from scratch or build from the parameters.
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


    // If order by is set on NUMOFVIDEOS and no value for order_direction was given, then set it to other default.
    if (self::get_param_value_order_by() == self::NUMOFVIDEOS && self::isset_default_param(self::ORDER_DIRECTION)) {
      self::set_param_value(self::ORDER_DIRECTION, mediamosa_db::ORDER_DIRECTION_DESC);
    }

    // Now search.
    $result = mediamosa_collection_search::collection_search(
      $a_app_ids,
      $cql,
      $this->get_param_value(self::ASSET_ID),
      $this->get_param_value(self::FAV_USER_ID),
      $this->get_param_value(self::AUT_USER_ID),
      $this->get_param_value(self::IS_APP_ADMIN),
      $this->get_param_value(self::IS_PUBLIC_LIST),
      $this->get_param_value_limit(),
      $this->get_param_value_offset()
    );

    // Total rows.
    $o_mediamosa->item_total_count = $result->found_rows;

    // Set result.
    foreach ($result as $a_row) {
      $o_mediamosa->add_item($a_row);
    }
  }
}
