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
  * The server streamer class.
  */

class mediamosa_server_streaming {
  // ------------------------------------------------------------------ Static Function.
  static public function is_streamable($container_type) {
    static $a_streamable_container_types = array();

    if (!isset($a_streamable_container_types[$container_type])) {
      $is_streamable = mediamosa_db::db_query(
        "SELECT COUNT(ss.#streaming_server_id) AS total
         FROM {#mediamosa_streaming_server} AS ss
         JOIN {#mediamosa_streaming_server_container} AS ssc ON ss.#streaming_server_id = ssc.#streaming_server_id
         WHERE ssc.#container = :container_type AND ss.#active = 1",
        array(
          '#streaming_server_id' => mediamosa_server_streaming_db::ID,
          '#mediamosa_streaming_server' => mediamosa_server_streaming_db::TABLE_NAME,
          '#mediamosa_streaming_server_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
          '#container' => mediamosa_server_streaming_container_db::CONTAINER,
          '#active' => mediamosa_server_streaming_db::ACTIVE,
          ':container_type' => $container_type,
        )
      )->fetchField();

      $a_streamable_container_types[$container_type] = $is_streamable ? TRUE : FALSE;
    }

    return ($a_streamable_container_types[$container_type]);
  }
}
