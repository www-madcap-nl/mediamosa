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
 * Installer for mediamosa_server_transcoding_tool module.
 */

require_once 'mediamosa_server_transcoding_tool_db.inc';

/**
 * Implements hook_install().
 */
function mediamosa_server_transcoding_tool_install() {

  // Download server.
  drupal_install_schema(mediamosa_server_transcoding_tool_db::TABLE_NAME);

  db_query(
    strtr(
      "INSERT INTO {#mediamosa_server_transcoding_tool} VALUES
      (171, 7, '5'),
      (170, 6, '6'),
      (199, 1, 'ANALYSE'),
      (160, 2, 'ANALYSE'),
      (168, 5, 'ANALYSE'),
      (200, 1, 'ffmpeg'),
      (158, 2, 'ffmpeg'),
      (167, 5, 'ffmpeg'),
      (201, 1, 'STILL'),
      (161, 2, 'STILL'),
      (169, 5, 'STILL'),
      (205, 8, 'test'),
      (206, 8, 'test2'),
      (207, 8, 'tet'),
      (166, 3, 'windows')",
      array(
        '#mediamosa_server_transcoding_tool' => mediamosa_server_transcoding_tool_db::TABLE_NAME
      )
    )
  );
}

/**
 * Implements hook_uninstall().
 */
function mediamosa_server_transcoding_tool_uninstall() {

  // Download server.
  drupal_uninstall_schema(mediamosa_server_transcoding_tool_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_transcoding_tool_schema() {

  $a_schema[mediamosa_server_transcoding_tool_db::TABLE_NAME] = array(
    'description' => t('Contains the tools for transcoding subtable.'),
    'fields' => array(
      mediamosa_server_transcoding_tool_db::ID => array(
        'description' => t('Primary key.'),
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_server_transcoding_tool_db::TRANSCODING_SERVER_ID => array(
        'description' => t('The parent transcoding server ID.'),
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_server_transcoding_tool_db::TOOL => array(
        'description' => t('The tool used for transcoding.'),
        'type' => 'varchar',
        'length' => mediamosa_server_transcoding_tool_db::TOOL_LENGTH,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_transcoding_tool_db::ID),
    'indexes' => array(
      'idx_tool_serverid' => array(mediamosa_server_transcoding_tool_db::TOOL, mediamosa_server_transcoding_tool_db::TRANSCODING_SERVER_ID),
    ),
  );

  return $a_schema;
}
