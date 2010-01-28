<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
  * The installation file of the asset metadata module.
  */

require_once 'mediamosa_asset_metadata.inc';
require_once 'mediamosa_asset_metadata_db.inc';
require_once 'property/mediamosa_asset_metadata_property_db.inc';
require_once 'property/mediamosa_asset_metadata_property.install.inc';

/**
 * Implement hook_install().
 */
function mediamosa_asset_metadata_install() {
  // Create the asset metadata property table first.
  mediamosa_asset_metadata_property_install();

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_metadata_db::TABLE_NAME);
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_asset_metadata_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_metadata_db::TABLE_NAME);

  // Drop the asset metadata property table.
  mediamosa_asset_metadata_property_uninstall();
}

/**
 * Implement hook_schema().
 */
function mediamosa_asset_metadata_schema() {

  $a_schema[mediamosa_asset_metadata_db::TABLE_NAME] = array(
  'description' => t('The asset metadata table.'),
    'fields' => array(
      mediamosa_asset_metadata_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_asset_metadata_db::ASSET_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => t('The asset ID.'),
      ),
      mediamosa_asset_metadata_db::PROP_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => t('The property ID of the metadata.'),
        'unsigned' => TRUE,
      ),
      mediamosa_asset_metadata_db::VAL_CHAR => array(
        'type' => 'text',
        'size' => 'big',
        'description' => t('The value of the property stored as an string.'),
      ),
      mediamosa_asset_metadata_db::VAL_CHAR_LFT => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_metadata_db::VAL_CHAR_LFT_LENGTH,
        'description' => t('The first 50 string characters of the property value.'),
      ),
      mediamosa_asset_metadata_db::VAL_CHAR_RGHT => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_metadata_db::VAL_CHAR_RGHT_LENGTH,
        'description' => t('The last 50 string characters of the property value in reverse order.'),
      ),
      mediamosa_asset_metadata_db::VAL_DATETIME => array(
        'type' => 'datetime',
        'description' => t('The date and time of the property value.'),
      ),
      mediamosa_asset_metadata_db::VAL_INT => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => t('The numeric value of the property.'),
        'default' => 0,
      ),
      mediamosa_asset_metadata_db::CREATED => array(
        'type' => 'datetime',
        'not null' => TRUE,
        'description' => t('The date and time when asset metadata was created.'),
      ),
      mediamosa_asset_metadata_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when asset metadata was changed.'),
      ),
    ),
    'primary key' => array(mediamosa_asset_metadata_db::ID),
    'indexes' => array(
      'idx_assetid_propid_valcharlft' => array(mediamosa_asset_metadata_db::ASSET_ID, mediamosa_asset_metadata_db::PROP_ID, mediamosa_asset_metadata_db::VAL_CHAR_LFT),
      'idx_assetid_propid_valcharrght' => array(mediamosa_asset_metadata_db::ASSET_ID, mediamosa_asset_metadata_db::PROP_ID, mediamosa_asset_metadata_db::VAL_CHAR_RGHT),
    ),
    'foreign keys' => array(
      mediamosa_asset_metadata_db::ASSET_ID => array(mediamosa_asset_db::TABLE_NAME => mediamosa_asset_db::ID),
      mediamosa_asset_metadata_db::PROP_ID => array(mediamosa_asset_metadata_property_db::TABLE_NAME => mediamosa_asset_metadata_property_db::ID),
    ),
  );

  return $a_schema;
}
