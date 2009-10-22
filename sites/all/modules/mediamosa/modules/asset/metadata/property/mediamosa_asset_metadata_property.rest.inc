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
  * The Asset Metadata Property REST calls.
  */

/**
 * URI: /metadata_tag (1.x)
 * Get a listing of the current asset media metadata defin
 */
class mediamosa_rest_call_asset_metadata_property_list extends mediamosa_rest_call {

  // Switch if we need to get all the default definitions that are 'global' over all apps.
  const INCLUDE_DEFAULT_DEFINITIONS = 'include_default_definitions';

  // Search on specific name.
  const NAME = 'name';

  public function get_var_setup() {
    $a_my_vars = array(
      self::VARS => array(
        self::INCLUDE_DEFAULT_DEFINITIONS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DEFAULT_VALUE => 'true',
          self::DESCRIPTION => "Switch if we need to get all the default definitions that are 'global' over all apps.",
        ),
        self::NAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ALPHA_NUM_UNDERSCORE,
          self::DESCRIPTION => 'Optional property name to search on.',
        ),
      ),
    );

    // Enrich with default REST vars limit / offset.
    $a_my_vars = self::set_var_setup_range($a_my_vars);

    // Add order by stuff.
    $a_my_vars = self::set_var_setup_order_by(
      $a_my_vars,
      '', // No default.
      array(
        mediamosa_asset_metadata_property_db::ID,
        mediamosa_asset_metadata_property_db::NAME,
        mediamosa_asset_metadata_property_db::TYPE,
        mediamosa_asset_metadata_property_db::CREATED,
        mediamosa_asset_metadata_property_db::CHANGED,
      ),
      mediamosa_type::ORDER_DIRECTION_ASC
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // limit, offset.
    $limit = $this->get_param_value(self::LIMIT);
    $offset = $this->get_param_value(self::OFFSET);

    // Order by, order direction.
    $order_by = $this->get_param_value(self::ORDER_BY);
    $order_direction = $this->get_param_value(self::ORDER_DIRECTION);

    // Include all default properties?
    $include_default_definitions = $this->get_param_value(self::INCLUDE_DEFAULT_DEFINITIONS);

    // Name to search on.
    $name = $this->get_param_value(self::NAME);

    // Get all properties.
    $a_properties = mediamosa_asset_metadata_property::property_list(
      $a_app_ids,
      $include_default_definitions,
      $name,
      $offset,
      $limit,
      $order_by,
      $order_direction
    );

    foreach ($a_properties as $a_property) {
      $o_mediamosa->add_item(
        array(
          'prop_id' => $a_property[mediamosa_asset_metadata_property_db::ID],
          'name' => $a_property[mediamosa_asset_metadata_property_db::NAME],
          'prop_group' => $a_property[mediamosa_asset_metadata_property_group_db::NAME],
          'type' => $a_property[mediamosa_asset_metadata_property_db::TYPE],
          'created' => $a_property[mediamosa_asset_metadata_property_db::CREATED],
          'changed' => $a_property[mediamosa_asset_metadata_property_db::CHANGED],
        )
      );
    }
  }
}


/**
 * URI: /metadata_tag/create (1.x)
 * Create a asset metadata property.
 */
class mediamosa_rest_call_asset_metadata_property_create extends mediamosa_rest_call {

  // Enter property name.
  const NAME = 'name';

  // Type.
  const TYPE = 'type';

  public function get_var_setup() {
    $a_my_vars = array(
      mediamosa_rest_call::VARS => array(
        self::NAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ALPHA_NUM_UNDERSCORE_TAG,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::DESCRIPTION => 'The name of the property to create.',
          self::VAR_RANGE_END => mediamosa_asset_metadata_property_db::NAME_LENGTH,
        ),
        self::TYPE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_ALLOWED_VALUES => array('datetime', 'int', 'char'),
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::DESCRIPTION => "The type of data to be stored, can either be 'datetime' (an iso-8601 standard date), 'int' (numbers), char (text).",
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Name of the new property.
    $name = $this->get_param_value(self::NAME);

    // Type of the new property.
    $type = $this->get_param_value(self::TYPE);

    // Create the property. Any problems will throw exception.
    $property_id = mediamosa_asset_metadata_property::property_create($a_app_ids, $name, $type);

    // Get the property.
    $a_property = mediamosa_asset_metadata_property::property_get($property_id);

    assert($a_property);

    // If we get here, it was successful.
    $o_mediamosa->add_item($a_property);
  }
}

/**
 * URI: /metadata_tag/delete (1.x)
 * Delete a asset metadata property.
 */
class mediamosa_rest_call_asset_metadata_property_delete extends mediamosa_rest_call {

  // Enter property name.
  const NAME = 'name';

  // Delete.
  const DELETE = 'delete';

  public function get_var_setup() {
    $a_my_vars = array(
      mediamosa_rest_call::VARS => array(
        self::NAME => array(
          self::VAR_TYPE => mediamosa_type::TYPE_ALPHA_NUM_UNDERSCORE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::DESCRIPTION => 'The name of the property to delete.',
          self::VAR_RANGE_END => mediamosa_asset_metadata_property_db::NAME_LENGTH,
        ),
        self::DELETE => array(
          self::VAR_TYPE => mediamosa_type::TYPE_STRING,
          self::VAR_ALLOWED_VALUES => array('cascade', ''),
          self::DESCRIPTION => "Specify as 'cascade' to delete all metadata as well. Deletion will fail if you try to delete an property that still has metadata.",
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_my_vars);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // The app_id(s).
    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Name of the new property.
    $name = $this->get_param_value(self::NAME);

    // Type of the new property.
    $delete = $this->get_param_value(self::DELETE);

    // Create the property. Any problems will throw exception.
    mediamosa_asset_metadata_property::property_delete($a_app_ids, $name, $delete);

    // If we get here, it was successful. We have no output, but we are ok.
    $o_mediamosa->set_result_okay();
  }
}
