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
 * Installer for mediamosa_server_upload module.
 */

require_once 'mediamosa_server_upload_db.inc';

/**
 * Implements hook_install().
 */
function mediamosa_server_upload_install() {

  // Download server.
  mediamosa::drupal_install_schema(mediamosa_server_upload_db::TABLE_NAME);

  // Default.
  db_query(
    strtr(
      "INSERT INTO {#mediamosa_server_upload} VALUES (1, 0, 'Upload server ', :uri, 1, 'http://example.org/mediafile/uploadprogress')",
      array(
        '#mediamosa_server_upload' => mediamosa_server_upload_db::TABLE_NAME
      )
    ),
    array(
      ':uri' => 'http://example.org/vpx/mediafile/upload?upload_ticket={TICKET}',
    )
  );
}

/**
 * Implements hook_uninstall().
 */
function mediamosa_server_upload_uninstall() {

  // Download server.
  mediamosa::drupal_uninstall_schema(mediamosa_server_upload_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_upload_schema() {

  $a_schema[mediamosa_server_upload_db::TABLE_NAME] = array(
    'description' => t('The upload server table.'),
    'fields' => array(
      mediamosa_server_upload_db::ID => array(
        'description' => t('The primary key.'),
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_upload_db::VERSION => array(
        'description' => t('The version of the server.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_upload_db::DESCRIPTION => array(
        'description' => t('The description of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_upload_db::DESCRIPTION_LENGTH,
      ),
      mediamosa_server_upload_db::URI => array(
        'description' => t('The URI of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_upload_db::URI_LENGTH,
      ),
      mediamosa_server_upload_db::STATUS => array(
        'description' => t('The status of the server.'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
      mediamosa_server_upload_db::URI_UPLOADPROGRESS => array(
        'description' => t('Default of the URI uploadprogress trigger page.'),
        'type' => 'varchar',
        'length' => mediamosa_server_upload_db::URI_UPLOADPROGRESS_LENGTH,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_upload_db::ID),
  );

  return $a_schema;
}
