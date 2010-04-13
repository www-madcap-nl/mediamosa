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
 * DB class for statistics stream request.
 */
class mediamosa_statistics_stream_request_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_statistics_stream_request'; // 1.x name statistics_stream_request.

  // Database fields;
  const ID = 'id';
  const APP_ID = 'app_id';
  const ASSET_ID = 'asset_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const OWNER_ID = 'owner_id';
  const GROUP_ID = 'group_id';
  const FILESIZE = 'filesize';
  const CONTAINER_TYPE = 'container_type';
  const CONTAINER_TYPE_LENGTH = 50;
  const PLAY_TYPE = 'play_type';
  const PLAY_TYPE_LENGTH = 20;
  const PLAYED = 'played'; // 1.x type timestamp, 2.x type datetime.
}
