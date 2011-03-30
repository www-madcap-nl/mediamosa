<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * The user installer.
  */

require_once 'mediamosa_user_db.inc';

require_once 'favorite/mediamosa_user_favorite.install.inc';
require_once 'group/mediamosa_user_group.install.inc';
require_once 'group/mediamosa_user_in_group.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_user_install() {

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_user_db::TABLE_NAME);

  // Installer user group.
  _mediamosa_user_group_install();

  // Installer user/group relation.
  _mediamosa_user_in_group_install();

  // Installer user group.
  _mediamosa_user_favorite_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_user_uninstall() {

  // Uninstaller user favorite.
  _mediamosa_user_favorite_uninstall();

  // Uninstaller user group.
  _mediamosa_user_group_uninstall();

  // Uninstaller user/group relation.
  _mediamosa_user_in_group_uninstall();

  // Remove our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_user_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_user_schema() {
  $schema[mediamosa_user_db::TABLE_NAME] = array(
  'description' => 'The user table.',
    'fields' => array(
      mediamosa_user_db::KEY => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_user_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the user.',
      ),
      mediamosa_user_db::NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_user_db::NAME_LENGTH,
        'not null' => TRUE,
        'description' => 'The name of user.',
      ),
      mediamosa_user_db::QUOTUM => array(
        'type' => 'int',
        'unsigned' => FALSE,
        'not null' => TRUE,
        'description' => 'The disk quota in MB.',
      ),
      mediamosa_user_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when user was created.',
      ),
      mediamosa_user_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when user was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_user_db::KEY),
    'unique keys' => array(
      'uni_id_name_and_app' => array(mediamosa_user_db::NAME, mediamosa_user_db::APP_ID),
    ),

  );

  return $schema;
}

/**
 * Implements hook_update_N().
 */
function mediamosa_user_update_7100() {
  $schema = mediamosa_user_schema();

  // Just fix incorrect comment first.
  $field_definition = $schema[mediamosa_user_db::TABLE_NAME]['fields'][mediamosa_user_db::QUOTUM];
  db_change_field(
      mediamosa_user_db::TABLE_NAME,
      mediamosa_user_db::QUOTUM,
      mediamosa_user_db::QUOTUM,
      $field_definition
    );

  // Primary key is downgraded to unique now.
  db_drop_primary_key(mediamosa_user_db::TABLE_NAME);
  db_add_unique_key(mediamosa_user_db::TABLE_NAME, 'uni_id_name_and_app',
    $schema[mediamosa_user_db::TABLE_NAME]['unique keys']['uni_id_name_and_app']);

  // Add brand new 'serial' primary.
  $field_definition = $schema[mediamosa_user_db::TABLE_NAME]['fields'][mediamosa_user_db::KEY];

  // Please note last parameter - array with primary key definition.
  db_add_field(mediamosa_user_db::TABLE_NAME, mediamosa_user_db::KEY, $field_definition,
    array('primary key' => array(mediamosa_user_db::KEY)));

  // Now process user_group table.
  _mediamosa_user_group_update_7100();

  // ...and user_in_group table.
  _mediamosa_user_in_group_update_7100();

  // Migration goes here.
  $query = mediamosa_db::db_select(mediamosa_user_db::TABLE_NAME, 'u');
  $query->fields('u');
  $query->join(mediamosa_user_group_db::TABLE_NAME, 'g', 'u.group_id = g.group_id AND u.app_id = g.app_id');
  $query->addField('g', 'group_key');
  $users_in_group = $query->execute()->fetchAll();

  $counter = 0;
  foreach ($users_in_group as $item) {
    mediamosa_db::db_query(
      strtr("INSERT INTO #table_name (#field_user_key, #field_group_key) VALUES (#user_key, #group_key)",
        array(
          '#table_name' => mediamosa_user_in_group_db::TABLE_NAME,
          '#field_user_key' => mediamosa_user_in_group_db::USER_KEY,
          '#field_group_key' => mediamosa_user_in_group_db::GROUP_KEY,
          '#user_key' => $item['user_key'],
          '#group_key' => $item['group_key'],
        )
      ));
    $counter++;
  }

  mediamosa_watchdog::log('Update: #number users belongs to groups. Group information migrated, multiple group membership enabled.',
    array('#number' => $counter));

  // Now we can get rid of the own old field.
  db_drop_field(mediamosa_user_db::TABLE_NAME, mediamosa_user_db::GROUP_ID);

}
