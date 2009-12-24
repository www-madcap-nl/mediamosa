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

require_once 'mediamosa_server_still_db.inc';

/**
 * Implements hook_install().
 */
function mediamosa_server_still_install() {

  // Download server.
  mediamosa::drupal_install_schema(mediamosa_server_still_db::TABLE_NAME);

  db_query(
    strtr(
      "INSERT INTO {#mediamosa_server_still} VALUES (1, 0, 'still download server', :uri, 0)",
      array(
        '#mediamosa_server_still' => mediamosa_server_still_db::TABLE_NAME,
      )
    ),
    array(
      ':uri' => 'http://localhost/still/{TICKET}',
    )
  );
}

/**
 * Implements hook_uninstall().
 */
function mediamosa_server_still_uninstall() {

  // Download server.
  mediamosa::drupal_uninstall_schema('mediamosa_still_download');
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_still_schema() {

  $a_schema[mediamosa_server_still_db::TABLE_NAME] = array(
    'description' => t('The still server table.'),
    'fields' => array(
      mediamosa_server_still_db::ID => array(
        'description' => t('The primary ID.'),
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_still_db::VERSION => array(
        'description' => t('Version of server'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_still_db::DESCRIPTION => array(
        'description' => t('The description of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_still_db::DESCRIPTION_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_still_db::URI => array(
        'description' => t('The URI of the still server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_still_db::URI_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_still_db::ACTIVE => array(
        'description' => t('Active setting.'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_still_db::ID),
  );

  return $a_schema;
}
