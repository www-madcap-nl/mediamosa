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
 * Installer for mediamosa_server_transcoding module.
 */

require_once 'tool/mediamosa_server_transcoding_tool.install.inc';
require_once 'mediamosa_server_transcoding_db.inc';

/**
 * Implements hook_install().
 */
function mediamosa_server_transcoding_install() {

  // Download server.
  drupal_install_schema(mediamosa_server_transcoding_db::TABLE_NAME);

  // Default.
  db_query(
    strtr(
      "INSERT INTO {#mediamosa_server_transcoding} VALUES (1, 0, 'Transcoding server 1 linux', 'localhost', '127.0.0.1', 2, 'FALSE', 'ON', 'ANALYSE|ffmpeg|STILL')",
      array(
        '#mediamosa_server_transcoding' => mediamosa_server_transcoding_db::TABLE_NAME
      )
    )
  );

  // Our child.
  mediamosa_server_transcoding_tool_install();
}

/**
 * Implements hook_uninstall().
 */
function mediamosa_server_transcoding_uninstall() {

  // Download server.
  drupal_uninstall_schema(mediamosa_server_transcoding_db::TABLE_NAME);

  // Our child.
  mediamosa_server_transcoding_tool_uninstall();
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_transcoding_schema() {

  $a_schema[mediamosa_server_transcoding_db::TABLE_NAME] = array(
    'description' => t('The transcoding server information.'),
    'fields' => array(
      mediamosa_server_transcoding_db::ID => array(
        'description' => t('Primary key.'),
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_transcoding_db::VERSION => array(
        'description' => t('Version of the server.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_transcoding_db::DESCRIPTION => array(
        'description' => t('Description of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_transcoding_db::DESCRIPTION_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_transcoding_db::SERVERNAME => array(
        'description' => t('Servername of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_transcoding_db::SERVERNAME_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_transcoding_db::IP_ADDRESS => array(
        'description' => t('IP address of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_transcoding_db::IP_ADDRESS_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_transcoding_db::SLOTS => array(
        'description' => t('Number of slots available.'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
      mediamosa_server_transcoding_db::IS_TEST_SERVER => array(
        'description' => t('Is test server switch.'),
        'type' => "enum('" . implode("','", array(mediamosa_server_transcoding_db::IS_TEST_SERVER_FALSE, mediamosa_server_transcoding_db::IS_TEST_SERVER_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_server_transcoding_db::IS_TEST_SERVER_FALSE, mediamosa_server_transcoding_db::IS_TEST_SERVER_TRUE)) . "')",
      ),
      mediamosa_server_transcoding_db::STATUS => array(
        'description' => t('Status of the server.'),
        'type' => "enum('" . implode("','", array(mediamosa_server_transcoding_db::STATUS_ON, mediamosa_server_transcoding_db::STATUS_OFF, mediamosa_server_transcoding_db::STATUS_CLOSE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_server_transcoding_db::STATUS_ON, mediamosa_server_transcoding_db::STATUS_OFF, mediamosa_server_transcoding_db::STATUS_CLOSE)) . "')",
      ),
      mediamosa_server_transcoding_db::TOOLS => array(
        'description' => t('The tools.'),
        'type' => 'varchar',
        'length' => mediamosa_server_transcoding_db::TOOLS_LENGTH,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_transcoding_db::ID),
  );
  return $a_schema;
}
