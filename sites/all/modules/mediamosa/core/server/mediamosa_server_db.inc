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
 * DB Class for mediamosa_server_upload module.
 */

class mediamosa_server_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server';

  // Database fields;
  const ID = 'server_id';
  const SERVER_STATUS = 'server_status';
  const SERVER_TYPE = 'server_type';
  const URI = 'uri';
  const URI_LENGTH = 255;
  const CONTAINERS = 'containers';
  const CONTAINERS_LENGTH = 255;
  const OBJECT_CODE = 'object_code';
  const SERVER_NAME = 'server_name';
  const SERVER_NAME_LENGTH = 255;
  const IP_ADDRESS = 'ip_address';
  const IP_ADDRESS_LENGTH = 15;
  const SLOTS = 'slots';
  const TOOLS = 'tools';
  const TOOLS_LENGTH = 255;
  const URI_UPLOAD_PROGRESS = 'uri_upload_progress';
  const URI_UPLOAD_PROGRESS_LENGTH = 255;

  // Server types
  const SERVER_TYPE_DOWNLOAD = 'download';
  const SERVER_TYPE_STILL = 'still';
  const SERVER_TYPE_STREAMING = 'streaming';
  const SERVER_TYPE_TRANSCODE = 'transcode';
  const SERVER_TYPE_UPLOAD = 'upload';
}
