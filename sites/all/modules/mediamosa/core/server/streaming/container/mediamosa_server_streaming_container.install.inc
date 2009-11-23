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
 * Installer for mediamosa_server_streaming_container module.
 */

include 'mediamosa_server_streaming_container_db.inc';

/**
 * Implementation of hook_install().
 */
function mediamosa_server_streaming_container_install() {

  // Download server.
  drupal_install_schema(mediamosa_server_streaming_container_db::TABLE_NAME);

  db_query(
    strtr(
      "INSERT INTO {#mediamosa_server_streaming_container} VALUES
      (143, 2, 'flv'),
      (144, 2, 'mp3'),
      (145, 3, 'mpeg'),
      (146, 3, 'mov;mp4;m4a;3gp;3g2;mj2'),
      (162, 1, 'asf'),
      (181, 5, 'sita'),
      (182, 5, 'duobak'),
      (183, 5, 'brabantia')",
      array(
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME
      )
    )
  );
}

/**
 * Implementation of hook_uninstall().
 */
function mediamosa_server_streaming_container_uninstall() {

  // Download server.
  drupal_uninstall_schema(mediamosa_server_streaming_container_db::TABLE_NAME);
}

/**
 * Implementation of hook_schema().
 */
function mediamosa_server_streaming_container_schema() {

  $a_schema[mediamosa_server_streaming_container_db::TABLE_NAME] = array(
    'description' => t('The streaming server container table.'),
    'fields' => array(
      mediamosa_server_streaming_container_db::ID => array(
        'description' => t('Primary key.'),
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_container_db::STREAMING_SERVER_ID => array(
        'description' => t('The parent server ID.'),
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_container_db::CONTAINER => array(
        'description' => t('The container.'),
        'type' => 'varchar',
        'length' => mediamosa_server_streaming_container_db::CONTAINER_LENGTH,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_streaming_container_db::ID),
  );

  return $a_schema;
}
