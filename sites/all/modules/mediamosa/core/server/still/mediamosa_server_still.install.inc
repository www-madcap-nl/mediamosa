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
 * Implementation of hook_install().
 */
function mediamosa_server_still_install() {

  // Download server.
  drupal_install_schema(mediamosa_server_still_db::TABLE_NAME);

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
 * Implementation of hook_uninstall().
 */
function mediamosa_server_still_uninstall() {

  // Download server.
  drupal_uninstall_schema('mediamosa_still_download');
}

/**
 * Implementation of hook_schema().
 */
function mediamosa_server_still_schema() {

  $a_schema[mediamosa_server_still_db::TABLE_NAME] = array(
    'description' => t('TODO: please describe this table!'),
    'fields' => array(
      'still_server_id' => array(
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
      'uri' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'varchar',
        'length' => '255',
        'not null' => TRUE,
      ),
      'active' => array(
        'description' => t('TODO: please describe this field!'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'disp-width' => '1',
      ),
    ),
    'primary key' => array('still_server_id'),
  );

  return $a_schema;
}
