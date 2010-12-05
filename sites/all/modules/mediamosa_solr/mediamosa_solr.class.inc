<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
 * Our extended classes for Solr connections.
 */

// We need SolrPhpClient class.
if (!class_exists('Apache_Solr_Service')) {
  // If we haven't install properly, the requirements hook will tell us.
  // So ignore if we can't find it here.
  // Installing the mediamosa_solr module will fail if during install the class
  // isn't found either. So don't worry about the problem we are not loading
  // MediamosaApacheSolrService for registry indexing (drupal autoloader).
  if (!file_exists(dirname(__FILE__) . '/3rdparty/SolrPhpClient/Apache/Solr/Service.php')) {
    return;
  }

  include_once dirname(__FILE__) . '/3rdparty/SolrPhpClient/Apache/Solr/Service.php';
}

assert(class_exists('Apache_Solr_Service'));

/**
 * We will extend on the Apache Solr Service just for future enhancements.
 *
 * All specific MediaMosa functions must start with mediamosa (mediamosaSolr..)
 * to prevent double usage on lower class names in future.
 */
class MediaMosa_Apache_Solr_Service extends Apache_Solr_Service {

  // ------------------------------------------------------------------- Consts.
  const MEDIAMOSA_SOLR_DEFAULT_HOST = 'solr.mediamosa.local';
  const MEDIAMOSA_SOLR_DEFAULT_PORT = 8983;
  const MEDIAMOSA_SOLR_DEFAULT_PATH = '/mediamosa/';

  // -------------------------------------------------------------- Constructor.
  public function __construct($url) {

    if (!isset($url)) {
      $url = self::mediamosaGetUrl();
    }

    // Split up into host, port and path.
    $host = self::mediamosaGetHost($url);
    $port = self::mediamosaGetPort($url);
    $path = self::mediamosaGetPath($url);

    parent::__construct($host, $port, $path);
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Return the url for the solr servlet.
   */
  static public function mediamosaGetUrl() {
    $url = variable_get('mediamosa_solr_url', 'http://' . self::MEDIAMOSA_SOLR_DEFAULT_HOST . '/');

    // Parse the url into parts.
    $parse_url = parse_url($url);

    // Path is mandatory.
    if (isset($parse_url['path']) && $parse_url['path'] == '/') {
      unset($parse_url['path']);
    }

    // Add possible missing stuff.
    $parse_url += array(
      'host' => self::MEDIAMOSA_SOLR_DEFAULT_HOST,
      'port' => self::MEDIAMOSA_SOLR_DEFAULT_PORT,
      'path' => self::MEDIAMOSA_SOLR_DEFAULT_PATH,
    );

    // Solr only uses host, port and path.
    return 'http://' . $parse_url['host'] . ':' . $parse_url['port'] . '/' . trim($parse_url['path'], '/') . '/';
  }

  /**
   * Sets the URL in Drupal variable.
   */
  static public function mediamosaSetUrl($url) {
    // Parse the url into parts.
    $parse_url = parse_url($url);

    // Path is mandatory.
    if (isset($parse_url['path']) && $parse_url['path'] == '/') {
      unset($parse_url['path']);
    }

    // Add possible missing stuff.
    $parse_url += array(
      'host' => self::MEDIAMOSA_SOLR_DEFAULT_HOST,
      'port' => self::MEDIAMOSA_SOLR_DEFAULT_PORT,
      'path' => self::MEDIAMOSA_SOLR_DEFAULT_PATH,
    );

    // Solr only uses host, port and path.
    variable_set('mediamosa_solr_url', 'http://' . $parse_url['host'] . ':' . $parse_url['port'] . '/' . trim($parse_url['path'], '/') . '/');
  }

  /**
   * Return the host for the solr servlet.
   */
  static public function mediamosaGetHost($url = NULL) {
    $url =  isset($url) ? $url : self::mediamosaGetUrl();

    // Parse.
    return parse_url($url, PHP_URL_HOST);
  }

  /**
   * Return the port for the solr servlet.
   */
  static public function mediamosaGetPort($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Get me the port.
    $port = (int) parse_url($url, PHP_URL_PORT);

    // Port is optional in the URL.
    return $port > 0 ? $port : self::MEDIAMOSA_SOLR_DEFAULT_PORT;
  }

  /**
   * Return the path for the solr servlet.
   */
  static public function mediamosaGetPath($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Get me the port.
    $path = trim(parse_url($url, PHP_URL_PATH), '/');

    // Path must start and end with '/'.
    return !empty($path) ? '/' . $path . '/' : self::MEDIAMOSA_SOLR_DEFAULT_PATH;
  }

  /**
   * Ping Solr with either given params or
   * @param string $url
   */
  static public function mediamosaPing($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Create the solr connection.
    $mediamosa_apache_solr_service = new MediaMosa_Apache_Solr_Service($url);

    // Ping it.
    return $mediamosa_apache_solr_service->ping();
  }
}
