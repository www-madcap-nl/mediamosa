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
 * Install for mediamosa_server_job table.
 */
require_once 'mediamosa_server_job_db.inc';

/**
 * Implement hook_install().
 */
function _mediamosa_server_job_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_server_job_db::TABLE_NAME);
}

/**
 * Implement hook_uninstall().
 */
function _mediamosa_server_job_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_server_job_db::TABLE_NAME);
}

/**
 * Implement hook_schema().
 */
function mediamosa_server_job_schema() {

  $a_schema[mediamosa_server_job_db::TABLE_NAME] = array(
  'description' => t('The transcode job table.'),
    'fields' => array(
      mediamosa_server_job_db::SERVER_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => t('The server ID points to mediamosa_server nid field.'),
      ),
      mediamosa_server_job_db::JOB_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => t('The job ID.'),
      ),
      mediamosa_server_job_db::PROGRESS => array(
        'type' => 'int',
        'size' => 'tiny',
        'unsigned' => TRUE,
        'description' => t('The progress.'),
        'default' => 0,
      ),
      mediamosa_server_job_db::CREATED => array(
        'type' => 'datetime',
        'not null' => TRUE,
        'description' => t('The date and time when server job was created.'),
      ),
      mediamosa_server_job_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when server job was changed last.'),
      ),
    ),
    'primary key' => array(mediamosa_server_job_db::SERVER_ID, mediamosa_server_job_db::JOB_ID),
    'foreign keys' => array(
      mediamosa_server_job_db::JOB_ID => array(mediamosa_job_db::TABLE_NAME => mediamosa_job_db::ID),
    ),
  );

  return $a_schema;
}
