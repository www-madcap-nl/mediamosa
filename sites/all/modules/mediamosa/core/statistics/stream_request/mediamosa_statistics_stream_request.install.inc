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
 * Install for statistics used diskspace.
 */

require_once 'mediamosa_statistics_stream_request_db.inc';
require_once 'collection/mediamosa_statistics_stream_request_collection.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_statistics_stream_request_install() {

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_statistics_stream_request_db::TABLE_NAME);

  // Child
  _mediamosa_statistics_stream_request_collection_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_statistics_stream_request_uninstall() {
  // Drop our table(s).

  // Child
  _mediamosa_statistics_stream_request_collection_uninstall();

  mediamosa::drupal_uninstall_schema(mediamosa_statistics_stream_request_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_statistics_stream_request_schema() {

  $a_schema[mediamosa_statistics_stream_request_db::TABLE_NAME] = array(
  'description' => t('The statistics stream request table.'),
    'fields' => array(
      mediamosa_statistics_stream_request_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_statistics_stream_request_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The application ID.'),
      ),
      mediamosa_statistics_stream_request_db::ASSET_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => t('The parent asset ID.'),
      ),
      mediamosa_statistics_stream_request_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => t('The parent mediafile ID.'),
      ),
      mediamosa_statistics_stream_request_db::OWNER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::OWNER_ID_LENGTH,
        'not null' => FALSE,
        'description' => t('The owner ID.'),
      ),
      mediamosa_statistics_stream_request_db::GROUP_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::GROUP_ID_LENGTH,
        'not null' => FALSE,
        'description' => t('The group ID.'),
      ),
      mediamosa_statistics_stream_request_db::FILESIZE => array(
        'type' => 'int',
        'size' => 'big',
        'not null' => FALSE,
        'unsigned' => FALSE,
        'description' => t('The file size of the request.'),
      ),
      mediamosa_statistics_stream_request_db::CONTAINER_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_statistics_stream_request_db::CONTAINER_TYPE_LENGTH,
        'not null' => FALSE,
        'description' => t('The container type of the request.'),
      ),
      mediamosa_statistics_stream_request_db::PLAY_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_statistics_stream_request_db::PLAY_TYPE_LENGTH,
        'not null' => TRUE,
        'description' => t('The container type of the request.'),
      ),
      mediamosa_statistics_stream_request_db::PLAYED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => t('The time and date of the request.'),
      ),
    ),
    'primary key' => array(mediamosa_statistics_stream_request_db::ID),
    'indexes' => array(
      'idx_mediafileid'  => array(mediamosa_statistics_stream_request_db::MEDIAFILE_ID),
      'idx_assetid'  => array(mediamosa_statistics_stream_request_db::ASSET_ID),
    ),
  );

  return $a_schema;
}
