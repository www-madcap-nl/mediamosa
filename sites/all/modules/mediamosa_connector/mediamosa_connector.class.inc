<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
 * @file
 * This module contains the login procedure, cookie handling and basic GET
 * and POST functions to communicate with VP-X.
 */

/**
 * mediamosa_connector is a class which conains all functions for communication with VP-X
 */
class mediamosa_connector {

  // ------------------------------------------------------------------ Consts.
  // Maximum of items in result.
  const MAX_RESULT_SET = 200;

  // Maximum of entries in our log, lets not overflow memory.
  // Log will store the last # of requests and will drop the first.
  const MAX_LOG_ENTRIES = 250;

  // ------------------------------------------------------------------ Members.
  // The URL to connect to.
  private $url;

  // The cookie for the MediaMosa session.
  private $cookie;

  // The user to use.
  private $user_id;

  // The group, optional.
  private $group_id;
  private $is_app_admin;
  private $log = array();

  // Stored username, password.
  private $username;
  private $password;

  // -------------------------------------------------------------- Constructor.
  public function __construct($username = NULL, $password = NULL, $url = NULL) {
    $this->url = isset($url) ? $url : variable_get('mediamosa_connector_url', NULL);
    $cookies = variable_get('mediamosa_connector_cookies', array());

    // Get cookie.
    $this->cookie = !empty($cookies[$this->url][$this->username][$this->password]) ? $cookies[$this->url][$this->username][$this->password] : NULL;

    $this->username = isset($username) ? $username : variable_get('mediamosa_connector_username', NULL);
    $this->password = isset($password) ? $password : variable_get('mediamosa_connector_password', NULL);
  }

  // --------------------------------------------------------- Public Functions.
  /**
   * @param string $uri
   *  The REST uri.
   * @param array $options
   *   (optional) An array which can have one or more of following keys:
   *   - headers
   *       An array containing request headers to send as name/value pairs.
   *   - method
   *       A string containing the request method. Defaults to 'GET'.
   *   - data
   *       A string containing the request body.
   *       Or
   *       A array as name => value.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 1.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 60 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   *
   */
  public function request($uri, array $options = array()) {

    if (empty($this->url)) {
      watchdog('mediamosa connector', 'Unable to do request, connector not setup.');
      return;
    }

    // If cookie not set, then login first.
    if (!isset($this->cookie)) {
      if (!self::do_login()) {
        return FALSE;
      }
    }

    // Try the request.
    $response = self::do_request($uri, $options);

    // If we return anything else but 1601, then return result. 1601 = failure login, cookie expired etc.
    if ($response && !in_array($response->xml->header->request_result_id, array(mediamosa_error::ERRORCODE_ACCESS_DENIED, mediamosa_error::ERRORCODE_NOT_AUTHORIZED))) {
      return $response;
    }

    // Login again.
    if (!self::do_login()) {
      return FALSE;
    }

    // Try the request again.
    $response = self::do_request($uri, $options);

    // Check again.
    if ($response && $response->xml->header->request_result_id != 1601) {
      return $response;
    }

    return FALSE;
  }

  // -------------------------------------------------------- Private Functions.
  private function do_login() {
    if (empty($this->url)) {
      watchdog('mediamosa connector', 'Unable to do request, connector not setup.');
      return FALSE;
    }

    // Remove cookie.
    $this->cookie = NULL;
    $cookies = variable_get('mediamosa_connector_cookies', array());
    if (isset($cookies[$this->url])) {
      unset($cookies[$this->url]);
      variable_set('mediamosa_connector_cookies', $cookies);
    }

    // Step 1: request the challenge
    $data = array('dbus' => 'AUTH DBUS_COOKIE_SHA1 '. $this->username);
    $method = 'POST';
    $response = $this->do_request('login', array('method' => $method, 'data' => $data));

    if ($response->xml->header->request_result_id == 404) {
      drupal_set_message(t('Enable the Application Authentication module, unable to login. '), 'error');
      return FALSE;
    }

    if (!$response || !$response->xml) {
      return FALSE;
    }

    // Check if we got DATA response.
    if (drupal_substr($response->xml->items->item->dbus, 0, 5) != 'DATA ') {
      return FALSE;
    }

    // Parse the response.
    list($this->cookie) = explode(';', $response->headers['Set-Cookie']);
    $cookies[$this->url] = $this->cookie;
    variable_set('mediamosa_connector_cookies', $cookies);

    // Set DBUS data.
    $dbus_data = explode(' ', $response->xml->items->item->dbus);

    // Step 2: Do challenge.
    $challenge = $dbus_data[3];
    $random = substr(md5(microtime(TRUE)), 0, 10);
    $challenge_response = sha1(sprintf('%s:%s:%s', $challenge, $random, $this->password));
    $data = array('dbus' => sprintf('DATA %s %s', $random, $challenge_response));
    $method = 'POST';
    $response = $this->do_request('login', array('method' => $method, 'data' => $data));
    if (!$response || !$response->xml) {
      return FALSE;
    }

    // Lets check if its ok.
    return drupal_substr($response->xml->items->item->dbus, 0, 2) == 'OK';
  }

  /**
   * @param string $uri
   *  The REST uri.
   * @param array $options
   *   (optional) An array which can have one or more of following keys:
   *   - headers
   *       An array containing request headers to send as name/value pairs.
   *   - method
   *       A string containing the request method. Defaults to 'GET'.
   *   - data
   *       A string containing the request body.
   *       Or
   *       A array as name => value.
   *       Note: The request body is only used on POST and PUT calls.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 1.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 60 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   */
  private function do_request($uri, array $options = array()) {
    // Merge the default options.
    $options += array(
      'headers' => array(),
      'method' => 'GET',
      'data' => NULL,
      'max_redirects' => 1, // REST call should not redirect more than 1.
      'timeout' => 60,
    );

    // Set cookie.
    if (isset($this->cookie)) {
      $options['headers']['Cookie'] = $this->cookie;
    }

    $data_log = '';
    if (isset($options['data'])) {
      if (is_array($options['data'])) {
        $data_log = ' ('. count($options['data']) .')';
        $options['data'] = http_build_query($options['data'], '', '&');
      }
      else {
        $data_log = ' ('. count(explode('&', $options['data'])) .')';
      }
    }

    $url = $this->url;
    $url .= (drupal_substr($url, -1, 1) == '/' ? '' : '/'); // Make sure it ends with '/'.

    switch ($options['method']) {
      case 'GET':
        // Method GET does not allow data body, put the urlencode after the uri.
        $query_string_pos = strpos($uri, '?');
        $query_string = array();
        if ($query_string_pos !== FALSE) {
          $query_string[] = substr($uri, $query_string_pos + 1);

          // Strip it.
          $uri = substr($uri, 0, $query_string_pos);
        }

        // If data is set, then add it to the query string.
        if (isset($options['data']) && $options['data'] != '') {
          $query_string[] = $options['data'];
        }

        // Quick fix for now.
        unset($options['data']);

        // Now add to uri (if any).
        if (count($query_string)) {
          $uri .= '?' . implode('&', $query_string);
        }
        break;

      case 'POST':
        // POST needs data as www-form-urlencoded.
        $options['headers']['Content-Type'] = 'application/x-www-form-urlencoded';
        break;
    }

    // Add clean uri.
    $url .= $uri;

    $start = microtime(TRUE);
    $response = mediamosa_http::drupal_http_request($url, $options);
    $end = microtime(TRUE);

    // If the log size overflows our buffer, lose the first.
    if (count($this->log) > self::MAX_LOG_ENTRIES) {
      array_shift($this->log);
    }

    // Log the request.
    $this->log[] = array(
      'method' => $options['method'] . $data_log,
      'url' => $url,
      'response_time' => round($end - $start, 3)
    );

    if (variable_get('mediamosa_connector_debug')) {
      $_SESSION['mediamosa_connector_debug_info'] = $this->log;
    }

    // Must be HTTP 200 response.
    if ((int)$response->code != 200) {
      return FALSE;
    }

    // Set to empty.
    $response->xml = NULL;

    // If we detect xml output, fill the xml member.
    if (strpos($response->headers['Content-Type'], 'text/xml') !== FALSE || drupal_substr($response->data, 0, strlen('<?xml')) == '<?xml') {
      try {
        $response->xml = new SimpleXMLElement($response->data);
      }
      catch (Exception $e) {
      }
    }

    // Return result.
    return $response;
  }
}
