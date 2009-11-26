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

require_once 'mediamosa_server_download_db.inc';

/**
 * Implements hook_install().
 */
function mediamosa_server_download_install() {

  // Download server.
  drupal_install_schema(mediamosa_server_download_db::TABLE_NAME);

  // Insert default.
  db_query(
    strtr(
      "INSERT INTO {#mediamosa_server_download} VALUES (1, 2, 'download server devel', :uri, 1)",
      array(
        '#mediamosa_server_download' => mediamosa_server_download_db::TABLE_NAME,
      )
    ),
    array(
      ':uri' => 'http://localhost/download/{TICKET}',
    )
  );
}

/**
 * Implements hook_uninstall().
 */
function mediamosa_server_download_uninstall() {

  // Download server.
  drupal_uninstall_schema(mediamosa_server_download_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_download_schema() {
  $a_schema[mediamosa_server_download_db::TABLE_NAME] = array(
    'description' => t('The download server table contains settings for the download servers.'),
    'fields' => array(
      mediamosa_server_download_db::ID => array(
        'description' => t('Primary ID.'),
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_server_download_db::VERSION => array(
        'description' => t('Version of the server.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_download_db::DESCRIPTION => array(
        'description' => t('The description.'),
        'type' => 'varchar',
        'length' => mediamosa_server_download_db::DESCRIPTION_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_download_db::URI => array(
        'description' => t('URI of the download server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_download_db::URI_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_download_db::ACTIVE => array(
        'description' => t('Active setting.'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_download_db::ID),
  );

  return $a_schema;
}
