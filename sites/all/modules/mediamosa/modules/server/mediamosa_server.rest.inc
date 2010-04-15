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
 * URI: /job/$job_id/status
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_jobs_set_job_status
 *
 */
class mediamosa_rest_call_server_status extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Fill up info about the server.
    $a_item = array(
      'server_id' => mediamosa::get_server_id(),
      'uri' => mediamosa::get_request_uri(),
      'host' => $_SERVER['HTTP_HOST'],
    );

    $o_mediamosa->add_item($a_item);
  }
}
