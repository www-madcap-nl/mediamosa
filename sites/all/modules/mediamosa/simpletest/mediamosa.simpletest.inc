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
 * The default simpletest class.
 */

/**
 * Defines for the groups.
 * MEDIAMOSA_TEST_GROUP_
 */
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE', 'MediaMosa');

/**
 * The basic lowest simpletest class.
 */
class MediaMosaTestCase extends DrupalWebTestCase {
  // ------------------------------------------------------------------ Functions (protected)
  // Default old user/group.
  const SIMPLETEST_USER_ID = 'Simpletest user id';
  const SIMPLETEST_GROUP_ID = 'Simpletest test group';

  // ------------------------------------------------------------------ Functions (protected)

  /**
   * Implements setUp().
   */
  protected function setUp() {
    $a_vars_to_copy = array('mediamosa_current_mount_point');
    $a_values = array();

    // Store certain vars.
    foreach ($a_vars_to_copy as $var_to_copy) {
      $a_values[$var_to_copy] = mediamosa::variable_get($var_to_copy);
    }

    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array('mediamosa', 'mediamosa_app'), $args));
    call_user_func_array(array($this, 'parent::setUp'), $args);

    // Now set it, so its in the sandbox.
    foreach ($a_values as $name => $value) {
      // Skip NULL.
      if (!is_null($value)) {
        mediamosa::variable_set($name, $value);
      }
    }
  }

  /**
   * Implements tearDown().
   * Because drupal doesn't tear down our tables, we have to do it.
   * FIXME: seems to drop mediamosa_app but skips the rest... might
   *        be fixed in future versions.
   */
  protected function tearDown() {
    global $db_prefix;

    // Dump the log.
    $a_logs = db_query('SELECT * FROM {watchdog}');
    $this->pass('Dumping contents of watchdog', 'watchdog dump');
    foreach ($a_logs as $o_log) {
      $this->pass(strtr($o_log->message, unserialize($o_log->variables)), 'watchdog dump');
    }

    // keep it.
    $old_prefix = $db_prefix;

    // Call parent.
    parent::tearDown();

    // Save change (db_prefix is changed by parent::teardown).
    $new_prefix = $db_prefix;
    $db_prefix = $old_prefix;

    if (preg_match('/simpletest\d+/', $db_prefix)) {

      $prefix = Database::getConnection()->prefixTables('{mediamosa_}') . '%';

      // Get tables that match our pattern.
      $tables = db_find_tables($prefix);

      $matches = $ret = array();
      foreach ($tables as $table) {
        // Make sure we drop correct; tables MUST start with simpletest.
        if (preg_match('/simpletest\d+mediamosa.*/', $table, $matches)) {
          list($tmp, $name) = explode('mediamosa_', $table, 2);

          try {
            db_drop_table($ret, 'mediamosa_' . $name);
          }
          catch (PDOException $e) {
            // ignore.
          }
        }
      }
    }

    // Restore.
    $db_prefix = $new_prefix;
  }

  /**
   * Pretty export of var_export.
   *
   * @param mixed $data
   */
  protected function var_export($data, $group = 'Other') {
    $this->pass(highlight_string('<?php ' . var_export($data, TRUE), TRUE), $group);
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
   * @param array $a_parameters
   *  name => value
   * @param array $a_headers
   *  Name: value
   */
  protected function restCall($uri, $request_method = 'POST', array $a_parameters = array(), array $a_headers = array()) {
    // Add MediaMosa menu path prefix
    //$uri = /*MEDIAMOSA_MENU_PATH . '/'*/ $uri;

    // Prepare the CURL options.
    $curl_opts = array(
      CURLOPT_NOBODY => FALSE,
      CURLOPT_HTTPHEADER => $a_headers,
      CURLOPT_HEADER => TRUE,
    );
    if (strcasecmp($request_method, 'GET') == 0) {
      $curl_opts += array(
        CURLOPT_HTTPGET => TRUE,
        CURLOPT_URL => url($uri, array('absolute' => TRUE, 'query' => $a_parameters)),
      );
    }
    else {
      $curl_opts += array(
        CURLOPT_POST => TRUE,
        CURLOPT_URL => url($uri, array('absolute' => TRUE)),
        CURLOPT_POSTFIELDS => http_build_query($a_parameters),
      );
    }

    $this->pass($this->session_id);

    // Perform the CURL request.
    $result = $this->curlExec($curl_opts);

    // Parse the headers and get the body.
    $lines = explode("\n", $result);
    $line_count = count($lines);

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
      $out['xml'] = new SimpleXMLElement($out['body']);
    }
    catch (Exception $e) {
      $this->assert(FALSE, $e->getMessage());
      return;
    }

    // Look for a request_result_id.
    $this->assertTrue(isset($out['xml']->header->request_result_id), $request_method . ' request to: "' . $uri . '". Result id: ' . $out['xml']->header->request_result_id . ', ' . $out['xml']->header->request_result_description . '<hr />');

    // Return the request data.
    return $out;
  }

  /**
   * Wrapper for POST restCall
   */
  protected function restCallPost($uri, array $a_parameters = array(), array $a_headers = array()) {
    return $this->restCall($uri, 'POST', $a_parameters, $a_headers);
  }

  /**
   * Wrapper for GET restCall
   */
  protected function restCallGet($uri, array $a_parameters = array(), array $a_headers = array()) {
    return $this->restCall($uri, 'GET', $a_parameters, $a_headers);
  }

  /**
   * Wrapper for PUT restCall
   */
  protected function restCallPut($uri, array $a_parameters = array(), array $a_headers = array()) {
    return $this->restCall($uri, 'PUT', $a_parameters, $a_headers);
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
   * Create a random app or specified one.
   * @param array $a_attributes
   *  Basic app parameters like the DB version (mediamosa_app_db).
   */
  protected function createApp($a_fields = array()) {
    // Merge in the default attributes.
    $a_fields += array(
      mediamosa_app_db::APP_ID => rand(1, 1000),
      mediamosa_app_db::SHARED_KEY => user_password(),
      mediamosa_app_db::NAME => $this->randomName(),
      mediamosa_app_db::ACTIVE => 'TRUE',
      mediamosa_app_db::TRANSCODING_POSITION_TO => 0,
      mediamosa_app_db::QUOTA => 0,
      mediamosa_app_db::QUOTA_USED => 0,
      mediamosa_app_db::ACTIVE_VERSION => '',
    );

    // Insert the app into the database.
    $a_fields[mediamosa_app_db::ID] = mediamosa_db::db_insert(mediamosa_app_db::TABLE_NAME)->fields($a_fields)->execute();

    // Log the creation in the resutls.
    $this->assert(TRUE, t("App created with unique ID; '%id',  app_id; '%app_id', name; '%name' and shared key; '%shared_key'.", array('%id' => $a_fields[mediamosa_app_db::ID], '%app_id' => $a_fields[mediamosa_app_db::APP_ID], '%name' => $a_fields[mediamosa_app_db::NAME], '%shared_key' => $a_fields[mediamosa_app_db::SHARED_KEY])));

    return $a_fields;
  }

  /**
   * Create a download server.
   */
  protected function createDownloadServer() {
    $node = new stdClass();
    $node->uid = 1; // Admin.
    $node->status = 1; // Published.
    $node->type = mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER;

    $node->title = 'http://localhost/download/{TICKET}';
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

    node_save(@$node);
    $this->pass(t('Created node Downloadserver with node ID @nid', array('@nid' => $node->nid)));
  }

  /**
   * Create a job.
   */
  protected function createJob($a_fields = array()) {
    $a_fields += array(
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
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert the job into the database.
    $a_fields[mediamosa_job_db::ID] = mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)->fields($a_fields)->execute();
    return $a_fields[mediamosa_job_db::ID];
  }

  /**
   * Create an upload job.
   */
  protected function createUploadJob($job_id, $a_fields = array()) {
    $a_fields[mediamosa_job_upload_db::JOB_ID] = $job_id;

    $a_fields += array(
      mediamosa_job_upload_db::FILE_SIZE => 0,
      mediamosa_job_upload_db::UPLOADED_FILE_SIZE => 0,
      mediamosa_job_upload_db::RETRANSCODE => mediamosa_job_upload_db::RETRANSCODE_FALSE,
      mediamosa_job_upload_db::CREATE_STILL => mediamosa_job_upload_db::CREATE_STILL_FALSE,
    );

    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert the upload job into the database.
    $a_fields[mediamosa_job_upload_db::ID] = mediamosa_db::db_insert(mediamosa_job_upload_db::TABLE_NAME)->fields($a_fields)->execute();
    return $a_fields[mediamosa_job_upload_db::ID];
  }

  /**
   * Create a transcode job.
   */
  protected function createTranscodeJob($job_id, $a_fields = array()) {
    $a_fields[mediamosa_job_transcode_db::JOB_ID] = $job_id;

    $a_fields += array(
      mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID => 1,
      mediamosa_job_transcode_db::TOOL => '',
      mediamosa_job_transcode_db::COMMAND => '',
      mediamosa_job_transcode_db::FILE_EXTENSION => '',
      mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL => '',
    );

    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    $a_fields[mediamosa_job_transcode_db::ID] = mediamosa_db::db_insert(mediamosa_job_transcode_db::TABLE_NAME)->fields($a_fields)->execute();
    return $a_fields[mediamosa_job_transcode_db::ID];
  }

}
