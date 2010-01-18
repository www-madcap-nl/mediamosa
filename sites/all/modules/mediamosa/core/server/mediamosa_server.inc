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

class mediamosa_server {

  // ------------------------------------------------------------------ Consts.
  /**
   * Get random upload server.
   */
  static public function get_random_upload() {
    return mediamosa_db::db_query_range(
      'SELECT * FROM {#mediamosa_server} WHERE #server_type = :server_type AND #status = :status_on ORDER BY RAND()',
      0,
      1,
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#status' => mediamosa_server_db::SERVER_STATUS,
        ':status_on' => mediamosa_server_db::SERVER_STATUS_ON,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_UPLOAD,
      )
    )->fetchAssoc();
  }

  /**
   * Get all enabled transcoding servers (either ON or CLOSE).
   */
  static public function get_enabled_transcode() {
    return mediamosa_db::db_select(mediamosa_server_db::TABLE_NAME, 'ms')
      ->fields('ms')
      ->condition(mediamosa_server_db::SERVER_TYPE, mediamosa_server_db::SERVER_TYPE_TRANSCODE, '=')
      ->condition(mediamosa_server_db::SERVER_STATUS, mediamosa_server_db::SERVER_STATUS_OFF, '!=')
      ->execute();
  }

  /**
   * Get all enabled download servers.
   */
  static public function get_enabled_download() {
    return mediamosa_db::db_select(mediamosa_server_db::TABLE_NAME, 'ms')
      ->fields('ms')
      ->condition(mediamosa_server_db::SERVER_TYPE, mediamosa_server_db::SERVER_TYPE_DOWNLOAD, '=')
      ->condition(mediamosa_server_db::SERVER_STATUS, mediamosa_server_db::SERVER_STATUS_ON, '=')
      ->execute();
  }

  /**
   * Get all enabled still servers.
   */
  static public function get_enabled_still() {
    return mediamosa_db::db_select(mediamosa_server_db::TABLE_NAME, 'ms')
      ->fields('ms')
      ->condition(mediamosa_server_db::SERVER_TYPE, mediamosa_server_db::SERVER_TYPE_STILL, '=')
      ->condition(mediamosa_server_db::SERVER_STATUS, mediamosa_server_db::SERVER_STATUS_ON, '=')
      ->execute();
  }

  /**
   * Return streamable servers.
   *
   * @param $container_type
   */
  static public function is_streamable($container_type) {
    static $a_streamable_container_types = array();

    if (!isset($a_streamable_container_types[$container_type])) {
      $is_streamable = mediamosa_db::db_query(
        "SELECT COUNT(ms.#nid) AS total FROM {#mediamosa_server} AS ms
         JOIN {#mediamosa_streaming_server_container} AS mssc ON ms.#nid = mssc.#nid
         WHERE mssc.#container = :container AND ms.#server_type = :server_type AND ms.#server_status = :server_status_on",
        array(
          '#nid' => mediamosa_server_db::NID,
          '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
          '#mediamosa_streaming_server_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
          '#container' => mediamosa_server_streaming_container_db::CONTAINER,
          ':container' => $container_type,
          '#server_type' => mediamosa_server_db::SERVER_TYPE,
          ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
          '#server_status' => mediamosa_server_db::SERVER_STATUS,
          ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
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
  static public function get_streaming($container_type, $video_codec) {
    static $a_servers = array();

    if (isset($a_servers[$container_type . ':' . $video_codec]) && $a_servers[$container_type . ':' . $video_codec]) {
      return $a_servers[$container_type . ':' . $video_codec];
    }

    // Choose a random available streaming server.
    // try to match container:codec first.
    $a_servers[$container_type . ':' . $video_codec] = mediamosa_db::db_query(
      "SELECT mss.* FROM {#mediamosa_server} AS mss
      JOIN {#mediamosa_server_streaming_container} AS mssc ON mssc.#nid = mss.#nid
      WHERE mssc.#container = :container AND mss.#server_type = :server_type AND mss.#server_status = :server_status_on ORDER BY rand() LIMIT 1",
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
        '#nid' => mediamosa_server_db::NID,
        '#container' => mediamosa_server_streaming_container_db::CONTAINER,
        ':container' => $container_type . ':' . $video_codec,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
        '#server_status' => mediamosa_server_db::SERVER_STATUS,
        ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
      )
    )->fetchAssoc();

    if ($a_servers[$container_type .':'. $video_codec]) {
      return $a_servers[$container_type .':'. $video_codec];
    }

    if (isset($a_servers[$container_type]) && $a_servers[$container_type]) {
      return $a_servers[$container_type];
    }

    $a_servers[$container_type] = mediamosa_db::db_query(
      "SELECT mss.* FROM {#mediamosa_server} AS mss
      JOIN {#mediamosa_server_streaming_container} AS mssc ON mssc.#nid = mss.#nid
      WHERE mssc.#container = :container AND mss.#server_type = :server_type AND mss.#server_status = :server_status_on ORDER BY rand() LIMIT 1",
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
        '#nid' => mediamosa_server_db::NID,
        '#container' => mediamosa_server_streaming_container_db::CONTAINER,
        ':container' => $container_type,
        '#server_status' => mediamosa_server_db::SERVER_STATUS,
        ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
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
  static public function get_objectcode_streaming($container_type, $video_codec) {

    $a_server = self::get_streaming($container_type, $video_codec);

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
  static public function get_uri_streaming($container_type, $video_codec) {

    $a_server = self::get_streaming($container_type, $video_codec);

    if ($a_server) {
      return $a_server[mediamosa_server_db::URI];
    }

    return FALSE;
  }

  /**
   * Get the default still server.
   */
  static public function get_still() {
    static $a_server = NULL;

    if (isset($a_server)) {
      return $a_server;
    }

    $a_server = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_server} WHERE #server_type = :server_type AND #server_status = :server_status_on LIMIT 1',
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_STILL,
        '#server_status' => mediamosa_server_db::SERVER_STATUS,
        ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
      )
    )->fetchAssoc();

    return $a_server;
  }

  /**
   * Get the URI of the default still server.
   */
  static public function get_uri_still() {
    $a_server = self::get_still();

    return $a_server[mediamosa_server_db::URI];
  }

  /**
   * Basic search, for now return all.
   */
  static public function search() {
    return mediamosa_db::db_select(mediamosa_server_db::TABLE_NAME, 'ms')
      ->fields('ms')
      ->execute();
  }

}
