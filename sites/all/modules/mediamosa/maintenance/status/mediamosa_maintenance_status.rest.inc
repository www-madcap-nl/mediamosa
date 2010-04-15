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
  * REST calls for status page.
  */

/**
 * URI: /cron/maintenance_status
 * Method: GET
 */
class mediamosa_rest_call_maintenance_status extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Run them.
    mediamosa_maintenance_status::run_simple_tests();

    // Set ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /external/status
 * Method: GET
 */
class mediamosa_rest_call_status extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Fill the output.
    $test_results = mediamosa::variable_get(
      'mediamosa_test_results',
      array(
        'tests' => array(
          mediamosa_settings::MEDIAMOSA_RUN_10_MINUTES => array(),
          mediamosa_settings::MEDIAMOSA_RUN_3_HOURS => array(),
          mediamosa_settings::MEDIAMOSA_RUN_DAILY => array(),
        )
      )
    );

    // For now, return the 10 minutes ones.
    foreach ($test_results[mediamosa_settings::MEDIAMOSA_RUN_10_MINUTES] as $name => $test_result) {
      $o_mediamosa->add_item(array(
        'test' => $test_result['info']['name'],
        'test_description' => $test_result['info']['description'],
        'passes' => $test_result['results']['#pass'],
        'fails' => $test_result['results']['#fail'],
        'exceptions' => $test_result['results']['#exception'],
        'last_run' => format_date(round($test_result['started']), 'custom', 'Y-m-d H:i:s'),
        'ttr' => round($test_result['ended'] - $test_result['started'], 2),
      ));
    }
  }
}
