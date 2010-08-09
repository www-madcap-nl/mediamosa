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
  * REST calls for mediamosa_server.
  */
/**
 * URI: /server/status
 * Method: GET
 *
 * Internal only.
 *
 */
class mediamosa_rest_call_server_status extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const INCLUDE_STATUS = 'include_status';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::INCLUDE_STATUS => array(
          self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include status information.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      )
    );

    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    //global $conf;

    $mediamosa = mediamosa::get();

    $include_status = $this->get_param_value(self::INCLUDE_STATUS);

    // Fill up info about the server.
    $item = array(
      'server_id' => mediamosa::get_server_id(),
      'uri' => mediamosa::get_request_uri(),
      'host' => $_SERVER['HTTP_HOST'],
    );

    if ($include_status) {
      // Check run-time requirements and status information.
      // Reset = TRUE, With Drupal variable = FALSE.
      $statuses = module_invoke_all('mediamosa_status_collect', TRUE, FALSE);

      $item['server_status'] = $statuses;
    }

    $mediamosa->add_item($item);
  }
}
