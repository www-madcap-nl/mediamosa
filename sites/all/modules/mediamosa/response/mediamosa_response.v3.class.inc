<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
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
 * Response object for usage of output format of 3.0.
 */

/**
 * Our class will reuse as much from version 2 as we can.
 */
class mediamosa_response_v3 extends mediamosa_response {

  /**
   * Returns TRUE or FALSE
   */
  static public function is_v3_output() {
    // Get the active version of current environment.
    $env_active_version = mediamosa::get_environment_setting(mediamosa::ENV_ACTIVE_VERSION);
    return ($env_active_version[mediamosa_version::MAJOR] >= 3);
  }

  /**
   * Generate output.
   */
  protected function generate_output() {
    // v2 specified, then let the v2 handle it. Note: will not fix output fixes like mediamosa_0 anymore(!).
    if (!self::is_v3_output()) {
      return parent::generate_output();
    }

    switch ($this->response_type) {
      case mediamosa_response::RESPONSE_TYPE_XML:

        // Render the template.
        $output = $this->generate_xml_v3();
        header('Connection: close');
        header('Content-Type: text/xml; charset=utf-8');
        header('Content-Length: ' . strlen($output));
        header('Date: ' . date('r'));
        exit($output);
    }

    // No handlers, fall back on v2 output.
    return parent::generate_output();
  }

  /**
   * Generate the XML output in v3 format.
   *
   * @return string
   */
  public function generate_xml_v3() {
    // Use dom as base.
    $doc = new DOMDocument('1.0', 'UTF-8');
    $doc->formatOutput = TRUE;

    // Response element.
    $dom_element_response = $doc->createElement('response');
    //$dom_element_response = $doc->createElementNS('http://a9.com/-/spec/opensearch/1.1/', 'response');

    // Add default attributes to response element.
    $version = mediamosa_version::get_current_version();
   // $root = $doc->createElementNS('http://a9.com/-/spec/opensearch/1.1/', 'opensearch');
   // $root = $doc->appendChild($root);

    $dom_element_response->setAttribute('version', $version[mediamosa_version::MAJOR] . '.' . $version[mediamosa_version::MINOR]);
    $dom_element_response->setAttributeNS('http://www.w3.org/2000/xmlns/' ,'xmlns:opensearch', 'http://a9.com/-/spec/opensearch/1.1/');

    // Header element.
    $dom_element_header = $doc->createElement('header');

    // Items element.
    $dom_element_items = $doc->createElement('items');

    // Add default elements to document.
    $doc->appendChild($dom_element_response);
    $dom_element_response->appendChild($dom_element_header);
    $dom_element_response->appendChild($dom_element_items);

    return $doc->saveXML();
  }
}
