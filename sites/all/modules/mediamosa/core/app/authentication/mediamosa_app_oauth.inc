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
 * sites/all/modules/mediamosa/lib/oauth/OAuth.php:
 *
 * The MIT License
 *
 * Copyright (c) 2007 Andy Smith
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

 /**
  * @file
  * The oAuth class.
  */

class mediamosa_app_oauth {

  // ------------------------------------------------------------------ Consts.
  const CLEAN_UP_EXPIRE_TIME = 300;
  const REQUEST_TOKEN_EXPIRE_TIME = 7200;

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Log for ticket.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'oauth');
  }

  /**
   * Get nonce.
   *
   * @param $nonce
   * @param $timestamp
   * @param $token
   *
   * @return $nonce
   */
  static public function get_nonce($nonce, $timestamp, $token) {
    $query = db_select(mediamosa_app_nonce_db::TABLE_NAME, 'nonce');

    $query->fields('nonce', array(mediamosa_app_nonce_db::NONCE));
    $query->condition('nonce.' . mediamosa_app_nonce_db::NONCE, $nonce);
    $query->condition('nonce.' . mediamosa_app_nonce_db::TIMESTAMP, $timestamp, '<=');
    if (empty($token->key)) {
      $query->isNull('nonce.' . mediamosa_app_nonce_db::TOKEN);
    }
    else {
      $query->condition('nonce.' . mediamosa_app_nonce_db::TOKEN, $token->key);
    }

    $result = $query->execute();

    $stored_nonce = $result->fetchField();

    return $stored_nonce;
  }

  /**
   * Get token.
   *
   * @param $token_key
   *
   * @return $token
   */
  static public function get_token($token_key) {
    $query = db_select(mediamosa_app_token_db::TABLE_NAME, 'token');

    $query->fields('token', array());
    $query->condition('token.' . mediamosa_app_token_db::TOKEN_KEY, $token_key);

    $result = $query->execute();

    $token = $result->fetchAssoc();

    return $token;
  }

  /**
   * Save nonce.
   *
   * @param $nonce
   * @param $timestamp
   * @param $token
   */
  static public function save_nonce($nonce, $timestamp, $token) {
    mediamosa_db::db_insert(mediamosa_app_nonce_db::TABLE_NAME)
    ->fields(array(
      mediamosa_app_nonce_db::NONCE => $nonce,
      mediamosa_app_nonce_db::TIMESTAMP => $timestamp,
      mediamosa_app_nonce_db::TOKEN => !empty($token->key) ? $token->key : NULL,
    ))
    ->execute();
  }

  /**
   * Save token.
   *
   * @param $parameters
   */
  static public function save_token($parameters) {
    $time = time();
    $parameters += array(
      mediamosa_app_token_db::CREATED => $time,
      mediamosa_app_token_db::CHANGED => $time,
      mediamosa_app_token_db::EXPIRES => $time + self::REQUEST_TOKEN_EXPIRE_TIME,
    );

    mediamosa_db::db_insert(mediamosa_app_token_db::TABLE_NAME)
    ->fields($parameters)
    ->execute();
  }

  /**
   * Update token.
   *
   * @param $parameters
   */
  static public function update_token($token_key, $parameters) {
    $time = time();
    $parameters += array(
      mediamosa_app_token_db::CHANGED => $time,
    );

    $num_updated = mediamosa_db::db_update(mediamosa_app_token_db::TABLE_NAME)
    ->fields($parameters)
    ->condition(mediamosa_app_token_db::TOKEN_KEY, $token_key)
    ->execute();
  }

  /**
   * Clean up the token table.
   */
  static public function clean_up_token() {
    mediamosa_db::db_query("
      DELETE FROM {#mediamosa_app_token}
      WHERE #expires != 0 AND #expires <= :expires
    ", array(
      '#mediamosa_app_token' => mediamosa_app_token_db::TABLE_NAME,
      '#expires' => mediamosa_app_token_db::EXPIRES,
      ':expires' => time(),
    ));
  }

  /**
   * Clean up the nonce table.
   */
  static public function clean_up_nonce() {
    mediamosa_db::db_query("
      DELETE FROM {#mediamosa_app_nonce}
      WHERE #timestamp < :timestamp
    ", array(
      '#mediamosa_app_nonce' => mediamosa_app_nonce_db::TABLE_NAME,
      '#timestamp' => mediamosa_app_nonce_db::TIMESTAMP,
      ':timestamp' => time() - self::CLEAN_UP_EXPIRE_TIME,
    ));
  }

  /**
   * Clean up the token and the nonce table.
   */
  static public function clean_up() {
    self::clean_up_token();
    self::clean_up_nonce();
  }
}


/**
 * MediaMosa oAuth server.
 */
class MediaMosaOAuthServer extends OAuthServer {
  public function get_signature_methods() {
    return $this->signature_methods;
  }
}


/**
 * MediaMosa data store.
 */
class MediaMosaOAuthDataStore extends OAuthDataStore {
  /*
  private $consumer;
  private $request_token;
  private $access_token;
  private $nonce;
  */

  function __construct() {
    /*
    $this->consumer = new OAuthConsumer("key", "secret", NULL);
    $this->request_token = new OAuthToken("requestkey", "requestsecret", 1);
    $this->access_token = new OAuthToken("accesskey", "accesssecret", 1);
    $this->nonce = "nonce";
    */
  }

  /**
   * Check if consumer exists from a given consumer key.
   *
   * @param string $consumer_key
   *   The consumer key.
   * @param bool $provider_consumer
   *  Whether the consumer should be a provider consumer
   * @return OAuthConsumer
   *  The consumer
   * @throws OAuthException
   *  An exception is thrown when the consumer cannot be found
   */
  function lookup_consumer($consumer_key) {
    $app = mediamosa_app::get_by_oauth_consumer_key($consumer_key);

    if ($consumer_key == $app[mediamosa_app_db::OAUTH_CONSUMER_KEY]) {
      $consumer = new OAuthConsumer($app[mediamosa_app_db::OAUTH_CONSUMER_KEY], $app[mediamosa_app_db::OAUTH_CONSUMER_SECRET], NULL);
      return $consumer;
    }

    return NULL;
  }

  /**
   * Check if the token exists.
   *
   * @param OAuthConsumer $consumer
   *  The consumer.
   * @param string $token_type
   *  The type of the token: 'request' or 'access'.
   * @param string $token
   *  The token key.
   * @param bool $provider_token
   *  Whether the token should be a provider token
   * @return DrupalOauthToken
   *  The matching token
   * @throws OAuthException
   *  An exception is thrown when the token cannot be found or doesn't match
   */
  function lookup_token($consumer, $token_type, $token) {
    echo '*** TODO: lookup_token';
    die;
    /*
    $token_attrib = $token_type . "_token";
    if ($consumer->key == $this->consumer->key && $token == $this->$token_attrib->key) {
      return $this->$token_attrib;
    }
    return NULL;
    */
  }
  /*
  public function lookup_token($consumer, $token_type, $token, $provider_token=TRUE) {
    $token = DrupalOAuthToken::load($token, $provider_token);
    if ($token) {
      if ($token->type==$token_type
          && $token->consumer_key==$consumer->key
          && $token->provider_token==$provider_token) {
        return $token;
      }
      throw new OAuthException('No matching token was found, token type or consumer key mismatch');
    }
    throw new OAuthException('Token not found');
  }
  */

  /**
   * Check if the nonce value exists. If not, generate one.
   *
   * @param OAuthConsumer $consumer
   *   The service consumer information with both key
   *   and secret values.
   * @param OAuthToken $token
   *   The current token.
   * @param string $nonce
   *   A new nonce value, in case a one doesn't current exit.
   * @param int $timestamp
   *   The current time.
   * @return string
   *   The existing nonce value or NULL in
   *   case it doesn't exist.
   */
  function lookup_nonce($consumer, $token, $nonce, $timestamp) {
    $stored_nonce = mediamosa_app_oauth::get_nonce($nonce, $timestamp, $token);

    if ($stored_nonce) {
      return $stored_nonce;
    }

    mediamosa_app_oauth::save_nonce($nonce, $timestamp, $token);
    return NULL;
  }

  /**
   * Generate a new request token.
   *
   * @param OAuthConsumer $consumer
   *  The consumer to generate a token for.
   * @return DrupalOAuthToken
   *  The request token
   */
  function new_request_token($consumer, $callback_url = null) {
    $token_key = user_password(32);
    $token_secret = user_password(32);
    $request_token = new OAuthToken($token_key, $token_secret);

    $parameters = array(
      // Same as $token_key.
      mediamosa_app_token_db::TOKEN_KEY => $request_token->key,
      // Same as $token_secret.
      mediamosa_app_token_db::SECRET => $request_token->secret,
      mediamosa_app_token_db::TYPE => mediamosa_app_token_db::TYPE_REQUEST,
      mediamosa_app_token_db::APP_ID => mediamosa_app_token_db::APP_ID_NON,
      mediamosa_app_token_db::CONSUMER_KEY => $consumer->key,
      mediamosa_app_token_db::AUTHORIZED => mediamosa_app_token_db::AUTHORIZED_NO,
      mediamosa_app_token_db::CALLBACK_URL => $callback_url,
    );
    mediamosa_app_oauth::save_token($parameters);

    return $request_token;

    // TODO ?
    //if ($consumer->key == $this->consumer->key) {
    //  return $this->request_token;
    //}
    //return NULL;
  }

  /**
   * Generate a new access token and delete the old request token.
   *
   * @param DrupalOAuthToken $token_old
   *   The old request token.
   * @param OAuthConsumer $consumer
   *   The service consumer information.
   */
  function new_access_token($token, $consumer) {
    echo '*** TODO: new_access_token';
    die;
    /*
    if ($consumer->key == $this->consumer->key && $token->key == $this->request_token->key) {
      return $this->access_token;
    }
    return NULL;
    */
  }
  /*
  function new_access_token($token_old, $consumer, $verifier = null) {
    module_load_include('inc', 'oauth_common');

    if ($token_old && $token_old->authorized) {
      $token_new = new DrupalOAuthToken(user_password(32), user_password(32), array(
        'type'            => 'access',
        'uid'             => $token_old->uid,
        'consumer_key'    => $consumer->key,
        'provider_token'  => TRUE,
        'services'        => $token_old->services,
        'authorized'      => 1,
      ));
      $token_old->delete();
      $token_new->write();
      return $token_new;
    }

    throw new OAuthException('Invalid request token');
  }
  */
}
