<?php
// $Id$


/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
 * The mediamosa_response class is used to output the response in XML or Plain format.
 */
class mediamosa_response extends mediamosa {

  // ------------------------------------------------------------------ Consts
  // Was ERRORMESSAGE_ERROR.
  const ERROR = 'error';
  const SUCCESS = 'success';

  // ------------------------------------------------------------------ Members
  // The stored response.
  private $response = array();
  private $response_success = FALSE;

  // The rest class object.
  protected $mediamosa_rest = NULL;

  // The request uri.
  protected $request_uri;
  protected $request_method;

  // Start in miliseconds when call was made.
  private $f_start_time = 0;

  // Content_type to use for output.
  private $content_type;

  // Public counters.
  public $item_count = 0;
  public $item_count_total = 0;
  public $item_offset = 0;

  // ------------------------------------------------------------------ Contructor
  public function __construct() {
    parent::__construct();

    // Save the time we start the rest response
    $this->f_start_time = microtime(TRUE);

    // Save the request URI.
    $this->request_uri = rawurldecode(mediamosa_unicode::substr(mediamosa::request_uri(), mediamosa_unicode::strlen(base_path())));
    $this->request_uri = $this->request_uri == '' ? '/' : $this->request_uri;

    // Save the method.
    $this->request_method = $_SERVER['REQUEST_METHOD'];

    // All ok, now reset the header.
    $this->reset();
  }

  // ------------------------------------------------------------------ Public Functions
  /**
   * Process possible rest call.
   */
  public function process_rest() {

    // If REST interface is disabled, then don't bother.
    if (!mediamosa::is_app_enabled()) {
      return;
    }

    // Default XML output.
    $response_type = mediamosa_rest_call::RESPONSE_TYPE_XML;

    try {
      // Set the response handler.
      $previous_handler = set_error_handler(array($this, 'mediamosa_error_handler_response'), E_ALL);

      // Create the REST interface class.
      $this->mediamosa_rest = new mediamosa_rest();

      // Find the rest call.
      $rest_call = $this->mediamosa_rest->match_call();

      // Rest call found and check if the REST and Admin interface are enabled and if we conflict with admin /user/login or /user/logout.
      if (!$rest_call || mediamosa::show_drupal_login()) {
        // Put back previous (drupal) handler.
        if (!empty($previous_handler)) {
          set_error_handler($previous_handler, E_ALL);
        }

        return; // At this point we override for (drupal) interface mode.
      }

      // At this point we take over the output from Drupal and will no longer
      // return to drupal.
      header(mediamosa_settings::X_MEDIAMOSA_VERSION . ': ' . mediamosa_version::get_current_version_str(TRUE));

      // Allow execution to continue even if the request gets canceled.
      // This might happen if the client is waiting for response and cancels.
      // Is enabled by default for POST rest calls.
      if ($rest_call[mediamosa_rest_call::NO_USER_ABORT]) {
        @ignore_user_abort(TRUE);
      }

      // Enable sessions.
      if (!drupal_session_started()) {
        drupal_session_start();
      }
      elseif (mediamosa::in_simpletest_sandbox()) {
        session_regenerate_id(FALSE);
      }

      // Make sure response type is set.
      if (!isset($rest_call[mediamosa_rest_call::RESPONSE_TYPE])) {
        $rest_call[mediamosa_rest_call::RESPONSE_TYPE] = mediamosa_rest_call::RESPONSE_TYPE_XML;
      }

      // Save it.
      $response_type = $rest_call[mediamosa_rest_call::RESPONSE_TYPE];

      // Now execute the call.
      $alt = $this->mediamosa_rest->process_call($rest_call);

      if ($alt) {
        $response_type = $alt;
      }
    }
    catch (mediamosa_exception_redirect_and_exit $e) {
      // Redirect to URI.
      header('Location: ' . $e->get_redirect_uri());

      // We redirect and exit.
      exit();
    }
    catch (mediamosa_exception_error $e) {
      // If we are in simpletest, we need to skip logging of mediamosa
      // exceptions, because these might be generated intentially for testing.
      // The test must always check the error response anyway and not rely on
      // logging here.
      if (!mediamosa::in_simpletest_sandbox()) {
        mediamosa_debug::error_log('mediamosa_exception_error caught; ' . $e->getMessage(), $e->getFile(), $e->getLine());
      }

      $e->mediamosa_exception_rest_response($this);
    }
    catch (PDOException $e) {
      mediamosa_debug::error_log('PDOException caught; ' . $e->getMessage() . ', trace; ' . $e->getTraceAsString(), $e->getFile(), $e->getLine());
      try {
        mediamosa_watchdog::log_export(strtr('PDOException caught file @file at line @line; @message, trace; @trace.', array('@file' => $e->getFile(), '@line' => $e->getLine(), '@message' => $e->getMessage(), '@trace' => $e->getTraceAsString())));
      }
      catch (Exception $e) {
        // ignore.
      }

      $this->set_result(self::ERROR, mediamosa_error::HTTP_INTERNAL_SERVER_ERROR, 'PDOException caught; ' . $e->getMessage() . ",\ntrace; " . $e->getTraceAsString() . ",\nQuery string; " . $e->query_string . ",\nArgs; " . print_r($e->args, TRUE));
    }
    catch (Exception $e) {
      mediamosa_debug::error_log('Exception caught; ' . $e->getMessage() . ', trace; ' . $e->getTraceAsString(), $e->getFile(), $e->getLine());
      try {
        mediamosa_watchdog::log_export(strtr('Exception caught file @file at line @line; @message, trace; @trace.', array('@file' => $e->getFile(), '@line' => $e->getLine(), '@message' => $e->getMessage(), '@trace' => $e->getTraceAsString())));
      }
      catch (Exception $e) {
        // ignore.
      }

      $this->set_result(self::ERROR, $e->getCode(), $e->getMessage());
    }

    // Make sure its set.
    $output = '';

    try {
      switch ($response_type) {
        case mediamosa_rest_call::RESPONSE_TYPE_PLAIN:
          header('Content-Type: text/plain; charset=utf-8');
          $output = $this->generate_plain_text();
          break;

        case mediamosa_rest_call::RESPONSE_TYPE_DOWNLOAD:
        case mediamosa_rest_call::RESPONSE_TYPE_STILL:

          if ($this->response['header']['request_result'] == self::ERROR) {
            header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found');
            $output = $this->response['header']['request_result_description'];
          }
          else {
            $output = '';
          }
          break;

        case mediamosa_rest_call::RESPONSE_TYPE_JSON:
          header('Content-Type: application/json; charset=utf-8');
          $output = $this->generate_json();
          break;

        case mediamosa_rest_call::RESPONSE_TYPE_ATOM:
          header('Content-Type: application/atom+xml; charset=utf-8');
          $output = $this->generate_atom();
          break;

        case mediamosa_rest_call::RESPONSE_TYPE_RSS:
          header('Content-Type: application/rss+xml; charset=utf-8');
          $output = $this->generate_rss();
          break;

        case mediamosa_rest_call::RESPONSE_TYPE_XML:
        default:
          header('Content-Type: text/xml; charset=utf-8');
          $output = $this->generate_xml();
          break;
      }
    }
    catch (Exception $e) {
      mediamosa_debug::error_log('Critical exception caught during output; ' . $e->getMessage());
      $this->set_result(self::ERROR, $e->getCode(), $e->getMessage());
    }

    // Stop and output our content.
    // Might run any added exit functions (drupal).
    exit($output);
  }

  /**
   * Reset the output buffers.
   */
  public function reset() {
    $errors = NULL;

    // We don't reset the errors if there are any.
    if (isset($this->response['errors'])) {
      $errors = $this->response['errors'];
    }

    /**
     * Because ERRORCODE_OKAY does not have a text, we do not retrieve it here.
     * This saves a call to the error table in the database when drupal mode is
     * active.
     */
    $this->response = array(
      'header' => array(
        'request_result' => self::SUCCESS,
        'request_result_id' => mediamosa_error::ERRORCODE_OKAY,
        'request_result_description' => '',//mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY),
        'request_uri' => '[' . $this->request_method . '] ' . (mediamosa_unicode::substr($this->request_uri, 0, mediamosa_unicode::strlen('internal')) == 'internal' ? 'Internal MediaMosa call' : $this->request_uri),
        'item_count' => 0,
        'version' =>mediamosa_version::MEDIAMOSA_VERSION,
      ),
      'items' => array()
    );

    // Reset setting.
    $this->response_success = FALSE;

    if ($errors) {
      $this->response['errors'] = $errors;
    }

    $this->item_count = $this->item_count_total = $this->item_offset = 0;
  }

  /**
   * Add one item to the output.
   *
   * @param array $a_item
   */
  public function add_item($item) {
    $this->response['items'][++$this->item_count] = $item;
  }

  /**
   * Add an error to the header.
   *
   * @param array/string $mixed_item
   */
  public function add_program_error($mixed_item) {

    if (!isset($this->response['errors'])) {
      $this->response['errors'] = array();
    }

    if (count($this->response['errors']) < 100) {
      $this->response['errors'][count($this->response['errors'])] = $mixed_item;
    }
    else {
      $this->response['errors'][100] = 'More than 100 errors, skipping.';
    }
  }

  /**
   * Set the header result.
   *
   * @param string $request_result
   * @param integer $request_result_id
   * @param string $request_result_description
   */
  public function set_result($request_result, $request_result_id, $request_result_description = FALSE) {
    $this->response['header']['request_result'] = $request_result;
    $this->response['header']['request_result_id'] = $request_result_id;
    $this->response['header']['request_result_description'] = $request_result_description !== FALSE ? $request_result_description : mediamosa_error::error_code_find_description($request_result_id);
  }

  /**
   * Set when the rest call was successful, but doesn't have any data.
   */
  public function set_result_okay() {
    self::set_result(self::SUCCESS, mediamosa_error::ERRORCODE_OKAY);
    $this->response_success = TRUE;
  }

  /**
   * For setting additional custom info in header.
   *
   * @param array/string $data
   * @param string $name
   */
  public function set_result_header_extra($mixed_data, $name = 'additional_info') {
    $this->response['header'][$name] = $mixed_data;
  }

  /**
   * Return the time in miliseconds of the duration of the call.
   *
   * @return integer
   */
  public function get_processed_time() {
    return round(microtime(TRUE) - $this->f_start_time, 4);
  }

  /**
   * Set the URI in the header.
   *
   * @param string $uri
   */
  public function set_matched_uri($uri) {
    $this->response['header']['request_matched_uri'] = (mediamosa_unicode::substr($uri, 0, 1) == '/' ? '' : '/') . $uri;
  }

  /**
   * Set the class in the header.
   *
   * @param string $uri
   */
  public function set_class($class_name) {
    $this->response['header']['request_class'] = $class_name;
  }

  /**
   * Set the method used in the header.
   *
   * @param string $method
   */
  public function set_matched_method($method) {
    $this->response['header']['request_matched_method'] = $method;
  }

  /**
   * Log the REST call.
   *
   */
  function log_event_restcall() {
    if (mediamosa_debug::is_debug()) {
      $this->response['header']['request_query_count'] = mediamosa_db::query_count();
    }

    if ($this->is_log_rest()) {
      mediamosa_statistics::log_event_restcall(
        '[' . $this->request_method . '] ' . $this->request_uri,
        $this->get_processed_time(),
        mediamosa_db::query_count()
      );
    }
  }

  /**
   * Generate output as plain text.
   *
   * @return string
   */
  public function generate_plain_text() {
    $output = '';

    for ($x = 1; $x <= $this->item_count; $x++) {
      $output .= $this->response['items'][$x]; // Just concat the data
    }

    $this->log_event_restcall();
    return $output;
  }

  /**
   * Return result as PHP array.
   *
   * @return string
   */
  public function generate_array() {

    $this->response['header']['request_process_time'] = $this->get_processed_time();

    $this->response['header']['item_count_total'] = (!$this->item_count_total ? $this->item_count : $this->item_count_total);
    $this->response['header']['item_offset'] = $this->item_offset;
    $this->response['header']['item_count'] = $this->item_count;

    if (!$this->item_count && $this->response['header']['request_result_id'] == mediamosa_error::ERRORCODE_OKAY && !$this->response_success) {
      self::set_result(self::SUCCESS, mediamosa_error::ERRORCODE_EMPTY_RESULT);
    }

    // Sort the header.
    ksort($this->response['header']);

    // Log the rest call.
    $this->log_event_restcall();

    // This return response, which is a array.
    return $this->response;
  }

  /**
   * Generate the JSON output.
   *
   * @return string
   */
  public function generate_json() {
    return drupal_json_encode($this->response['items']);
  }

  /**
   * Generate the Atom output.
   *
   * @return string
   */
  public function generate_atom() {
    // TODO
    return 'Atom: ' . $this->generate_xml();
  }

  /**
   * Generate the RSS output.
   *
   * @return string
   */
  public function generate_rss() {
    // TODO
    return 'RSS: ' . $this->generate_xml();
  }

  /**
   * Generate the XML output.
   *
   * @return string
   */
  public function generate_xml() {
    $this->response['header']['request_process_time'] = $this->get_processed_time();

    $this->response['header']['item_count_total'] = (!$this->item_count_total ? $this->item_count : $this->item_count_total);
    $this->response['header']['item_offset'] = $this->item_offset;
    $this->response['header']['item_count'] = $this->item_count;

    if (!$this->item_count && $this->response['header']['request_result_id'] == mediamosa_error::ERRORCODE_OKAY && !$this->response_success) {
      self::set_result(self::SUCCESS, mediamosa_error::ERRORCODE_EMPTY_RESULT);
    }

    // Sort the header.
    ksort($this->response['header']);

    // Log the rest call.
    $this->log_event_restcall();

    // Create the SimpleXML element.
    $xml_response = new SimpleXMLElement('<?xml version="1.0" encoding="utf-8" ?><response></response>');

    // Add the header node.
    $xml_response_header = $xml_response->addChild('header');

    // Add Items array.
    $xml_response_items = $xml_response->addChild('items');

    foreach ($this->response['header'] as $key => $value) {
      $this->generate_plain_xml_add_child($xml_response_header, $key, $value);
    }

    $item_counter = 0;

    if (!empty($this->response['errors'])) {
      $xml_response_errors = $xml_response_header->addChild('errors');

      foreach ($this->response['errors'] as $key => $error) {
        $xml_response_errors->addChild('error', htmlspecialchars($error));
      }
    }

    foreach ($this->response['items'] as $item) {
      $xml_response_items_item = $xml_response_items->addChild('item');
      $xml_response_items_item->addAttribute('id', ++$item_counter);

      // Process items with a maximum recursion depth (last parameter).
      //
      foreach ($item as $key => $value) {
        $this->generate_plain_xml_add_child($xml_response_items_item, $key, $value);
      }
    }

    $dom_document = new DOMDocument('1.0', 'UTF-8');
    $dom_document->formatOutput = TRUE;

    $dom_document_node = dom_import_simplexml($xml_response);
    $dom_document_node = $dom_document->importNode($dom_document_node, TRUE);
    $dom_document->appendChild($dom_document_node);
    return $dom_document->saveXML();
  }

  /**
   * Add child item to XML node.
   *
   * @param array $item
   * @param string $key
   * @param array/string $mixed_value
   * @param integer $max_depth
   * @return NULL or added item.
   */
  private function generate_plain_xml_add_child($item, $key, $mixed_value, $max_depth = 99) {
    if (!--$max_depth) {
      return NULL;
    }

    if (!is_array($mixed_value)) {
      $item_child = $item->addChild($key, htmlspecialchars($mixed_value));

      if ($key[0] == '#') {
        $attributes = @unserialize(mediamosa_unicode::substr($key, 1));
        if (is_array($attributes)) {
          foreach ($attributes as $attr_name => $attr_value) {
            assert(count($attr_name));
            $item_child->addAttribute($attr_name, $attr_value);
          }
        }
      }

      return $item_child;
    }

    $first_key = key($mixed_value);
    if (is_numeric($first_key)) {
      foreach ($mixed_value as $mixed_key => $value) {
        $item->addChild($key, htmlspecialchars($value));
      }

      return NULL;
    }

    if ($first_key[0] == '#') {
      foreach ($mixed_value as $mixed_key => $value) {
        $item_child = $this->generate_plain_xml_add_child($item, $key, $value, $max_depth);

        if ($mixed_key != '#0') {
          $attributes = @unserialize(mediamosa_unicode::substr($mixed_key, 1));
          if (is_array($attributes)) {
            foreach ($attributes as $attr_name => $attr_value) {
              assert(count($attr_name));
              $item_child->addAttribute($attr_name, $attr_value);
            }
          }
        }
      }

      return NULL;
    }

    $item_child = $item->addChild($key);
    foreach ($mixed_value as $key_child => $value_child) {
      $this->generate_plain_xml_add_child($item_child, $key_child, $value_child, $max_depth);
    }

    return $item_child;
  }

  /**
   * Replacement for the current error handler.
   *
   * @param string $error_level
   * @param string $message
   * @param string $filename
   * @param integer $line
   * @param string $context
   * @return boolean
   */
  public function mediamosa_error_handler_response($error_level, $message, $filename, $line, $context) {

    // Possible backtrace caller.
    $caller = NULL;

    if ($error_level) {
      // All these constants are documented at http://php.net/manual/en/errorfunc.constants.php.
      $a_types = array(
        E_ERROR => 'Error',
        E_WARNING => 'Warning',
        E_PARSE => 'Parse error',
        E_NOTICE => 'Notice',
        E_CORE_ERROR => 'Core error',
        E_CORE_WARNING => 'Core warning',
        E_COMPILE_ERROR => 'Compile error',
        E_COMPILE_WARNING => 'Compile warning',
        E_USER_ERROR => 'User error',
        E_USER_WARNING => 'User warning',
        E_USER_NOTICE => 'User notice',
        E_STRICT => 'Strict warning',
        E_RECOVERABLE_ERROR => 'Recoverable fatal error'
      );

      $caller = _drupal_get_last_caller(debug_backtrace());

      // We treat recoverable errors as fatal.
      $params = array(
        '@type' => (isset($a_types[$error_level]) ? $a_types[$error_level] : 'Unknown error'),
        '@message' => $message,
        '@function' => $caller['function'],
        '@file' => $caller['file'],
        '@line' => $caller['line']
      );

      // Add to XML output.
      $this->add_program_error(strtr('@type: @message, @function() in file @file on line @line.', $params));
    }

    // Call our parent error handler.
    return parent::mediamosa_error_handler($error_level, $message, $filename, $line, $context, $caller);
  }

  // ------------------------------------------------------------------ Static Function.
}
