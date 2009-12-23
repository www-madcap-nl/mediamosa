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
require_once 'mediamosa_transcode_profile_db.inc';

/**
 * Implement hook_install().
 */
function mediamosa_transcode_profile_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_transcode_profile_db::TABLE_NAME);

  $a_default_profiles = array(
    array('standaard flash' ,'FALSE', 'ffmpeg', 'flv', 'videobitrate:500000;size:320x200;audiobitrate:64000;audiosamplingrate:22050;maintain_aspect_ratio:yes'),
    array('qcif flash', 'TRUE', 'ffmpeg', 'flv', 'videobitrate:200000;size:176x144;audiosamplingrate:22050;maintain_aspect_ratio:yes'),
    array('standaard wmv', 'FALSE', 'windows', 'wmv', ''),
    array('standaard mp4', 'FALSE', 'ffmpeg', 'mp4', ''),
    array('FLASH 800x600', 'FALSE', 'ffmpeg', 'flv', 'size:800x600;audiobitrate:128000;audiosamplingrate:22050'),
    array('h.264 flash', 'FALSE', 'ffmpeg', 'flv', 'videobitrate:800000;size:640x480;videocodec:libx264;audiocodec:libmp3lame;audiobitrate:256000;audiosamplingrate:44100;maintain_aspect_ratio:yes'),
  );

  foreach ($a_default_profiles as $a_default_profile) {
    $a_fields = array(
      mediamosa_transcode_profile_db::VERSION => 0,
      mediamosa_transcode_profile_db::PROFILE => $a_default_profile[0],
      mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE => $a_default_profile[1],
      mediamosa_transcode_profile_db::TOOL => $a_default_profile[2],
      mediamosa_transcode_profile_db::FILE_EXTENSION => $a_default_profile[3],
      mediamosa_transcode_profile_db::COMMAND => $a_default_profile[4],
      mediamosa_transcode_profile_db::APP_ID => 0,
    );

    // Add changed/created.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_transcode_profile_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_transcode_profile_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schemainstall_schema(mediamosa_transcode_profile_db::TABLE_NAME);
}

/**
 * Implement hook_schema().
 */
function mediamosa_transcode_profile_schema() {

  $a_schema[mediamosa_transcode_profile_db::TABLE_NAME] = array(
  'description' => t('The transcode profile table.'),
    'fields' => array(
      mediamosa_transcode_profile_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_transcode_profile_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The application ID.'),
      ),
      mediamosa_transcode_profile_db::VERSION => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'default' => NULL,
        'description' => t('Version profile.'),
      ),
      mediamosa_transcode_profile_db::PROFILE => array(
        'type' => 'varchar',
        'length' => mediamosa_transcode_profile_db::PROFILE_LENGTH,
        'description' => t('Profile data.'),
      ),
      mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE => array(
        'description' => t('The asset is private.'),
        'type' => "enum('" . implode("','", array(mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE, mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE, mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE)) . "')",
        'default' => mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE,
      ),
      mediamosa_transcode_profile_db::TOOL => array(
        'type' => 'varchar',
        'length' => mediamosa_transcode_profile_db::TOOL_LENGTH,
        'description' => t('Tool used.'),
      ),
      mediamosa_transcode_profile_db::FILE_EXTENSION => array(
        'type' => 'char',
        'length' => mediamosa_transcode_profile_db::FILE_EXTENSION_LENGTH,
        'description' => t('file extension.'),
      ),
      mediamosa_transcode_profile_db::COMMAND => array(
        'type' => 'text',
        'description' => t('Command.'),
      ),
      mediamosa_transcode_profile_db::CREATED => array(
        'type' => 'datetime',
        'not null' => TRUE,
        'description' => t('The date and time when transcode profile was created.'),
      ),
      mediamosa_transcode_profile_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when transcode profile was changed last.'),
      ),
    ),
    'primary key' => array(mediamosa_transcode_profile_db::ID),
    'unique keys' => array(
      'uni_appid_profile' => array(mediamosa_transcode_profile_db::APP_ID, mediamosa_transcode_profile_db::PROFILE),
    ),
    'indexes' => array(
      array(mediamosa_transcode_profile_db::APP_ID, mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE),
    ),
  );

  return $a_schema;
}
