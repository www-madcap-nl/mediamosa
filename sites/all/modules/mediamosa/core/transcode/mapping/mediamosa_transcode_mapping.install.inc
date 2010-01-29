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
function _mediamosa_transcode_mapping_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_transcode_mapping_db::TABLE_NAME);

  $a_default_mappings = array(
    array('ffmpeg', 'videobitrate', '-b', 128000, 800000, NULL, '200000', 'FALSE'),
    array('ffmpeg', 'fps', '-r', 10, 30, NULL, '25', 'FALSE'),
    array('ffmpeg', 'size', '-s', NULL, NULL, '128x96;176x144;352x288;704x576;160x120;320x240;640x480;800x600;1024x768;1600x1200;2048x1536;1280x1024;2560x2048;5120x4096;852x480;1366x768;1600x1024;1920x1200;2560x1600;3200x2048;3840x2400;6400x4096;7680x4800;320x200;640x350;852x480;1280x720;1920x1080;480x270;480x360', '320x240', 'FALSE'),
    array('ffmpeg', 'internal_file_extension', '', NULL, NULL, 'flv;wmv;mp4;avi;ogg', NULL, 'FALSE'),
    array('ffmpeg', 'qmax', '-qmax', 1, 31, NULL, NULL, 'FALSE'),
    array('ffmpeg', 'videocodec', '-vcodec', NULL, NULL, 'flv;wmv;mpeg4;libx264;rawvideo;libtheora', NULL, 'FALSE'),
    array('ffmpeg', 'audiocodec', '-acodec', NULL, NULL, 'libmp3lame;wma;pcm_s16le;libfaac;libvorbis', NULL, 'FALSE'),
    array('ffmpeg', 'audiobitrate', '-ab', NULL, NULL, '40000;64000;128000;192000;256000;512000', '64000', 'FALSE'),
    array('ffmpeg', 'audiosamplingrate', '-ar', NULL, NULL, '22050;44100', '44100', 'FALSE'),
    array('ffmpeg', 'audiochannels', '-ac', 0, 2, NULL, '1', 'FALSE'),
    array('ffmpeg', 'duration', '-t', 0, NULL, NULL, NULL, 'FALSE'),
    array('ffmpeg', 'startposition', '-ss', 0, NULL, NULL, NULL, 'FALSE'),
    array('ffmpeg', 'internal_previous_job', '', NULL, NULL, NULL, NULL, 'FALSE'),
    array('windows', 'internal_file_extension', '', NULL, NULL, 'wmv', NULL, 'FALSE'),
    array('windows', 'fps', '-v_framerate', 10, 30, NULL, '25', 'FALSE'),
    array('windows', 'internal_previous_job', '', NULL, NULL, NULL, NULL, 'FALSE'),
    array('windows', 'videobitrate', '-v_bitrate', 128, 1024, NULL, NULL, 'FALSE'),
    array('windows', 'width', '-v_width', 128, 7680, NULL, NULL, 'FALSE'),
    array('windows', 'height', '-v_height', 96, 4800, NULL, NULL, 'FALSE'),
    array('windows', 'videocodec', '-v_codec', NULL, NULL, 'flv;wmv;mpeg4;libx264;rawvideo', NULL, 'FALSE'),
    array('windows', 'audiocodec', '-a_codec', NULL, NULL, 'mp3;wma;pcm_s16le;libfaac', NULL, 'FALSE'),
    array('windows', 'audiosetting', '-a_setting', NULL, NULL, '40_22_1;40_22_2;40_44_1;40_44_2;64_22_1;64_22_2;64_44_1;64_44_2;128_22_1;128_22_2;128_44_1;128_44_2;192_22_1;192_22_2;192_44_1;192_44_2;256_22_1;256_22_2;256_44_1;256_44_2', NULL, 'FALSE'),
    array('ffmpeg', 'maintain_aspect_ratio', '', NULL, NULL, 'yes;no', 'yes', 'FALSE')
  );

  // Insert default mappings as nodes.
  foreach ($a_default_mappings as $a_default_mapping) {
    $a_fields = array(
      mediamosa_transcode_mapping_db::TOOL => $a_default_mapping[0],
      mediamosa_transcode_mapping_db::NICE_PARAMETER => $a_default_mapping[1],
      mediamosa_transcode_mapping_db::TOOL_PARAMETER => $a_default_mapping[2],
      mediamosa_transcode_mapping_db::MIN_VALUE => $a_default_mapping[3],
      mediamosa_transcode_mapping_db::MAX_VALUE => $a_default_mapping[4],
      mediamosa_transcode_mapping_db::ALLOWED_VALUE => $a_default_mapping[5],
      mediamosa_transcode_mapping_db::DEFAULT_VALUE => $a_default_mapping[6],
      mediamosa_transcode_mapping_db::REQUIRED => $a_default_mapping[7],
    );

    // Add changed/created.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    db_insert(mediamosa_transcode_mapping_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }
}

/**
 * Implement hook_uninstall().
 */
function _mediamosa_transcode_mapping_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_transcode_mapping_db::TABLE_NAME);
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
      'uni_niceparam_tool' => array(mediamosa_transcode_mapping_db::NICE_PARAMETER, mediamosa_transcode_mapping_db::TOOL),
    ),
    'indexes' => array(
      'idx_tool' => array(mediamosa_transcode_mapping_db::TOOL),
    )
  );

  return $a_schema;
}
