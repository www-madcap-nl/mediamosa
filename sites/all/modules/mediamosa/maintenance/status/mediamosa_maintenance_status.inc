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
  * Static functions for status page.
  */

 class mediamosa_maintenance_status {
  // ------------------------------------------------------------------ Consts.
  const RUNPHASE_WAITING = 0;
  const RUNPHASE_STARTED = 1; // If we find the test in this mode, the test was still running when the script timed-out.
  const RUNPHASE_COMPLETED = 2;

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Log for mediamosa_status.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa::log($message, $variables, $severity, 'mediamosa_status');
  }

  /**
   * Return a array with the class of the tests
   * we wish to run. If modules are not enabled and
   * there for their class is not loaded, we will need
   * to report this.
   */
  static public function get_default_tests() {
    return array(
      'MediaMosaAppAuthenticationTestCase',
      'MediaMosaConfigurationStorageTestCase',
      'MediaMosaAssetMediafileDownloadTestCase',
      'MediaMosaAssetMediafileStillDownloadTestCase',
      'MediaMosaAssetMediafileUploadTestCaseEga',
    );
  }

  /**
   * returns the interval for cron in seconds.
   * defaults to 600 (10 minutes).
   */
  static public function get_interval() {
    return 600;
  }

  /**
   * Returns TRUE / FALSE if we may run the tests again.
   */
  static public function may_run() {
    $run_last = variable_get('mediamosa_maintenance_status_cron_last', NULL);

    // may run?
    if (round(REQUEST_TIME - $run_last) < self::get_interval()) {
      return FALSE;
    }

    // Before we start, we count the number of tables. If simpletest fails,
    // it doesn't clean up. We stop running the tests if we have to many tables.
    // Also we'll try to clean up if we detect to many tables.
    $count_tables = mediamosa_db::db_count_tables();

    // Check if we have to many tables, else clean up.
    if ($count_tables > mediamosa_settings::STATUS_MAX_TABLES) {
      self::log('To many simpletest tables detected, trying to clean up, found @count tables.', array('@count' => $count_tables), WATCHDOG_ALERT);

      MediaMosaTestCase::clean();

      $count_tables = mediamosa_db::db_count_tables();
      if ($count_tables > mediamosa_settings::STATUS_MAX_TABLES) {
        // Clean failed.
        self::log('Status page update down, unable to clean up old tests, still @count tables left.', array('@count' => $count_tables), WATCHDOG_CRITICAL);
      }
    }

    // All ok, run new tests.
    return TRUE;
  }

  /**
   * Staticly store the results.
   *
   * @param array $value
   */
  static function test_results(array $a_value = NULL) {
    $test_results = &drupal_static(__FUNCTION__);

    if ($a_value) {
      $test_results = $a_value;
    }

    return $test_results;
  }

  /**
   * Run the tests one by one.
   */
  static public function run_simple_tests() {

    mediamosa_debug::log_debug('run_simple_tests() started.');

    // Prevent us running twice and make sure we are enabled.
    if (!self::may_run()) {
      mediamosa_debug::log_debug('run_simple_tests() not yet.');
      return;
    }

    mediamosa_debug::log_debug('run_simple_tests() running.');

    // Set time when we started, so we aren't twice.
    mediamosa::variable_set('mediamosa_maintenance_status_cron_last', REQUEST_TIME);

    // Because we don't know how long these tests run, we will set the time-out
    // of the script to the length of the interval of the cron.
    set_time_limit(self::get_interval() - 1);

    // Where to store the results.
    $a_test_results = array('a_tests' => array());

    // Create the tests first so we can store the names etc.
    $a_tests = array();

    foreach (self::get_default_tests() as $class_name) {
      if (!mediamosa::autoload_class($class_name)) {
        // log that we cant find the test.
        self::log("Warning; can't run simpletest on class '@class', unable to load class, either typo in name or module not enabled!", array('@class' => $class_name));
        continue;
      }

      // Now create and add.
      $test_id = db_insert('simpletest_test_id')
        ->useDefaults(array('test_id'))
        ->execute();

      // Create.
      $a_tests[$class_name] = new $class_name($test_id);

      // Add to run list and result.
      $a_test_results['a_tests'][$class_name] = array('info' => $a_tests[$class_name]->getInfo(), 'runphase' => self::RUNPHASE_WAITING, 'results' => array());
    }

    // Set start point run of all tests.
    $a_test_results['#started_at'] = mediamosa::utc_current_timestamp_now(TRUE);
    $a_test_results['#completed'] = FALSE;

    // Store the unrun stuff in our static.
    self::test_results($a_test_results);

    // Set the shutdown function so we can see tests that still where not completed.
    register_shutdown_function('mediamosa_maintenance_status::shutdown');

    // Now loop through the tests and store the results.
    foreach ($a_tests as $class_name => $simpletest_class) {
      $a_test_results['a_tests'][$class_name]['runphase'] = self::RUNPHASE_STARTED;

      // Ok Run.
      $start = microtime(TRUE);
      $simpletest_class->run();
      $end = microtime(TRUE);

      // Store results.
      $a_test_results['a_tests'][$class_name]['results'] = $simpletest_class->results;
      $a_test_results['a_tests'][$class_name]['started'] = $start;
      $a_test_results['a_tests'][$class_name]['ended'] = $end;

      // Set to completed.
      $a_test_results['a_tests'][$class_name]['runphase'] = self::RUNPHASE_COMPLETED;

      // Store in static.
      self::test_results($a_test_results);

      // Unset so we might free some memory.
      unset($a_tests[$class_name]);
      unset($simpletest_class);

      mediamosa::log('Peak usage after unit test @test; @peak.', array('@test' => $class_name, '@peak' => memory_get_peak_usage()));
    }

    // Set end point run of all tests.
    $a_test_results['#ended_at'] = mediamosa::utc_current_timestamp_now(TRUE);
    $a_test_results['#completed'] = TRUE;

    // Store in static also.
    self::test_results($a_test_results);

    // Now store in variable.
    mediamosa::variable_set('mediamosa_test_results', $a_test_results);

    // Done.
    mediamosa_debug::log_debug('run_simple_tests() ended.');
  }

  /**
   * Triggered when end of script or timed-out so er still
   * have some results.
   */
  static public function shutdown() {
    $a_test_results = self::test_results();

    if (isset($a_test_results['#ended_at'])) {
      return; // no need.
    }

    // Log it.
    self::log('run_simple_tests() overrun.', array(), WATCHDOG_EMERGENCY);

    // Set end point run of runned tests.
    $a_test_results['#ended_at'] = mediamosa::utc_current_timestamp_now(TRUE);

    // Now store in variable.
    mediamosa::variable_set('mediamosa_test_results', $a_test_results);
  }

  /**
   * Trigger the maintenance cron.
   *
   * @param string $uri
   *  If you need to change the uri to trigger.
   */
  static public function trigger_maintenance_status() {

    // Trigger.
    mediamosa_http::do_head_call_cron('/internal/cron/maintenance_status');
  }
}
