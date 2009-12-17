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
  * The functions class of server_still module.
  */

class mediamosa_server_still {
  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the default still server.
   */
  static public function get() {
    static $a_server = NULL;

    if (isset($a_server)) {
      return $a_server;
    }

    $a_server = mediamosa_db::db_query(
     'SELECT * FROM {#mediamosa_server_still} WHERE #active = 1 LIMIT 1',
     array(
       '#mediamosa_server_still' => mediamosa_server_still_db::TABLE_NAME,
       '#active' => mediamosa_server_still_db::ACTIVE,
     )
    )->fetchAssoc();

    return $a_server;
  }

  /**
   * Get the URI of the default still server.
   */
  static public function get_uri() {
    $a_server = self::get();

    return $a_server[mediamosa_server_still_db::URI];
  }
}
