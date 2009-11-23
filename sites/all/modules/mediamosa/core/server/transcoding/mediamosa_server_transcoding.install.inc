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
 * Implementation of hook_install().
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
 * Implementation of hook_uninstall().
 */
function mediamosa_server_transcoding_uninstall() {

  // Download server.
  drupal_uninstall_schema(mediamosa_server_transcoding_db::TABLE_NAME);

  // Our child.
  mediamosa_server_transcoding_tool_uninstall();
}

/**
 * Implementation of hook_schema().
 */
function mediamosa_server_transcoding_schema() {

  $a_schema[mediamosa_server_transcoding_db::TABLE_NAME] = array(
    'description' => t('TODO: please describe this table!'),
    'fields' => array(
      'transcoding_server_id' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'disp-width' => '11',
      ),
      'version' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'disp-width' => '11',
      ),
      'description' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'varchar',
        'length' => '255',
        'not null' => TRUE,
      ),
      'servername' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'varchar',
        'length' => '255',
        'not null' => TRUE,
      ),
      'ip_address' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'varchar',
        'length' => '15',
        'not null' => TRUE,
      ),
      'slots' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'disp-width' => '1',
      ),
      'is_test_server' => array(
        'description' => t('Is test server switch.'),
        'type' => "enum('" . implode("','", array(mediamosa_server_transcoding_db::IS_TEST_SERVER_FALSE, mediamosa_server_transcoding_db::IS_TEST_SERVER_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_server_transcoding_db::IS_TEST_SERVER_FALSE, mediamosa_server_transcoding_db::IS_TEST_SERVER_TRUE)) . "')",
      ),
      'status' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => "enum('" . implode("','", array(mediamosa_server_transcoding_db::STATUS_ON, mediamosa_server_transcoding_db::STATUS_OFF, mediamosa_server_transcoding_db::STATUS_CLOSE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_server_transcoding_db::STATUS_ON, mediamosa_server_transcoding_db::STATUS_OFF, mediamosa_server_transcoding_db::STATUS_CLOSE)) . "')",
      ),

      'tools' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'varchar',
        'length' => '255',
        'not null' => TRUE,
      ),
    ),
    'primary key' => array('transcoding_server_id'),
    'indexes' => array(
      'name' => array('description'),
    ),
  );
  return $a_schema;
}
