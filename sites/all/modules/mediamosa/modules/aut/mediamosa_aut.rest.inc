<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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

  // ------------------------------------------------------------------ Consts.

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    // Enrich with limit and offset vars.
    $a_var_setup = $this->get_var_setup_range();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
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
    $o_mediamosa->item_count_total = $result->found_rows;

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

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The group name to retrieve',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);

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

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const LIMIT = 'limit';
  const OFFSET = 'offset';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    // Enrich with limit and offset vars.
    $a_var_setup = $this->get_var_setup_range();

    $a_var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);

    $offset = $this->get_param_value_offset();
    $limit = $this->get_param_value_limit();

    // Get the group.
    $a_row = mediamosa_aut_group::get_by_group_name($a_app_ids, $group_name);
    if (!$a_row) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_AUT_GROUP_NOT_FOUND, array('@group' => $group_name));
    }

    // Get the group.
    $result = mediamosa_aut_group::get_hostnames_by_group_name($a_app_ids, $group_name, $offset, $limit);

    // Add to result.
    foreach ($result as $a_row) {
      $a_row['hostname'] = $a_row['aut_name'];
      unset($a_row['aut_name']);
      $o_mediamosa->add_item($a_row);
    }
  }
}

/**
 * URI: /autorisation_group/$groupname/hostname/create
 * Method: POST
 */
class mediamosa_rest_call_aut_group_create_hostnames extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const HOSTNAME = 'hostname';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group, which shared the hostname(s).',
    );
    $a_var_setup[mediamosa_rest_call::VARS][self::HOSTNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => "The hostname that the group should be linked.\n\nIn case of multiple hostnames you can using the '[]' notation",
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $a_host_names = $this->get_param_value(self::HOSTNAME);

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

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const HOSTNAME = 'hostname';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group to delete from.',
    );
    $a_var_setup[mediamosa_rest_call::VARS][self::HOSTNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the host to delete.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $a_host_names = $this->get_param_value(self::HOSTNAME);

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

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const GROUP_TYPE = 'group_type';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group to create.',
    );
    $a_var_setup[mediamosa_rest_call::VARS][self::GROUP_TYPE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_AUT_GROUP_TYPE,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The type of the group to create.',
      mediamosa_rest_call::VAR_ALLOWED_VALUES => array(mediamosa_aut_group_db::AUT_GROUP_TYPE_DOMAIN, mediamosa_aut_group_db::AUT_GROUP_TYPE_REALM),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $group_type = $this->get_param_value(self::GROUP_TYPE);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    if (mediamosa_db::db_exists(
      mediamosa_aut_group_db::TABLE_NAME,
      array(
        mediamosa_aut_group_db::APP_ID => $app_id,
        mediamosa_aut_group_db::AUT_GROUP_NAME => $group_name
      )
    )) {
      throw new mediamosa_exception_error_aut_group_already_exists(array('@group' => $group_name));
    }

    // Create group.
    mediamosa_aut_group::create($app_id, $group_name, $group_type);

    // Get created group.
    $a_row = mediamosa_aut_group::get_by_group_name($a_app_ids, $group_name);

    // Add created group.
    $o_mediamosa->add_item($a_row);
  }
}

/**
 * URI: /autorisation_group/$groupname
 * Method: POST
 */
class mediamosa_rest_call_aut_group_update extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const GROUP_TYPE = 'group_type';
  const NEW_GROUPNAME = 'new_groupname';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group to update.',
    );
    $a_var_setup[mediamosa_rest_call::VARS][self::GROUP_TYPE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_AUT_GROUP_TYPE,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The type of the group.',
    );
    $a_var_setup[mediamosa_rest_call::VARS][self::NEW_GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The new name of the group.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $new_group_type = $this->get_param_value(self::GROUP_TYPE);
    $new_group_name = $this->get_param_value(self::NEW_GROUPNAME);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Test of de groep bestaat
    mediamosa_db::db_must_exists(mediamosa_aut_group_db::TABLE_NAME, array(mediamosa_aut_group_db::APP_ID => $app_id, mediamosa_aut_group_db::AUT_GROUP_NAME => $group_name));

    // Fields to update.
    $a_fields = array();

    // If we rename, then the renamed group must not exist
    if ($this->isset_given_param(self::NEW_GROUPNAME) && $new_group_name != '') {
      if (mediamosa_db::db_exists(mediamosa_aut_group_db::TABLE_NAME, array(mediamosa_aut_group_db::APP_ID => $app_id, mediamosa_aut_group_db::AUT_GROUP_NAME => $new_group_name))) {
        throw new mediamosa_exception_error_aut_group_already_exists(array('@group' => $new_group_name));
      }

      $a_fields[mediamosa_aut_group_db::AUT_GROUP_NAME] = $new_group_name;
    }

    // Change of type.
    if ($this->isset_given_param(self::GROUP_TYPE) && $new_group_type != '') {

      $a_aut_group = mediamosa_aut_group::get_by_group_name($a_app_ids, $group_name);
      assert($a_aut_group);

      if (mediamosa_unicode::strtoupper($a_aut_group[mediamosa_aut_group_db::AUT_GROUP_TYPE]) != mediamosa_unicode::strtoupper($new_group_type)) {
        // If we change group type, then the group must be empty.
        if (mediamosa_db::db_count_rows(mediamosa_aut_name_db::TABLE_NAME, array(mediamosa_aut_name_db::AUT_GROUP_ID => $a_aut_group[mediamosa_aut_group_db::ID]))) {
          throw new mediamosa_exception_error_cant_retype_group_must_be_empty(array('@group' => $group_name));
        }

        $a_fields[mediamosa_aut_group_db::AUT_GROUP_TYPE] = $new_group_type;
      }
    }

    // If not empty, we update the row.
    if (!empty($a_fields)) {
      mediamosa_db::db_update(mediamosa_aut_group_db::TABLE_NAME)
        ->fields($a_fields)
        ->condition(mediamosa_aut_group_db::AUT_GROUP_NAME, $group_name)
        ->execute();
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

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const DELETE = 'delete';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group to delete.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::DELETE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_ALPHA,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Provide value "cascade" to delete also contents of group. Will fail if group is not empty and this value is not set to "cascade".',
      mediamosa_rest_call::VAR_ALLOWED_VALUES => array('', 'cascade'),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $delete = $this->get_param_value(self::DELETE);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    mediamosa_db::db_must_exists(mediamosa_aut_group_db::TABLE_NAME, array(mediamosa_aut_group_db::APP_ID => $app_id, mediamosa_aut_group_db::AUT_GROUP_NAME => $group_name));

    // Get the Id of the group.
    $group_id = mediamosa_db::db_query(
      'SELECT aut_group_id FROM {mediamosa_aut_group} WHERE app_id = :app_id AND (aut_group_type IS NULL OR aut_group_type IS NOT NULL) AND aut_group_name = :aut_group_name',
      array(
        ':app_id' => $app_id,
        ':aut_group_name' => $group_name,
      )
    )->fetchField();
    assert($group_id);

    // Only with cascade will all be deleted without checks.
    if ($delete != 'cascade') {
      if (mediamosa_db::db_count_rows(mediamosa_aut_name_db::TABLE_NAME, array(mediamosa_aut_name_db::AUT_GROUP_ID => $group_id))) {
        throw new mediamosa_exception_error_group_not_empty(array('@group' => $group_name));
      }
    }

    // Remove link between aut_object and aut_name/group
    $a_aut_ids = mediamosa_db::db_query(
      'SELECT #aut_name_id FROM {#mediamosa_aut_name} WHERE #aut_group_id = :aut_group_id',
      array(
        '#mediamosa_aut_name' => mediamosa_aut_name_db::TABLE_NAME,
        '#aut_name_id' => mediamosa_aut_name_db::ID,
        '#aut_group_id' => mediamosa_aut_name_db::AUT_GROUP_ID,
        ':aut_group_id' => $group_id
      )
    )->fetchCol();

    // Remove all links between aut_object -> aut_name.
    if (!empty($a_aut_ids)) {
      mediamosa_db::db_query(
        'DELETE FROM {#mediamosa_aut_object} WHERE aut_type = :aut_type_name AND #aut_id IN(:aut_id)',
        array(
          '#mediamosa_aut_object' => mediamosa_aut_object_db::TABLE_NAME,
          '#aut_type' => mediamosa_aut_object_db::AUT_TYPE,
          ':aut_type_name' => mediamosa_aut_object_db::AUT_TYPE_NAME,
          '#aut_id' => mediamosa_aut_object_db::AUT_ID,
          ':aut_id' => $a_aut_ids,
        )
      );
    }

    // Remove all links from aut_object -> aut_group.
    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_aut_object} WHERE #aut_type = :aut_type_group AND #aut_id = :aut_id',
      array(
        '#mediamosa_aut_object' => mediamosa_aut_object_db::TABLE_NAME,
        '#aut_type' => mediamosa_aut_object_db::AUT_TYPE,
        ':aut_type_group' => mediamosa_aut_object_db::AUT_TYPE_GROUP,
        '#aut_id' => mediamosa_aut_object_db::AUT_ID,
        ':aut_id' => $group_id,
      )
    );

    // Remove all from aut_name.
    mediamosa_db::db_query(
      "DELETE FROM {#mediamosa_aut_name} WHERE #aut_group_id = :aut_group_id",
      array(
        '#mediamosa_aut_name' => mediamosa_aut_name_db::TABLE_NAME,
        '#aut_group_id' => mediamosa_aut_name_db::AUT_GROUP_ID,
        ':aut_group_id' => $group_id
      )
    );

    // Remove group.
    mediamosa_db::db_query(
      "DELETE FROM {#mediamosa_aut_group} WHERE #aut_group_id = :aut_group_id",
      array(
        '#mediamosa_aut_group' => mediamosa_aut_group_db::TABLE_NAME,
        '#aut_group_id' => mediamosa_aut_group_db::ID,
        ':aut_group_id' => $group_id
      )
    );

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/$mediafile_id/acl
 * Method: POST
 */
class mediamosa_rest_call_aut_mediafile_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const AUT_APP = 'aut_app';
  const AUT_USER = 'aut_user';
  const AUT_GROUP = 'aut_group';
  const AUT_DOMAIN = 'aut_domain';
  const AUT_REALM = 'aut_realm';
  const AUT_USER_ID = 'aut_user_id';
  const AUT_GROUP_ID = 'aut_group_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::MEDIAFILE_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The mediafile ID of which to change the rights.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Must match the owner fo the mediafile.',
      );

    $a_var_setup[mediamosa_rest_call::VARS][self::REPLACE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_BOOL,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DEFAULT_VALUE => 'TRUE',
      mediamosa_rest_call::VAR_DESCRIPTION => "'TRUE' of 'FALSE' if the current right has to be replaced. Standard is TRUE. Define if 'FALSE' if you want to keep the old rights.",
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_APP] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_INT,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Application(s) that gain access to the mediafile.',
      mediamosa_rest_call::VAR_DEFAULT_VALUE => array(),
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_USER] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'User(s) that gain access to the mediafile.',
      mediamosa_rest_call::VAR_DEFAULT_VALUE => array(),
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_GROUP] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Group(s) that gain access to the mediafile.',
      mediamosa_rest_call::VAR_DEFAULT_VALUE => array(),
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_DOMAIN] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_DOMAIN,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Domain(s) that gain access to the mediafile.',
      mediamosa_rest_call::VAR_DEFAULT_VALUE => array(),
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_REALM] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_REALM,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Realm(s) that gain access to the mediafile.',
      mediamosa_rest_call::VAR_DEFAULT_VALUE => array(),
    );

    // alias.
    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Alias of aut_user.',
      mediamosa_rest_call::VAR_DEFAULT_VALUE => array(),
    );

    // alias.
    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_GROUP_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Alias of aut_group.',
      mediamosa_rest_call::VAR_DEFAULT_VALUE => array(),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
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
    mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);

    $a_app_ids = $this->get_param_value('aut_app');
    $a_aut_user_ids = $this->get_param_value('aut_user');
    $a_aut_group_ids = $this->get_param_value('aut_group');
    $a_aut_domains = $this->get_param_value('aut_domain');
    $a_aut_realms = $this->get_param_value('aut_realm');
    $b_replace = $this->get_param_value('replace');

    // Get the media file.
    $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id, $app_id);

    // Fix for difference in aut_user and aut_user_id
    if (!count($a_aut_user_ids)) {
      // Hmmm no a_aut_user_ids, then try aut_user_id and set user_id if we find one
      $a_aut_user_ids = $this->get_param_value('aut_user_id');
      $this->set_param_value('aut_user', $a_aut_user_ids);
    }
    else {
      $this->set_param_value('aut_user_id', $a_aut_user_ids);
    }

    if (!count($a_aut_group_ids)) {
      // Hmmm no a_aut_group_ids, then try aut_group_id and set group_id if we find one
      $a_aut_group_ids = $this->get_param_value('aut_group_id');
      $this->set_param_value('aut_group', $a_aut_group_ids);
    }
    else {
      $this->set_param_value('aut_group_id', $a_aut_group_ids);
    }

    // Set access rights
    $a_result_rights_set = mediamosa_aut::rights_set($app_id, $user_id, mediamosa_aut::AUT_TYPE_MEDIAFILE, $a_mediafile, $a_app_ids, $a_aut_user_ids, $a_aut_group_ids, $a_aut_domains, $a_aut_realms, $b_replace);

    $a_types = array(
      mediamosa_aut::AUT_NAME_TYPE_APP => 'aut_app',
      mediamosa_aut::AUT_NAME_TYPE_USER => 'aut_user',
      mediamosa_aut::AUT_NAME_TYPE_USER_GROUP => 'aut_group',
      mediamosa_aut::AUT_NAME_TYPE_DOMAIN => 'aut_domain',
      mediamosa_aut::AUT_NAME_TYPE_REALM => 'aut_realm',
    );

    foreach ($a_result_rights_set as $a_error) {
      $type = (isset($a_types[$a_error['type']]) ? $a_types[$a_error['type']] : 'unknown');

      if ($a_error['a_error'] === FALSE) {
        $o_mediamosa->add_item(
          array(
            $type => array(
              'value' => $a_error['value'],
              'result' => mediamosa_response::SUCCESS,
              'result_id' => mediamosa_error::ERRORCODE_OKAY,
              'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY),
            )
          )
        );
      }
      else {
        $o_mediamosa->add_item(
          array(
            $type => array(
              'value' => $a_error['value'],
              'result' => mediamosa_response::ERROR,
              'result_id' => $a_error['a_error']['code'],
              'result_description' => $a_error['a_error']['message'],
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
class mediamosa_rest_call_aut_mediafile_delete_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::MEDIAFILE_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The mediafile ID where from to remove the ACL rights',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The owner of the mediafile.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists.
    mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);

    // Get the media file
    $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id, $app_id);

    // Remove rights.
    mediamosa_aut::rights_clear($app_id, $user_id, mediamosa_aut::AUT_TYPE_MEDIAFILE, $a_mediafile);

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/$mediafile_id/acl
 * Method: GET
 */
class mediamosa_rest_call_aut_mediafile_get_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::MEDIAFILE_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_MEDIAFILE_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The mediafile ID from which to receive the ACL rights',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Must be the owner of the mediafile object to alter rights.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
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
    mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));

    // Get the media file
    // Removed the app_id so the mediafile can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id);

    // Get rights.
    $a_rights = mediamosa_aut::rights_get($app_id, $user_id, mediamosa_aut::AUT_TYPE_MEDIAFILE, $a_mediafile, $is_app_admin);

    // Fill response.
    if (isset($a_rights[mediamosa_aut::OBJ_LINK_TYPE_NAME])) {
      foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_NAME] as $a_right) {
        $a_right['type'] = ($a_right['type'] == 'aut_user_group' ? 'aut_group' : $a_right['type']);
        $o_mediamosa->add_item(array($a_right['type'] => $a_right['name']));
      }
    }

    if (isset($a_rights[mediamosa_aut::OBJ_LINK_TYPE_GROUP])) {
      foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_GROUP] as $a_right) {
        $o_mediamosa->add_item(array($a_right['type'] => $a_right['name']));
      }
    }

    if (isset($a_rights[mediamosa_aut::OBJ_LINK_TYPE_APP])) {
      foreach ($a_rights[mediamosa_aut::OBJ_LINK_TYPE_APP] as $a_right) {
        $o_mediamosa->add_item(array($a_right['type'] => $a_right['app_id_master']));
      }
    }
  }
}

/**
 * URI: asset/$asset_id/acl
 * Method: POST
 */
class mediamosa_rest_call_aut_asset_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const AUT_APP = 'aut_app';
  const AUT_USER = 'aut_user';
  const AUT_GROUP = 'aut_group';
  const AUT_DOMAIN = 'aut_domain';
  const AUT_REALM = 'aut_realm';
  const AUT_USER_ID = 'aut_user_id';
  const AUT_GROUP_ID = 'aut_group_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::ASSET_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The asset_id.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'User ID, owner of the asset.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::REPLACE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_BOOL,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DEFAULT_VALUE => 'TRUE',
      mediamosa_rest_call::VAR_DESCRIPTION => '\'TRUE\' or \'FALSE\'. Is the rest call overwrite the existing rules? Default is TRUE. Defined as \'FALSE\' means the old rules will not be overwritten.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_APP] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_INT,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Application(s) that get acces to the asset.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_USER] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'User(s) that get acces to the asset.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_GROUP] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Group(s) that get acces to the asset.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_DOMAIN] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_DOMAIN,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Domain(s) that get acces to the asset.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_REALM] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_REALM,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Realm(s) that get acces to the asset.',
    );

    // alias.
    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Alias of aut_user. If both are set, aut_user has priority.',
    );

    // alias.
    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_GROUP_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Alias of aut_group. If both are set, aut_group has priority.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the asset exists
    $a_asset = mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::APP_ID => $app_id, mediamosa_asset_db::ID => $asset_id));

    $a_aut_app_ids = $this->get_param_value(self::AUT_APP);
    $a_aut_user_ids = $this->get_param_value(self::AUT_USER);
    $a_aut_group_ids = $this->get_param_value(self::AUT_GROUP);
    $a_aut_domains = $this->get_param_value(self::AUT_DOMAIN);
    $a_aut_realms = $this->get_param_value(self::AUT_REALM);
    $b_replace = $this->get_param_value(self::REPLACE);

    // Fix for difference in aut_user and aut_user_id
    if (!count($a_aut_user_ids)) {
      // Hmmm no a_aut_user_ids, then try aut_user_id and set user_id if we find one
      $a_aut_user_ids = $this->get_param_value(self::AUT_USER_ID);
      $this->set_param_value(self::AUT_USER, $a_aut_user_ids);
    }
    else {
      $this->set_param_value(self::AUT_USER_ID, $a_aut_user_ids);
    }

    if (!count($a_aut_group_ids)) {
      // Hmmm no a_aut_group_ids, then try aut_group_id and set group_id if we find one
      $a_aut_group_ids = $this->get_param_value(self::AUT_GROUP_ID);
      $this->set_param_value(self::AUT_GROUP, $a_aut_group_ids);
    }
    else {
      $this->set_param_value(self::AUT_GROUP_ID, $a_aut_group_ids);
    }

    // Set access rights
    $a_result_rights_set = mediamosa_aut::rights_set($app_id, $user_id, mediamosa_aut::AUT_TYPE_ASSET, $a_asset, $a_aut_app_ids, $a_aut_user_ids, $a_aut_group_ids, $a_aut_domains, $a_aut_realms, $b_replace);

    $a_types = array(
      mediamosa_aut::AUT_NAME_TYPE_APP => 'aut_app',
      mediamosa_aut::AUT_NAME_TYPE_USER => 'aut_user',
      mediamosa_aut::AUT_NAME_TYPE_USER_GROUP => 'aut_group',
      mediamosa_aut::AUT_NAME_TYPE_DOMAIN => 'aut_domain',
      mediamosa_aut::AUT_NAME_TYPE_REALM => 'aut_realm',
    );

    foreach ($a_result_rights_set as $a_error) {
      $type = (isset($a_types[$a_error['type']]) ? $a_types[$a_error['type']] : 'unknown');

      if ($a_error['a_error'] === FALSE) {
        $o_mediamosa->add_item(
          array(
            $type => array(
              'value' => $a_error['value'],
              'result' => mediamosa_response::SUCCESS,
              'result_id' => mediamosa_error::ERRORCODE_OKAY,
              'result_description' => '',
            )
          )
        );
      }
      else {
        $o_mediamosa->add_item(
          array(
            $type => array(
              'value' => $a_error['value'],
              'result' => mediamosa_response::ERROR,
              'result_id' => $a_error['a_error']['code'],
              'result_description' => $a_error['a_error']['message'],
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

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::ASSET_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The asset ID where from to remove the ACL rights',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The owner of the asset for access.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the asset exists.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Get the media file.
    $a_asset = mediamosa_asset::get($asset_id, $app_id);

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

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::ASSET_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_ASSET_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The ID of the asset to set the rights on.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'This must be the owner ID of the asset.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the asset exists.
    // Removed the app_id so the asset can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $a_asset = mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Get the rights on asset.
    $a_rights = mediamosa_aut::rights_get($app_id, $user_id, mediamosa_aut::AUT_TYPE_ASSET, $a_asset, $is_app_admin);

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

    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /collection/$coll_id/acl/delete
 * Method: POST
 */
class mediamosa_rest_call_aut_collection_delete_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::COLL_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_COLLECTION_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Collection id, unique identifier of the collection.',

    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'User id, to determine whether the applicant has the right to remove authorization or not.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $coll_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists.
    mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::APP_ID => $app_id, mediamosa_collection_db::ID => $coll_id));

    // Get the media file
    $a_collection = mediamosa_collection::get($coll_id, $app_id);

    // Remove rights.
    mediamosa_aut::rights_clear($app_id, $user_id, mediamosa_aut::AUT_TYPE_COLLECTION, $a_collection);

    // All ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /collection/$coll_id/acl
 * Method: GET
 */
class mediamosa_rest_call_aut_collection_get_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::COLL_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_COLLECTION_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The coll id.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The user id.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $collection_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the collection exists.
    // Removed the app_id so the collection can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $a_collection = mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $collection_id));

    // Get the rights.
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
 * URI: collection/$coll_id/acl
 * Method: POST
 */
class mediamosa_rest_call_aut_collection_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const AUT_APP = 'aut_app';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][self::COLL_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_COLLECTION_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Collection ID',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::USER_ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'User id, for authorization the changing  process.',
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::REPLACE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_BOOL,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DEFAULT_VALUE => 'true',
      mediamosa_rest_call::VAR_DESCRIPTION => "'TRUE' or 'FALSE'. Whether the existing rules overwritten or not. Default is 'TRUE'. Defining it as 'FALSE', if you want to keep the old rights."
    );

    $a_var_setup[mediamosa_rest_call::VARS][self::AUT_APP] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_APP_ID,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Application(s) that access to the collection.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app id(s).
    $a_app_ids = $this->get_param_value_app();
    $collection_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // First is always the main app.
    $app_id = reset($a_app_ids);

    // Make sure the media file exists
    $a_collection = mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $collection_id));

    $a_aut_app_ids = $this->get_param_value(self::AUT_APP);
    $b_replace = $this->get_param_value(self::REPLACE);

    // Set access rights
    $a_result_rights_set = mediamosa_aut::rights_set(
      $app_id,
      $user_id,
      mediamosa_aut::AUT_TYPE_COLLECTION,
      $a_collection,
      $a_aut_app_ids,
      array(),
      array(),
      array(),
      array(),
      $b_replace,
      array(mediamosa_aut::RIGHT_ACCESS),
      $is_app_admin
    );

    $a_types = array(
      mediamosa_aut::AUT_NAME_TYPE_APP => 'aut_app',
    );

    foreach ($a_result_rights_set as $a_error) {
      $type = (isset($a_types[$a_error['type']]) ? $a_types[$a_error['type']] : 'unknown');

      if ($a_error['a_error'] === FALSE) {
        $o_mediamosa->add_item(
          array(
            $type => array(
              'value' => $a_error['value'],
              'result' => mediamosa_response::SUCCESS,
              'result_id' => mediamosa_error::ERRORCODE_OKAY,
              'result_description' => '',
            )
          )
        );
      }
      else {
        $o_mediamosa->add_item(
          array(
            $type => array(
              'value' => $a_error['value'],
              'result' => mediamosa_response::ERROR,
              'result_id' => $a_error['a_error']['code'],
              'result_description' => $a_error['a_error']['message'],
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
