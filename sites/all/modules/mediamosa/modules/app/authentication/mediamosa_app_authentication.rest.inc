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
  * The REST calls of the Application Authentication.
  */

/**
 * URI: /login
 * Login onto MediaMosa.
 */
class mediamosa_rest_call_app_ident extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  const DBUS = 'dbus';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call_app_ident::DBUS] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => '@todo: add a description',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the dbus line.
    $dbus = $this->get_param_value(mediamosa_rest_call_app_ident::DBUS);

    // Login.
    $result = mediamosa_app_authentication::app_dbus_protocol($dbus);

    if (mediamosa_unicode::substr($result, 0, 5) != 'ERROR') {
      $o_mediamosa->set_result(mediamosa_response::SUCCESS, mediamosa_error::ERRORCODE_OKAY, 'DBUS authentication protocol');
    }
    else {
      $o_mediamosa->set_result(mediamosa_response::ERROR, mediamosa_error::ERRORCODE_DBUS_PROTOCOL_ERROR, 'DBUS ' . $result);
    }

    $o_mediamosa->add_item(array('dbus' => $result));
  }
}
