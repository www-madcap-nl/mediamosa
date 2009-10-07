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
  * The REST call object provides the REST call function data and excecution.
  */
abstract class mediamosa_rest_call {
  // ------------------------------------------------------------------ Members.
  // Original REST call arguments.
  private $a_rest_args = array();

  // Check if there are unspecified arguments and throw exception when we have unspecified arguments.
  protected $m_check_for_unspecified = FALSE;

  // ------------------------------------------------------------------ Constants.
  const ID = 'uri_id';
  const URI = 'uri';
  const URI_PARAMS = 'uri_params';

  const RESPONSE_TYPE = 'response_type';
  const RESPONSE_TYPE_XML = 'text/xml';
  const RESPONSE_TYPE_PLAIN = 'text/plain';

  const METHOD = 'method';
  const METHOD_GET = 'method';
  const METHOD_POST = 'method';
  const METHOD_PUT = 'method';
  const METHOD_DELETE = 'method';

  const CLASS_NAME = 'class_name';

  const MODULE_NAME = 'module_name';

  const STATUS = 'status';
  const STATUS_ACTIVE = 'active';
  const STATUS_DEPRECATED = 'deprecated';
  const STATUS_DISABLED = 'disabled';

  const VERSION = 'version';
  const VERSION_MAX_LEN = 12;

  const DESCRIPTION = 'description';   // Description of the REST call (In English)

  // Default var names.
  const LIMIT = 'limit';
  const OFFSET = 'offset';

  // VAR array names.
  const VARS = 'vars';
  const VARS_DESCRIPTION = 'description';

  const VAR_TYPE = 'type';
  const VAR_DEFAULT_VALUE = 'default_value';
  const VAR_RANGE_START = 'range_start';
  const VAR_RANGE_END = 'range_end';
  const VAR_IS_ARRAY = 'is_array';         // default no.
  const VAR_IS_REQUIRED = 'is_required';   // default no.
  const VAR_TRIM_VALUE = 'trim_value';     // default no.
  const VAR_DESCRIPTION = 'description';   // Description of the var (In English).
  const VAR_ALLOWED_VALUES = 'a_allowed_values'; // Optional array for checking against allowed values.
  const VAR_IS_READ_ONLY = 'read_only'; // default no. Some vars may not be changed.

  // Values VAR_IS_REQUIRED
  const VAR_IS_REQUIRED_YES = 'YES';
  const VAR_IS_REQUIRED_NO = 'NO';

  // Values VAR_IS_ARRAY
  const VAR_IS_ARRAY_YES = 'YES';        // Make it an array, if not.
  const VAR_IS_ARRAY_NO = 'NO';          // May not be array, if not, take 1st element.

  // Values TRIM_VALUE
  const VAR_TRIM_VALUE_YES = 'YES';
  const VAR_TRIM_VALUE_NO = 'NO';

  // Values TRIM_VALUE
  const VAR_IS_READ_ONLY_YES = 'YES';
  const VAR_IS_READ_ONLY_NO = 'NO';

  // Param fields
  const PARAM_VALUE = 'value';
  const PARAM_ISSET = 'isset';

  /**
   * Main constructor.
   *
   * @param rest_call $o_rest_call
   */
  function __construct($a_rest_call) {
    // Copy us.
    $this->{self::URI} = $a_rest_call[self::URI];
    $this->{self::RESPONSE_TYPE} = $a_rest_call[self::RESPONSE_TYPE];
    $this->{self::METHOD} = $a_rest_call[self::METHOD];
    $this->{self::CLASS_NAME} = $a_rest_call[self::CLASS_NAME];
    $this->{self::STATUS} = $a_rest_call[self::STATUS];
    $this->{self::VERSION} = $a_rest_call[self::VERSION];
    $this->{self::DESCRIPTION} = $a_rest_call[self::DESCRIPTION];

    // Other.
    $this->{self::URI_PARAMS} = isset($a_rest_call[self::URI_PARAMS]) ? $a_rest_call[self::URI_PARAMS] : array();
  }

  // ------------------------------------------------------------------ Functions (abstract).

  // Get the rest input info.
  abstract function get_var_setup();
  // The execution of the call.
  abstract function do_call();

  // ------------------------------------------------------------------ Functions (public).

  /**
   * Dispatch the rest call processing.
   */
  public function process_call() {
    // Make sure its not disabled.
    if ($this->{self::STATUS} == self::STATUS_DISABLED) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_CALL_IS_DISABLED, array('@uri' => $this->{self::URI}));
    }

    // Get the var setup.
    $a_var_setup = $this->get_var_setup();

    // Assert when failed.
    assert($a_var_setup);

    // @todo: make this an setting per webservice or else we are not 1.x compatible. Default should be FALSE.
    $this->m_check_for_unspecified = TRUE;

    // Check rest args.
    $this->process_rest_args($a_var_setup);

    // Call the function.
    $this->do_call();
  }

  /**
   * Get the default vars we always expect in every rest call
   */
  protected function get_var_setup_default($a_my_vars) {
//    $a_my_vars['vars'][vp_db::APP_ID] = array(vp_rest_db::VAR_TYPE => vp_type::TYPE_APP_ID, vp_rest_db::VAR_IS_ARRAY => vp_rest_db::VAR_IS_ARRAY_NO, vp_rest_db::VAR_IS_REQUIRED => vp_rest_db::VAR_IS_REQUIRED_YES);

    return $a_my_vars;
  }

  /**
   * Enrich the VAR setup with the default LIMIT and OFFSET
   *
   * @param array $a_my_vars
   */
  protected function set_var_setup_range($a_my_vars, $limit_max = mediamosa_db::LIMIT_MAX) {

    $a_my_vars[mediamosa_rest_call::VARS][mediamosa_rest_call::LIMIT] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_UINT,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_NO,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DEFAULT_VALUE => $limit_max,
      mediamosa_rest_call::VAR_IS_READ_ONLY => mediamosa_rest_call::VAR_IS_READ_ONLY_YES, // Once set, dont change.
    );

    $a_my_vars[mediamosa_rest_call::VARS][mediamosa_rest_call::OFFSET] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_type::TYPE_UINT,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_NO,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DEFAULT_VALUE => 0,
      mediamosa_rest_call::VAR_IS_READ_ONLY => mediamosa_rest_call::VAR_IS_READ_ONLY_YES, // Once set, dont change.
      );

    return $a_my_vars;
  }

  /**
   * Retrieve the value of a supplied parameter.
   *
   * @param string $param
   * @return mixed
   */
  public function get_param_value($param) {
    if (!isset($this->a_rest_args[$param])) {
      throw new mediamosa_exception_program_error('Calling get_param with non-existing param (@param), should not happen; define it in allowed vars for your rest call class "@restcall"', array('@param' => $param, '@restcall' => $this->{self::CLASS_NAME}));
    }

    return $this->a_rest_args[$param][self::PARAM_VALUE];
  }

  /**
   * Set the value of the supplied parameter
   *
   * @param string $param
   * @param string $value
   */
  public function set_param_value($param, $value) {
    if ($this->a_rest_args[$param][self::VAR_IS_READ_ONLY] == self::VAR_IS_READ_ONLY_YES) {
      throw new mediamosa_exception_program_error('Do not set param @param, its read only.', array('@param' => $param));
    }

    $this->a_rest_args[$param][self::PARAM_VALUE] = $value;
  }

  /**
   * Will return TRUE if the param was set during call using GET / POST.
   *
   * @param string $param
   * @return boolean
   */
  public function isset_param($param) {
    return isset($this->a_rest_args[$param]) && $this->a_rest_args[$param][self::PARAM_ISSET];
  }

  // ------------------------------------------------------------------ Functions (protected).
  /**
   * Process the supplied parameters and type validation.
   *
   * @param array $a_var_setup
   */
  protected function process_rest_args($a_var_setup) {

    // Clear it.
    $this->a_rest_args = array();
    foreach ($this->{self::URI_PARAMS} as $key_uri => $value_uri) {
      if (!isset($a_var_setup[self::VARS][$key_uri]) && $this->m_check_for_unspecified) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_uri));
      }

      $this->a_rest_args[$key_uri][self::PARAM_VALUE] = $value_uri;
    }

    // Fill the $a_rest_args array with rest input.
    foreach ($_GET as $key_get => $value_get) {
      if ($key_get == 'q') {
        continue; // Ignore the q variable.
      }

      if (!isset($a_var_setup[self::VARS][$key_get]) && $this->m_check_for_unspecified) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_get));
      }

      if (isset($this->a_rest_args[$key_get])) {
        if ($this->a_rest_args[$key_get][self::PARAM_VALUE] != $value_get) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR, array('@var' => $key_get, '@value1' => $this->a_rest_args[$key_get][self::PARAM_VALUE], '@value2' => $value_get));
        }

        continue;
      }

      $this->a_rest_args[$key_get][self::PARAM_VALUE] = $value_get;
    }

    foreach ($_POST as $key_post => $value_post) {
      if (!isset($a_var_setup[self::VARS][$key_post]) && $this->m_check_for_unspecified) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_post));
      }

      if (isset($this->a_rest_args[$key_post])) {
        if ($this->a_rest_args[$key_post][self::PARAM_VALUE] != $value_post) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR, array('@var' => $key_post, '@value1' => $this->a_rest_args[$key_post][self::PARAM_VALUE], '@value2' => $value_post));
        }

        continue;
      }

      $this->a_rest_args[$key_post][self::PARAM_VALUE] = $value_post;
    }

    // All args are now known. Now walk through the setup and check the formats and contents.
    foreach ($a_var_setup[self::VARS] as $param => $a_var) {

      // Make sure its set, even when empty.
      $this->a_rest_args[$param][self::PARAM_ISSET] = isset($this->a_rest_args[$param]);
      if (!$this->a_rest_args[$param][self::PARAM_ISSET]) {
        $this->a_rest_args[$param][self::PARAM_VALUE] = ''; // Indicate is was not set

        // Defaults.
        if (isset($a_var[self::VAR_DEFAULT_VALUE])) {
          $this->a_rest_args[$param][self::PARAM_VALUE] = $a_var[self::VAR_DEFAULT_VALUE];
          $this->a_rest_args[$param][self::PARAM_ISSET] = true; // set with default
        }
      }

      // array check
      if (!isset($a_var[self::VAR_IS_ARRAY]) || $a_var[self::VAR_IS_ARRAY] == self::VAR_IS_ARRAY_NO) {
        if (is_array($this->a_rest_args[$param][self::PARAM_VALUE])) {
          $this->a_rest_args[$param][self::PARAM_VALUE] = reset($this->a_rest_args[$param][self::PARAM_VALUE]);
        }
      }
      elseif (!is_array($this->a_rest_args[$param][self::PARAM_VALUE])) {
        $this->a_rest_args[$param][self::PARAM_VALUE] = ($this->a_rest_args[$param][self::PARAM_ISSET] ? array($this->a_rest_args[$param][self::PARAM_VALUE]): array());
      }

      // Check if we need to trim it.
      if (isset($a_var[self::VAR_TRIM_VALUE]) && $a_var[self::VAR_TRIM_VALUE] == self::VAR_TRIM_VALUE_YES) {
        $this->a_rest_args[$param][self::PARAM_VALUE] = trim($this->a_rest_args[$param][self::PARAM_VALUE]);
      }

      // Set default value for is_required.
      if (!isset($a_var[self::VAR_IS_REQUIRED])) {
        $a_var[self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;
      }

      // Set read only to no, if not set.
      if (!isset($a_var[self::VAR_IS_READ_ONLY])) {
        $a_var[self::VAR_IS_READ_ONLY] = self::VAR_IS_READ_ONLY_NO;
      }

      // Copy Read only status.
      $this->a_rest_args[$param][self::VAR_IS_READ_ONLY] = $a_var[self::VAR_IS_READ_ONLY];

      // if param is specified with range and the param is provided during rest call or had defaulft value, then check range.
      $range_start = isset($a_var[self::VAR_RANGE_START]) && $this->isset_param($param) ? $a_var[self::VAR_RANGE_START] : NULL;
      $range_end = isset($a_var[self::VAR_RANGE_END]) && $this->isset_param($param) ? $a_var[self::VAR_RANGE_END] : NULL;

      // Check type and type conversion.
      // Store the value returned.
      // Will throw Exception when anything goes wrong.
      assert(!isset($a_var[self::VAR_ALLOWED_VALUES]) || (isset($a_var[self::VAR_ALLOWED_VALUES]) && is_array($a_var[self::VAR_ALLOWED_VALUES])));
      $this->a_rest_args[$param][self::PARAM_VALUE] = mediamosa_type::check($param, $a_var[self::VAR_TYPE], $this->a_rest_args[$param][self::PARAM_VALUE], $a_var[self::VAR_IS_REQUIRED] == self::VAR_IS_REQUIRED_YES, $range_start, $range_end, isset($a_var[self::VAR_ALLOWED_VALUES]) && count($a_var[self::VAR_ALLOWED_VALUES]) ? $a_var[self::VAR_ALLOWED_VALUES] : NULL);
    }
  }
}
