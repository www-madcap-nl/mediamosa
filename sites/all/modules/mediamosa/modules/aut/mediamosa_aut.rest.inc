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
 * URI: /autorisation_group
 * Method: GET
 *
 * @param array $a_args
 * @return object
 */
class mediamosa_rest_call_aut_group_list extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    // Enrich with limit and offset vars.
    $a_my_vars = $this->set_var_setup_range();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $offset = $this->get_param_value_offset();
    $limit = $this->get_param_value_limit();

    // Get the list.
    $result = mediamosa_aut_group::get_list($a_app_ids, $offset, $limit);

    // Total rows?
    $o_mediamosa->item_total_count = $result->found_rows;

    // Set result.
    foreach ($result as $a_row) {
      $o_mediamosa->add_item($a_row);
    }
  }
}

/**
 * URI: /autorisation_group/$group_name
 * Method: GET
 */
class mediamosa_rest_call_aut_group_get extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['groupname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value('groupname');

    // Get the list.
    $a_row = mediamosa_aut_group::get_by_group_name($a_app_ids, $group_name);
    if (!$a_row) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_AUT_GROUP_NOT_FOUND, array('@group' => $group_name));
    }

    // Add to result.
    $o_mediamosa->add_item($a_row);
  }
}

/**
 *  URI: /autorisation_group/$groupname/hostname
 *  Method: GET
 */
class mediamosa_rest_call_aut_group_get_hostnames extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['groupname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value('groupname');

    // Get the group.
    $a_row = mediamosa_aut_group::get_by_group_name($a_app_ids, $group_name);
    if (!$a_row) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_AUT_GROUP_NOT_FOUND, array('@group' => $group_name));
    }

    // Get the group.
    $result = mediamosa_aut_group::get_hostnames_by_group_name($a_app_ids, $group_name);

    // Add to result.
    foreach ($result as $a_row) {
      $a_row["hostname"]  = $a_row["aut_name"];
      unset($a_row["aut_name"]);
      $o_mediamosa->add_item($a_row);
    }
  }
}

/**
 * URI: /autorisation_group/$groupname/hostname/create
 * Method: POST
 */
class mediamosa_rest_call_aut_group_create_hostnames extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['groupname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );
    $a_my_vars[mediamosa_rest_call::VARS]['hostname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value('groupname');
    $a_host_names = $this->get_param_value('hostname');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Get the group.
    $a_result = mediamosa_aut_group::create_hostnames_for_group($app_id, $group_name, $a_host_names);

    foreach ($a_result as $a_row) {
      $o_mediamosa->add_item($a_row);
    }
  }
}

/**
 * URI: /autorisation_group/$groupname/hostname/delete
 * Method: POST
 */
class mediamosa_rest_call_aut_group_delete_hostnames extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['groupname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );
    $a_my_vars[mediamosa_rest_call::VARS]['hostname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value('groupname');
    $a_host_names = $this->get_param_value('hostname');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    $a_result = mediamosa_aut_group::delete_hostname_for_group($app_id, $group_name, $a_host_names);

    foreach ($a_result as $a_row) {
      $o_mediamosa->add_item($a_row);
    }
  }
}

/**
 * URI: /autorisation_group/create
 * Method: POST
 */
class mediamosa_rest_call_aut_group_create extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['groupname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );
    $a_my_vars[mediamosa_rest_call::VARS]['group_type'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_AUT_GROUP_TYPE,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value('groupname');
    $group_type = $this->get_param_value('group_type');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    if (mediamosa_db::db_exists(
      mediamosa_aut_group_db::TABLE_NAME,
      array(
        mediamosa_aut_group_db::APP_ID => $app_id,
        mediamosa_aut_group_db::AUT_GROUP_NAME => $group_name
      )
    )) {
      throw new mediamosa_exception_error_aut_group_already_exists(array("@group" => $group_name));
    }

    // Create group.
    mediamosa_aut_group::create($app_id, $group_name, $group_type);

    // Get created group.
    $a_row = mediamosa_aut_group::get_by_group_name($group_name);

    // Add created group.
    $o_mediamosa->add_item($a_row);
  }
}

/**
 * URI: /autorisation_group/$groupname
 * Method: POST
 */
class mediamosa_rest_call_aut_group_update extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['groupname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );
    $a_my_vars[mediamosa_rest_call::VARS]['group_type'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_AUT_GROUP_TYPE,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );
    $a_my_vars[mediamosa_rest_call::VARS]['new_groupname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value('groupname');
    $new_group_type = $this->get_param_value('group_type');
    $new_group_name = $this->get_param_value('new_groupname');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Test of de groep bestaat
    mediamosa_db::db_must_exists(mediamosa_aut_group_db::TABLE_NAME, array(mediamosa_aut_group_db::APP_ID => $app_id, mediamosa_aut_group_db::AUT_GROUP_NAME => $group_name));

    $a_set = array();

    // If we rename, then the renamed group must not exist
    if ($this->isset_param('new_groupname') && $new_group_name != '') {
      if (mediamosa_db::db_exists(mediamosa_aut_group_db::TABLE_NAME, array(mediamosa_aut_group_db::APP_ID => $app_id, mediamosa_aut_group_db::AUT_GROUP_NAME => $new_group_name))) {
        throw new mediamosa_exception_error_aut_group_already_exists(array('@group' => $new_group_name));
      }

      $a_set[] = sprintf("aut_group_name = '%s'", $new_group_name);
    }

    // Change of type.
    if ($this->isset_param('group_type') && $new_group_type != '') {

      $a_aut_group = mediamosa_db::db_query("SELECT * FROM {mediamosa_aut_group} WHERE aut_group_name = '%s'", $group_name)->fetchAssoc();
      assert($a_aut_group);

      if (strtoupper($a_aut_group['aut_group_type']) != strtoupper($new_group_type)) {
        // If we change group type, then the group must be empty.
        if (mediamosa_db::db_count_rows('mediamosa_aut_name', array('aut_group_id' => $a_aut_group['aut_group_id']))) {
          throw new mediamosa_exception_error_cant_retype_group_must_be_empty(array('@group' => $group_name));
        }

        $a_set[] = sprintf("aut_group_type='%s'", $new_group_type);
      }
    }

    if (count($a_set)) {
      mediamosa_db::db_query(sprintf("UPDATE {aut_group} SET %s WHERE aut_group_name='%s'", implode(",", $a_set), mediamosa_db::escape_string($group_name)));
    }

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /autorisation_group/$groupname/delete
 * Method: POST
 */
class mediamosa_rest_call_aut_group_delete extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['groupname'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['delete'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_DELETE,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value('groupname');
    $delete = $this->get_param_value('delete');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    mediamosa_db::db_must_exists(mediamosa_aut_group_db::TABLE_NAME, array(mediamosa_aut_group_db::APP_ID => $app_id, mediamosa_aut_group_db::AUT_GROUP_NAME => $group_name));

    // Get the Id of the group.
    $group_id = db_query("SELECT aut_group_id FROM {mediamosa_aut_group} WHERE app_id=%d AND (aut_group_type IS NULL OR aut_group_type IS NOT NULL) AND aut_group_name='%s'", $app_id, $group_name)->fetchField();
    assert($group_id);

    // Only with cascade will all be deleted without checks.
    if ($delete != 'cascade') {
      if (mediamosa_db::db_count_rows(mediamosa_aut_name_db::TABLE_NAME, array(mediamosa_aut_name_db::AUT_GROUP_ID => $group_id))) {
        throw new mediamosa_exception_error_group_not_empty(array('@group' => $group_name));
      }
    }

    // Remove link between aut_object and aut_name/group
    $result = mediamosa_db::db_query("SELECT aut_name_id FROM {mediamosa_aut_name} WHERE aut_group_id = %d", $group_id);
    $a_ids = array();
    foreach ($result as $dbrow) {
      $a_ids[] = $dbrow['aut_name_id'];
    }

    // Verwijder alle koppelingen tussen aut_object -> aut_name.
    if (count($a_ids)) {
      mediamosa_db::db_query("DELETE FROM {mediamosa_aut_object} WHERE aut_type='NAME' AND aut_id IN(%s)", implode(',', $a_ids));
    }

    // Remove all links from aut_object -> aut_group.
    mediamosa_db::db_query("DELETE FROM {mediamosa_aut_object} WHERE aut_type='GROUP' AND aut_id = %d", $group_id);

    // Remove all from aut_name.
    mediamosa_db::db_query("DELETE FROM {mediamosa_aut_name} WHERE aut_group_id = %d", $group_id);

    // Verwijder group.
    mediamosa_db::db_query("DELETE FROM {mediamosa_aut_group} WHERE aut_group_id = %d", $group_id);

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/$mediafile_id/acl
 * Method: POST
 */
class mediamosa_rest_call_aut_medium_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['mediafile_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_INT,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['replace'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_BOOL,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DEFAULT_VALUE => 'TRUE',
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_app'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_INT,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_user'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_group'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_domain'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_DOMAIN,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_realm'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_REALM,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    // alias.
    $a_my_vars[mediamosa_rest_call::VARS]['aut_user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    // alias.
    $a_my_vars[mediamosa_rest_call::VARS]['aut_group_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $mediafile_id = $this->get_param_value('mediafile_id');
    $user_id = $this->get_param_value('user_id');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists.
    mediamosa_db::db_must_exists(mediamosa_asset_medium_db::TABLE_NAME, array('app_id' => $app_id, 'medium_id' => $mediafile_id));

    $a_app_ids = $this->get_param_value('aut_app');
    $a_aut_user_ids = $this->get_param_value('aut_user');
    $a_aut_group_ids = $this->get_param_value('aut_group');
    $a_aut_domains = $this->get_param_value('aut_domain');
    $a_aut_realms = $this->get_param_value('aut_realm');
    $b_replace = $this->get_param_value('replace');

    // Get the media file
    $a_medium = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_asset_medium} WHERE #app_id = :app_id AND #medium_id = :medium_id",
      array(
        '#mediamosa_asset_medium' => mediamosa_asset_medium_db::TABLE_NAME,
        '#app_id' => mediamosa_asset_medium_db::APP_ID,
        '#medium_id' => mediamosa_asset_medium_db::ID,
        ':app_id' => $app_id,
        ':medium_id' => $mediafile_id
      )
    );


    // Fix for difference in aut_user and aut_user_id
    if (!count($a_aut_user_ids)) {
      // Hmmm no a_aut_user_ids, then try aut_user_id and set user_id if we find one
      $a_aut_user_ids = $this->get_param_value('aut_user_id');
      $o_mediamosa->set_param_value('aut_user', $a_aut_user_ids);
    }
    else {
      $o_mediamosa->set_param_value('aut_user_id', $a_aut_user_ids);
    }

    if (!count($a_aut_group_ids)) {
      // Hmmm no a_aut_group_ids, then try aut_group_id and set group_id if we find one
      $a_aut_group_ids = $this->get_param_value('aut_group_id');
      $o_mediamosa->set_param_value('aut_group', $a_aut_group_ids);
    }
    else {
      $o_mediamosa->set_param_value('aut_group_id', $a_aut_group_ids);
    }

    // Set access rights
    $a_result_rights_set = mediamosa_aut::rights_set($app_id, $user_id, mediamosa_aut::AUT_TYPE_MEDIUM, $a_medium, $a_app_ids, $a_aut_user_ids, $a_aut_group_ids, $a_aut_domains, $a_aut_realms, $b_replace);

    $a_types = array(
      mediamosa_aut::AUT_NAME_TYPE_APP => "aut_app",
      mediamosa_aut::AUT_NAME_TYPE_USER => "aut_user",
      mediamosa_aut::AUT_NAME_TYPE_USER_GROUP => "aut_group",
      mediamosa_aut::AUT_NAME_TYPE_DOMAIN => "aut_domain",
      mediamosa_aut::AUT_NAME_TYPE_REALM => "aut_realm",
    );

    foreach ($a_result_rights_set as $a_error) {
      $type = (isset($a_types[$a_error['type']]) ? $a_types[$a_error['type']] : "unknown");

      if ($a_error['a_error'] === FALSE) {
        $o_mediamosa->add_item(
          array(
            $type => array(
              "value" => $a_error["value"],
              "result" => mediamosa_response::SUCCESS,
              "result_id" => mediamosa_error::ERRORCODE_OKAY,
              "result_description" => "",
            )
          )
        );
      }
      else {
        $o_mediamosa->add_item(
          array(
            $type => array(
              "value" => $a_error["value"],
              "result" => $a_error['a_error']['status'],
              "result_id" => $a_error['a_error']['id'],
              "result_description" => $a_error['a_error']['description'],
            )
          )
        );
      }
    }
  }
}

/**
 * URI: /mediafile/$mediafile_id/acl/delete
 * Method: POST
 */
class mediamosa_rest_call_aut_medium_delete_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['mediafile_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_INT,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $mediafile_id = $this->get_param_value('mediafile_id');
    $user_id = $this->get_param_value('user_id');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists.
    mediamosa_db::db_must_exists(mediamosa_asset_medium_db::TABLE_NAME, array('app_id' => $app_id, 'medium_id' => $mediafile_id));

    // Get the media file
    $a_medium = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_asset_medium} WHERE #app_id = :app_id AND #medium_id = :medium_id",
      array(
        '#mediamosa_asset_medium' => mediamosa_asset_medium_db::TABLE_NAME,
        '#app_id' => mediamosa_asset_medium_db::APP_ID,
        '#medium_id' => mediamosa_asset_medium_db::ID,
        ':app_id' => $app_id,
        ':medium_id' => $mediafile_id
      )
    );

    mediamosa_aut::rights_clear($app_id, $user_id, mediamosa_aut::AUT_TYPE_MEDIUM, $a_medium);

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/$mediafile_id/acl
 * Method: GET
 */
class mediamosa_rest_call_aut_medium_get_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['mediafile_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_INT,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $mediafile_id = $this->get_param_value('mediafile_id');
    $user_id = $this->get_param_value('user_id');
    $is_app_admin = $this->get_param_value('is_app_admin');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists.
    // Removed the app_id so the mediafile can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    mediamosa_db::db_must_exists(mediamosa_asset_medium_db::TABLE_NAME, array(mediamosa_asset_medium_db::ID => $mediafile_id));

    // Get the media file
    // Removed the app_id so the mediafile can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $a_medium = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_asset_medium} WHERE #medium_id = :medium_id",
      array(
        '#mediamosa_asset_medium' => mediamosa_asset_medium_db::TABLE_NAME,
        '#medium_id' => mediamosa_asset_medium_db::ID,
        ':medium_id' => $mediafile_id
      )
    );

    $a_rights = mediamosa_aut::rights_get($app_id, $user_id, mediamosa_aut::AUT_TYPE_MEDIUM, $a_medium, $is_app_admin);

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_NAME] as $a_right) {
      $a_right['type'] = ($a_right['type'] == 'aut_user_group' ? 'aut_group' : $a_right['type']);
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['name']));
    }

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_GROUP] as $a_right) {
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['name']));
    }

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_APP] as $a_right) {
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['app_id_master']));
    }
  }
}

/**
 * URI: asset/$asset_id/acl
 * Method: POST
 */
class mediamosa_rest_call_aut_asset_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['asset_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['replace'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_BOOL,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DEFAULT_VALUE => 'TRUE',
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_app'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_INT,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_user'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_group'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_domain'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_DOMAIN,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['aut_realm'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_REALM,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    // alias.
    $a_my_vars[mediamosa_rest_call::VARS]['aut_user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    // alias.
    $a_my_vars[mediamosa_rest_call::VARS]['aut_group_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value('asset_id');
    $user_id = $this->get_param_value('user_id');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::APP_ID => $app_id, mediamosa_asset_db::ID => $asset_id));

    $a_app_ids = $this->get_param_value('aut_app');
    $a_aut_user_ids = $this->get_param_value('aut_user');
    $a_aut_group_ids = $this->get_param_value('aut_group');
    $a_aut_domains = $this->get_param_value('aut_domain');
    $a_aut_realms = $this->get_param_value('aut_realm');
    $b_replace = $this->get_param_value('replace');

    // Fix for difference in aut_user and aut_user_id
    if (!count($a_aut_user_ids)) {
      // Hmmm no a_aut_user_ids, then try aut_user_id and set user_id if we find one
      $a_aut_user_ids = $this->get_param_value('aut_user_id');
      $o_mediamosa->set_param_value('aut_user', $a_aut_user_ids);
    }
    else {
      $o_mediamosa->set_param_value('aut_user_id', $a_aut_user_ids);
    }

    if (!count($a_aut_group_ids)) {
      // Hmmm no a_aut_group_ids, then try aut_group_id and set group_id if we find one
      $a_aut_group_ids = $this->get_param_value('aut_group_id');
      $o_mediamosa->set_param_value('aut_group', $a_aut_group_ids);
    }
    else {
      $o_mediamosa->set_param_value('aut_group_id', $a_aut_group_ids);
    }

    // Get the asset
    $a_asset = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_asset} WHERE #app_id = :app_id AND #asset_id = :asset_id",
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#app_id' => mediamosa_asset_db::APP_ID,
        '#asset_id' => mediamosa_asset_db::ID,
        ':app_id' => $app_id,
        ':asset_id' => $asset_id
      )
    );

    // Set access rights
    $a_result_rights_set = mediamosa_aut::rights_set($app_id, $user_id, mediamosa_aut::AUT_TYPE_ASSET, $a_asset, $a_app_ids, $a_aut_user_ids, $a_aut_group_ids, $a_aut_domains, $a_aut_realms, $b_replace);

    $a_types = array(
      mediamosa_aut::AUT_NAME_TYPE_APP => "aut_app",
      mediamosa_aut::AUT_NAME_TYPE_USER => "aut_user",
      mediamosa_aut::AUT_NAME_TYPE_USER_GROUP => "aut_group",
      mediamosa_aut::AUT_NAME_TYPE_DOMAIN => "aut_domain",
      mediamosa_aut::AUT_NAME_TYPE_REALM => "aut_realm",
    );

    foreach ($a_result_rights_set as $a_error) {
      $type = (isset($a_types[$a_error['type']]) ? $a_types[$a_error['type']] : "unknown");

      if ($a_error['a_error'] === FALSE) {
        $o_mediamosa->add_item(
          array(
            $type => array(
              "value" => $a_error["value"],
              "result" => mediamosa_response::SUCCESS,
              "result_id" => mediamosa_error::ERRORCODE_OKAY,
              "result_description" => "",
            )
          )
        );
      }
      else {
        $o_mediamosa->add_item(
          array(
            $type => array(
              "value" => $a_error["value"],
              "result" => $a_error['a_error']['status'],
              "result_id" => $a_error['a_error']['id'],
              "result_description" => $a_error['a_error']['description'],
            )
          )
        );
      }
    }
  }
}

/**
 * URI: /asset/$asset_id/acl/delete
 * Method: POST
 */
class mediamosa_rest_call_aut_asset_delete_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['asset_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value('asset_id');
    $user_id = $this->get_param_value('user_id');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::APP_ID => $app_id, mediamosa_asset_db::ID => $asset_id));

    // Get the media file
    $a_asset = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_asset} WHERE #app_id = :app_id AND #asset_id = :asset_id",
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#app_id' => mediamosa_asset_db::APP_ID,
        '#asset_id' => mediamosa_asset_db::ID,
        ':app_id' => $app_id,
        ':asset_id' => $asset_id
      )
    );

    // Remove rights.
    mediamosa_aut::rights_clear($app_id, $user_id, mediamosa_aut::AUT_TYPE_ASSET, $a_asset);

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id/acl
 * Method: GET
 */
class mediamosa_rest_call_aut_asset_get_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['asset_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value('asset_id');
    $user_id = $this->get_param_value('user_id');
    $is_app_admin = $this->get_param_value('is_app_admin');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists.
    // Removed the app_id so the asset can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::APP_ID => $app_id, mediamosa_asset_db::ID => $asset_id));

    // Get the media file
    // Removed the app_id so the asset can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $a_asset = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_asset} WHERE #asset_id = :assetm_id",
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    );

    $a_rights = mediamosa_aut::rights_get($app_id, $user_id, mediamosa_aut::AUT_TYPE_MEDIUM, $a_asset, $is_app_admin);

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_NAME] as $a_right) {
      $a_right['type'] = ($a_right['type'] == 'aut_user_group' ? 'aut_group' : $a_right['type']);
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['name']));
    }

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_GROUP] as $a_right) {
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['name']));
    }

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_APP] as $a_right) {
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['app_id_master']));
    }
  }
}

/**
 * URI: /collection/$collection_id/acl/delete
 * Method: POST
 */
class mediamosa_rest_call_aut_collection_delete_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['coll_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_COLLECTION_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $collection_id = $this->get_param_value('coll_id');
    $user_id = $this->get_param_value('user_id');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists.
    mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::APP_ID => $app_id, mediamosa_collection_db::ID => $collection_id));

    // Get the media file
    $a_collection = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_collection} WHERE #app_id = :app_id AND #collection_id = :collection_id",
      array(
        '#mediamosa_collection' => mediamosa_collection_db::TABLE_NAME,
        '#app_id' => mediamosa_collection_db::APP_ID,
        '#collection_id' => mediamosa_collection_db::ID,
        ':app_id' => $app_id,
        ':collection_id' => $collection_id
      )
    );

    // Remove rights.
    mediamosa_aut::rights_clear($app_id, $user_id, mediamosa_aut::AUT_TYPE_COLLECTION, $a_collection);

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /collection/$collection_id/acl
 * Method: GET
 */
class mediamosa_rest_call_aut_collection_get_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['coll_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_COLLECTION_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $collection_id = $this->get_param_value('coll_id');
    $user_id = $this->get_param_value('user_id');
    $is_app_admin = $this->get_param_value('is_app_admin');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the collection exists.
    // Removed the app_id so the collection can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $collection_id));

    // Get the media file
    // Removed the app_id so the collection can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $a_collection = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_collection} WHERE #collection_id = :collection_id",
      array(
        '#mediamosa_collection' => mediamosa_collection_db::TABLE_NAME,
        '#collection_id' => mediamosa_collection_db::ID,
        ':collection_id' => $collection_id
      )
    );

    $a_rights = mediamosa_aut::rights_get($app_id, $user_id, mediamosa_aut::AUT_TYPE_COLLECTION, $a_collection, $is_app_admin);

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_NAME] as $a_right) {
      $a_right['type'] = ($a_right['type'] == 'aut_user_group' ? 'aut_group' : $a_right['type']);
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['name']));
    }

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_GROUP] as $a_right) {
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['name']));
    }

    foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_APP] as $a_right) {
      $o_mediamosa->add_item(array($a_right['type'] => $a_right['app_id_master']));
    }
  }
}


/**
 * URI: collection/$collection_id/acl
 * Method: POST
 */
class mediamosa_rest_call_aut_collection_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_my_vars = array();

    $a_my_vars[mediamosa_rest_call::VARS]['coll_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_COLLECTION_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['user_id'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    $a_my_vars[mediamosa_rest_call::VARS]['replace'] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_BOOL,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DEFAULT_VALUE => 'TRUE',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $collection_id = $this->get_param_value('coll_id');
    $user_id = $this->get_param_value('user_id');
    $is_app_admin = $this->get_param_value('is_app_admin');

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists
    mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $collection_id));

    $a_app_ids = $this->get_param_value('aut_app');
    $b_replace = $this->get_param_value('replace');

    // Get the collection
    $a_collection = mediamosa_db::db_query(
      "SELECT * FROM {#mediamosa_collection} WHERE #collection_id = :collection_id",
      array(
        '#mediamosa_collection' => mediamosa_collection_db::TABLE_NAME,
        '#collection_id' => mediamosa_collection_db::ID,
        ':collection_id' => $collection_id
      )
    );

    // Set access rights
    $a_result_rights_set = mediamosa_aut::rights_set(
      $app_id,
      $user_id,
      VPX_ACL_AUT_TYPE_COLLECTION,
      $a_collection,
      $a_app_ids,
      array(),
      array(),
      array(),
      array(),
      $b_replace,
      array(VPX_ACL_RIGHT_ACCESS),
      $is_app_admin
    );

    $a_types = array(
      mediamosa_aut::AUT_NAME_TYPE_APP => 'aut_app',
    );

    foreach ($a_result_rights_set as $a_error) {
      $type = (isset($a_types[$a_error['type']]) ? $a_types[$a_error['type']] : "unknown");

      if ($a_error['a_error'] === FALSE) {
        $o_mediamosa->add_item(
          array(
            $type => array(
              "value" => $a_error["value"],
              "result" => mediamosa_response::SUCCESS,
              "result_id" => mediamosa_error::ERRORCODE_OKAY,
              "result_description" => "",
            )
          )
        );
      }
      else {
        $o_mediamosa->add_item(
          array(
            $type => array(
              "value" => $a_error["value"],
              "result" => $a_error['a_error']['status'],
              "result_id" => $a_error['a_error']['id'],
              "result_description" => $a_error['a_error']['description'],
            )
          )
        );
      }
    }
  }
}

/*
@todo:


function vpx_acl_rest_owner_check($a_args) { // TODO: werkt nu alleen voor mediafiles...

  try {
    vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'mediafile_id', VPX_TYPE_MEDIAFILE_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'user_id', VPX_TYPE_USER_ID, TRUE);
    vpx_funcparam_add($a_funcparam, $a_args, 'group_id', VPX_TYPE_GROUP_ID);

    $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');
    $mediafile_id = vpx_funcparam_get_value($a_funcparam, 'mediafile_id');

    // Make sure the media file exists
    mediamosa_db::db_must_exists("mediafile", array("app_id" => $app_id, "mediafile_id" => $mediafile_id));

    $user_id = vpx_funcparam_get_value($a_funcparam, 'user_id');

    // controleer de acl
    db_set_active('data');
    $db_result = mediamosa_db::db_query("SELECT app_id, owner_id FROM {mediafile} where mediafile_id='%s' ", $mediafile_id);
    $a_row = db_fetch_array($db_result);
    assert($a_row);

    $asset_app_id = $a_row['app_id'];
    $asset_owner  = $a_row['owner_id'];
    db_set_active();

    vpx_acl_owner_check($app_id, $user_id, $asset_app_id, $asset_owner);

    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  catch (vpx_exception $e) {
    return $e->vpx_exception_rest_response();
  }
}

  /**
 * Wrapper REST call om in 1 keer alle mediafiles van een asset van ACL te voorzien
 */
  /*
  function vpx_acl_asset_mediafile_wrapper($a_args) {
    $a_parameters = array(
      'asset_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'asset_id'),
        'type' => 'skip',
        'required' => TRUE
      ),
      'app_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
        'type' => 'int',
        'required' => TRUE
      ),
      'user_id' => array(
        'value' => vpx_get_parameter_2($a_args['get'], 'user_id'),
        'type' => TYPE_USER_ID,
        'required' => TRUE
      ),
    );
    $result = vpx_validate($a_parameters);
    if (vpx_check_result_for_error($result)) {
      return new rest_response($result);
    }

    if (!is_array($a_parameters['asset_id']['value'])) {
      $a_parameters['asset_id']['value'] = array($a_parameters['asset_id']['value']);
    }

    $o_mediamosa = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    foreach ($a_parameters['asset_id']['value'] as $asset_id) {
      $asset = array(
        'asset_id' => array(
        'value' => $asset_id,
        'type' => 'alphanum',
        'required' => TRUE
        ),
      );
      $result_object = vpx_validate($asset);
      if (vpx_check_result_for_error($result_object)) {
        $o_mediamosa->add_item(array(
        'asset_id' => $asset_id,
        'result' => $result_object->response['header']['request_result'],
        'result_id' => $result_object->response['header']['request_result_id'],
        'result_description' => $result_object->response['header']['request_result_description'],
        ));
      }
      else {
        // zoek alle mediafile op van deze asset
        db_set_active('data');
        $resource = mediamosa_db::db_query(
        "SELECT m.mediafile_id FROM {asset} a
        JOIN {mediafile} m ON a.asset_id = m.asset_id
        WHERE a.asset_id = '%s' OR a.parent_id = '%s'",
        $asset_id,
        $asset_id
        );
        db_set_active();

        while ($mediafile_id = db_result($resource)) {
          $a_args['uri']['mediafile_id'] = $mediafile_id;
          $result_object = vpx_acl_mediafile_set_rights($a_args);
          $o_mediamosa->add_result_item(array(
          'asset_id' => $asset_id,
          'mediafile_id' => $mediafile_id,
          'result' => $result_object->response['header']['request_result'],
          'result_id' => $result_object->response['header']['request_result_id'],
          'result_description' => $result_object->response['header']['request_result_description'],
          ));
        }
      }
    }

    return $o_mediamosa;
  }


  /**
 * REST call alle voor jouw beschikbare 'foreign' applicaties op te halen
 */
  /*
  function vpx_acl_get_foreign_apps($a_args) {
    $a_parameters = array(
    'app_id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'app_id'),
      'type' => 'int',
      'required' => TRUE
      ),
    'secret' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'secret'),
      'type' => 'enum',
      'enum_values' => array('34yher8hio34jtksmdf823r'),
      'required' => TRUE
      ),
    'is_oai' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'is_oai'),
      'type' => 'bool',
      ),
    );
    $result = vpx_validate($a_parameters);
    if (vpx_check_result_for_error($result)) {
      return new rest_response($result);
    }

    $o_mediamosa = new rest_response(vpx_return_error(ERRORCODE_OKAY));

    $resource = mediamosa_db::db_query(
    "SELECT id AS app_id, name, play_proxy_url, view_asset_url, download_url
    FROM {client_applications}
    WHERE id <> %d OR %d
    ORDER BY name ASC",
    $a_parameters['app_id']['value'], ($a_parameters['is_oai']['value'] == 'TRUE' ? 1 : 0)
    );
    while ($app = db_fetch_array($resource)) {
      $o_mediamosa->add_item($app);
    }

    return $o_mediamosa;
  }
*/
