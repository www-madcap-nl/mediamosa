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
 * DB Class for mediamosa_server_transcoding module.
 */

class mediamosa_server_transcoding_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server_transcoding'; // 1.x name transcoding_server.

  // Database fields;
  const ID = 'transcoding_server_id';
  const VERSION = 'version';
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 255;
  const SERVERNAME = 'servername';
  const SERVERNAME_LENGTH = 255;
  const IP_ADDRESS = 'ip_address';
  const IP_ADDRESS_LENGTH = 15;
  const SLOTS = 'slots';
  const IS_TEST_SERVER = 'is_test_server';
  const IS_TEST_SERVER_TRUE = 'TRUE';
  const IS_TEST_SERVER_FALSE = 'FALSE';
  const STATUS = 'status';
  const STATUS_ON = 'ON';
  const STATUS_OFF = 'OFF';
  const STATUS_CLOSE = 'CLOSE';
  const TOOLS = 'tools';
  const TOOLS_LENGTH = 255;
}

class mediamosa_server_transcoding_profile_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server_transcoding_profile';

  // Database fields;
  const APP_ID = 'app_id';
  const VERSION = 'version';
  const IS_DEFAULT_PROFILE = 'is_default_profile';
  const TOOL = 'tool';
  const FILE_EXTENSION = 'file_extension';
  const OPTIONS = 'options';
}

class mediamosa_server_transcoding_profile_mapping_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server_transcoding_profile_mapping';

  // Database fields;
  const ID = 'tmid';
  const TOOL = mediamosa_server_transcoding_profile_db::TOOL;
  const TITLE = 'field_title';
  const TYPE = 'field_type';
  const DESCRIPTION = 'field_description';
  const TOOL_PARAMETER = 'tool_parameter';
  const MIN_VALUE = 'min_value';
  const MAX_VALUE = 'max_value';
  const ALLOWED_VALUE = 'allowed_value';
  const DEFAULT_VALUE = 'default_value';
  const REQUIRED = 'required';
}
