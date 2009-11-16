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
  * The installer for ACL.
  */

/**
 * Implement hook_install().
 */
function mediamosa_aut_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema('mediamosa_aut');
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_aut_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema('mediamosa_aut');
}

/**
 * Implement hook_schema().
 */
function mediamosa_aut_schema() {
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
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_name_db::AUT_TYPE_DOMAIN, mediamosa_aut_name_db::AUT_TYPE_REALM, mediamosa_aut_name_db::AUT_TYPE_USER, mediamosa_aut_name_db::AUT_TYPE_USER_GROUP)) . "')",
        'default' => NULL,
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

  $a_schema[mediamosa_aut_group_db::TABLE_NAME] = array(
  'description' => t('This table stores grouped named entities on which autorization can be linked.'),
    'fields' => array(
      mediamosa_aut_group_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_aut_group_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The application ID.'),
      ),
      mediamosa_aut_group_db::AUT_GROUP_NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_aut_group_db::AUT_GROUP_NAME_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => t('The group name.'),
      ),
      mediamosa_aut_group_db::AUT_GROUP_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_aut_group_db::AUT_GROUP_TYPE_REALM, mediamosa_aut_group_db::AUT_GROUP_TYPE_DOMAIN)) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_group_db::AUT_GROUP_TYPE_REALM, mediamosa_aut_group_db::AUT_GROUP_TYPE_DOMAIN)) . "')",
        'default' => NULL,
        'description' => t('The type of group.'),
      ),
    ),
    'primary key' => array(mediamosa_aut_group_db::ID),
    'unique keys' => array(
      'uni_app_type_name' => array(mediamosa_aut_group_db::APP_ID, mediamosa_aut_group_db::AUT_GROUP_TYPE, mediamosa_aut_group_db::AUT_GROUP_NAME),
    ),
  );

  $a_schema[mediamosa_aut_object_db::TABLE_NAME] = array(
  'description' => t('This table stores authorization links between named/group entities to objects.'),
    'fields' => array(
      mediamosa_aut_object_db::AUT_OBJECT_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_aut_object_db::AUT_OBJECT_ID_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => t('The object ID, links to mediamosa_asset_medium, mediamosa_asset or mediamosa_colletion.'),
      ),
      mediamosa_aut_object_db::AUT_OBJECT_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_aut_object_db::AUT_OBJECT_TYPE_MEDIAFILE, mediamosa_aut_object_db::AUT_OBJECT_TYPE_ASSET, mediamosa_aut_object_db::AUT_OBJECT_TYPE_COLLECTION)) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_object_db::AUT_OBJECT_TYPE_MEDIAFILE, mediamosa_aut_object_db::AUT_OBJECT_TYPE_ASSET, mediamosa_aut_object_db::AUT_OBJECT_TYPE_COLLECTION)) . "')",
        'default' => NULL,
        'description' => t('The type of object, links to mediamosa_asset_medium, mediamosa_asset or mediamosa_colletion.'),
      ),
      mediamosa_aut_object_db::AUT_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The entity ID, links to aut_name or aut_group.'),
      ),
      mediamosa_aut_object_db::AUT_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_aut_object_db::AUT_TYPE_NAME, mediamosa_aut_object_db::AUT_TYPE_GROUP)) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_object_db::AUT_TYPE_NAME, mediamosa_aut_object_db::AUT_TYPE_GROUP)) . "')",
        'default' => NULL,
        'description' => t('The type of entity, links to aut_name or aut_group.'),
      ),
    ),
    'unique keys' => array(
      'uni_link_to_obj' => array(mediamosa_aut_object_db::AUT_OBJECT_TYPE, mediamosa_aut_object_db::AUT_OBJECT_ID, mediamosa_aut_object_db::AUT_TYPE, mediamosa_aut_object_db::AUT_ID),
    ),
    'indexes' => array(
      'idx_type_id' => array(mediamosa_aut_object_db::AUT_TYPE, mediamosa_aut_object_db::AUT_ID),
      'idx_objid' => array(mediamosa_aut_object_db::AUT_OBJECT_ID)
    ),
  );

//  $a_schema[mediamosa_app_master_slave_db::TABLE_NAME] = array(
//  'description' => t('This table stores master/slave links to media.'),
//    'fields' => array(

  return $a_schema;
}
