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
        "SELECT COUNT(ss.#server_id) AS total
         FROM {#mediamosa_streaming_server} AS ss
         JOIN {#mediamosa_streaming_server_container} AS ssc ON ss.#server_id = ssc.#streaming_server_id
         WHERE ssc.#container = :container AND ss.#active = 1 AND ss.#server_type = :server_type",
        array(
          '#server_id' => mediamosa_server_db::ID,
          '#streaming_server_id' => mediamosa_server_streaming_container_db::STREAMING_SERVER_ID,
          '#mediamosa_streaming_server' => mediamosa_server_db::TABLE_NAME,
          '#mediamosa_streaming_server_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
          '#container' => mediamosa_server_streaming_container_db::CONTAINER,
          ':container' => $container_type,
          '#active' => mediamosa_server_db::SERVER_STATUS,
          '#server_type' => mediamosa_server_db::SERVER_TYPE,
          ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
        )
      )->fetchField();

      $a_streamable_container_types[$container_type] = $is_streamable ? TRUE : FALSE;
    }

    return ($a_streamable_container_types[$container_type]);
  }

  /**
   * Find the streaming server based on container and codec)
   *
   * @param string $container_type
   *  Container type like mp4;mpg etc.
   * @param string $video_codec
   *  Codec.
   */
  static public function get($container_type, $video_codec) {
    static $a_servers = array();

    if (isset($a_servers[$container_type . ':' . $video_codec])) {
      return $a_servers[$container_type . ':' . $video_codec];
    }

    // Choose a random available streaming server.
    // try to match container:codec first.
    $a_servers[$container_type . ':' . $video_codec] = mediamosa_db::db_query(
      "SELECT mss.* FROM {#mediamosa_server_streaming} AS mss WHERE
      JOIN {#mediamosa_server_streaming_container} AS mssc USING(#streaming_server_id)
      WHERE mssc.#container = :container AND mss.#active = 1 AND #server_type = :server_type ORDER BY rand() LIMIT 1",
      array(
        '#mediamosa_server_streaming' => mediamosa_server_db::TABLE_NAME,
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
        '#streaming_server_id' => mediamosa_server_db::ID,
        '#container' => mediamosa_server_streaming_container_db::CONTAINER,
        ':container' => $container_type .':'. $video_codec,
        '#active' => mediamosa_server_db::ACTIVE,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
      )
    )->fetchAssoc();

    if ($a_servers[$container_type .':'. $video_codec]) {
      return $a_servers[$container_type .':'. $video_codec];
    }

    if (isset($a_servers[$container_type])) {
      return $a_servers[$container_type];
    }

    $a_servers[$container_type] = mediamosa_db::db_query(
      "SELECT mss.* FROM {#mediamosa_server_streaming} AS mss WHERE
      JOIN {#mediamosa_server_streaming_container} AS mssc USING(#streaming_server_id)
      WHERE mssc.#container = :container AND mss.#active = 1 AND mss.#server_type = :server_type ORDER BY rand() LIMIT 1",
      array(
        '#mediamosa_server_streaming' => mediamosa_server_db::TABLE_NAME,
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
        '#streaming_server_id' => mediamosa_server_db::ID,
        '#container' => mediamosa_server_streaming_container_db::CONTAINER,
        ':container' => $container_type,
        '#active' => mediamosa_server_db::ACTIVE,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
      )
    )->fetchAssoc();

    return $a_servers[$container_type];
  }

  /**
   * Find the object code based on container and video_codec (optional).
   *
   * @param string $container_type
   *  Container type like mp4;mpg etc.
   * @param string $video_codec
   *  Codec.
   */
  static public function get_objectcode($container_type, $video_codec) {

    $a_server = self::get($container_type, $video_codec);

    if ($a_server) {
      return $a_server[mediamosa_server_db::OBJECT_CODE];
    }

    return FALSE;
  }

  /**
   * Find the uri based on container and video_codec (optional).
   *
   * @param string $container_type
   *  Container type like mp4;mpg etc.
   * @param string $video_codec
   *  Codec.
   */
  static public function get_uri($container_type, $video_codec) {

    $a_server = self::get($container_type, $video_codec);

    if ($a_server) {
      return $a_server[mediamosa_server_db::URI];
    }

    return FALSE;
  }
}
