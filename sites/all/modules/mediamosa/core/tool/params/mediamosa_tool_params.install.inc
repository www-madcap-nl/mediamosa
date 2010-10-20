<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * Tool params installer.
  */

require_once 'mediamosa_tool_params_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_tool_params_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_tool_params_db::TABLE_NAME);
}

/**
 * Implements hook_unstall().
 */
function _mediamosa_tool_params_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_tool_params_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_tool_params_schema() {

  $a_schema[mediamosa_tool_params_db::TABLE_NAME] = array(
  'description' => t('The tool parameter table.'),
    'fields' => array(
      mediamosa_tool_params_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_tool_params_db::TOOL => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::TOOL_LENGTH,
        'description' => t('Tool.'),
      ),
      mediamosa_tool_params_db::NICE_PARAMETER => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::NICE_PARAMETER_LENGTH,
        'description' => t('Nice parameter.'),
      ),
      mediamosa_tool_params_db::TOOL_PARAMETER => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::TOOL_PARAMETER_LENGTH,
        'description' => t('Tool parameter.'),
      ),
      mediamosa_tool_params_db::MIN_VALUE => array(
        'type' => 'int',
        'description' => t('The minimum value.'),
        'default' => NULL,
      ),
      mediamosa_tool_params_db::MAX_VALUE => array(
        'type' => 'int',
        'description' => t('The maximum value.'),
        'default' => NULL,
      ),
      mediamosa_tool_params_db::ALLOWED_VALUE => array(
        'type' => 'text',
        'description' => t('Allowed value.'),
      ),
      mediamosa_tool_params_db::DEFAULT_VALUE => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::DEFAULT_VALUE_LENGTH,
        'description' => t('Default value.'),
      ),
      mediamosa_tool_params_db::REQUIRED => array(
        'description' => t('It is a required parameter or not.'),
        'type' => "enum('" . implode("','", array(mediamosa_tool_params_db::REQUIRED_FALSE, mediamosa_tool_params_db::REQUIRED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_tool_params_db::REQUIRED_FALSE, mediamosa_tool_params_db::REQUIRED_TRUE)) . "')",
        'default' => mediamosa_tool_params_db::REQUIRED_TRUE,
      ),
      mediamosa_tool_params_db::CREATED => array(
        'type' => 'datetime',
        'not null' => TRUE,
        'description' => t('The date and time when tool parameter was created.'),
      ),
      mediamosa_tool_params_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when tool parameter was changed last.'),
      ),
    ),
    'primary key' => array(mediamosa_tool_params_db::ID),
    'unique keys' => array(
      'uni_niceparam_tool' => array(mediamosa_tool_params_db::NICE_PARAMETER, mediamosa_tool_params_db::TOOL),
    ),
    'indexes' => array(
      'idx_tool' => array(mediamosa_tool_params_db::TOOL),
    )
  );

  return $a_schema;
}
