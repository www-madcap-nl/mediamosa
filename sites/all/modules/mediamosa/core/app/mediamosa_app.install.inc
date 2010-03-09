<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
  * Installer for the App module.
  */

require_once 'mediamosa_app.inc';
require_once 'mediamosa_app_db.inc';

/**
 * Implement hook_install().
 */
function _mediamosa_app_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_app_db::TABLE_NAME);
}

/**
 * Implement hook_uninstall().
 */
function _mediamosa_app_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_app_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 * @return array
 */
function mediamosa_app_schema() {
  $schema[mediamosa_app_db::TABLE_NAME] = array(
    'description' => t('The client applications are stored here. Each app is one client application.'),
    'fields' => array(
      mediamosa_app_db::NID => array(
        'description' => t('The Drupal node id.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_app_db::VID => array(
        'description' => t('The Drupal node revision id.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_app_db::NAME => array(
        'description' => t('The name of the client.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::NAME_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_app_db::APP_ID => array(
        'description' => t('The unique application ID.'),
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_app_db::TRANSCODING_POSITION_FROM => array(
        'description' => t('Starting range of allowed transcoding.'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_db::TRANSCODING_POSITION_TO => array(
        'description' => t('Ending range of allowed transcoding.'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_db::QUOTA => array(
        'description' => t('The total allowed quota in gigabytes.'),
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_db::QUOTA_USED => array(
        'description' => t('The current usage in gigabytes.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'size' => 'big',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_db::SHARED_KEY => array(
        'description' => t('The key to identify the client.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::SHARED_KEY_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ACTIVE => array(
        'description' => t('The client can be enabled or disabled.'),
        'type' => "enum('" . implode("','", array(mediamosa_app_db::ACTIVE_FALSE, mediamosa_app_db::ACTIVE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::ACTIVE_FALSE, mediamosa_app_db::ACTIVE_TRUE)) . "')",
        'default' => mediamosa_app_db::ACTIVE_FALSE,
      ),
      mediamosa_app_db::ACTIVE_VERSION => array(
        'description' => t('Use as global versions for REST calls. When specified, use this version or lower of called REST calls.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::ACTIVE_VERSION_LENGTH,
        'not null' => TRUE,
        'default' => 0, // @TODO: which version???
      ),
      mediamosa_app_db::DESCRIPTION => array(
        'description' => t('The description of the client.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::DESCRIPTION_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::OWNER => array(
        'description' => t('De owner of the used drupal rol.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::OWNER_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::TECHNICAL_NAME => array(
        'description' => t('Technical name of the client contact person.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::TECHNICAL_NAME_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::TECHNICAL_PHONE => array(
        'description' => t('Technical phone of the client contact person.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::TECHNICAL_PHONE_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::TECHNICAL_EMAIL => array(
        'description' => t('Technical email address of the client contact person.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::TECHNICAL_EMAIL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ADMINISTRATIVE_NAME => array(
        'description' => t('Administrative name of the client contact person.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::ADMINISTRATIVE_NAME_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ADMINISTRATIVE_PHONE => array(
        'description' => t('Administrative phone of the client contact person.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::ADMINISTRATIVE_PHONE_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ADMINISTRATIVE_EMAIL => array(
        'description' => t('Administrative email of the client contact person.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::ADMINISTRATIVE_EMAIL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::PLAY_PROXY_URL => array(
        'description' => t('The URL to use for playing mediafiles.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::PLAY_PROXY_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::VIEW_ASSET_URL => array(
        'description' => t('The URL to view the mediafiles.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::VIEW_ASSET_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::PREVIEW_PROFILE_ID => array(
        'description' => t('Transcoding profile used for previewing.'),
        'type' => 'int',
        'not null' => FALSE,
      ),
      mediamosa_app_db::DOWNLOAD_URL => array(
        'description' => t('Shown in the asset details rest calls. Available tags: {asset_id}, {mediafile_id} and {mediafile_filename}'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::DOWNLOAD_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::STREAM_URL => array(
        'description' => t('Shown in the asset details rest calls. Available tags: {asset_id}, {mediafile_id} and {mediafile_filename}.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::STREAM_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::STILL_URL => array(
        'description' => t('Shown by OAI for still images. Available tags: {asset_id}.'),
        'type' => 'varchar',
        'length' => mediamosa_app_db::STILL_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ALWAYS_HINT_MP4 => array(
        'description' => t('Always hint (make streamable) mp4 files.'),
        'type' => "enum('" . implode("','", array(mediamosa_app_db::ALWAYS_HINT_MP4_FALSE, mediamosa_app_db::ALWAYS_HINT_MP4_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::ALWAYS_HINT_MP4_FALSE, mediamosa_app_db::ALWAYS_HINT_MP4_TRUE)) . "')",
        'default' => mediamosa_app_db::ALWAYS_HINT_MP4_TRUE,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ALWAYS_INSERT_MD => array(
        'description' => t('Always insert extra metadata flv files'),
        'type' => "enum('" . implode("','", array(mediamosa_app_db::ALWAYS_INSERT_MD_FALSE, mediamosa_app_db::ALWAYS_INSERT_MD_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::ALWAYS_INSERT_MD_FALSE, mediamosa_app_db::ALWAYS_INSERT_MD_TRUE)) . "')",
        'default' => mediamosa_app_db::ALWAYS_INSERT_MD_TRUE,
        'not null' => FALSE,
      ),
      mediamosa_app_db::IS_METADATA_PUBLIC => array(
        'description' => t('Is metadata of this client application public visible in OAI?'),
        'type' => "enum('" . implode("','", array(mediamosa_app_db::IS_METADATA_PUBLIC_FALSE, mediamosa_app_db::IS_METADATA_PUBLIC_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::IS_METADATA_PUBLIC_FALSE, mediamosa_app_db::IS_METADATA_PUBLIC_TRUE)) . "')",
        'default' => mediamosa_app_db::IS_METADATA_PUBLIC_TRUE,
        'not null' => FALSE,
      ),
      mediamosa_app_db::SHOW_EXTERNAL_ASSETS => array(
        'description' => t('Show external references in OAI (in field referenceId)?'),
        'type' => "enum('" . implode("','", array(mediamosa_app_db::SHOW_EXTERNAL_ASSETS_FALSE, mediamosa_app_db::SHOW_EXTERNAL_ASSETS_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::SHOW_EXTERNAL_ASSETS_FALSE, mediamosa_app_db::SHOW_EXTERNAL_ASSETS_TRUE)) . "')",
        'default' => mediamosa_app_db::SHOW_EXTERNAL_ASSETS_TRUE,
        'not null' => FALSE,
      ),
      mediamosa_app_db::TRANSCODING_MAX_SLOTS => array(
        'description' => t('Set the maximum number of transcoding jobs allowed for this application.'),
        'type' => 'int',
        'not null' => TRUE,
        'default' => 5,
      ),
      mediamosa_app_db::ALLOW_MASTERSLAVE => array(
        'description' => t('Control how other clientapplications can access this application.'),
        'type' => "enum('" . implode("','", array(mediamosa_app_db::ALLOW_MASTERSLAVE_FALSE, mediamosa_app_db::ALLOW_MASTERSLAVE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::ALLOW_MASTERSLAVE_FALSE, mediamosa_app_db::ALLOW_MASTERSLAVE_TRUE)) . "')",
        'default' => mediamosa_app_db::ALLOW_MASTERSLAVE_FALSE,
        'not null' => FALSE,
      ),
    ),
    'primary key' => array(mediamosa_app_db::NID, mediamosa_app_db::VID),
    'unique keys' => array(
      'uni_appid' => array(mediamosa_app_db::APP_ID),
    ),
  );

  return $schema;
}
