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
  * Webservice app installer.
  */

require_once 'mediamosa_webservice_app.inc';
require_once 'mediamosa_webservice_app_db.inc';

/**
 * Implement hook_install().
 */
function mediamosa_webservice_app_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_webservice_app_db::TABLE_NAME);
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_webservice_app_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_webservice_app_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 * @return array
 */
function mediamosa_webservice_app_schema() {
  $schema[mediamosa_webservice_app_db::TABLE_NAME] = array(
    'description' => t('The webservice are modules of mediamosa, which can be enabled/disabled per application.'),
    'fields' => array(
      mediamosa_webservice_app_db::ID => array(
        'description' => t('The webservice app ID.'),
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_webservice_app_db::WEBSERVICE_HANDLE => array(
        'description' => t('The handle of the webservice.'),
        'type' => 'varchar',
        'length' => mediamosa_webservice_app_db::WEBSERVICE_HANDLE_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_webservice_app_db::APP_ID => array(
        'description' => t('The APP ID.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_webservice_app_db::STATUS => array(
        'description' => t('The webservice application switch.'),
        'type' => "enum('" . implode("','", array(mediamosa_webservice_app_db::STATUS_FALSE, mediamosa_webservice_app_db::STATUS_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_webservice_app_db::STATUS_FALSE, mediamosa_webservice_app_db::STATUS_TRUE)) . "')",
        'default' => mediamosa_webservice_app_db::STATUS_TRUE,
      ),
    ),
    'primary key' => array(mediamosa_webservice_app_db::ID, mediamosa_webservice_app_db::WEBSERVICE_HANDLE),
  );

  return $schema;
}

