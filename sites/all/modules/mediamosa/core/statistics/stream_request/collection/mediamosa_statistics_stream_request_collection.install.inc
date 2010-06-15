<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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

require_once 'mediamosa_statistics_stream_request_collection_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_statistics_stream_request_collection_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_statistics_stream_request_collection_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_statistics_stream_request_collection_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_statistics_stream_request_collection_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_statistics_stream_request_collection_schema() {

  $a_schema[mediamosa_statistics_stream_request_collection_db::TABLE_NAME] = array(
    'description' => t('The statistics stream request collection table.'),
    'fields' => array(
      mediamosa_statistics_stream_request_collection_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_statistics_stream_request_collection_db::REQUEST_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The parent request ID.'),
      ),
      mediamosa_statistics_stream_request_collection_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The application ID.'),
      ),
      mediamosa_statistics_stream_request_collection_db::COLL_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => t('The owner ID.'),
      ),
      mediamosa_statistics_stream_request_collection_db::OWNER_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => t('The owner ID.'),
      ),
      mediamosa_statistics_stream_request_collection_db::GROUP_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => t('The group ID.'),
      ),
      mediamosa_statistics_stream_request_collection_db::TITLE => array(
        'type' => 'varchar',
        'length' => mediamosa_statistics_stream_request_collection_db::TITLE_LENGTH,
        'default' => NULL,
        'description' => t('The title.'),
      ),
      mediamosa_statistics_stream_request_collection_db::DESCRIPTION => array(
        'type' => 'varchar',
        'length' => mediamosa_statistics_stream_request_collection_db::DESCRIPTION_LENGTH,
        'default' => NULL,
        'description' => t('The description.'),
      ),
    ),
    'primary key' => array(mediamosa_statistics_stream_request_collection_db::ID),
    'indexes' => array(
      'idx_requestid' => array(mediamosa_statistics_stream_request_collection_db::REQUEST_ID),
    ),
  );

  return $a_schema;
}
