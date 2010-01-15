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
abstract class MediaMosaTestCase extends DrupalWebTestCase {
  // ------------------------------------------------------------------ Functions (protected)

  /**
   * Implements setUp().
   */
  protected function setUp() {
    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array('mediamosa_app', 'mediamosa'), $args));
    call_user_func_array(array($this, 'parent::setUp'), $args);
  }

  /**
   * Do a REST call inside the sandbox.
   *
   * @param array $a_app
   *  The app used during the call.
   * @param string $uri
   *  The uri of the REST call.
   * @param string $request_method
   *  POST / GET / PUT / DELETE.
   * @param array $parameters
   * @param unknown_type $headers
   */
  protected function restCall(array $a_app, $uri, $request_method = 'POST', array $a_parameters = array(), array $a_headers = array()) {
    // Add MediaMosa menu path prefix
    //$uri = /*MEDIAMOSA_MENU_PATH . '/'*/ $uri;

    // Add default parameters
    $a_parameters += array(
      'app_id' => $a_app[mediamosa_app_db::APP_ID],
    );

    // Prepare the CURL options
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

    // Perform the CURL request
    $result = $this->curlExec($curl_opts);

    // Parse the headers and get the body
    $lines = explode("\n", $result);
    $line_count = count($lines);

    // Unset the HTTP header line
    unset($lines[0]);

    // Iterate until an empty row is found
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

    // Create a SimpleXMLElement from the restcall's output
    try {
      $out['xml'] = new SimpleXMLElement($out['body']);
    }
    catch (Exception $e) {
      $this->assert(FALSE, $e->getMessage());
      return;
    }

    // Look for a request_result_id
    $this->assertTrue(isset($out['xml']->header->request_result_id), $request_method .' request to: "'. $uri .'". Result id: ' . $out['xml']->header->request_result_id .'<hr />');

    // Return the request data
    return $out;
  }

  /**
   * Create a random app or specified one.
   * @param array $a_attributes
   *  Basic app parameters like the DB version (mediamosa_app_db).
   */
  protected function createApp($a_attributes = array()) {
    // Merge in the default attributes.
    $a_attributes += array(
      mediamosa_app_db::APP_ID => rand(1, 1000),
      mediamosa_app_db::SHARED_KEY => user_password(),
      mediamosa_app_db::NAME => $this->randomName(),
      mediamosa_app_db::ACTIVE => 'TRUE',
      mediamosa_app_db::TRANSCODING_POSITION_TO => 0,
      mediamosa_app_db::QUOTA => 0,
      mediamosa_app_db::QUOTA_USED => 0,
      mediamosa_app_db::ACTIVE_VERSION => '',
    );

    // Insert the app into the database
    $query = mediamosa_db::db_insert(mediamosa_app_db::TABLE_NAME)->fields($a_attributes)->execute();

    // Log the creation in the resutls
    $this->assert(TRUE, t('App created with id %app_id, name %name and shared key %pass', array('%app_id' => $a_attributes['app_id'], '%name' => $a_attributes['name'], '%pass' => $a_attributes['shared_key'])));

    return $a_attributes;
  }
}
