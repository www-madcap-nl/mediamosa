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
 * The install file for asset media
 */

require_once 'mediamosa_asset_mediafile_metadata.inc';
require_once 'mediamosa_asset_mediafile_metadata_db.inc';

/**
 * Implement hook_install().
 */
function mediamosa_asset_mediafile_metadata_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
}

/**
 * Implement hook_uninstall().
 */
function mediamosa_asset_mediafile_metadata_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
}

/**
 * Implement hook_schema().
 */
function mediamosa_asset_mediafile_metadata_schema() {

  $a_schema[mediamosa_asset_mediafile_metadata_db::TABLE_NAME] = array(
  'description' => t('The asset media metadata table.'),
    'fields' => array(
      mediamosa_asset_mediafile_metadata_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => t('Primary Key: Unique ID.'),
      ),
      mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => t('The asset mediafile ID.'),
      ),
      mediamosa_asset_mediafile_metadata_db::VIDEO_CODEC => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::VIDEO_CODEC_LENGTH,
        'description' => t('The codec used during creation.'),
      ),
      mediamosa_asset_mediafile_metadata_db::COLORSPACE => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::COLORSPACE_LENGTH,
        'description' => t('The codec used during creation.'),
      ),
      mediamosa_asset_mediafile_metadata_db::WIDTH => array(
        'type' => 'int',
        'description' => t('The width of the object.'),
        'unsigned' => TRUE,
      ),
      mediamosa_asset_mediafile_metadata_db::HEIGHT => array(
        'type' => 'int',
        'description' => t('The height of the object.'),
        'unsigned' => TRUE,
      ),
      mediamosa_asset_mediafile_metadata_db::FPS => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::FPS_LENGTH,
        'description' => t('The frame per seconds.'),
      ),
      mediamosa_asset_mediafile_metadata_db::AUDIO_CODEC => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::AUDIO_CODEC_LENGTH,
        'description' => t('The audio codec used.'),
      ),
      mediamosa_asset_mediafile_metadata_db::SAMPLE_RATE => array(
        'type' => 'int',
        'description' => t('The height of the object.'),
        'unsigned' => TRUE,
      ),
      mediamosa_asset_mediafile_metadata_db::CHANNELS => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::CHANNELS_LENGTH,
        'description' => t('The number of channels.'),
      ),
      mediamosa_asset_mediafile_metadata_db::FILE_DURATION => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::FILE_DURATION_LENGTH,
        'description' => t('The length of object in time.'),
      ),
      mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE_LENGTH,
        'description' => t('The container type, e.g. avi.'),
      ),
      mediamosa_asset_mediafile_metadata_db::BITRATE => array(
        'type' => 'int',
        'description' => t('The bitrate of audio and video.'),
        'unsigned' => TRUE,
      ),
      mediamosa_asset_mediafile_metadata_db::BPP => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::BPP_LENGTH,
        'description' => t('The average number of bits in a pixel.'),
      ),
      mediamosa_asset_mediafile_metadata_db::FILESIZE => array(
        'type' => 'int',
        'size' => 'big',
        'description' => t('The size of object in bytes.'),
        'unsigned' => TRUE,
      ),
      mediamosa_asset_mediafile_metadata_db::MIME_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::MIME_TYPE_LENGTH,
        'description' => t('The mime type of the object.'),
        'not null' => TRUE,
        'default' => 'application/octet-stream',
      ),
      mediamosa_asset_mediafile_metadata_db::CREATED => array(
        'type' => 'datetime',
        'not null' => TRUE,
        'description' => t('The date and time when media metadata was created.'),
      ),
      mediamosa_asset_mediafile_metadata_db::CHANGED => array(
        'type' => 'datetime',
        'default' => NULL,
        'description' => t('The date and time when media metadata was changed.'),
      ),
      mediamosa_asset_mediafile_metadata_db::IS_HINTED => array(
        'description' => t('The video file is hinted, making the video streamable (needed for mp4 video) and enables skipping to new positions in the video.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_db::IS_HINTED_FALSE, mediamosa_asset_mediafile_metadata_db::IS_HINTED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_db::IS_HINTED_FALSE, mediamosa_asset_mediafile_metadata_db::IS_HINTED_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_metadata_db::IS_HINTED_FALSE,
      ),
      mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD => array(
        'description' => t('Has extra metadata available.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD_FALSE, mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD_FALSE, mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_metadata_db::IS_INSERTED_MD_FALSE,
      ),
      mediamosa_asset_mediafile_metadata_db::STILL_TIME_CODE => array(
        'type' => 'int',
        'description' => t('The position where the still was taken.'),
        'unsigned' => TRUE,
      ),
      mediamosa_asset_mediafile_metadata_db::STILL_ORDER => array(
        'type' => 'int',
        'description' => t('The order between stills.'),
        'default' => 0,
      ),
      mediamosa_asset_mediafile_metadata_db::STILL_TYPE => array(
        'description' => t('The type of the still.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_db::STILL_TYPE_NONE, mediamosa_asset_mediafile_metadata_db::STILL_TYPE_NORMAL, mediamosa_asset_mediafile_metadata_db::STILL_TYPE_SECONDS, mediamosa_asset_mediafile_metadata_db::STILL_TYPE_SCENE, mediamosa_asset_mediafile_metadata_db::STILL_TYPE_PICTURE)) . "')",
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_db::STILL_TYPE_NONE, mediamosa_asset_mediafile_metadata_db::STILL_TYPE_NORMAL, mediamosa_asset_mediafile_metadata_db::STILL_TYPE_SECONDS, mediamosa_asset_mediafile_metadata_db::STILL_TYPE_SCENE, mediamosa_asset_mediafile_metadata_db::STILL_TYPE_PICTURE)) . "')",
        'default' => NULL,
      ),
      mediamosa_asset_mediafile_metadata_db::STILL_FORMAT => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_db::STILL_FORMAT_LENGTH,
        'description' => t('The format of the still.'),
      ),
      mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT => array(
        'description' => t('Is this the default still.'),
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_FALSE, mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_FALSE, mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_metadata_db::STILL_DEFAULT_TRUE,
      ),
    ),
    'primary key' => array(mediamosa_asset_mediafile_metadata_db::ID),
    'indexes' => array(
      'idx_mediafileid_mimetype' => array(
        mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID,
        mediamosa_asset_mediafile_metadata_db::MIME_TYPE,
      ),
    ),
    'foreign keys' => array(
      mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => array(mediamosa_asset_mediafile_db::TABLE_NAME => mediamosa_asset_mediafile_db::ID),
    ),
  );

  return $a_schema;
}
