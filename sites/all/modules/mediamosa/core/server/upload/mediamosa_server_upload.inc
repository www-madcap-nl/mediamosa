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
 * Upload server functions.
 */

class mediamosa_server_upload {

  // ------------------------------------------------------------------ Consts.
  static public function get_random() {
    return mediamosa_db::db_query_range(
      'SELECT * FROM {#mediamosa_server_upload} WHERE #status = 1 ORDER BY RAND()',
      0,
      1,
      array(
        '#mediamosa_server_upload' => mediamosa_server_upload_db::TABLE_NAME,
        '#status' => mediamosa_server_upload_db::STATUS,
      )
    )->fetchAssoc();
  }
}
