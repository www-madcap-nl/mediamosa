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
  * The installation file of the asset metadata group module.
  */

require_once 'mediamosa_asset_metadata_property_group.inc';
require_once 'mediamosa_asset_metadata_property_group_db.inc';

/**
 * Implement hook_install().
 */
function mediamosa_asset_metadata_property_group_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_metadata_property_group_db::TABLE_NAME);
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_asset_metadata_property_group_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_metadata_property_group_db::TABLE_NAME);
}

/**
 * Implement hook_schema().
 */
function mediamosa_asset_metadata_property_group_schema() {

  $a_schema[mediamosa_asset_metadata_property_group_db::TABLE_NAME] = array(
  'description' => t('The asset metadata property group table.'),
    'fields' => array(
      mediamosa_asset_metadata_property_group_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_asset_metadata_property_group_db::NAME => array(
        'type' => 'varchar',
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
        'length' => mediamosa_asset_metadata_property_group_db::NAME_LENGTH,
      ),
      mediamosa_asset_metadata_property_group_db::CREATED => array(
        'type' => 'datetime',
        'not null' => TRUE,
        'description' => t('The date and time when asset metadata property was created.'),
      ),
      mediamosa_asset_metadata_property_group_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when asset metadata property was changed.'),
      ),
    ),
    'primary key' => array(mediamosa_asset_metadata_property_group_db::ID),
    'unique keys' => array(
      'uni_propgroupname' => array(mediamosa_asset_metadata_property_group_db::NAME),
    ),
  );

  return $a_schema;
}
