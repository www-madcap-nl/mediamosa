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
 * oAuth rest calls.
 */

/**
 * oAuth php.
 * Origin: http://oauth.googlecode.com/svn/code/php/
 */
require_once 'sites/all/modules/mediamosa/lib/oauth/OAuth.php';


// TODO
/**
 * URI: /services/oembed
 * Method: GET
 */
/*
class mediamosa_rest_call_oauth extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const STILL_ID = 'still_id';
  const MAXWIDTH = 'maxwidth';
  const MAXHEIGHT = 'maxheight';
  const FORMAT = 'format';
  const FORMAT_XML = 'xml';
  const FORMAT_JSON = 'json';
  const PROVIDER_NAME = 'provider_name';
  const PROVIDER_URL = 'provider_url';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STILL_ID,
          self::VAR_DESCRIPTION => 'Default this specific still.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::MAXWIDTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The maximum width of the embedded resource.',
          self::VAR_RANGE_START => 0,
        ),
        self::MAXHEIGHT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The maximum height of the embedded resource.',
          self::VAR_RANGE_START => 0,
        ),
        self::FORMAT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_DESCRIPTION => 'The required response format.',
          self::VAR_ALLOWED_VALUES => array(
            self::FORMAT_XML,
            self::FORMAT_JSON,
          ),
          self::VAR_DEFAULT_VALUE => self::FORMAT_JSON,
        ),
        self::PROVIDER_NAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Name of the provider.',
        ),
        self::PROVIDER_URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'URL of the provider.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Get the 1st app.
    $app_id = reset($app_ids);

    // Get params.
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $still_id = $this->get_param_value(self::STILL_ID);
    $maxwidth = $this->get_param_value(self::MAXWIDTH);
    $maxheight = $this->get_param_value(self::MAXHEIGHT);
    $format = $this->get_param_value(self::FORMAT);
    $provider_name = $this->get_param_value(self::PROVIDER_NAME);
    $provider_url = $this->get_param_value(self::PROVIDER_URL);

    // Change the input value to the response value.
    $format = ($format == self::FORMAT_JSON ? mediamosa_rest_call::RESPONSE_TYPE_JSON_OEMBED : mediamosa_rest_call::RESPONSE_TYPE_XML_OEMBED);
    $this->set_param_value(self::FORMAT, $format);

    $mediamosa->add_item($output);
  }
}
 */
