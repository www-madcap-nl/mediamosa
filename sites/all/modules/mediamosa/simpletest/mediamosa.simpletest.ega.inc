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
  * Higher class for REST call unit tests with EGA (app) login.
  */

/**
 * Use this class for tests that uses a EGA default login for doing
 * REST calls.
 *
 */
class MediaMosaTestCaseEga extends MediaMosaTestCase {
  // ------------------------------------------------------------------ Members.
  protected $a_app = NULL;
  protected $a_app_2 = NULL;
  protected $a_app_3 = NULL;

  // By default all goes through app 1.
  protected $switch_to_app = NULL;

  // Contains the login sessions, app_id => array('curlHandle' => curlHandle)
  protected $a_app_sessions = array();

  // Array with mediafile ids of mediafiles that have files that need to be
  // deleted on teardown.
  private $a_mediafiles_cleanup = array();


  // ------------------------------------------------------------------ Functions.
  /**
   * Implements setUp().
   */
  protected function setUp() {
    // Reset.
    $this->switch_to_app = NULL;

    // Get the current mediamosa server content.
    // @todo: convert these to nodes.
    $a_servers = mediamosa_server::search();
    $a_server_containers = mediamosa_server::search_containers();

    // Run parent first so we are inside sandbox.
    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array(
      'mediamosa_app_authentication',
      'mediamosa_asset',
      'mediamosa_aut',
      'mediamosa_collection'), $args));

    call_user_func_array(array($this, 'parent::setUp'), $args);

    // Now we are in sandbox, create default APP, our default EGA.
    $this->a_app = $this->createApp();

    // Create 2 extra.
    $this->a_app_2 = $this->createApp(array(mediamosa_app_db::APP_ID => $this->a_app[mediamosa_app_db::APP_ID] + 1)); // in case we need other.
    $this->a_app_3 = $this->createApp(array(mediamosa_app_db::APP_ID => $this->a_app_2[mediamosa_app_db::APP_ID] + 1)); // in case we need other.

    // Create and login user
    $rest_user = $this->drupalCreateUser(array('administer mediamosa'));
    $this->drupalLogin($rest_user);

    // Truncate current and insert new.
    db_truncate(mediamosa_server_db::TABLE_NAME)->execute();

    // Copy the current servers to sandbox.
    foreach ($a_servers as $a_server) {
      db_insert(mediamosa_server_db::TABLE_NAME)
        ->fields($a_server)
        ->execute();
    }

    // Truncate current and insert new containers.
    db_truncate(mediamosa_server_streaming_container_db::TABLE_NAME)->execute();

    // Copy the containers.
    // Copy the current server containers to sandbox.
    foreach ($a_server_containers as $a_server_container) {
      db_insert(mediamosa_server_streaming_container_db::TABLE_NAME)
        ->fields($a_server_container)
        ->execute();
    }

    // Do EGA / APP login (app 1).
    // If your test needs more, call doEgaLogin_2 and doEgaLogin_3 in your test.
    $this->doEgaLogin();
  }

  /**
   * Implements tearDown().
   */
  protected function tearDown() {

    // Delete possible uploaded files.
    foreach ($this->a_mediafiles_cleanup as $mediafile_id) {
      // Get the mediafile.
      $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id);

      // Delete uploaded file by deleting the mediafile.
      mediamosa_asset_mediafile::delete($mediafile_id);

      // Check if the file has been deleted.
      $this->assertFalse(file_exists(mediamosa_configuration_storage::mediafile_filename_get($a_mediafile)), 'File has been deleted.');
    }

    // Clean up array.
    $this->a_mediafiles_cleanup = array();

    // Call parent.
    parent::tearDown();
  }

  /**
   * Login using generated app, name and shared_key (password).
   */
  protected function doEgaLogin(array $a_app = NULL) {
    if (empty($a_app)) {
      $a_app = $this->a_app;
    }

    // Unset the session.
    unset($this->a_app_sessions[$a_app[mediamosa_rest_call::APP_ID]]);
    $this->curlHandle = NULL;

    // Do login.
    $app_id = $a_app[mediamosa_app_db::APP_ID];
    $app_name = $a_app[mediamosa_app_db::NAME];
    $shared_key = $a_app[mediamosa_app_db::SHARED_KEY];

    // Step 1: start auth and capture cookie.

    // We must call parent to skip the different session stuff.
    $response = parent::restCall('login', 'POST', array('dbus' => 'AUTH DBUS_COOKIE_SHA1 ' . $app_name));
    $this->pass(check_plain(var_export($response['xml'], TRUE)));
    $dbus_data = explode(' ', $response['xml']->items->item->dbus);

    $this->assertTrue(drupal_substr($response['xml']->items->item->dbus, 0, 5) == 'DATA ', 'Login fase 1 success');

    list($cookie) = explode(';', $response['headers']['Set-Cookie']);
    $this->assertTrue(isset($response['headers']['Set-Cookie']), 'Cookie received in HTTP header: '. $cookie);

    // Step 2: send the credentials.
    $challenge = $dbus_data[3];
    $random = substr(md5(microtime(TRUE)), 0, 10);
    $post_data = array('dbus' => 'DATA '. $random .' '. sha1(sprintf('%s:%s:%s', $challenge, $random, $shared_key)));

    // We must call parent to skip the different session stuff.
    $response = parent::restCall('login', 'POST', $post_data);

    $this->assertTrue($response['xml']->items->item->dbus == 'OK server_guid', 'Login fase 2 success');
    $this->pass(check_plain(var_export($response['xml'], TRUE)));

    // Store session stuff so we can switch sessions during REST calls.
    // Although cookieFile is not 'working' or doesnt do much, will included anyway.
    $this->a_app_sessions[$app_id] = array('curlHandle' => $this->curlHandle, 'cookieFile' => $this->cookieFile);
  }

  /*
   * Create login for EGA app 2.
   */
  protected function doEgaLogin_2() {
    $this->doEgaLogin($this->a_app_2);
  }

  /*
   * Create login for EGA app 3.
   */
  protected function doEgaLogin_3() {
    $this->doEgaLogin($this->a_app_3);
  }

  /**
   * Call to switch session to EGA 1.
   */
  protected function toEga_1() {
    $this->switch_to_app = $this->a_app[mediamosa_app_db::APP_ID];
  }

  /**
   * Call to switch session to EGA 2.
   */
  protected function toEga_2() {
    $this->switch_to_app = $this->a_app_2[mediamosa_app_db::APP_ID];
  }

  /**
   * Call to switch session to EGA 3.
   */
  protected function toEga_3() {
    $this->switch_to_app = $this->a_app_3[mediamosa_app_db::APP_ID];
  }

  /**
   * Enable the master/slave on app.
   */
  protected function enableMasterSlave($app_id) {
    // Change the master setting.
    mediamosa_app::update($app_id, array(mediamosa_app_db::ALLOW_MASTERSLAVE => mediamosa_app_db::ALLOW_MASTERSLAVE_TRUE));
  }

  /**
   * Enable the master/slave on app.
   */
  protected function disableMasterSlave($app_id) {
    // Change the master setting.
    mediamosa_app::update($app_id, array(mediamosa_app_db::ALLOW_MASTERSLAVE => mediamosa_app_db::ALLOW_MASTERSLAVE_FALSE));
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
   * @param array $a_expected_result_ids
   *  Contains the mediamosa_error::ERRORCODE_* that we expect. For example, you
   *  might expect an error instead of OKAY; specify the error code here.
   */
  protected function restCall($uri, $request_method = 'POST', array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Switch to what EGA login?
    $switch_to_app = $this->switch_to_app;

    // Now if app_id is not set, we use app_id of app 1.
    if (empty($switch_to_app)) {
      $switch_to_app = $this->a_app[mediamosa_app_db::APP_ID];
    }

    // Save old handle, if we overwrite it, put it back.
    $old_curlHandle = NULL;

    // If session was set, use it.
    if (isset($this->a_app_sessions[$switch_to_app])) {
      $old_curlHandle = $this->curlHandle;
      $this->curlHandle = $this->a_app_sessions[$switch_to_app]['curlHandle'];
    }

    // Do the REST call.
    try {
      $result = parent::restCall($uri, $request_method, $a_parameters, $a_headers, $a_expected_result_ids);
    }
    catch (Exception $e) {
      if (isset($old_curlHandle)) {
        $this->curlHandle = $old_curlHandle;
      }
      throw $e;
    }

    // If changed curlHandle, then put it back.
    if (isset($old_curlHandle)) {
      $this->curlHandle = $old_curlHandle;
    }

    return $result;
  }

  /**
   * Wrapper for restCall, for app 2.
   */
  protected function restCall_2($uri, $request_method = 'POST', array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 2.
    $this->switch_to_app = $this->a_app_2[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $result = $this->restCall($uri, $request_method, $a_parameters, $a_headers, $a_expected_result_ids);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;

    // Return result.
    return $result;
  }

  /**
   * Wrapper for restCall, for app 3.
   */
  protected function restCall_3($uri, $request_method = 'POST', array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 2.
    $this->switch_to_app = $this->a_app_3[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $result = $this->restCall($uri, $request_method, $a_parameters, $a_headers, $a_expected_result_ids);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;

    // Return result.
    return $result;
  }

  /**
   * Wrapper for POST restCall, for app 3.
   */
  protected function restCallPost_2($uri, array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Do call.
    return $this->restCall_2($uri, 'POST', $a_parameters, $a_headers, $a_expected_result_ids);
  }

  /**
   * Wrapper for POST restCall, for app 3.
   */
  protected function restCallPost_3($uri, array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Do call.
    return $this->restCall_3($uri, 'POST', $a_parameters, $a_headers, $a_expected_result_ids);
  }

  /**
   * Wrapper for GET restCall, for app 2.
   */
  protected function restCallGet_2($uri, array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Do call.
    return $this->restCall_2($uri, 'GET', $a_parameters, $a_headers, $a_expected_result_ids);
  }

  /**
   * Wrapper for GET restCall, for app 3.
   */
  protected function restCallGet_3($uri, array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Do call.
    return $this->restCall_3($uri, 'GET', $a_parameters, $a_headers, $a_expected_result_ids);
  }

  /**
   * Wrapper for PUT restCall, for app 2.
   */
  protected function restCallPut_2($uri, array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Do call.
    return $this->restCall_2($uri, 'PUT', $a_parameters, $a_headers, $a_expected_result_ids);
  }

  /**
   * Wrapper for PUT restCall, for app 3.
   */
  protected function restCallPut_3($uri, array $a_parameters = array(), array $a_headers = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Do call.
    return $this->restCall_3($uri, 'PUT', $a_parameters, $a_headers, $a_expected_result_ids);
  }

  /**
   * Basic upload of test wmf file.
   */
  protected function uploadTestFile() {
    // Create an asset.
    $asset_id = $this->createAsset();

    // Create mediafile.
    $mediafile_id = $this->createMediafile($asset_id);

    // Create upload ticket.
    $a_uploadticket = $this->createMediafileUploadTicket($mediafile_id);

    $result = mediamosa_db::db_query('SELECT * FROM {mediamosa_ticket}');
    foreach ($result as $row) {
      $this->var_export($row, 'Dump Ticket');
    }

    // Check ticket link etc.
    $this->assertTrue(!empty($a_uploadticket['action']), 'action set');
    $this->assertTrue(!empty($a_uploadticket['uploadprogress_url']), 'uploadprogress_url set');

    // Get link to upload file.
    $filename = self::getTestVideo();

    // URL.
    $action = $a_uploadticket['action'];

    // File.
    $a_post['file'] = '@' . $filename;

    // No headers.
    $headers = array();

    // Upload the test file.
    $out = $this->curlExec(array(CURLOPT_URL => $action, CURLOPT_POST => TRUE, CURLOPT_POSTFIELDS => $a_post, CURLOPT_HTTPHEADER => $headers));

    // Add for deletion.
    $this->a_mediafiles_cleanup[] = $mediafile_id;

    // Replace original page output with new output from redirected page(s).
    $new = $this->checkForMetaRefresh();
    if ($new) {
      $out = $new;
    }

    // Our upload.
    $this->verbose('POST request to: ' . $action .
                   '<hr />Ending URL: ' . $this->getUrl() .
                   '<hr />Fields: ' . highlight_string('<?php ' . var_export($a_post, TRUE), TRUE) .
                   '<hr />' . $out);

    list($response, $data) = explode("\r\n\r\n", $out, 2);
    $response = preg_split("/\r\n|\n|\r/", $response);

    // Parse the response status line.
    list($protocol, $code, $status_message) = explode(' ', trim(array_shift($response)), 3);

    // Check 200 resonse.
    $this->assertTrue($code == 200, 'Upload HTML response 200.');

    // Get the upload job.
    $a_job = mediamosa_job::get_by_mediafileid($mediafile_id);
    $this->var_export($a_job, 'Dump Job');

    // Other.
    $a_job_upload = mediamosa_job_upload::get($a_job[mediamosa_job_db::ID]);
    $this->var_export($a_job_upload, 'Dump Job Upload');

    // Get the mediafile.
    $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id);

    // Check if the file has been uploaded.
    $this->assertTrue(file_exists(mediamosa_configuration_storage::mediafile_filename_get($a_mediafile)), 'File has been uploaded.');

    // Lets return the mediafile_id for now.
    return array('asset_id' => $asset_id, 'mediafile_id' => $mediafile_id);
  }

  /**
   * Compare input insert/update with result.
   * @param string $table
   *  Table name.
   * @param string $object_id
   * @param array $a_parameters
   *  Values to compare. Remember will only work if parameters match the database field names.
   * @param array $a_db_row
   */
  protected function do_compare($table, $object_id, array $a_parameters, array $a_db_row) {
    $fail = FALSE;
    foreach ($a_parameters as $key => $value) {
      if (!isset($a_db_row[$key]) || $a_db_row[$key] != $value) {
        $this->fail(strtr("Row value of table @table with row ID @object_id different from input; got '@output', expecting '@input' on field @key.",
          array(
            '@table' => $table,
            '@object_id' => $object_id,
            '@input' => $value,
            '@output' => $a_db_row[$key],
            '@key' => $key
          )
        ));

        $fail = TRUE;
      }
    }

    if (!$fail) {
      $this->pass(strtr('Row values of table @table with row ID @object_id passed.',
        array(
          '@table' => $table,
          '@object_id' => $object_id,
        )
      ));
    }
  }

  // ------------------------------------------------------------------ Get items functions.
  /**
   * Get an asset.
   * Based on the GET rest call /asset/$asset_id (GET)
   */
  protected function getAsset($asset_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_asset_get::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_get::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Required.

    $uri = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Get asset with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Must be ok to return object.
    if (!in_array(mediamosa_error::ERRORCODE_OKAY, $a_expected_result_ids)) {
      return FALSE;
    }

    $a_asset = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Did we get something?
    $this->assertTrue(
      $a_asset[mediamosa_asset_db::ID] == $asset_id,
      'Got the asset'
    );

    // Return data.
    return $a_asset;
  }

  /**
   * Get an mediafile.
   * Based on the rest call /mediafile/$mediafile_id (GET)
   */
  protected function getMediafile($mediafile_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set uri.
    $uri = strtr('mediafile/@mediafile_id', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Get mediafile with ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Get response.
    $a_mediafile = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Did we get something?
    $this->assertTrue(
      $a_mediafile[mediamosa_asset_mediafile_db::ID] == $mediafile_id,
      'Got the mediafile'
    );

    // Return data.
    return $a_mediafile;
  }

  /**
   * Get metadata definitions.
   * Based on the REST call /metadata_tag (GET)
   */
  protected function getMetadataDefinitions(array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    $uri = 'metadata_tag';

    // Do Post call.
    $response = $this->restCallGet($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Get metadata tags, got result @result (@result_description)",
        array(
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    $result = mediamosa_lib::simplexml2array($response['xml']->items);
    return $result['item'];
  }

  /**
   * Get an user.
   * Based on the rest call /user/$user_id (GET)
   */
  protected function getUser($user_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set uri.
    $uri = strtr('user/@user_id', array('@user_id' => $user_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Get user with ID: @user_id, got result @result (@result_description)",
        array(
          '@user_id' => $user_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Get response.
    $a_user = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Did we get something?
    $this->assertTrue(
      isset($a_user[mediamosa_user_db::CREATED]),
      'Got the user'
    );

    // Return data.
    return $a_user;
  }

  /**
   * Get user list.
   * Based on the rest call /user (GET)
   */
  protected function getUserList(array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set uri.
    $uri = 'user';

    // Do Get call.
    $response = $this->restCallGet($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Getting list users, got result @result (@result_description)",
        array(
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return mediamosa_lib::simplexml2array($response['xml']->items);
  }

  /**
   * Get an user.
   * Based on the rest call /group/$group_id (GET)
   */
  protected function getGroup($group_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set uri.
    $uri = strtr('group/@group_id', array('@group_id' => $group_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Get user group with ID: @group_id, got result @result (@result_description)",
        array(
          '@group_id' => $group_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Get response.
    $a_group = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Did we get something?
    $this->assertTrue(
      isset($a_group[mediamosa_user_group_db::CREATED]),
      'Got the user group'
    );

    // Return data.
    return $a_group;
  }

  // ------------------------------------------------------------------ Create items functions.
  /**
   * Create an asset.
   * Based on REST call /asset/create (POST)
   */
  protected function createAsset(array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_asset_create::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_asset_create::GROUP_ID => self::SIMPLETEST_GROUP_ID,
      mediamosa_rest_call_asset_create::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_create::PROVIDER_ID => '',
      mediamosa_rest_call_asset_create::REFERENCE_ID => '',
    );

    // Set uri.
    $uri = 'asset/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Get the new asset_id.
    $asset_id = empty($response['xml']->items->item[0]->asset_id) ? NULL : (string)$response['xml']->items->item[0]->asset_id;

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Created asset with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Return the asset_id.
    return $asset_id;
  }

  /**
   * Create asset metadata.
   * Based on REST call /asset/$asset_id/metadata (POST)
   */
  protected function createAssetMetadata($asset_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_asset_metadata_create::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Required.
    $a_parameters[mediamosa_rest_call_create_mediafile::ASSET_ID] = $asset_id;

    // Set the URI.
    $uri = strtr('asset/@asset_id/metadata', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Created metadata for asset ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return $response['xml'];
  }
  /**
   * Create an mediafile
   * Based on REST call /mediafile/create (POST)
   */
  protected function createMediafile($asset_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_create_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_create_mediafile::IS_APP_ADMIN => 'false',
    );

    // Required.
    $a_parameters[mediamosa_rest_call_create_mediafile::ASSET_ID] = $asset_id;

    // Do Post call.
    $response = $this->restCallPost('mediafile/create', $a_parameters, array(), $a_expected_result_ids);

    // Get the new mediafile_id.
    $mediafile_id = empty($response['xml']->items->item[0]->mediafile_id) ? NULL : (string)$response['xml']->items->item[0]->mediafile_id;

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Created mediafile with ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Return the mediafile ID.
    return $mediafile_id;
  }

  /**
   * Create upload ticket.
   * Based on REST call /mediafile/$mediafile_id/uploadticket/create (POST)
   */
  protected function createMediafileUploadTicket($mediafile_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_asset_mediafile_uploadticket_create::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_asset_mediafile_uploadticket_create::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_mediafile_uploadticket_create::GROUP_ID => '',
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/uploadticket/create', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Get values.
    $action = empty($response['xml']->items->item[0]->action) ? NULL : (string)$response['xml']->items->item[0]->action;
    $uploadprogress_url = empty($response['xml']->items->item[0]->uploadprogress_url) ? NULL : (string)$response['xml']->items->item[0]->uploadprogress_url;

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Created upload ticket with action '@action', uploadprogress_url '@uploadprogress_url', got result @result (@result_description)",
        array(
          '@action' => $action,
          '@uploadprogress_url' => $uploadprogress_url,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Return data instead of one ID, is different behavour than other create functions.
    return mediamosa_lib::simplexml2array($response['xml']->items->item[0]);
  }

  /**
   * Create a asset supplement.
   * Based on REST call asset/$asset_id/supplement/create (POST)
   */
  protected function createAssetSupplement($asset_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_asset_supplement_create::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('asset/@asset_id/supplement/create', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Get the new asset_supplement_id.
    $asset_supplement_id = empty($response['xml']->items->item[0]->supplement_id) ? NULL : (string)$response['xml']->items->item[0]->supplement_id;

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Created asset supplement with ID '@asset_supplement_id', got result @result (@result_description)",
        array(
          '@asset_supplement_id' => $asset_supplement_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Return data.
    return $asset_supplement_id;
  }

  /**
   * Create a quota user.
   * Based on REST call user/create (POST)
   */
  protected function createUser($user_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_user_create::QUOTUM => 0,
    );

    // Required.
    $a_parameters[mediamosa_rest_call_user_create::USER_ID] = $user_id;

    // Set the URI.
    $uri = 'user/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Created user with ID '@user_id', got result @result (@result_description)",
        array(
          '@user_id' => $user_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );
  }

  /**
   * Create a quota group.
   * Based on REST call group/create (POST)
   */
  protected function createGroup($group_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_group_create::QUOTUM => 0,
    );

    // Required.
    $a_parameters[mediamosa_rest_call_group_create::GROUP_ID] = $group_id;

    // Set the URI.
    $uri = 'group/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Created group with ID '@group_id', got result @result (@result_description)",
        array(
          '@group_id' => $group_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );
  }

  // ------------------------------------------------------------------ Update items functions.
  /**
   * Update mediafile.
   * Based on the GET rest call /mediafile/$mediafile_id (POST)
   */
  protected function updateMediafile($mediafile_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_update_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Test response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr("Updated mediafile with ID: @mediafile_id, got result @result (@result_description)", array('@mediafile_id' => $mediafile_id, '@result' => (string)$response['xml']->header->request_result_id, '@result_description' => $response['xml']->header->request_result_description))
    );

    // Return response.
    return $response;
  }

  /**
   * Updates an asset.
   * Based on the REST call /asset/$asset_id (POST)
   */
  protected function updateAsset($asset_id, array $a_parameters = array(), $description = 'Update on asset ID', array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_asset_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // More than one, we need to do something.
    $this->assertFalse(count($a_parameters) == 1, 'Any update parameters set');

    // The uri.
    $uri = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Test return status.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "@description: @asset_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@asset_id' => $asset_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Updates an user.
   * Based on the REST call /user/$user_id (POST)
   */
  protected function updateUser($user_id, array $a_parameters = array(), $description = 'Update on user', array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // More than one, we need to do something.
    $this->assertTrue(count($a_parameters), 'Any update parameters set');

    // The uri.
    $uri = strtr('user/@user_id', array('@user_id' => $user_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Test return status.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "@description: @user_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@user_id' => $user_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Updates an group.
   * Based on the REST call /group/$group_id (POST)
   */
  protected function updateGroup($group_id, array $a_parameters = array(), $description = 'Update on group', array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // More than one, we need to do something.
    $this->assertTrue(count($a_parameters), 'Any update parameters set');

    // The uri.
    $uri = strtr('group/@group_id', array('@group_id' => $group_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Test return status.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "@description: @group_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@group_id' => $group_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Updates ACL/AUT rights on mediafile
   * Based on the REST call /mediafile/$mediafile_id/acl (POST)
   *
   * mediafile_id
   * user_id      must match the owner fo the mediafile.
   * replace      Standard is TRUE. Define if 'FALSE' if you want to keep the old rights.
   * aut_app[]    application(s) that gain access to the mediafile
   * aut_user[]   user(s) that gain access to the mediafile
   * aut_group[]  group(s) that gain access to the mediafile
   * aut_domain[] domein(s) that gain access to the mediafile
   * aut_realm[]  realm(s) that gain access to the mediafile
   *
   */
  protected function setMediafileAut($mediafile_id, array $a_parameters = array(), $description = 'Setting ACL on mediafile', array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_aut_mediafile_set_rights::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_aut_mediafile_set_rights::REPLACE => 'true',
    );

    // The uri.
    $uri = strtr('mediafile/@mediafile_id/acl', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Test return status.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "@description: @mediafile_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@mediafile_id' => $mediafile_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Same as setMediafileAut, except will add to rights and not replace.
   */
  protected function addMediafileAut($mediafile_id, array $a_parameters = array(), $description = 'Setting ACL on mediafile', array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    $a_parameters[mediamosa_rest_call_aut_mediafile_set_rights::REPLACE] = 'false';
    $this->setMediafileAut($mediafile_id, $a_parameters, $description, $a_expected_result_ids);
  }

  // ------------------------------------------------------------------ Delete items functions.
  /**
   * Delete a asset.
   * Based on the REST call /asset/$asset_id/delete (POST)
   */
  protected function deleteAsset($asset_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters.
    $a_parameters += array(
      mediamosa_rest_call_asset_supplement_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('asset/@asset_id/delete', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Delete asset with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete asset supplement.
   * Based on the REST call /asset/$asset_id/supplement/$supplement_id/delete (POST)
   */
  protected function deleteAssetSupplement($asset_id, $supplement_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Parameters.
    $a_parameters += array(
      mediamosa_rest_call_asset_supplement_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('asset/@asset_id/supplement/@supplement_id/delete', array('@asset_id' => $asset_id, '@supplement_id' => $supplement_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Delete asset supplement with ID: @supplement_id, got result @result (@result_description)",
        array(
          '@supplement_id' => $supplement_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete ACL rights on mediafile.
   * Based on the REST call /asset/$asset_id/supplement/$supplement_id/delete (POST)
   */
  protected function deleteMediafileAut($mediafile_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_aut_mediafile_set_rights::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('mediafile/@mediafile_id/acl/delete', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids);

    // Test return status.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Removed ACL rights from @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    return $response;

  }


  // ------------------------------------------------------------------ CQL functions.
  public function cql_do_search_test_assets($cql, $a_expected_asset_ids, array $a_parameters = array(), $aut_user_id = FALSE, $aut_group_id = FALSE, $aut_domain = FALSE, $aut_realm = FALSE, $granted = FALSE, $is_public_list = FALSE) {
    $a_parameters += array(
      mediamosa_rest_call_asset_search::CQL => $cql,
      mediamosa_rest_call_asset_search::GRANTED => $granted ? 'true' : 'false',
    );

    if ($aut_user_id !== FALSE) {
      $a_parameters[mediamosa_rest_call_asset_search::AUT_USER_ID] = $aut_user_id;
    }

    if ($aut_group_id !== FALSE) {
      $a_parameters[mediamosa_rest_call_asset_search::AUT_GROUP_ID] = $aut_group_id;
    }

    if ($aut_domain !== FALSE) {
      $a_parameters[mediamosa_rest_call_asset_search::AUT_DOMAIN] = $aut_domain;
    }

    if ($aut_realm !== FALSE) {
      $a_parameters[mediamosa_rest_call_asset_search::AUT_REALM] = $aut_realm;
    }

    // private listing ?
    $a_parameters[mediamosa_rest_call_asset_search::IS_PUBLIC_LIST] = $is_public_list ? 'true' : 'false';

    // Set uri.
    $uri = 'asset';
    $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY, mediamosa_error::ERRORCODE_EMPTY_RESULT);

    // Do get call.
    $response = $this->restCallGet($uri, $a_parameters, array(), $a_expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Executed CQL: '@cql', got result @result (@result_description)",
        array(
          '@cql' => $a_parameters[mediamosa_rest_call_asset_search::CQL],
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string)$response['xml']->header->request_result_description
        )
      )
    );

    // Make array.
    $a_found = isset($response['xml']->items) && !empty($response['xml']->items) ? mediamosa_lib::simplexml2array($response['xml']) : array('items' => array('item' => array()));
    $a_found = !isset($a_found['items']['item'][0]) && !empty($a_found['items']['item']) ? array(0 => $a_found['items']['item']) : $a_found['items']['item'];

    $expected = $this->cql_expected_ids($a_found, $a_expected_asset_ids, 'asset_id', $granted);

    $a_found_ids = array();
    foreach ($a_found as $key_item => $a_item) {
      $a_found_ids[] = $a_item['asset_id'];
    }

    // Now see if we got what we wanted.
    $this->assertTrue($expected, strtr('Found assets; @found, expected; @expected.', array('@found' => implode(', ', $a_found_ids), '@expected' => implode(', ', array_keys($a_expected_asset_ids)))));
  }

  function cql_expected_ids($a_found, $a_ids, $name, $granted) {
    if (!count($a_found) && count($a_ids)) {
      return FALSE;
    }

    // If we dont expect results we must know it at this point
    $is_empty = count($a_ids) ? FALSE : TRUE;
//$this->var_export($a_found);
    foreach ($a_found as $key_item => $a_item) {
      if (!isset($a_ids[$a_item[$name]])) {
        continue;
      }

      // If granted flag matches, then we found it.
      if (!isset($a_item['granted']) || (($a_ids[$a_item[$name]] && $a_item['granted'] == 'TRUE') || (!$a_ids[$a_item[$name]] && $a_item['granted'] == 'FALSE')) ) {
        // Remove so we know we found it...
        unset($a_ids[$a_item[$name]]);// any left overs where not found
        unset($a_found[$key_item]);// remove the found asset
      }
    }

    // If we still have found, thats not ok...
    if (count($a_found)) {
      return FALSE;
    }

    // If we dont expect any results, then we response must empty as well
    if ($is_empty && !count($a_found)) {
      return TRUE;
    }

    return (count($a_ids) ? FALSE : TRUE);
  }

  // Search for app 1.
  public function cql_do_search_test_assets_1($cql, $a_expected_asset_ids, array $a_parameters = array(), $aut_user_id = FALSE, $aut_group_id = FALSE, $aut_domain = FALSE, $aut_realm = FALSE, $granted = FALSE, $is_public_list = FALSE) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 1.
    $this->switch_to_app = $this->a_app[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $this->cql_do_search_test_assets($cql, $a_expected_asset_ids, $a_parameters, $aut_user_id, $aut_group_id, $aut_domain, $aut_realm, $granted, $is_public_list);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;
  }

  // Search for app 2.
  public function cql_do_search_test_assets_2($cql, $a_expected_asset_ids, array $a_parameters = array(), $aut_user_id = FALSE, $aut_group_id = FALSE, $aut_domain = FALSE, $aut_realm = FALSE, $granted = FALSE, $is_public_list = FALSE) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 2.
    $this->switch_to_app = $this->a_app_2[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $this->cql_do_search_test_assets($cql, $a_expected_asset_ids, $a_parameters, $aut_user_id, $aut_group_id, $aut_domain, $aut_realm, $granted, $is_public_list);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;
  }

  // Search for app 3.
  public function cql_do_search_test_assets_3($cql, $a_expected_asset_ids, array $a_parameters = array(), $aut_user_id = FALSE, $aut_group_id = FALSE, $aut_domain = FALSE, $aut_realm = FALSE, $granted = FALSE, $is_public_list = FALSE) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 3.
    $this->switch_to_app = $this->a_app_3[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $this->cql_do_search_test_assets($cql, $a_expected_asset_ids, $a_parameters, $aut_user_id, $aut_group_id, $aut_domain, $aut_realm, $granted, $is_public_list);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;
  }
}
