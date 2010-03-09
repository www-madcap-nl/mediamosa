<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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

  // ------------------------------------------------------------------ Constructor.
  public function __construct() {
    $this->url = variable_get('mediamosa_connector_url', NULL);
    $this->cookie = variable_get('mediamosa_connector_cookie', NULL);
  }

  // ------------------------------------------------------------------ Public Functions.
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
    if ($response->xml->header->request_result_id != 1601) {
      return $response;
    }

    // Login again.
    if (!self::do_login()) {
      return FALSE;
    }

    // Try the request again.
    $response = self::do_request($uri, $options);

    // Check again.
    if ($response->xml->header->request_result_id != 1601) {
      return $response;
    }

    return FALSE;
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
   *       Note: only for POST and PUT calls.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 1.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 60 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   *
   */
  public function request_internal($uri, array $options = array()) {
    if (empty($this->url)) {
      watchdog('mediamosa connector', 'Unable to do internal call, connector not setup.');
      return;
    }

    // Try the request.
    return self::do_request('internal/' . $uri, $options);
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
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 1.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 60 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   *
   */
  public function request_internal_head($uri, array $options = array()) {

    if (empty($this->url)) {
      watchdog('mediamosa connector', 'Unable to do internal head call, connector not setup.');
      return;
    }

    // Add password.
    $options['headers'][mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS] = mediamosa::get_internal_password();

    $url = $this->url;
    $url .= (drupal_substr($url, -1, 1) == '/' ? '' : '/'); // Make sure it ends with '/'.
    $url .= 'internal/' . $uri;

    // Trigger.
    mediamosa_http::do_head_call($url, $options);
   }

  // ------------------------------------------------------------------ Private Functions.
  private function do_login() {
    // Remove cookie.
    $this->cookie = NULL;
    variable_del('mediamosa_connector_cookie');

    // get the authentication credentials
    $username = variable_get('mediamosa_connector_username', NULL);
    $password = variable_get('mediamosa_connector_password', NULL);

    // Step 1: request the challenge
    $data = array('dbus' => 'AUTH DBUS_COOKIE_SHA1 '. $username);
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
    list($this->cookie) = explode(";", $response->headers['Set-Cookie']);
    variable_set('mediamosa_connector_cookie', $this->cookie);

    // Set DBUS data.
    $dbus_data = explode(' ', $response->xml->items->item->dbus);

    // Step 2: Do challenge.
    $challenge = $dbus_data[3];
    $random = substr(md5(microtime(TRUE)), 0, 10);
    $challenge_response = sha1(sprintf('%s:%s:%s', $challenge, $random, $password));
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

    // If the URI starts with internal, we do an iternal call.
    $is_internal = substr($uri, 0, strlen('internal/')) == 'internal/';

    // Set cookie.
    if (isset($this->cookie) && !$is_internal) {
      $options['headers']['Cookie'] = $this->cookie;
    }

    // If we are internal calling, then we need to add the internal password so we have always access.
    if ($is_internal) {
      $options['headers'][mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS] = mediamosa::get_internal_password();
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

/*
  private $log = array();





  private function add_user_info($uri, &$data = NULL) {
    $uri .= (strpos($uri, '?') !== FALSE) ? '&' : '?';
    //If a user_id is passed with the URL then don't add the current user
    if (strpos($uri, 'user_id=') === FALSE) {
      $uri .= 'user_id='. $this->user_id;
    }
    if ($this->group_id != '') {
      $uri .= '&group_id='. $this->group_id;
    }
    if ($this->masquerade_owner || $this->is_app_admin) {
      if (strpos($uri, 'is_app_admin=TRUE') === FALSE) {
        $uri .= '&is_app_admin=TRUE';
      }
      if ($data) {
        if (is_array($data)) {
          $data['is_app_admin'] = 'TRUE';
        }
        else {
          $data .= '&is_app_admin=TRUE';
        }
      }
    }
    return $uri;
  }

  private function _check_login($xml) {
    preg_match('@<request_result_id>(.*)</request_result_id>@', $xml, $matches);
    return ($matches[1] != '1601'); // 1601 == not logged in
  }

  public function request($method, $uri, $data = NULL, $process_result = TRUE, $app_id = FALSE) {
    if ($this->rest) {
      // Add user info to the uri
      $uri = $this->add_user_info($uri);

      // first try to communicate with VP-X without logging in
      if (($response = $this->_request($method, $uri, $data)) === FALSE) { // request failed
        return FALSE;
      }
      $xml = $response->data;

      // parse the request_result_id of the response
      if ($this->_check_login($xml) === FALSE) {
        $this->_login();
        $response = $this->_request($method, $uri, $data); // perform the same request again
        $xml = $response->data;
      }

      // parse the request_result_id of the response
      if ($this->_check_login($xml) === FALSE) {
        return FALSE;
      }
    }
    else { // The non-REST, direct connection to VP-X
      // Add user info to the uri
      $uri = $this->add_user_info($uri, $data);

      // Gather all GET and POST parameters
      $a_uri = explode('?', $uri);
      $uri = substr($a_uri[0], 1);
      (isset($a_uri[1])) ? $get = explode('&', $a_uri[1]) : $get = array();

      $args = array();
      foreach ($get as $get_item) {
        $a_get_item = explode('=', $get_item);
        if (strpos($a_get_item[0], '[]') === strlen($a_get_item[0]) - 2) { // detect array notation
          $args['get'][substr($a_get_item[0], 0, -2)][] = rawurldecode($a_get_item[1]);
        }
        else {
          $args['get'][$a_get_item[0]] = rawurldecode($a_get_item[1]);
        }
      }
      if ($data) {
        if (is_array($data)) {
          $args['post'] = $data;
        }
        else {
          $post = explode('&', $data);

          foreach ($post as $post_item) {
            $a_post_item = explode('=', $post_item);
            if (strpos($a_post_item[0], '[]') === strlen($a_post_item[0]) - 2) { // detect array notation
              $args['post'][substr($a_post_item[0], 0, -2)][] = rawurldecode($a_post_item[1]);
            }
            else {
              $args['post'][$a_post_item[0]] = rawurldecode($a_post_item[1]);
            }
          }
        }
      }

      // Get all app_id's available for this user
      if ($app_id === FALSE || !$this->masquerade_owner) {

        global $user;

        $resource = db_query("SELECT id, owner FROM {client_applications} WHERE owner IN ('', '". implode("', '", $user->roles) ."')");
        $app_ids = array();

        // Filter out test apps when we are not super user (uid == 1)
        $app_ids_to_skip = ($user->uid == 1 ? array() : mediamosa_shared_is_simpletest_app_get());

        while ($result = db_fetch_array($resource)) {
          if (!in_array($result['id'], $app_ids_to_skip)) {
            $app_ids[] = $result['id'];
          }
        }

        $args['get']['app_id'] = $app_ids;
      }
      else {
        $args['get']['app_id'] = $app_id;
      }

      // Get the MadREST definition
      $madrest_definition = madrest_get_uri_definition($uri, explode('/', $uri), $args['uri'], $method);

      // Perform the request
      $start = microtime(TRUE);
      $output = call_user_func($madrest_definition['function_call'], $args);
      $xml = $output->create_output($start);

      // Log the request
      if (variable_get('mediamosa_connector_debug', FALSE)) {
        $this->log[] = array(
          'uri' => $madrest_definition['function_call'],
          'method' => $method,
          'response_time' => round(microtime(TRUE) - $start, 3),
        );
        $_SESSION['mediamosa_connector_debug_info'] = $this->log;
      }
    }

    // return the result
    if ($process_result) {
      try {
        $ret = new SimpleXMLElement($xml);
      }
      catch (Exception $e) {
        watchdog('mediamosa_connector', "Parse XML: ". $e->getMessage() ." (". print_r($xml, TRUE) .").");
        drupal_set_message("Parse XML: ". $e->getMessage() ." (". print_r($xml, TRUE) .").");
      }
      return $ret;
    }
    return $xml;
  }

  public function check_result($result, $ok = FALSE, $error = FALSE) {
    $result_id = (int)$result->header->request_result_id;
    if ($result_id != 601 && $result_id != 705) {
      if ($error) {
        $error = t($error, array('%code' => $result_id, '%message' => $result->header->request_result_description));
        drupal_set_message($error, 'error');
      }
      return FALSE;
    }
    else {
      if ($ok) {
        drupal_set_message(t($ok));
      }
      return TRUE;
    }
  }*/
}
