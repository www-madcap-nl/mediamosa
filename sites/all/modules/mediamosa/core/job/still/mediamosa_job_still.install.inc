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
 * Install for mediamosa_job_still table.
 */
require_once 'mediamosa_job_still_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_job_still_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_job_still_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_job_still_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_job_still_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_job_still_schema() {

  $a_schema[mediamosa_job_still_db::TABLE_NAME] = array(
  'description' => t('The transcode job table.'),
    'fields' => array(
      mediamosa_job_still_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_job_still_db::JOB_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => t('The job ID.'),
      ),
      mediamosa_job_still_db::SIZE => array(
        'type' => 'varchar',
        'length' => mediamosa_job_still_db::SIZE_LENGTH,
        'default' => NULL,
        'description' => t('The size for still.'),
      ),
      mediamosa_job_still_db::FRAMETIME => array(
        'type' => 'int',
        'description' => t('The still frametime.'),
      ),
      mediamosa_job_still_db::H_PADDING => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'default' => NULL,
        'description' => t('The horizontal padding.'),
      ),
      mediamosa_job_still_db::V_PADDING => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'default' => NULL,
        'description' => t('The vertical padding.'),
      ),
      mediamosa_job_still_db::BLACKSTILL_CHECK => array(
        'description' => t('Enable to prevent dark empty stills.'),
        'type' => "enum('" . implode("','", array(mediamosa_job_still_db::BLACKSTILL_CHECK_FALSE, mediamosa_job_still_db::BLACKSTILL_CHECK_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_still_db::BLACKSTILL_CHECK_FALSE, mediamosa_job_still_db::BLACKSTILL_CHECK_TRUE)) . "')",
        'default' => mediamosa_job_still_db::BLACKSTILL_CHECK_FALSE,
      ),
      mediamosa_job_still_db::STILL_PARAMETERS => array(
        'type' => 'text',
        'default' => NULL,
        'description' => t('Still parameters.'),
      ),
      mediamosa_job_still_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => t('The date and time when still job was created.'),
      ),
      mediamosa_job_still_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => t('The date and time when still job was changed last.'),
      ),
    ),
    'primary key' => array(mediamosa_job_still_db::ID),
    'foreign keys' => array(
      mediamosa_job_still_db::JOB_ID => array(mediamosa_job_db::TABLE_NAME => mediamosa_job_db::ID),
    ),
  );

  return $a_schema;
}
