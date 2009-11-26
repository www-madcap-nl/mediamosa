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
    $a_my_vars = array();

    $a_my_vars = array(
      self::VARS => array(
        self::OLD_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::DESCRIPTION => 'The current group ID to replace.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::NEW_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_GROUP_ID,
          self::DESCRIPTION => 'The new group ID to replace with.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::OLD_OWNER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::DESCRIPTION => 'The current owner to replace.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::NEW_OWNER_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_USER_ID,
          self::DESCRIPTION => 'The new owner to replace with.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
        ),
        self::OLD_APP_ID => array( // Unused.
          self::VAR_TYPE => mediamosa_type::TYPE_APP_ID,
          self::DESCRIPTION => 'The current app to replace.',
        ),
        self::NEW_APP_ID => array( // Unused.
          self::VAR_TYPE => mediamosa_type::TYPE_APP_ID,
          self::DESCRIPTION => 'The new app to replace with.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
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
    $a_my_vars = array();

    $a_my_vars = array(
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
    return self::get_var_setup_default($a_my_vars);
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
