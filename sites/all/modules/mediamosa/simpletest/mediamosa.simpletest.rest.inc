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
  * REST calls specific for use inside the simpletest sandbox.
  */

/**
 * URI: /simpletest/whoami
 * Returns information about current EGA login / session.
 */
class mediamosa_rest_call_simpletest_whoami extends mediamosa_rest_call {

  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Return app id.
    $app_id = $this->get_param_value_app();

    // Fill it.
    $a_result = array(
      'app_id' => $app_id,
      'mediamosa_dbus' => $_SESSION['mediamosa_dbus'],
      'cookie' => $_COOKIE,
      'mediamosa_app' => $_SESSION['mediamosa_app'],
    );

    // Add info to output.
    $o_mediamosa->add_item($a_result);
  }
}
