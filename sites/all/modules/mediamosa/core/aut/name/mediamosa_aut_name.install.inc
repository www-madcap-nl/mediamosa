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
  * Installer for mediamosa_aut_name.
  */

require_once 'mediamosa_aut_name_db.inc';

/**
 * Our sub install.
 */
function _mediamosa_aut_name_install() {
// Create our table.
  mediamosa::drupal_install_schema('mediamosa_aut_name');
}

/**
 * Our sub uninstall.
 */
function _mediamosa_aut_name_uninstall() {
// Create our table.
  mediamosa::drupal_uninstall_schema('mediamosa_aut_name');
}

/**
 * Implement hook_schema().
 */
function mediamosa_aut_name_schema() {
  $a_schema[mediamosa_aut_name_db::TABLE_NAME] = array(
    'description' => t('This table stores named entities on which autorization can be linked.'),
    'fields' => array(
      mediamosa_aut_name_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_aut_name_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The application ID.'),
      ),
      mediamosa_aut_name_db::AUT_GROUP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => FALSE,
        'description' => t('The optional group ID if name is part of group.'),
      ),
      mediamosa_aut_name_db::AUT_NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_aut_name_db::AUT_NAME_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => t('The name of the entity.'),
      ),
      mediamosa_aut_name_db::AUT_PREFIX => array(
        'type' => 'varchar',
        'length' => mediamosa_aut_name_db::AUT_PREFIX_LENGTH,
        'not null' => FALSE,
        'default' => '',
        'description' => t('The optional prefix of the entity, only used with REALM type.'),
      ),
      mediamosa_aut_name_db::AUT_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_aut_name_db::AUT_TYPE_DOMAIN, mediamosa_aut_name_db::AUT_TYPE_REALM, mediamosa_aut_name_db::AUT_TYPE_USER, mediamosa_aut_name_db::AUT_TYPE_USER_GROUP)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_name_db::AUT_TYPE_DOMAIN, mediamosa_aut_name_db::AUT_TYPE_REALM, mediamosa_aut_name_db::AUT_TYPE_USER, mediamosa_aut_name_db::AUT_TYPE_USER_GROUP)) . "')",
        'default' => mediamosa_aut_name_db::AUT_TYPE_DOMAIN,
        'description' => t('The type of name entity.'),
      ),
    ),
    'primary key' => array(mediamosa_aut_name_db::ID),
    'unique keys' => array(
      'uni_app_type_name_prefix' => array(mediamosa_aut_name_db::APP_ID, mediamosa_aut_name_db::AUT_TYPE, mediamosa_aut_name_db::AUT_NAME, mediamosa_aut_name_db::AUT_PREFIX),
    ),
    'indexes' => array(
      'idx_groupid' => array(mediamosa_aut_name_db::AUT_GROUP_ID),
      'idx_app_type_groupid_name_prefix' => array(mediamosa_aut_name_db::APP_ID, mediamosa_aut_name_db::AUT_TYPE, mediamosa_aut_name_db::AUT_GROUP_ID, mediamosa_aut_name_db::AUT_NAME, mediamosa_aut_name_db::AUT_PREFIX),
    ),
  );

  return $a_schema;
}
