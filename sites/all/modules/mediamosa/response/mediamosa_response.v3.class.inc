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
    $env_active_version = mediamosa::get_environment_setting(mediamosa::ENV_MEDIAMOSA_VERSION);
    return ($env_active_version[mediamosa_version::MAJOR] >= 3);
  }

  /**
   * Generate output.
   */
  protected function generate_output() {
    // v2 specified, then let the v2 handle it. Note: will not fix output fixes
    // like mediamosa_0 anymore(!).
    if (!self::is_v3_output()) {
      return parent::generate_output();
    }

    switch ($this->response_type) {
      case mediamosa_response::RESPONSE_TYPE_XML:
        // Render the template.
        $this->xml();
    }

    // No handlers, fall back on v2 output.
    return parent::generate_output();
  }

  /**
   * Generate the XML output in v3 format.
   *
   * @return string
   */
  public function xml() {
    // Get default template.
    $xml = self::render_template(drupal_get_path('module', 'mediamosa') . '/response/mediamosa_response.v3.tpl.inc');

    // Output the XML.
    header('Connection: close');
    header('Content-Type: text/xml; charset=utf-8');
    header('Content-Length: ' . strlen($xml));
    header('Date: ' . date('r'));
    exit($xml);
  }
}
