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
 * DB Class for mediamosa_server_streaming module.
 */

class mediamosa_server_streaming_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server_streaming'; // 1.x name streaming_server.

  // Database fields;
  const ID = 'streaming_server_id';
  const VERSION = 'version';
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 255;
  const URI = 'uri';
  const URI_LENGTH = 255;
  const ACTIVE = 'active';
  const CONTAINERS = 'containers';
  const CONTAINERS_LENGTH = 255;
  const OBJECTCODE = 'objectcode';
}
