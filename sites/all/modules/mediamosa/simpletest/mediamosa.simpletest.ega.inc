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
    $args = array_unique(array_merge(array('mediamosa_asset', 'mediamosa_app_authentication'), $args));
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
   * Create an asset (using /asset/create).
   *
   * @param array $a_parameters
   */
  protected function createAsset(array $a_parameters = array()) {
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

    $this->pass(strtr("Create asset with ID '@id'.", array('@id' => $response['xml']->items->item[0]->asset_id)));

    // Return the asset_id.
    return $response['xml']->items->item[0]->asset_id;
  }


    /**
   * Create an mediafile (using /mediafile/create).
   *
   * @param string $asset_id
   * @param string $user_id
   * @param array $a_parameters
   */
  protected function createMediafile($asset_id, $user_id = self::SIMPLETEST_USER_ID, array $a_parameters = array()) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_create_mediafile::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_create_mediafile::IS_DOWNLOADABLE => 'false',
    );

    // Required.
    $a_parameters[mediamosa_rest_call_create_mediafile::ASSET_ID] = $asset_id;
    $a_parameters[mediamosa_rest_call_create_mediafile::USER_ID] = $user_id;

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_asset_create::APP_ID]));

    // Do Post call.
    $response = $this->restCallPost('mediafile/create', $a_parameters);

    $this->pass(strtr("Create mediafile with ID '@id'.", array('@id' => $response['xml']->items->item[0]->mediafile_id)));

    // Return the mediafile ID.
    return $response['xml']->items->item[0]->mediafile_id;
  }

  /**
   * Create upload ticket.
   *
   * @param string $mediafile_id
   * @param string $user_id
   * @param array $a_parameters
   */
  protected function createMediafileUploadTicket($mediafile_id, $user_id = self::SIMPLETEST_USER_ID, array $a_parameters = array()) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_asset_mediafile_uploadticket_create::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_mediafile_uploadticket_create::GROUP_ID => '',
    );

    // Required.
    $a_parameters[mediamosa_rest_call_asset_mediafile_uploadticket_create::USER_ID] = $user_id;

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($a_parameters[mediamosa_rest_call_asset_create::APP_ID]));

    $uri = strtr('mediafile/@mediafile_id/uploadticket/create', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters);

    $this->pass(strtr("Create upload ticket with action '@action', uploadprogress_url '@uploadprogress_url'.", array('@action' => $response['xml']->items->item[0]->action, '@uploadprogress_url' => $response['xml']->items->item[0]->uploadprogress_url)));

    // Return data.
    return array('action' => $response['xml']->items->item[0]->action, 'uploadprogress_url' => $response['xml']->items->item[0]->uploadprogress_url);
  }
}
