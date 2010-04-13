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
 * The default simpletest class.
 */

/**
 * Defines for the groups.
 * MEDIAMOSA_TEST_GROUP_
 */
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE', 'MediaMosa');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_APP', 'MediaMosa End User Applications');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_ASSET', 'MediaMosa Assets');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_ASSET_CQL', 'MediaMosa Assets Search / CQL');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_AUT', 'MediaMosa Authorization');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_FTP_USER', 'MediaMosa FTP User');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_JOB', 'MediaMosa Jobs');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_PLAYPROXY', 'MediaMosa PlayProxy');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_STATISTICS', 'MediaMosa Statistics');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_STILL', 'MediaMosa Stills');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_STORAGE', 'MediaMosa Storage');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_USER', 'MediaMosa User / Groups');

/**
 * The basic lowest simpletest class.
 */
class MediaMosaTestCase extends DrupalWebTestCase {

  // file to unlink on teardown.
  protected $unlinks = array();

  // ------------------------------------------------------------------ Consts.
  // Default old user/group.
  const SIMPLETEST_USER_ID = 'Simpletest user id';
  const SIMPLETEST_GROUP_ID = 'Simpletest test group';

  /**
   * Clean up tables, copy of simpletest_clean_environment without the
   * drupal messages.
   *
   * Only run when you are not running simpletests(!). Any running tests will
   * fail.
   *
   * @see simpletest_clean_environment()
   */
  static public function clean() {
    simpletest_clean_database();
    simpletest_clean_temporary_directories();
    simpletest_clean_results_table();

    // Detect test classes that have been added, renamed or deleted.
    registry_rebuild();
    cache_clear_all('simpletest', 'cache');
  }

  // ------------------------------------------------------------------ Functions (protected)

  /**
   * Implements setUp().
   */
  protected function setUp() {

    // Make this global so we can output debug to the test.
    $GLOBALS['mediamosa_simpletest'] = $this;

    $a_vars_to_copy = array('mediamosa_current_mount_point');
    $a_values = array();

    // Store certain vars.
    foreach ($a_vars_to_copy as $var_to_copy) {
      $a_values[$var_to_copy] = mediamosa::variable_get($var_to_copy);
    }

    // Set debug on high.
    $a_values['mediamosa_debug_level'] = 10;

    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array('simpletest', 'mediamosa', 'mediamosa_app'), $args));
    call_user_func_array(array($this, 'parent::setUp'), $args);

    // Now set it, so its in the sandbox.
    foreach ($a_values as $name => $value) {
      // Skip NULL.
      if (!is_null($value)) {
        mediamosa::variable_set($name, $value);
      }
    }

    // Lets put debug on high.
    mediamosa::variable_set('mediamosa_debug_level', 10);
  }

  /**
   * Implements tearDown().
   * Because drupal doesn't tear down our tables, we have to do it.
   */
  protected function tearDown() {
    global $db_prefix;

    // Dump the log.
    $this->dump_watchdog();

    // Unlink files.
    foreach ($this->unlinks as $filename) {
      mediamosa_io::unlink($filename);
    }

    // Clean up...
    $this->unlinks = array();

    // For some reason our tables are not dropped by teardown parent.
    // So we need to do it ourselfs.
    if (preg_match('/simpletest\d+/', $db_prefix)) {

      // We'll only drop tables from this simpletest.
      $prefix = Database::getConnection()->prefixTables('{mediamosa_}') . '%';

      // Get tables that match our pattern.
      $tables = db_find_tables($prefix);

      $matches = $ret = array();
      foreach ($tables as $table) {
        // Make sure we drop correct; tables MUST start with simpletest.
        if (preg_match('/simpletest\d+mediamosa.*/', $table, $matches)) {
          list($tmp, $name) = explode('mediamosa_', $table, 2);

          // If the drop fails then we consider that its already dropped.
          try {
            db_drop_table('mediamosa_' . $name);
          }
          catch (PDOException $e) {
            // ignore.
          }
        }
      }
    }

    // Call parent.
    parent::tearDown();
  }

  /**
   * Dump the log so we know what happened.
   */
  protected function dump_watchdog() {
    // Dump the log.
    $result = db_query('SELECT * FROM {watchdog}');

    $texts[] = 'Dumping contents of watchdog';

    foreach ($result as $log_item) {
      $items = empty($log_item->variables) ? array() : unserialize($log_item->variables);
      $texts[] = strtr($log_item->message, is_array($items) ? $items : array());
    }

    // In verbose mode only.
    $this->verbose(implode("<br />", $texts), 'watchdog dump');
  }

  /**
   * Pretty export of var_export.
   *
   * @param mixed $data
   */
  public function var_export($data, $group = 'Other') {
    $this->pass(highlight_string('<?php ' . var_export($data, TRUE), TRUE), $group);
  }

  /**
   * Pretty export of var_export.
   *
   * @param mixed $data
   */
  public function var_export_verbose($data, $group = 'Other') {
    $this->verbose(highlight_string('<?php ' . var_export($data, TRUE), TRUE), $group);
  }

  /**
   * Pretty export of var_export.
   *
   * @param mixed $data
   */
  public function var_export_fail($data, $group = 'Other') {
    $this->fail(highlight_string('<?php ' . var_export($data, TRUE), TRUE), $group);
  }

  /**
   * Call if your outside the unittest and you want to log.
   *
   * @param $data
   */
  static public function staticMessage($data) {
    if (isset($GLOBALS['mediamosa_simpletest'])) {
      $GLOBALS['mediamosa_simpletest']->var_export_fail($data);
    }
  }

  /**
   * Static call to restCall
   */
  static public function staticRestCall($uri, $request_method = 'POST', array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    assert(isset($GLOBALS['mediamosa_simpletest']));
    $GLOBALS['mediamosa_simpletest']->restCall($uri, $request_method, $parameters, $headers, $expected_result_ids, $do_internal);
  }

  /**
   * Static call to restCall POST
   */
  static public function staticRestCallPost($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    assert(isset($GLOBALS['mediamosa_simpletest']));
    $GLOBALS['mediamosa_simpletest']->restCall($uri, 'POST', $parameters, $headers, $expected_result_ids, $do_internal);
  }

  /**
   * Static call to restCall GET
   */
  static public function staticRestCallGet($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    assert(isset($GLOBALS['mediamosa_simpletest']));
    $GLOBALS['mediamosa_simpletest']->restCall($uri, 'GET', $parameters, $headers, $expected_result_ids, $do_internal);
  }

  /**
   * Static call to restCall POST
   */
  static public function staticInternalRestCallPost($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    assert(isset($GLOBALS['mediamosa_simpletest']));
    $GLOBALS['mediamosa_simpletest']->restCall($uri, 'POST', $parameters, $headers, $expected_result_ids, TRUE);
  }

  /**
   * Static call to restCall GET
   */
  static public function staticInternalRestCallGet($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    assert(isset($GLOBALS['mediamosa_simpletest']));
    $GLOBALS['mediamosa_simpletest']->restCall($uri, 'GET', $parameters, $headers, $expected_result_ids, TRUE);
  }

  /**
   * Override drupalGet so we can override the request on the drupal interface.
   *
   * @param $path
   * @param $options
   * @param $headers
   */
  protected function drupalGet($path, array $options = array(), array $headers = array()) {

    // Just let them know we need the Drupal part.
    $headers[] = mediamosa_settings::X_MEDIAMOSA_DISABLE_APP . ': True';

    $this->pass('Passing through drupalGet()');
    return parent::drupalGet($path, $options, $headers);
  }

  /**
   * Override drupalPost
   *
   * @param string $path
   * @param string $edit
   * @param string $submit
   * @param array $options
   * @param array $headers
   */
  protected function drupalPost($path, $edit, $submit, array $options = array(), array $headers = array()) {
    // Just let them know we need the Drupal part.
    $headers[] = mediamosa_settings::X_MEDIAMOSA_DISABLE_APP . ': True';

    return parent::drupalPost($path, $edit, $submit, $options, $headers);
  }

  /**
   * Do a REST call inside the sandbox.
   *
   * @param array $a_app
   *  The app_id used during the call, this can be more than one.
   * @param string $uri
   *  The uri of the REST call.
   * @param string $request_method
   *  POST / GET / PUT / DELETE.
   * @param array $parameters
   *  name => value
   * @param array $headers
   *  Name: value
   * @param array $expected_result_ids
   *  Contains the mediamosa_error::ERRORCODE_* that we expect. For example, you
   *  might expect an error instead of OKAY; specify the error code here.
   */
  protected function restCall($uri, $request_method = 'POST', array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    if ($do_internal) {
      // Set internal password.
      $headers[] = mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS . ': ' . mediamosa::get_internal_password();

      // Add internal to uri.
      if (strpos($uri, 'internal/') === FALSE) {
        $uri = 'internal/' . $uri;
      }
    }

    // Prepare the CURL options.
    $curl_opts = array(
      CURLOPT_NOBODY => FALSE,
      CURLOPT_HTTPHEADER => $headers,
      CURLOPT_HEADER => TRUE,
    );

    if (strcasecmp($request_method, 'GET') == 0) {
      $curl_opts += array(
        CURLOPT_HTTPGET => TRUE,
        CURLOPT_URL => url($uri, array('absolute' => TRUE, 'query' => $parameters)),
      );
    }
    else {
      $curl_opts += array(
        CURLOPT_POST => TRUE,
        CURLOPT_URL => url($uri, array('absolute' => TRUE)),
        CURLOPT_POSTFIELDS => http_build_query($parameters),
      );
    }

    $this->verbose('Session ID: ' . $this->session_id);

    // Perform the CURL request.
    $result = $this->curlExec($curl_opts);

    // Parse the headers and get the body.
    $lines = explode("\n", $result);
    $line_count = count($lines);

    $http_result = explode(' ', $lines[0]);

    // Unset the HTTP header line.
    unset($lines[0]);

    // Iterate until an empty row is found.
    $out = array();
    for ($i = 1; $i < $line_count; $i++) {
      if (trim($lines[$i]) != '') {
        list($key, $value) = explode(':', $lines[$i], 2);
        $out['headers'][trim($key)] = trim($value);
        unset($lines[$i]);
      }
      else {
        $out['body'] = trim(implode("\n", $lines));
        break;
      }
    }

    // Create a SimpleXMLElement from the restcall's output.
    try {
      // We expect char '<' on REST calls, if not we need to output it.
      if ($out['body']{0} != '<') {
        $this->var_export($out['body']);
      }
      else {
        $this->var_export_verbose(mediamosa_unicode::substr($out['body'], 0, 10000) . '...<snap>...');
      }
    }
    catch (Exception $e) {
      $this->assert(FALSE, 'var_export of REST response body failed! ' . $e->getMessage());
      return;
    }

    try {
      $out['xml'] = new SimpleXMLElement($out['body']);
    }
    catch (Exception $e) {
      $this->assert(FALSE, 'simple XML creation failed! ' . $e->getMessage());
      return;
    }

    // Get the result ID.
    $result_id = empty($out['xml']->header->request_result_id) ? 0 : (int)$out['xml']->header->request_result_id;
    $result_description = empty($out['xml']->header->request_result_description) ? 'no description' : (string)$out['xml']->header->request_result_description;

    // Look for a request_result_id.
    $this->assertTrue(in_array($result_id, $expected_result_ids), t('@method request to: "@uri", got result ID @result_id (@description) was expecting one of; {@expected}.', array('@method' => $request_method, '@uri' => $uri, '@result_id' => $result_id, '@description' => $result_description, '@expected' => implode(', ', $expected_result_ids))));

    $this->assertFalse($http_result[1] == 404, 'REST call was found');

    // Return the request data.
    return $out;
  }

  /**
   * Wrapper for POST restCall
   */
  protected function restCallPost($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    return $this->restCall($uri, 'POST', $parameters, $headers, $expected_result_ids, $do_internal);
  }

  /**
   * Wrapper for GET restCall
   */
  protected function restCallGet($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    return $this->restCall($uri, 'GET', $parameters, $headers, $expected_result_ids);
  }

  /**
   * Wrapper for PUT restCall
   */
  protected function restCallPut($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    return $this->restCall($uri, 'PUT', $parameters, $headers, $expected_result_ids);
  }

  /**
   * IO Stuff.
   */
  protected function copy($source, $dest) {
    $this->assertTrue(
      file_exists($source),
      t("Copy: Source file '@source' exists.", array('@source' => $source))
    );

    // Copy the test video
    $this->assertTrue(
      mediamosa_io::copy($source, $dest) != FALSE,
      t("Copy: Copied from '@source' to target '@target'", array('@source' => $source, '@target' => $dest))
    );

    $this->assertTrue(
      file_exists($dest),
      t("Copy: New file '@target' exists.", array('@target' => $dest))
    );
  }

  /**
   * Create symlink.
   *
   * @param string $source
   * @param string $dest
   */
  protected function symlink($source, $dest) {
    // Copy the test video
    $this->assertTrue(
      mediamosa_io::symlink($source, $dest) != FALSE,
      t("Symlink: Created symlink from '@source' to target '@target'", array('@source' => $source, '@target' => $dest))
    );

    $this->assertTrue(
      file_exists($dest),
      t("Symlink: New file '@target' exists.", array('@target' => $dest))
    );

    // Register to be removed.
    $this->unlinks[] = $dest;
  }

  /**
   * Remove file.
   *
   * @param string $dest
   */
  protected function unlink($dest) {
    $this->assertTrue(
      mediamosa_io::unlink($dest) !== FALSE,
      t("Unlink: Removing file '@target'", array('@target' => $dest))
    );

    $this->assertTrue(
      file_exists($dest) == FALSE,
      t("Unlink: File '@target' is removed.", array('@target' => $dest))
    );
  }

  /**
   * Create folder.
   *
   * @param string $location
   */
  protected function mkdir($location) {
    // Create the directory first.
    $this->assertTrue(mediamosa_io::mkdir($location), t('Check if @location can be created by Mediamosa', array('@location' => $location)));
  }

  /**
   * Create empty file.
   *
   * @param string $filename
   * @param string $text
   */
  protected function touch($filename, $text = 'Create empty file.') {
    mediamosa_io::touch($filename);
    $this->unlinks[] = $filename;
    return $this->file_exists($filename, $text);
  }

  /**
   * Check if file or path exists.
   *
   * @param string $filename
   */
  protected function file_exists($filename, $text = 'Check if file exists.') {
    mediamosa_io::clearstatcache();

    $this->assertTrue(
      file_exists($filename),
      $text
    );

    return file_exists($filename);
  }

  /**
   * Check if file or path does not exists.
   *
   * @param string $filename
   */
  protected function file_not_exists($filename, $text = 'Check if file does not exists.') {
    mediamosa_io::clearstatcache();

    $this->assertFalse(
      file_exists($filename),
      $text
    );

    return file_exists($filename);
  }

  /**
   * Return the path+filename to the test video.
   */
  protected function getTestVideo() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/test_video.wmv';
    $this->pass(t('Got test video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Return the path+filename to the test image (jpeg).
   */
  protected function getTestImage_NotBlackJpg() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/notblack-001.jpeg';
    $this->pass(t('Got test video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Return the path+filename to the test image (gif).
   */
  protected function getTestImage_BlobGif() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/test_blob.gif';
    $this->pass(t('Got test video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Create a random app or specified one.
   * @param array $a_attributes
   *  Basic app parameters like the DB version (mediamosa_app_db).
   */
  protected function createApp($fields = array()) {

    // Generate the name.
    $name = $this->randomName();

    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_APP, $name);

    $node->{mediamosa_app_db::APP_ID} = rand(1, 1000);
    $node->{mediamosa_app_db::SHARED_KEY} = user_password();
    $node->{mediamosa_app_db::APP_NAME} = $name;
    $node->{mediamosa_app_db::ACTIVE} = 'TRUE';
    $node->{mediamosa_app_db::TRANSCODING_POSITION_TO} = 0;
    $node->{mediamosa_app_db::QUOTA} = 0;
    $node->{mediamosa_app_db::QUOTA_USED} = 0;
    $node->{mediamosa_app_db::ACTIVE_VERSION} = '';

    node_save(&$node);

    // Log the creation in the resutls.
    $this->assert(TRUE, t("App created with node ID; '%id',  app_id; '%app_id', name; '%name' and shared key; '%shared_key'.", array('%id' => $node->nid, '%app_id' => $node->{mediamosa_app_db::APP_ID}, '%name' => $node->title, '%shared_key' => $node->{mediamosa_app_db::SHARED_KEY})));

    return (array)$node;
  }

  /**
   * Create a download server.
   */
  protected function createDownloadServer() {
    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER, 'http://localhost/download/{TICKET}');

    $node->{mediamosa_server_db::VERSION} = 0;
    $node->{mediamosa_server_db::SERVER_STATUS} = mediamosa_server_db::SERVER_STATUS_ON;
    $node->{mediamosa_server_db::SERVER_TYPE} = mediamosa_server_db::SERVER_TYPE_DOWNLOAD;
    $node->{mediamosa_server_db::URI} = 'http://localhost/download/{TICKET}';
    $node->{mediamosa_server_db::CONTAINERS} = NULL;
    $node->{mediamosa_server_db::OBJECT_CODE} = NULL;
    $node->{mediamosa_server_db::SERVER_NAME} = NULL;
    $node->{mediamosa_server_db::SLOTS} = NULL;
    $node->{mediamosa_server_db::TOOLS} = NULL;
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS} = NULL;

    node_save(&$node);
    $this->pass(t('Created node Downloadserver with node ID @nid', array('@nid' => $node->nid)));
  }

  /**
   * Create a job.
   */
  protected function createJob($fields = array()) {
    $fields += array(
      mediamosa_job_db::APP_ID => rand(1, 1000),
      mediamosa_job_db::ASSET_ID => mediamosa_db::uuid(rand(1, 1000)),
      mediamosa_job_db::MEDIAFILE_ID => mediamosa_db::uuid(rand(1, 1000)),
      mediamosa_job_db::OWNER_ID => mediamosa_db::uuid(rand(1, 1000)),
      mediamosa_job_db::JOB_STATUS => mediamosa_job_db::JOB_STATUS_INPROGRESS,
      mediamosa_job_db::PROGRESS => '',
      mediamosa_job_db::PRIORITY => 0,
      mediamosa_job_db::JOB_TYPE => mediamosa_job_db::JOB_TYPE_UPLOAD,
      mediamosa_job_db::STARTED => mediamosa::db_current_timestamp_now(),
      mediamosa_job_db::FINISHED => mediamosa::db_current_timestamp_now(),
      mediamosa_job_db::ERROR_DESCRIPTION => '',
      mediamosa_job_db::CREATE_STILL => mediamosa_job_db::CREATE_STILL_FALSE,
      mediamosa_job_db::NO_HINT => mediamosa_job_db::NO_HINT_FALSE,
    );
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert the job into the database.
    $fields[mediamosa_job_db::ID] = mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)->fields($fields)->execute();
    return $fields[mediamosa_job_db::ID];
  }

  /**
   * Create an upload job.
   */
  protected function createUploadJob($job_id, $fields = array()) {
    $fields[mediamosa_job_upload_db::JOB_ID] = $job_id;

    $fields += array(
      mediamosa_job_upload_db::FILE_SIZE => 0,
      mediamosa_job_upload_db::UPLOADED_FILE_SIZE => 0,
      mediamosa_job_upload_db::RETRANSCODE => mediamosa_job_upload_db::RETRANSCODE_FALSE,
      mediamosa_job_upload_db::CREATE_STILL => mediamosa_job_upload_db::CREATE_STILL_FALSE,
    );

    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert the upload job into the database.
    $fields[mediamosa_job_upload_db::ID] = mediamosa_db::db_insert(mediamosa_job_upload_db::TABLE_NAME)->fields($fields)->execute();
    return $fields[mediamosa_job_upload_db::ID];
  }

  /**
   * Create a transcode job.
   */
  protected function createTranscodeJob($job_id, $fields = array()) {
    $fields[mediamosa_job_transcode_db::JOB_ID] = $job_id;

    $fields += array(
      mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID => 1,
      mediamosa_job_transcode_db::TOOL => '',
      mediamosa_job_transcode_db::COMMAND => '',
      mediamosa_job_transcode_db::FILE_EXTENSION => '',
      mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL => '',
    );

    $fields = mediamosa_db::db_insert_enrich($fields);

    $fields[mediamosa_job_transcode_db::ID] = mediamosa_db::db_insert(mediamosa_job_transcode_db::TABLE_NAME)->fields($fields)->execute();
    return $fields[mediamosa_job_transcode_db::ID];
  }

}
