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
  * The REST calls of the REST interface.
  */

/**
 * URI: /rest
 * Rest calls for/of rest interface.
 */
class mediamosa_rest_call_list extends mediamosa_rest_call {

  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(),
    );

    // Enrich with default REST vars limit / offset.
    $a_var_setup = self::get_var_setup_range($a_var_setup);

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $limit = $this->get_param_value(mediamosa_rest_call::LIMIT);
    $offset = $this->get_param_value(mediamosa_rest_call::OFFSET);

    // Get all rest calls.
    $a_calls = mediamosa_rest::get_rest_calls($limit, $offset);

    foreach ($a_calls as $a_call_2) { // Method
      foreach ($a_call_2 as $a_call) { // Uri
        $o_mediamosa->add_item($a_call);
      }
    }
  }
}

/**
 * URI: /rest/#$uri_id
 * Retrieve specific information about an REST call.
 */
class mediamosa_rest_call_get extends mediamosa_rest_call {

  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_SERIAL,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => mediamosa::t('The numeric ID of the REST call.'),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get URI.
    $uri_id = $this->get_param_value(mediamosa_rest_call::ID);

    // Get all rest calls.
    $a_calls = mediamosa_rest::get_rest_calls(0, 0, mediamosa_rest_call::ID, NULL);

    if (!isset($a_calls[$uri_id])) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_NOT_FOUND, array('@uri_id' => $uri_id));
    }

    // Create class from it.
    $o_rest_call = new $a_calls[$uri_id][mediamosa_rest_call::CLASS_NAME]($a_calls[$uri_id]);

    // Turn off so we can call the get_var_setup without problems.
    // Although get_var_setup should not test variables.
    $o_rest_call->check_for_unspecified = FALSE;

    // Get the var setup for this call.
    $a_var_setup = $o_rest_call->get_var_setup();

    // Now include the variables of this REST call.
    $a_calls[$uri_id][mediamosa_rest_call::VARS] = $a_var_setup[mediamosa_rest_call::VARS];

    // Add it to the XML output.
    $o_mediamosa->add_item($a_calls[$uri_id]);
  }
}
