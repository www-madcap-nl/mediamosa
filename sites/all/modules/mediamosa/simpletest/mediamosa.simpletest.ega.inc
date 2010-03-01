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

  // Array with mediafile ids of mediafiles that have files that need to be
  // deleted on teardown.
  private $a_mediafiles_cleanup = array();


  // ------------------------------------------------------------------ Functions.
  /**
   * Implements setUp().
   */
  protected function setUp() {
    // Get the current mediamosa server content.
    $a_servers = mediamosa_server::search();

    // Run parent first so we are inside sandbox.
    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array('mediamosa_app_authentication', 'mediamosa_asset'), $args));
    call_user_func_array(array($this, 'parent::setUp'), $args);

    // Now we are in sandbox, create default APP, our default EGA.
    $this->a_app = $this->createApp();

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

    // Do EGA / APP login.
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

    // Call parent.
    parent::tearDown();
  }

  /**
   * Login using generated app, name and shared_key (password).
   */
  protected function doEgaLogin() {
    // Do login.
    $app_id = $this->a_app[mediamosa_app_db::APP_ID];
    $app_name = $this->a_app[mediamosa_app_db::NAME];
    $shared_key = $this->a_app[mediamosa_app_db::SHARED_KEY];

    // Step 1: start auth and capture cookie
    $response = $this->restCall('login', 'POST', array('dbus' => 'AUTH DBUS_COOKIE_SHA1 ' . $app_name));
    $this->pass(check_plain(var_export($response['xml'], TRUE)));
    $dbus_data = explode(' ', $response['xml']->items->item->dbus);

    $this->assertTrue(drupal_substr($response['xml']->items->item->dbus, 0, 5) == 'DATA ', 'Login fase 1 success');

    list($cookie) = explode(';', $response['headers']['Set-Cookie']);
    $this->assertTrue(isset($response['headers']['Set-Cookie']), 'Cookie received in HTTP header: '. $cookie);

    // Step 2: send the credentials
    $challenge = $dbus_data[3];
    $random = substr(md5(microtime(TRUE)), 0, 10);
    $post_data = array('dbus' => 'DATA '. $random .' '. sha1(sprintf('%s:%s:%s', $challenge, $random, $shared_key)));
    $response = $this->restCall('login', 'POST', $post_data);

    $this->assertTrue($response['xml']->items->item->dbus == 'OK server_guid', 'Login fase 2 success');
    $this->pass(check_plain(var_export($response['xml'], TRUE)));
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
    $this->pass('POST request to: ' . $action .
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

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_asset_get::APP_ID]));

    $uri = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $a_parameters);

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
    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_get_mediafile::APP_ID]));

    $uri = strtr('mediafile/@mediafile_id', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $a_parameters);

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
    $response = $this->restCallGet($uri, $a_parameters);

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

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_asset_create::APP_ID]));

    // Do Post call.
    $response = $this->restCallPost('asset/create', $a_parameters);

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
    $response = $this->restCallPost($uri, $a_parameters);

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

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_asset_create::APP_ID]));

    // Do Post call.
    $response = $this->restCallPost('mediafile/create', $a_parameters);

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

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_asset_create::APP_ID]));

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/uploadticket/create', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters);

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

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_asset_create::APP_ID]));

    // Set the URI.
    $uri = strtr('asset/@asset_id/supplement/create', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters);

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

    // Required.

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_asset_create::APP_ID]));

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters);

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
    $response = $this->restCallPost($uri, $a_parameters);

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
    $response = $this->restCallPost($uri, $a_parameters);

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
    $response = $this->restCallPost($uri, $a_parameters);

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
}
