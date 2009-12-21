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
 * Installer for mediamosa_server_download module.
 */

require_once 'mediamosa_server_db.inc';

/**
 * Implements hook_install().
 */
function mediamosa_server_install() {

  // Server node extensions.
  drupal_install_schema(mediamosa_server_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function mediamosa_server_uninstall() {

  // Server node extensions.
  drupal_uninstall_schema(mediamosa_server_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_schema() {
  $types = array(
    mediamosa_server_db::SERVER_TYPE_DOWNLOAD,
    mediamosa_server_db::SERVER_TYPE_STILL,
    mediamosa_server_db::SERVER_TYPE_STREAMING,
    mediamosa_server_db::SERVER_TYPE_TRANSCODE,
    mediamosa_server_db::SERVER_TYPE_UPLOAD,
  );

  $a_schema[mediamosa_server_db::TABLE_NAME] = array(
    'description' => t('The download server table contains settings for the download servers.'),
    'fields' => array(
      'nid' => array(
        'description' => t('The Drupal node id.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'vid' => array(
        'description' => t('The Drupal node revision id.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_db::SERVER_STATUS => array(
        'description' => t('The server status.'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
      mediamosa_server_db::SERVER_TYPE => array(
        'description' => t('The server type.'),
        'type' => 'enum(' . implode(',', $types) . ')',
        'mysql_type' => 'enum(' . implode(',', $types) . ')',
        'not null' => TRUE,
        'default' => mediamosa_server_db::STATE_NEW,
      ),
      ),
      mediamosa_server_db::URI => array(
        'description' => t('The optional URI of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::URI_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::CONTAINERS => array(
        'description' => t('The optional container names this server can serve.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::CONTAINERS_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::OBJECT_CODE => array(
        'description' => t('The optional container names the server can serve.'),
        'type' => 'text',
        'not null' => FALSE,
      ),
      mediamosa_server_db::SERVER_NAME => array(
        'description' => t('The optional name of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::SERVER_NAME_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::IP_ADDRESS => array(
        'description' => t('The optional ip address of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::IP_ADDRESS_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::SLOTS => array(
        'description' => t('The maximum number of concurrent transcodes.'),
        'type' => 'int',
        'not null' => FALSE,
      ),
      mediamosa_server_db::TOOLS => array(
        'description' => t('The optional tools this server serves.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::TOOLS_LENGTH,
        'not null' => FALSE,
      ),
    ),
    'primary key' => array('nid', 'vid'),
  );

  return $a_schema;
}
