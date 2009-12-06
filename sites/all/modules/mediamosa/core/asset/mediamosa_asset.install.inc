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

require_once 'mediamosa_asset.inc';
require_once 'mediamosa_asset_db.inc';
require_once 'mediafile/mediamosa_asset_mediafile.install.inc';
require_once 'metadata/mediamosa_asset_metadata.install.inc';
require_once 'collection/mediamosa_asset_collection.install.inc';
require_once 'delete/mediamosa_asset_delete.install.inc';

/**
 * Implement hook_install().
 */
function mediamosa_asset_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_db::TABLE_NAME);

  // Call installer asset_mediafile.
  mediamosa_asset_mediafile_install();

  // Call installer asset_metadata.
  mediamosa_asset_metadata_install();

  // Call installer asset_collection.
  mediamosa_asset_collection_install();

  // Call installer asset_delete.
  mediamosa_asset_delete_install();
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_asset_uninstall() {
  // Call uninstaller asset_delete.
  mediamosa_asset_delete_install();

  // Call uninstaller asset_collection.
  mediamosa_asset_collection_uninstall();

  // Call uninstaller asset_metadata.
  mediamosa_asset_metadata_uninstall();

  // Call uninstaller asset_mediafile.
  mediamosa_asset_mediafile_uninstall();

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
        'description' => t('The application ID of the asset.'),
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
        'description' => t('The owner ID.'),
      ),
      mediamosa_asset_db::GROUP_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::GROUP_ID_LENGTH,
        'default' => NULL,
        'description' => t('The group ID.'),
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
      mediamosa_asset_db::IS_LOCKED => array(
        'description' => t('The asset can be locked or unlocked.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_LOCKED_FALSE, mediamosa_asset_db::IS_LOCKED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_LOCKED_FALSE, mediamosa_asset_db::IS_LOCKED_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_LOCKED_FALSE,
      ),
      mediamosa_asset_db::REFERENCE_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::REFERENCE_ID_LENGTH,
        'default' => NULL,
        'description' => t('The reference ID.'),
      ),
      mediamosa_asset_db::IS_PRIVATE => array(
        'description' => t('The asset is private.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_PRIVATE_FALSE, mediamosa_asset_db::IS_PRIVATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_PRIVATE_FALSE, mediamosa_asset_db::IS_PRIVATE_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_PRIVATE_FALSE,
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
      mediamosa_asset_db::CREATED => array(
        'type' => 'datetime',
        'not null' => TRUE,
        'description' => t('The date and time when asset was created.'),
      ),
      mediamosa_asset_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when asset was changed last.'),
      ),
      mediamosa_asset_db::IS_UNAPPROPRIATE => array(
        'description' => t('If the asset has unappropieate contents.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_UNAPPROPRIATE_FALSE, mediamosa_asset_db::IS_UNAPPROPRIATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_UNAPPROPRIATE_FALSE, mediamosa_asset_db::IS_UNAPPROPRIATE_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_UNAPPROPRIATE_FALSE,
      ),
      mediamosa_asset_db::IS_EXTERNAL => array(
        'description' => t('If the asset has external contents.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EXTERNAL_FALSE, mediamosa_asset_db::IS_EXTERNAL_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EXTERNAL_FALSE, mediamosa_asset_db::IS_EXTERNAL_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_EXTERNAL_FALSE,
      ),
      mediamosa_asset_db::IS_EMPTY_ASSET => array(
        'description' => t('True if the asset has media.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EMPTY_ASSET_FALSE, mediamosa_asset_db::IS_EMPTY_ASSET_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EMPTY_ASSET_FALSE, mediamosa_asset_db::IS_EMPTY_ASSET_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_EMPTY_ASSET_TRUE,
      ),
      mediamosa_asset_db::VIEWED => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
        'description' => t('Number of times the asset was viewed.'),
      ),
      mediamosa_asset_db::PLAYED => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
        'description' => t('Number of times the asset was played.'),
      ),
      mediamosa_asset_db::IS_PROTECTED => array(
        'description' => t('Set kind of protection on the asset.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_PROTECTED_FALSE, mediamosa_asset_db::IS_PROTECTED_USER_USERGROUP, mediamosa_asset_db::IS_PROTECTED_DOMAIN_REALM, mediamosa_asset_db::IS_PROTECTED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_PROTECTED_FALSE, mediamosa_asset_db::IS_PROTECTED_USER_USERGROUP, mediamosa_asset_db::IS_PROTECTED_DOMAIN_REALM, mediamosa_asset_db::IS_PROTECTED_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_PROTECTED_FALSE,
      ),
    ),
    'primary key' => array(mediamosa_asset_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_asset_db::PARENT_ID => array(mediamosa_asset_db::PARENT_ID),
      'idx_' . mediamosa_asset_db::PROVIDER_ID => array(mediamosa_asset_db::PROVIDER_ID),
      'idx_' . mediamosa_asset_db::OWNER_ID => array(mediamosa_asset_db::OWNER_ID),
      'idx_' . mediamosa_asset_db::GROUP_ID => array(mediamosa_asset_db::GROUP_ID),
      'idx_' . mediamosa_asset_db::CREATED => array(mediamosa_asset_db::CREATED),
      'idx_' . mediamosa_asset_db::CHANGED => array(mediamosa_asset_db::CHANGED),
      'idx_' . mediamosa_asset_db::IS_PROTECTED => array(mediamosa_asset_db::IS_PROTECTED),
      'idx_app_parent_priv_unapp_emptyasset_owner' => array(
        mediamosa_asset_db::APP_ID,
        mediamosa_asset_db::PARENT_ID,
        mediamosa_asset_db::IS_PRIVATE,
        mediamosa_asset_db::IS_UNAPPROPRIATE,
        mediamosa_asset_db::IS_EMPTY_ASSET,
        mediamosa_asset_db::OWNER_ID,
      ),
    ),
    'foreign keys' => array(
      mediamosa_asset_db::PARENT_ID => array(mediamosa_asset_db::TABLE_NAME => mediamosa_asset_db::ID),
    ),
  );

  return $a_schema;
}
