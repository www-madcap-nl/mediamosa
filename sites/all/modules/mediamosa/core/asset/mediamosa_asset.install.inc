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
  * The installation file of the asset module.
  */

require_once 'mediamosa_asset_db.inc';
require_once 'mediamosa_asset.inc';

/**
 * Implement hook_install().
 */
function mediamosa_asset_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_db::TABLE_NAME);
}

/**
 * Implement hook_install().
 */
function mediamosa_asset_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_db::TABLE_NAME);
}

/**
 * Implement hook_schema().
 */
function mediamosa_asset_schema() {
  $a_schema[mediamosa_asset_db::TABLE_NAME] = array(
  'description' => t('The asset table.'),
    'fields' => array(
      mediamosa_asset_db::ID => array(
        'type' => 'varchar',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_asset_db::PARENT_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => t('The parent asset ID.'),
      ),
      mediamosa_asset_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The application ID of the asset..'),
      ),
      mediamosa_asset_db::PROVIDER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::PROVIDER_ID_LENGTH,
        'default' => NULL,
        'description' => t('The provider ID.'),
      ),
      mediamosa_asset_db::OWNER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::OWNER_ID_LENGTH,
        'default' => NULL,
        'description' => t('The provider ID.'),
      ),
      mediamosa_asset_db::GROUP_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::GROUP_ID_LENGTH,
        'default' => NULL,
        'description' => t('The provider ID.'),
      ),
      mediamosa_asset_db::VIDEOTIMESTAMP => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The time and date of the asset.'),
      ),
      mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The modified time and date of the asset.'),
      ),
      mediamosa_asset_db::PLAY_RESTRICTION_START => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The start time in an range of time to restrict.'),
      ),
      mediamosa_asset_db::PLAY_RESTRICTION_END => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The end time in an range of time to restrict.'),
      ),
      mediamosa_asset_db::LOCKED => array(
        'description' => t('The asset can be locked or unlocked.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::LOCKED_FALSE, mediamosa_asset_db::LOCKED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::LOCKED_FALSE, mediamosa_asset_db::LOCKED_TRUE)) . "')",
        'default' => mediamosa_asset_db::LOCKED_FALSE,
      ),
      mediamosa_asset_db::REFERENCE_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::REFERENCE_ID_LENGTH,
        'default' => NULL,
        'description' => t('The reference ID.'),
      ),
      mediamosa_asset_db::IS_PRIVATE => array(
        'description' => t('The asset is private.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::PRIVATE_FALSE, mediamosa_asset_db::PRIVATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::PRIVATE_FALSE, mediamosa_asset_db::PRIVATE_TRUE)) . "')",
        'default' => mediamosa_asset_db::PRIVATE_FALSE,
      ),
      mediamosa_asset_db::MEDIAFILE_DURATION => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::MEDIAFILE_DURATION_LENGTH,
        'default' => NULL,
        'description' => t('The mediafile duration length.'),
      ),
      mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE_LENGTH,
        'default' => NULL,
        'description' => t('The container type length.'),
      ),
      mediamosa_asset_db::HAS_STREAMABLE_MEDIAFILES => array(
        'description' => t('Has streamable mediafiles.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::HAS_STREAMABLE_MEDIAFILES_FALSE, mediamosa_asset_db::HAS_STREAMABLE_MEDIAFILES_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::HAS_STREAMABLE_MEDIAFILES_FALSE, mediamosa_asset_db::HAS_STREAMABLE_MEDIAFILES_TRUE)) . "')",
        'default' => mediamosa_asset_db::HAS_STREAMABLE_MEDIAFILES_FALSE,
      ),
      mediamosa_asset_db::CREATED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when asset was created.'),
      ),
      mediamosa_asset_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when asset was changed last.'),
      ),
      mediamosa_asset_db::IS_UNAPPROPIATE => array(
        'description' => t('If the asset has unappropieate contents.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_UNAPPROPIATE_FALSE, mediamosa_asset_db::IS_UNAPPROPIATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_UNAPPROPIATE_FALSE, mediamosa_asset_db::IS_UNAPPROPIATE_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_UNAPPROPIATE_FALSE,
      ),
      mediamosa_asset_db::IS_EXTERNAL => array(
        'description' => t('If the asset has external contents.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EXTERNAL_FALSE, mediamosa_asset_db::IS_EXTERNAL_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EXTERNAL_FALSE, mediamosa_asset_db::IS_EXTERNAL_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_EXTERNAL_FALSE,
      ),


  `viewed` int(11) NOT NULL default '0',
  `played` int(11) NOT NULL default '0',
  `is_empty_asset` enum('TRUE','FALSE') default 'TRUE',
  `is_protected` enum('TRUE','USER_USERGROUP','DOMAIN_REALM','FALSE') NOT NULL default 'FALSE',

      mediamosa_asset_db::MESSAGE => array(
        'type' => 'text',
        'not null' => TRUE,
        'description' => t('The message to display.'),
      ),
    ),
    'primary key' => array(mediamosa_asset_db::ID),
    'unique keys' => array(
      mediamosa_asset_db::NAME => array(mediamosa_asset_db::NAME),
      mediamosa_asset_db::CODE => array(mediamosa_asset_db::CODE),
    ),
  );

  return $a_schema;
}
