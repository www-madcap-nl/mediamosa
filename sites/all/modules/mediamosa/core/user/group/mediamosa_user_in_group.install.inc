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
  * The user group installer.
  *
  * Although users are not part of a user group, I still decided to put the
  * file hierargy for future logic.
  */

require_once 'mediamosa_user_in_group_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_user_in_group_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_user_in_group_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_user_in_group_uninstall() {
  // Remove our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_user_in_group_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_user_in_group_schema() {
  $schema[mediamosa_user_in_group_db::TABLE_NAME] = array(
  'description' => 'The user/group relation table.',
    'fields' => array(
      mediamosa_user_in_group_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_user_in_group_db::USER_KEY => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'User ID.',
      ),
      mediamosa_user_in_group_db::GROUP_KEY => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Group ID.',
      ),
    ),
    'primary key' => array(mediamosa_user_in_group_db::ID),
    'unique keys' => array(
      'uni_relation' => array(mediamosa_user_in_group_db::USER_KEY, mediamosa_user_in_group_db::GROUP_KEY),
    ),
    'foreign keys' => array(
      mediamosa_user_in_group_db::USER_KEY => array(mediamosa_user_db::TABLE_NAME => mediamosa_user_db::KEY),
      mediamosa_user_in_group_db::GROUP_KEY => array(mediamosa_user_group_db::TABLE_NAME => mediamosa_user_group_db::KEY),
    ),
  );

  return $schema;
}

/**
 * Implements hook_update_N().
 */
function _mediamosa_user_in_group_update_7100() {

  $schema = mediamosa_user_in_group_schema();

  // Create our new table.
  db_create_table(mediamosa_user_in_group_db::TABLE_NAME, $schema[mediamosa_user_in_group_db::TABLE_NAME]);
  // Manually add foreign key.
  // Sooner or later Drupal 7 will add foreign key support, so we will just silently catch exceptions.
  $fk_template = "ALTER TABLE {#table_name} ADD CONSTRAINT `fk_#table_name_#column` FOREIGN KEY (`#column`) REFERENCES `{#foreign_table_name}` (`#foreign_column`)";
  $query_fk1 = strtr($fk_template,
    array(
      '#table_name' => mediamosa_user_in_group_db::TABLE_NAME,
      '#column' => mediamosa_user_in_group_db::USER_KEY,
      '#foreign_table_name' => mediamosa_user_db::TABLE_NAME,
      '#foreign_column' => mediamosa_user_db::KEY,
    )
  );

  $query_fk2 = strtr($fk_template,
    array(
      '#table_name' => mediamosa_user_in_group_db::TABLE_NAME,
      '#column' => mediamosa_user_in_group_db::GROUP_KEY,
      '#foreign_table_name' => mediamosa_user_group_db::TABLE_NAME,
      '#foreign_column' => mediamosa_user_group_db::KEY,
    )
  );

  try {
    db_query($query_fk1);
    db_query($query_fk2);
  }
  catch (Exception $e) {
    // ignore.
  }

}
