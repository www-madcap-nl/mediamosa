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
 * HTTP related functions.
 */
class mediamosa_http {

  const METHOD_GET = 'GET';
  const METHOD_POST = 'POST';

  /**
   * Do a internal REST call.
   *
   * @param string $ip_address
   * @param string $rest_url
   */
  static public function do_internal_call($ip_address, $rest_url, $data = NULL, $method = self::METHOD_GET) {
    $url = strtr(
      'http://@ipaddress/@build_url/internal/@rest_url',
      array(
        '@ipaddress' => $ip_address,
        '@build_url' => MEDIAMOSA_BUILD_URL,
        '@rest_url' =>  $rest_url
      )
    );

    $a_options = array('method' => $method);

    if (isset($data)) {
      $a_options['data'] = $data;
    }

    // Include the internal password so we gain access.
    $a_options['headers'][mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS] = mediamosa::get_internal_password();

    // Do the request.
    return self::drupal_http_request($url, $a_options);
  }

  /**
   * Perform an HTTP request.
   *
   * Copy of the Drupal HTTP Request.
   * It overwrites our 'Host:' entry in our headers definition, old versions
   * worked fine. We need 'Host:' todo localhost calls with Host name
   * specified in header. We can not always call directly on hostname because
   * of firewall, so we call localhost with hostname in header instead.
   *
   *
   *
   * This is a flexible and powerful HTTP client implementation. Correctly
   * handles GET, POST, PUT or any other HTTP requests. Handles redirects.
   *
   * @param $url
   *   A string containing a fully qualified URI.
   * @param $options
   *   (optional) An array which can have one or more of following keys:
   *   - headers
   *       An array containing request headers to send as name/value pairs.
   *   - method
   *       A string containing the request method. Defaults to 'GET'.
   *   - data
   *       A string containing the request body. Defaults to NULL.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 3.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 30 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   * @return
   *   An object which can have one or more of the following parameters:
   *   - request
   *       A string containing the request body that was sent.
   *   - code
   *       An integer containing the response status code, or the error code if
   *       an error occurred.
   *   - protocol
   *       The response protocol (e.g. HTTP/1.1 or HTTP/1.0).
   *   - status_message
   *       The status message from the response, if a response was received.
   *   - redirect_code
   *       If redirected, an integer containing the initial response status code.
   *   - redirect_url
   *       If redirected, a string containing the redirection location.
   *   - error
   *       If an error occurred, the error message. Otherwise not set.
   *   - headers
   *       An array containing the response headers as name/value pairs.
   *   - data
   *       A string containing the response body that was received.
   */
  function drupal_http_request($url, array $options = array()) {
    global $db_prefix;

    $result = new stdClass();

    // Parse the URL and make sure we can handle the schema.
    $uri = @parse_url($url);

    if ($uri == FALSE) {
      $result->error = 'unable to parse URL';
      $result->code = -1001;
      return $result;
    }

    if (!isset($uri['scheme'])) {
      $result->error = 'missing schema';
      $result->code = -1002;
      return $result;
    }

    timer_start(__FUNCTION__);

    // Merge the default options.
    $options += array(
      'headers' => array(),
      'method' => 'GET',
      'data' => NULL,
      'max_redirects' => 3,
      'timeout' => 30,
    );

    switch ($uri['scheme']) {
      case 'http':
        $port = isset($uri['port']) ? $uri['port'] : 80;
        $host = $uri['host'] . ($port != 80 ? ':' . $port : '');
        $fp = @fsockopen($uri['host'], $port, $errno, $errstr, $options['timeout']);
        break;
      case 'https':
        // Note: Only works when PHP is compiled with OpenSSL support.
        $port = isset($uri['port']) ? $uri['port'] : 443;
        $host = $uri['host'] . ($port != 443 ? ':' . $port : '');
        $fp = @fsockopen('ssl://' . $uri['host'], $port, $errno, $errstr, $options['timeout']);
        break;
      default:
        $result->error = 'invalid schema ' . $uri['scheme'];
        $result->code = -1003;
        return $result;
    }

    // Make sure the socket opened properly.
    if (!$fp) {
      // When a network error occurs, we use a negative number so it does not
      // clash with the HTTP status codes.
      $result->code = -$errno;
      $result->error = trim($errstr);

      // Mark that this request failed. This will trigger a check of the web
      // server's ability to make outgoing HTTP requests the next time that
      // requirements checking is performed.
      // @see system_requirements()
      variable_set('drupal_http_request_fails', TRUE);

      return $result;
    }

    // Construct the path to act on.
    $path = isset($uri['path']) ? $uri['path'] : '/';
    if (isset($uri['query'])) {
      $path .= '?' . $uri['query'];
    }

    // Merge the default headers.
    $options['headers'] += array(
      'User-Agent' => 'Drupal (+http://drupal.org/)',
    );

    // RFC 2616: "non-standard ports MUST, default ports MAY be included".
    // We don't add the standard port to prevent from breaking rewrite rules
    // checking the host that do not take into account the port number.
    if (!isset($options['headers']['Host'])) { // HOTFIX Mediamosa.
      $options['headers']['Host'] = $host;
    }
    // Only add Content-Length if we actually have any content or if it is a POST
    // or PUT request. Some non-standard servers get confused by Content-Length in
    // at least HEAD/GET requests, and Squid always requires Content-Length in
    // POST/PUT requests.
    $content_length = strlen($options['data']);
    if ($content_length > 0 || $options['method'] == 'POST' || $options['method'] == 'PUT') {
      $options['headers']['Content-Length'] = $content_length;
    }

    // If the server URL has a user then attempt to use basic authentication.
    if (isset($uri['user'])) {
      $options['headers']['Authorization'] = 'Basic ' . base64_encode($uri['user'] . (!empty($uri['pass']) ? ":" . $uri['pass'] : ''));
    }

    // If the database prefix is being used by SimpleTest to run the tests in a copied
    // database then set the user-agent header to the database prefix so that any
    // calls to other Drupal pages will run the SimpleTest prefixed database. The
    // user-agent is used to ensure that multiple testing sessions running at the
    // same time won't interfere with each other as they would if the database
    // prefix were stored statically in a file or database variable.
    if (is_string($db_prefix) && preg_match("/simpletest\\d+/", $db_prefix, $matches)) {
      $options['headers']['User-Agent'] = drupal_generate_test_ua($matches[0]);
    }

    $request = $options['method'] . ' ' . $path . " HTTP/1.0\r\n";
    foreach ($options['headers'] as $name => $value) {
      $request .= $name . ': ' . trim($value) . "\r\n";
    }
    $request .= "\r\n" . $options['data'];
    $result->request = $request;

    fwrite($fp, $request);

    // Fetch response.
    $response = '';
    while (!feof($fp)) {
      // Calculate how much time is left of the original timeout value.
      $timeout = $options['timeout'] - timer_read(__FUNCTION__) / 1000;
      if ($timeout <= 0) {
        $result->code = HTTP_REQUEST_TIMEOUT;
        $result->error = 'request timed out';
        return $result;
      }
      stream_set_timeout($fp, floor($timeout), floor(1000000 * fmod($timeout, 1)));
      $response .= fread($fp, 1024);
    }
    fclose($fp);

    // Parse response headers from the response body.
    list($response, $result->data) = explode("\r\n\r\n", $response, 2);
    $response = preg_split("/\r\n|\n|\r/", $response);

    // Parse the response status line.
    list($protocol, $code, $status_message) = explode(' ', trim(array_shift($response)), 3);
    $result->protocol = $protocol;
    $result->status_message = $status_message;

    $result->headers = array();

    // Parse the response headers.
    while (TRUE) {
      $line = trim(array_shift($response));

      if (!$line) {
        break;
      }

      list($header, $value) = explode(':', $line, 2);
      if (isset($result->headers[$header]) && $header == 'Set-Cookie') {
        // RFC 2109: the Set-Cookie response header comprises the token Set-
        // Cookie:, followed by a comma-separated list of one or more cookies.
        $result->headers[$header] .= ',' . trim($value);
      }
      else {
        $result->headers[$header] = trim($value);
      }
    }

    $responses = array(
      100 => 'Continue',
      101 => 'Switching Protocols',
      200 => 'OK',
      201 => 'Created',
      202 => 'Accepted',
      203 => 'Non-Authoritative Information',
      204 => 'No Content',
      205 => 'Reset Content',
      206 => 'Partial Content',
      300 => 'Multiple Choices',
      301 => 'Moved Permanently',
      302 => 'Found',
      303 => 'See Other',
      304 => 'Not Modified',
      305 => 'Use Proxy',
      307 => 'Temporary Redirect',
      400 => 'Bad Request',
      401 => 'Unauthorized',
      402 => 'Payment Required',
      403 => 'Forbidden',
      404 => 'Not Found',
      405 => 'Method Not Allowed',
      406 => 'Not Acceptable',
      407 => 'Proxy Authentication Required',
      408 => 'Request Time-out',
      409 => 'Conflict',
      410 => 'Gone',
      411 => 'Length Required',
      412 => 'Precondition Failed',
      413 => 'Request Entity Too Large',
      414 => 'Request-URI Too Large',
      415 => 'Unsupported Media Type',
      416 => 'Requested range not satisfiable',
      417 => 'Expectation Failed',
      500 => 'Internal Server Error',
      501 => 'Not Implemented',
      502 => 'Bad Gateway',
      503 => 'Service Unavailable',
      504 => 'Gateway Time-out',
      505 => 'HTTP Version not supported',
    );
    // RFC 2616 states that all unknown HTTP codes must be treated the same as the
    // base code in their class.
    if (!isset($responses[$code])) {
      $code = floor($code / 100) * 100;
    }
    $result->code = $code;

    switch ($code) {
      case 200: // OK
      case 304: // Not modified
        break;
      case 301: // Moved permanently
      case 302: // Moved temporarily
      case 307: // Moved temporarily
        $location = $result->headers['Location'];
        $options['timeout'] -= timer_read(__FUNCTION__) / 1000;
        if ($options['timeout'] <= 0) {
          $result->code = HTTP_REQUEST_TIMEOUT;
          $result->error = 'request timed out';
        }
        elseif ($options['max_redirects']) {
          // Redirect to the new location.
          $options['max_redirects']--;
          $result = self::drupal_http_request($location, $options);
          $result->redirect_code = $code;
        }
        $result->redirect_url = $location;
        break;
      default:
        $result->error = $status_message;
    }

    return $result;
  }
}
