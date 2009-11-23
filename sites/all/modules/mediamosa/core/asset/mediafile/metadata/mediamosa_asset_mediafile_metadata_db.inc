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
  * The database class of the asset_mediafile metadata module.
  */

class mediamosa_asset_mediafile_metadata_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_mediafile_metadata';

  // Database fields;
  const ID = 'metadata_id';
  const MEDIAFILE_ID = 'mediafile_id'; // 1.x name: mediafile_id.
  const VIDEO_CODEC = 'video_codec';
  const VIDEO_CODEC_LENGTH = 50;
  const COLORSPACE = 'colorspace';
  const COLORSPACE_LENGTH = 10;
  const WIDTH = 'width';
  const HEIGHT = 'height';
  const FPS = 'fps';
  const FPS_LENGTH = 10;
  const AUDIO_CODEC = 'audio_codec';
  const AUDIO_CODEC_LENGTH = 50;
  const SAMPLE_RATE = 'sample_rate';
  const CHANNELS = 'channels';
  const CHANNELS_LENGTH = 10;
  const FILE_DURATION = 'file_duration';
  const FILE_DURATION_LENGTH = 10;
  const CONTAINER_TYPE = 'container_type';
  const CONTAINER_TYPE_LENGTH = 50;
  const BITRATE = 'bitrate';
  const BPP = 'bpp';
  const BPP_LENGTH = 5;
  const FILESIZE = 'filesize';
  const MIME_TYPE = 'mime_type';
  const MIME_TYPE_LENGTH = 80;
  const CREATED = 'created';
  const CHANGED = 'changed';
  const IS_HINTED = 'is_hinted';
  const IS_HINTED_TRUE = 'TRUE';
  const IS_HINTED_FALSE = 'FALSE';
  const IS_INSERTED_MD = 'is_inserted_md';
  const IS_INSERTED_MD_TRUE = 'TRUE';
  const IS_INSERTED_MD_FALSE = 'FALSE';
  const STILL_TIME_CODE = 'still_time_code';
  const STILL_ORDER = 'still_order';
  const STILL_TYPE = 'still_type';
  const STILL_TYPE_NONE = 'NONE';
  const STILL_TYPE_NORMAL = 'NORMAL';
  const STILL_TYPE_SECONDS = 'SECONDS';
  const STILL_TYPE_SCENE = 'SCENE';
  const STILL_TYPE_PICTURE = 'PICTURE';
  const STILL_FORMAT = 'still_format';
  const STILL_FORMAT_LENGTH = 50;
  const STILL_DEFAULT = 'still_default';
  const STILL_DEFAULT_TRUE = 'TRUE';
  const STILL_DEFAULT_FALSE = 'FALSE';
}
