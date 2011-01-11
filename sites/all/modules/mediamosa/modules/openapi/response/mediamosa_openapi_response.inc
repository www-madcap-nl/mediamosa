<?php
// $Id$


/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
class mediamosa_openapi_response extends mediamosa_response {
  // ------------------------------------------------------------------ Public Functions
  /**
   * Process possible rest call.
   */
  public function process_rest() {

    // If REST interface is disabled, then don't bother.
    if (!mediamosa::is_app_enabled()) {
      return;
    }

    // Default output.
    $response_type = variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_DEFAULT_RESPONSE_TYPE, mediamosa_rest_call::RESPONSE_TYPE_ATOM);

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

      // Change the response type depending the HTTP header.
      if (!empty($_SERVER[mediamosa_settings::MEDIAMOSA_OPENAPI_HEADER_HTTP_ACCEPT_TYPE])) {
        $response_type = $_SERVER[mediamosa_settings::MEDIAMOSA_OPENAPI_HEADER_HTTP_ACCEPT_TYPE];
      }
      elseif (!empty($_SERVER[mediamosa_settings::MEDIAMOSA_OPENAPI_HEADER_HTTP_ACCEPT])) {
        $response_type = $_SERVER[mediamosa_settings::MEDIAMOSA_OPENAPI_HEADER_HTTP_ACCEPT];
      }
      elseif (!empty($_SERVER[mediamosa_settings::MEDIAMOSA_OPENAPI_HEADER_ACCEPT_TYPE])) {
        $response_type = $_SERVER[mediamosa_settings::MEDIAMOSA_OPENAPI_HEADER_ACCEPT_TYPE];
      }

      // Now execute the call.
      $response_type_parameter = $this->mediamosa_rest->process_call($rest_call);
      // Check the response parameter.
      if ($response_type_parameter) {
        $response_type = $response_type_parameter;
      }
    }
    catch (mediamosa_exception_redirect_and_exit $e) {
      // Redirect to URI.
      header('Location: ' . $e->get_redirect_uri());

      // We redirect and exit.
      exit();
    }
    catch (mediamosa_exception_error_403 $e) {
      // Forbidden
      header($_SERVER['SERVER_PROTOCOL'] . ' 403 Forbidden');
      exit();
    }
    catch (mediamosa_exception_error_404 $e) {
      // Not dounf
      header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found');
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

    $output = $this->generate_output($response_type);

    // Stop and output our content.
    // Might run any added exit functions (drupal).
    exit($output);
  }

  /**
   * Generate the JSON output.
   *
   * @return string
   */
  public function generate_json() {
    return drupal_json_encode($this->response);
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
}
