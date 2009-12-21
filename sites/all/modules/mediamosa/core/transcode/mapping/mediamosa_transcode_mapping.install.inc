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
 *
 */
require_once 'mediamosa_transcode_mapping_db.inc';

/**
 * Implement hook_install().
 */
function mediamosa_transcode_mapping_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_transcode_mapping_db::TABLE_NAME);
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_transcode_mapping_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schemainstall_schema(mediamosa_transcode_mapping_db::TABLE_NAME);
}

/**
 * Implement hook_schema().
 */
function mediamosa_transcode_mapping_schema() {

  $a_schema[mediamosa_transcode_mapping_db::TABLE_NAME] = array(
  'description' => t('The transcode mapping table.'),
    'fields' => array(
      mediamosa_transcode_mapping_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_transcode_mapping_db::TOOL => array(
        'type' => 'varchar',
        'length' => mediamosa_transcode_mapping_db::TOOL_LENGTH,
        'description' => t('Tool.'),
      ),
      mediamosa_transcode_mapping_db::NICE_PARAMETER => array(
        'type' => 'varchar',
        'length' => mediamosa_transcode_mapping_db::NICE_PARAMETER_LENGTH,
        'description' => t('Nice parameter.'),
      ),
      mediamosa_transcode_mapping_db::TOOL_PARAMETER => array(
        'type' => 'varchar',
        'length' => mediamosa_transcode_mapping_db::TOOL_PARAMETER_LENGTH,
        'description' => t('Tool parameter.'),
      ),
      mediamosa_transcode_mapping_db::MIN_VALUE => array(
        'type' => 'int',
        'description' => t('The minimum value.'),
        'default' => NULL,
      ),
      mediamosa_transcode_mapping_db::MAX_VALUE => array(
        'type' => 'int',
        'description' => t('The maximum value.'),
        'default' => NULL,
      ),
      mediamosa_transcode_mapping_db::ALLOWED_VALUE => array(
        'type' => 'text',
        'description' => t('Allowed value.'),
      ),
      mediamosa_transcode_mapping_db::DEFAULT_VALUE => array(
        'type' => 'varchar',
        'length' => mediamosa_transcode_mapping_db::DEFAULT_VALUE_LENGTH,
        'description' => t('Default value.'),
      ),
      mediamosa_transcode_mapping_db::REQUIRED => array(
        'description' => t('The asset is private.'),
        'type' => "enum('" . implode("','", array(mediamosa_transcode_mapping_db::REQUIRED_FALSE, mediamosa_transcode_mapping_db::REQUIRED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_transcode_mapping_db::REQUIRED_FALSE, mediamosa_transcode_mapping_db::REQUIRED_TRUE)) . "')",
        'default' => mediamosa_transcode_mapping_db::REQUIRED_TRUE,
      ),
      mediamosa_transcode_mapping_db::CREATED => array(
        'type' => 'datetime',
        'not null' => TRUE,
        'description' => t('The date and time when transcode mapping was created.'),
      ),
      mediamosa_transcode_mapping_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when transcode mapping was changed last.'),
      ),
    ),
    'primary key' => array(mediamosa_transcode_mapping_db::ID),
    'unique keys' => array(
      'uni_tool_niceparam' => array(mediamosa_transcode_mapping_db::TOOL, mediamosa_transcode_mapping_db::NICE_PARAMETER),
    ),
    'indexes' => array(
      'idx_niceparam' => array(mediamosa_transcode_mapping_db::NICE_PARAMETER),
    ),
  );

  return $a_schema;
}
