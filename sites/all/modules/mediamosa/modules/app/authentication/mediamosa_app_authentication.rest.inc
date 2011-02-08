<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * The REST calls of the Application Authentication.
  */

/**
 * URI: /login
 * Login onto MediaMosa.
 */
class mediamosa_rest_call_app_ident extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  const DBUS = 'dbus';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS] = array(
      self::DBUS => array(
        mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
        mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
        mediamosa_rest_call::VAR_DESCRIPTION => 'The DBUS login value.',
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the dbus line.
    $dbus = $this->get_param_value(mediamosa_rest_call_app_ident::DBUS);

    // Login.
    $result = mediamosa_app_authentication::app_dbus_protocol($dbus);

    // Must return OK.
    if (mediamosa_unicode::substr($result, 0, 5) == 'ERROR' || mediamosa_unicode::substr($result, 0, 8) == 'REJECTED') {
      $mediamosa->set_result(mediamosa_response::ERROR, mediamosa_error::ERRORCODE_DBUS_PROTOCOL_ERROR, 'DBUS ' . $result);
    }
    else {
      $mediamosa->set_result(mediamosa_response::SUCCESS, mediamosa_error::ERRORCODE_OKAY, 'DBUS authentication protocol');
    }

    $mediamosa->add_item(array('dbus' => $result));
  }
}

/**
 * URI: /check_cookie (deprecated)
 *      /login/cookie
 *
 * Retrieve cookie information.
 */
class mediamosa_rest_call_check_cookie extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Output cookie.
    $mediamosa->add_item($_COOKIE);

    // Ok.
    $mediamosa->set_result_okay();
  }
}

/*
 * URI: /login/oauth/request_token
 * Method: GET
 */
class mediamosa_rest_call_oauth_request_token extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const OAUTH_CONSUMER_KEY = 'oauth_consumer_key';
  const OAUTH_SIGNATURE_METHOD = 'oauth_signature_method';
  const OAUTH_SIGNATURE_METHOD_HMAC_SHA1 = 'HMAC-SHA1';
  const OAUTH_SIGNATURE_METHOD_PLAINTEXT = 'PLAINTEXT';
  const OAUTH_SIGNATURE_METHOD_RSA_SHA1 = 'RSA-SHA1';
  const OAUTH_TIMESTAMP = 'oauth_timestamp';
  const OAUTH_NONCE = 'oauth_nonce';
  const OAUTH_CALLBACK = 'oauth_callback';
  const OAUTH_SIGNATURE = 'oauth_signature';
  const OAUTH_VERSION = 'oauth_version';
  const OAUTH_VERSION_10 = '1.0';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::OAUTH_CONSUMER_KEY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'The identifier portion of the client credentials (equivalent to a username).',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAUTH_SIGNATURE_METHOD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OAUTH_SIGNATURE_METHOD,
          self::VAR_DESCRIPTION => 'The name of the signature method used by the client to sign the request.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_ALLOWED_VALUES => array(
            self::OAUTH_SIGNATURE_METHOD_HMAC_SHA1,
            self::OAUTH_SIGNATURE_METHOD_PLAINTEXT,
            // TODO: Implement RSA.
            //self::OAUTH_SIGNATURE_METHOD_RSA_SHA1,
          ),
          //self::VAR_DEFAULT_VALUE => self::OAUTH_SIGNATURE_METHOD_RSA_SHA1,
        ),
        self::OAUTH_TIMESTAMP => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The timestamp.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 0,
        ),
        self::OAUTH_NONCE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'A nonce is a random string, uniquely generated by the client to allow the server to verify that a request has never been made before and helps prevent replay attacks when requests are made over a non-secure channel.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAUTH_CALLBACK => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URL,
          self::VAR_DESCRIPTION => 'An absolute URI back to which the server will redirect the resource owner when the Resource Owner Authorization step is completed.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAUTH_SIGNATURE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OAUTH_SIGNATURE,
          self::VAR_DESCRIPTION => 'A password portion of the client credentials encoded by the given signature method.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAUTH_VERSION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OAUTH_VERSION,
          self::VAR_DESCRIPTION => 'Provides the version of the authentication process.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            self::OAUTH_VERSION_10,
          ),
          self::VAR_DEFAULT_VALUE => self::OAUTH_VERSION_10,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get params.
    $oauth_consumer_key = $this->get_param_value(self::OAUTH_CONSUMER_KEY);
    $oauth_signature_method = $this->get_param_value(self::OAUTH_SIGNATURE_METHOD);
    $oauth_timestamp = $this->get_param_value(self::OAUTH_TIMESTAMP);
    $oauth_nonce = $this->get_param_value(self::OAUTH_NONCE);
    $oauth_callback = $this->get_param_value(self::OAUTH_CALLBACK);
    $oauth_signature = $this->get_param_value(self::OAUTH_SIGNATURE);
    $oauth_version = $this->get_param_value(self::OAUTH_VERSION);

    // oAuth server.
    $oauth_server = new MediaMosaOAuthServer(new MediaMosaOAuthDataStore());

    $hmac_method = new OAuthSignatureMethod_HMAC_SHA1();
    $plaintext_method = new OAuthSignatureMethod_PLAINTEXT();
    //$rsa_method = new TestOAuthSignatureMethod_RSA_SHA1();

    $oauth_server->add_signature_method($hmac_method);
    $oauth_server->add_signature_method($plaintext_method);
    //$oauth_server->add_signature_method($rsa_method);

    try {
      $req = OAuthRequest::from_request();
      $token = $oauth_server->fetch_request_token($req);
      $token .= '&oauth_callback_confirmed=true';
    } catch (OAuthException $e) {
      print($e->getMessage() . "\n<hr />\n");
      print_r($req);
      die();
    }

    $mediamosa->add_item($token);
  }
}

/*
 * URI: /login/oauth/access_token
 * Method: GET
 */
class mediamosa_rest_call_oauth_access_token extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const OAUTH_CONSUMER_KEY = 'oauth_consumer_key';
  const OAUTH_SIGNATURE_METHOD = 'oauth_signature_method';
  const OAUTH_SIGNATURE_METHOD_HMAC_SHA1 = 'HMAC-SHA1';
  const OAUTH_SIGNATURE_METHOD_PLAINTEXT = 'PLAINTEXT';
  const OAUTH_SIGNATURE_METHOD_RSA_SHA1 = 'RSA-SHA1';
  const OAUTH_TIMESTAMP = 'oauth_timestamp';
  const OAUTH_NONCE = 'oauth_nonce';
  const OAUTH_SIGNATURE = 'oauth_signature';
  const OAUTH_VERSION = 'oauth_version';
  const OAUTH_VERSION_10 = '1.0';
  const OAUTH_TOKEN = 'oauth_token';
  const OAUTH_VERIFIER = 'oauth_verifier';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::OAUTH_CONSUMER_KEY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'The identifier portion of the client credentials (equivalent to a username).',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAUTH_SIGNATURE_METHOD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OAUTH_SIGNATURE_METHOD,
          self::VAR_DESCRIPTION => 'The name of the signature method used by the client to sign the request.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_ALLOWED_VALUES => array(
            self::OAUTH_SIGNATURE_METHOD_HMAC_SHA1,
            self::OAUTH_SIGNATURE_METHOD_PLAINTEXT,
            // TODO: Implement RSA.
            //self::OAUTH_SIGNATURE_METHOD_RSA_SHA1,
          ),
          //self::VAR_DEFAULT_VALUE => self::OAUTH_SIGNATURE_METHOD_RSA_SHA1,
        ),
        self::OAUTH_TIMESTAMP => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The timestamp.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 0,
        ),
        self::OAUTH_NONCE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'A nonce is a random string, uniquely generated by the client to allow the server to verify that a request has never been made before and helps prevent replay attacks when requests are made over a non-secure channel.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAUTH_SIGNATURE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OAUTH_SIGNATURE,
          self::VAR_DESCRIPTION => 'A password portion of the client credentials encoded by the given signature method.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAUTH_VERSION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OAUTH_VERSION,
          self::VAR_DESCRIPTION => 'Provides the version of the authentication process.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            self::OAUTH_VERSION_10,
          ),
          self::VAR_DEFAULT_VALUE => self::OAUTH_VERSION_10,
        ),
        self::OAUTH_TOKEN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OAUTH_TOKEN,
          self::VAR_DESCRIPTION => 'The token identifier.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAUTH_VERIFIER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OAUTH_VERIFIER,
          self::VAR_DESCRIPTION => 'The verification code.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get params.
    $oauth_consumer_key = $this->get_param_value(self::OAUTH_CONSUMER_KEY);
    $oauth_signature_method = $this->get_param_value(self::OAUTH_SIGNATURE_METHOD);
    $oauth_timestamp = $this->get_param_value(self::OAUTH_TIMESTAMP);
    $oauth_nonce = $this->get_param_value(self::OAUTH_NONCE);
    $oauth_signature = $this->get_param_value(self::OAUTH_SIGNATURE);
    $oauth_version = $this->get_param_value(self::OAUTH_VERSION);
    $oauth_token = $this->get_param_value(self::OAUTH_TOKEN);
    $oauth_verifier = $this->get_param_value(self::OAUTH_VERIFIER);

    // oAuth server.
    $oauth_server = new MediaMosaOAuthServer(new MediaMosaOAuthDataStore());

    $hmac_method = new OAuthSignatureMethod_HMAC_SHA1();
    $plaintext_method = new OAuthSignatureMethod_PLAINTEXT();
    //$rsa_method = new TestOAuthSignatureMethod_RSA_SHA1();

    $oauth_server->add_signature_method($hmac_method);
    $oauth_server->add_signature_method($plaintext_method);
    //$oauth_server->add_signature_method($rsa_method);


    try {
      $req = OAuthRequest::from_request();
      $token = $oauth_server->fetch_access_token($req);
    } catch (OAuthException $e) {
      print($e->getMessage() . "\n<hr />\n");
      print_r($req);
      die();
    }

    $mediamosa->add_item($token);
  }
}
