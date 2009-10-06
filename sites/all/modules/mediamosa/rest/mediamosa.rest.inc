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
 * The REST class object contains the REST layer;
 * translates uri into rest call functions and handles all out going data
 * and errors.
 */
class mediamosa_rest {
  // ------------------------------------------------------------------ Consts


  // ------------------------------------------------------------------ Members
  private $enable_interface = FALSE;
  private $enable_restcalls = FALSE;
  private $enable_restcalls_upload = FALSE;

  // ------------------------------------------------------------------ Constructor
  function __construct() {
    $this->set_interface();
    $this->set_restcalls();

    if (!$this->is_interface() && !$this->is_restcalls()) {
      throw new mediamosa_exception_no_rest_interface();
    }
  }

  // ------------------------------------------------------------------ Static Functions
  static function get_uri_definition($s_request_uri, $request_method = FALSE) {

    if (!$request_method) {
      $request_method = mediamosa_response::get_method();
    }

    // Find all REST calls of enabled modules.
    $a_rest_calls = self::get_rest_calls();

    // Split the current request URI.
    $a_request_uri = explode('/', $s_request_uri);

    $a_rest_call_return = FALSE;

    // Now find a match.
    foreach ($a_rest_calls as $uri => $a_rest_call) {
      $a_uri_parts = ($uri != '' ? explode('/', $uri) : array());

      if (count($a_request_uri) != count($a_uri_parts)) {
        continue;
      }

      if (!count($a_request_uri) && !count($a_uri_parts)) {
        $a_rest_call[mediamosa_rest_call::URI] = $uri;
        $a_rest_call_return = $a_rest_call;
        break;
      }

      $a_regex = array();
      foreach ($a_uri_parts as $pos => $s_uri_part) {
        if (mediamosa_unicode::substr($s_uri_part, 0, 2) == '#$') {
          $a_regex[] = "[0-9]*(:[a-zA-Z]+)?";
          $a_rest_call[mediamosa_rest_call::URI_PARAMS][mediamosa_unicode::substr($s_uri_part, 2)] = $a_request_uri[$pos];
        }
        elseif (mediamosa_unicode::substr($s_uri_part, 0, 1) == '$') {
          $a_regex[] = "[A-Za-z0-9:-]*";
          $a_rest_call[mediamosa_rest_call::URI_PARAMS][mediamosa_unicode::substr($s_uri_part, 1)] = $a_request_uri[$pos];
        }
        else {
          $a_regex[] = $s_uri_part;
        }
      }

      $s_regex = '@^' . implode('/', $a_regex) . '$@';
      if (preg_match($s_regex, $s_request_uri)) {
        $a_rest_call[mediamosa_rest_call::URI] = $uri;
        $a_rest_call_return = $a_rest_call;
        break;
      }
    }

    // If we return the found REST call, then make sure all defaults are set.
    if ($a_rest_call_return !== FALSE) {
      // Make sure all are valid values.
      assert(in_array($a_rest_call_return[mediamosa_rest_call::METHOD], array(mediamosa_rest_call::METHOD_GET, mediamosa_rest_call::METHOD_DELETE, mediamosa_rest_call::METHOD_POST, mediamosa_rest_call::METHOD_PUT)));
      assert(in_array($a_rest_call_return[mediamosa_rest_call::STATUS], array(mediamosa_rest_call::STATUS_ACTIVE, mediamosa_rest_call::STATUS_DEPRECATED, mediamosa_rest_call::METHOD_POST, mediamosa_rest_call::STATUS_DISABLED)));
      assert(isset($a_rest_call_return[mediamosa_rest_call::CLASS_NAME]));
      assert(isset($a_rest_call_return[mediamosa_rest_call::MODULE_NAME]));
      assert(isset($a_rest_call_return[mediamosa_rest_call::VERSION]));

      // Default values.
      if (!isset($a_rest_call_return[mediamosa_rest_call::METHOD])) {
        $a_rest_call_return[mediamosa_rest_call::METHOD] = mediamosa_rest_call::METHOD_GET;
      }
      if (!isset($a_rest_call_return[mediamosa_rest_call::STATUS])) {
        $a_rest_call_return[mediamosa_rest_call::STATUS] = mediamosa_rest_call::STATUS_ACTIVE;
      }
      if (!isset($a_rest_call_return[mediamosa_rest_call::RESPONSE_TYPE])) {
        $a_rest_call_return[mediamosa_rest_call::RESPONSE_TYPE] = mediamosa_rest_call::RESPONSE_TYPE_XML;
      }
    }

    return $a_rest_call_return;
  }
  // ------------------------------------------------------------------ Functions (Public)
  static public function get_rest_calls($limit = 0, $offset = 0, $key = mediamosa_rest_call::URI) {
    // Find all REST calls of enabled modules.
    $a_rest_calls = module_invoke_all('mediamosa_register_rest_call');
    $a_result = array();

    $pos = 1;
    foreach ($a_rest_calls as $uri => $a_rest_call) {
      if ($offset && $offset--) {
        $pos++;
        continue;
      }

      if (!isset($a_rest_call[mediamosa_rest_call::METHOD])) {
        $a_rest_call[mediamosa_rest_call::METHOD] = mediamosa_rest_call::METHOD_GET;
      }
      if (!isset($a_rest_call[mediamosa_rest_call::STATUS])) {
        $a_rest_call[mediamosa_rest_call::STATUS] = mediamosa_rest_call::STATUS_ACTIVE;
      }
      if (!isset($a_rest_call[mediamosa_rest_call::RESPONSE_TYPE])) {
        $a_rest_call[mediamosa_rest_call::RESPONSE_TYPE] = mediamosa_rest_call::RESPONSE_TYPE_XML;
      }

      $a_rest_call[mediamosa_rest_call::URI] = $uri;
      $a_rest_call[mediamosa_rest_call::ID] = $pos++;
      $a_result[$a_rest_call[$key]] = $a_rest_call;
      if ($limit && !--$limit) {
        break;
      }
    }

    return $a_result;
  }

  /**
   * Dispatch the REST request execution.
   */
  public function process_call() {
    // Get the mediamosa object.
    $o_mediamosa = mediamosa::get();

    // Translate the URI to possible REST call.
    $a_rest_call = self::get_uri_definition($o_mediamosa->get_request_uri());

    if ($a_rest_call == FALSE) {
      if (self::is_interface()) {
        return FALSE;
      }

      throw new mediamosa_exception_error(mediamosa_error::HTTP_NOT_FOUND);
    }

    // Add the matched uri to the response.
    $o_mediamosa->set_matched_uri($a_rest_call[mediamosa_rest_call::URI]);

    // Add the method uri to the response.
    $o_mediamosa->set_matched_method($a_rest_call[mediamosa_rest_call::METHOD]);

    // Create class from it.
    $o_rest_call = new $a_rest_call[mediamosa_rest_call::CLASS_NAME]($a_rest_call);

    // Process the REST call.
    $o_rest_call->process_call();

    // If offset was specified, then include it in the header.
    if ($o_rest_call->isset_param(mediamosa_rest_call::OFFSET)) {
      $o_mediamosa->item_offset = $o_rest_call->get_param_value(mediamosa_rest_call::OFFSET);
    }

    // Return the processed REST call.
    return $a_rest_call;
  }

  /**
   * Bypass Drupal 'user/' path collisions.
   *
   * @return boolean
   */
  public function show_drupal_login() {
    // Check if interface is enabled, if so, check if we are logged in.
    // If not logged in, allow /user/login/* url to show the login form
    if ($this->is_interface()) {
      $path_login = user_is_logged_in() ? '/user/logout' : '/user/login';
      return (mediamosa_unicode::substr($_SERVER['REQUEST_URI'], 0, mediamosa_unicode::strlen($path_login)) == $path_login);
    }

    return FALSE;
  }

  /**
   * Is the Drupal Interface enabled?
   * @return bool
   */
  public function is_interface() {
    return $this->enable_interface;
  }

  /**
   * Is the REST Interface enabled?
   *
   * @return bool
   */
  public function is_restcalls() {
    return $this->enable_restcalls;
  }

  /**
   * Decide weither the Drupal interface is active or not.
   */
  public function set_interface() {
    $this->enable_interface = getenv('ENABLE_INTERFACE') !== FALSE ? getenv('ENABLE_INTERFACE') == 'YES' : TRUE;
  }

  /**
   * Decide weither the REST interface is active or not.
   */
  public function set_restcalls() {
    global $db_prefix;

    $is_cli = php_sapi_name() == 'cli' ? FALSE : TRUE;
    $this->enable_restcalls = getenv('ENABLE_RESTCALLS') !== FALSE ? getenv('ENABLE_RESTCALLS') == 'YES' || getenv('ENABLE_RESTCALLS') == 'UPLOAD' : $is_cli;
    $this->enable_restcalls_upload = getenv('ENABLE_RESTCALLS') !== FALSE ? getenv('ENABLE_RESTCALLS') == 'UPLOAD' : $is_cli;

    // If we are running an unit test, then enable REST calls.
    if (preg_match('/simpletest\d+/', $db_prefix)) {
      $this->enable_restcalls = TRUE; // Override during unit tests
    }
  }
  // ------------------------------------------------------------------ Functions (Private)
}
