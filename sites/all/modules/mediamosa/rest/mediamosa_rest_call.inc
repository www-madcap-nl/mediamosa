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
  protected $check_for_unspecified = FALSE;

  // ------------------------------------------------------------------ Constants.
  const ID = 'uri_id';
  const URI = 'uri';
  const URI_PARAMS = 'uri_params';

  const RESPONSE_TYPE = 'response_type';
  const RESPONSE_TYPE_XML = 'text/xml';
  const RESPONSE_TYPE_PLAIN = 'text/plain';

  const METHOD = 'method';
  const METHOD_GET = 'GET';
  const METHOD_POST = 'POST';
  const METHOD_PUT = 'PUT';
  const METHOD_DELETE = 'DELETE';

  const CLASS_NAME = 'class_name';

  const MODULE_NAME = 'module_name';

  const STATUS = 'status';
  const STATUS_ACTIVE = 'active';
  const STATUS_DEPRECATED = 'deprecated';
  const STATUS_DISABLED = 'disabled';

  const VERSION = 'version';
  const VERSION_MAX_LEN = 12;

  // Access to the REST call.
  const ACCESS = 'access';      // Value OR'd;
  const ACCESS_INTERNAL = 1;    // Access to REST call when internally accessed.
  const ACCESS_INTERNAL_ONLY = 1;    // Access to REST call internally only..
  const ACCESS_EXTERNAL = 2;    // Access to REST call when externally accessed.
  const ACCESS_EXTERNAL_ONLY = 2;    // Access to REST call externally only.
  const ACCESS_AUTHENTICATED = 4; // Access to REST call when authenticated (true came back in the auth hook).
  const ACCESS_AUTHENTICATED_ONLY = 4; // Access to REST call when authenticated only.
  const ACCESS_DEFAULT = 5;     // Access default allowes external and authenticated access.

  const DESCRIPTION = 'description';   // Description of the REST call (In English, required).

  /**
   * 'alias_for' contains the URI of the REST call that also runs on this rest URI.
   * Any settings in this array is instead used of the original.
   */
  const ALIAS_FOR = 'alias_for';

  // Default var names / values.
  const APP_ID = 'app_id';
  const LIMIT = 'limit';
  const OFFSET = 'offset';
  const ORDER_BY = 'order_by';
  const ORDER_DIRECTION = 'order_direction';
  const IS_APP_ADMIN = 'is_app_admin';

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
  const VAR_IS_INTERNAL_ONLY = 'internal_only'; // If true, then this variable is only accepted by internals.

  // Values VAR_IS_REQUIRED
  const VAR_IS_REQUIRED_YES = 'YES';
  const VAR_IS_REQUIRED_NO = 'NO';

  // Values VAR_IS_ARRAY
  const VAR_IS_ARRAY_YES = 'YES';        // Make it an array, if not.
  const VAR_IS_ARRAY_NO = 'NO';          // May not be array, if not, take 1st element.

  // Values TRIM_VALUE
  const VAR_TRIM_VALUE_YES = 'YES';
  const VAR_TRIM_VALUE_NO = 'NO';

  // Values VAR_IS_READ_ONLY
  const VAR_IS_READ_ONLY_YES = 'YES';
  const VAR_IS_READ_ONLY_NO = 'NO';

    // Values VAR_IS_INTERNAL_ONLY
  const VAR_IS_INTERNAL_ONLY_YES = 'YES';
  const VAR_IS_INTERNAL_ONLY_NO = 'NO';

  // Param fields
  const PARAM_VALUE = 'value';
  const PARAM_TYPE = 'type';
  const PARAM_ISSET_DEFAULT = 'isset_default'; // Value isset, but was set with default value.
  const PARAM_VALIDATED = 'validated';
  const PARAM_RANGE_START = 'range_start';
  const PARAM_RANGE_END = 'range_end';
  const PARAM_IS_READ_ONLY = 'read_only';
  const PARAM_IS_REQUIRED = 'is_required';
  const PARAM_ALLOWED_VALUES = 'a_allowed_values';

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

    // Access.
    $this->{self::ACCESS} = $a_rest_call[self::ACCESS];

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
    $this->check_for_unspecified = TRUE;

    // Check rest args.
    $a_var_setup = $this->process_rest_args($a_var_setup);
    assert($a_var_setup);

    // Validate reg args.
    $this->validate_rest_args($a_var_setup);

    // Call the function.
    $this->do_call();
  }

  /**
   * Get the default vars we always expect in every rest call
   */
  protected function get_var_setup_default(array $a_var_setup) {
    $a_var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::APP_ID] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_APP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => mediamosa::t('The application ID.'),
    );

    // Optional.
    // @todo: only in some case, like internals?
    $a_var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::IS_APP_ADMIN] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_BOOL,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DESCRIPTION => mediamosa::t('A boolean to indicate override by global APP admin. Is ignored by most REST calls.'),
      self::VAR_DEFAULT_VALUE => 'FALSE',
    );

    return $a_var_setup;
  }

  /**
   * Enrich the VAR setup with the default LIMIT and OFFSET
   *
   * @param array $a_var_setup
   *   Provide current array to merge.
   */
  protected function get_var_setup_range(array $a_var_setup = array(), $limit_max = mediamosa_settings::LIMIT_MAX, $limit_default = mediamosa_settings::LIMIT_DEFAULT) {

    $a_var_setup[self::VARS][self::LIMIT] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_LIMIT,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => $limit_default,
      self::VAR_IS_READ_ONLY => self::VAR_IS_READ_ONLY_YES, // Once set, dont change.
      self::VAR_DESCRIPTION => mediamosa::t('Limit of the items of the result set.'),
      self::VAR_RANGE_START => 0,
      self::VAR_RANGE_END => $limit_max,
    );

    $a_var_setup[self::VARS][self::OFFSET] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_UINT,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 0,
      self::VAR_IS_READ_ONLY => self::VAR_IS_READ_ONLY_YES, // Once set, dont change.
      self::VAR_DESCRIPTION => mediamosa::t('The starting items position in the result set.'),
    );

    return $a_var_setup;
  }

  /**
   * Gives true back if call is internal call.
   * @todo: actualy return true when internal.
   */
  protected function call_is_internal() {
  	return FALSE;
  }

  /**
   * Gives true back if call is internal call.
   * @todo: actualy return true when external.
   */
  protected function call_is_external() {
    return FALSE;
  }

  /**
   * Enrich the VAR setup with the default ORDER BY, ORDER_DIRECTION
   *
   * @param array $a_var_setup
   *   Provide current array to merge.
   * @param string $default_order_by
   *  Default field to order on.
   * @param array $a_allowed_values_order_by
   *  array containing allowed values.
   * @param string $default_order_direction
   *  ASC (default) / DESC direction.
   * @return array
   *  Returns the altered $a_var_setup.
   */
  protected function get_var_setup_order_by(array $a_var_setup = array(), $default_order_by = '', array $a_allowed_values_order_by = array(), $default_order_direction = mediamosa_type::ORDER_DIRECTION_ASC) {

    // Either must be set.
    assert(in_array($default_order_direction, array(mediamosa_type::ORDER_DIRECTION_ASC, mediamosa_type::ORDER_DIRECTION_DESC)));

    $a_var_setup[self::VARS][self::ORDER_BY] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_ALPHA_NUM,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_DEFAULT_VALUE => $default_order_by,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DESCRIPTION => mediamosa::t('On what column the result set should be ordered.'),
    );

    // Add allowed values for order by.
    if (count($a_allowed_values_order_by)) {
      $a_var_setup[self::VARS][self::ORDER_BY][self::VAR_ALLOWED_VALUES] = $a_allowed_values_order_by;
    }

    $a_var_setup[self::VARS][self::ORDER_DIRECTION] = array(
      self::VAR_TYPE => mediamosa_type::TYPE_ORDER_DIRECTION,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_DEFAULT_VALUE => $default_order_direction,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DESCRIPTION => mediamosa::t('The direction of the result set.'),
    );

    return $a_var_setup;
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

    if (!$this->a_rest_args[$param][self::PARAM_VALIDATED]) {
      $a_param = $this->a_rest_args[$param];
      $this->a_rest_args[$param][self::PARAM_VALUE] = mediamosa_type::check($param, $a_param[self::PARAM_TYPE], $a_param[self::PARAM_VALUE], $a_param[self::PARAM_IS_REQUIRED] == self::VAR_IS_REQUIRED_YES, $a_param[self::PARAM_RANGE_START], $a_param[self::PARAM_RANGE_END], isset($a_param[self::PARAM_ALLOWED_VALUES]) && count($a_param[self::PARAM_ALLOWED_VALUES]) ? $a_param[self::PARAM_ALLOWED_VALUES] : NULL);
      $this->a_rest_args[$param][self::PARAM_VALIDATED] = TRUE;
    }

    return $this->a_rest_args[$param][self::PARAM_VALUE];
  }

  /**
   * Set the value of the supplied parameter.
   * Use only internally, expected to be correct value, not value checked.
   *
   * @param string $param
   * @param mixed $value
   */
  public function set_param_value($param, $value) {
    if ($this->a_rest_args[$param][self::PARAM_IS_READ_ONLY] == self::VAR_IS_READ_ONLY_YES) {
      throw new mediamosa_exception_program_error('Do not set param @param, its read only.', array('@param' => $param));
    }

    $this->a_rest_args[$param][self::PARAM_VALUE] = $value;

    // Expected to ok, we cant validate anyway.
    $this->a_rest_args[$param][self::PARAM_VALIDATED] = TRUE;
  }

  /**
   * Unset param from var listing.
   *
   * @param string $param
   */
  public function unset_param($param) {
    $this->set_param_value($param, NULL);
  }

  /**
   * Will return TRUE if the param is set.
   *
   * @param string $param
   * @return boolean
   */
  public function isset_param($param) {
    return isset($this->a_rest_args[$param]) && isset($this->a_rest_args[$param][self::PARAM_VALUE]);
  }

  /**
   * Will return TRUE if the param was set by default and not by input.
   *
   * @param string $param
   * @return boolean
   */
  public function isset_default_param($param) {
    return isset($this->a_rest_args[$param]) && $this->a_rest_args[$param][self::PARAM_ISSET_DEFAULT];
  }

  /**
   * Will return TRUE if the param is empty.
   *
   * @param string $param
   * @return boolean
   */
  public function empty_param($param) {
    if (!self::isset_param($param)) {
      return TRUE; // Not set, is empty.
    }

    if (is_string($this->a_rest_args[$param][self::PARAM_VALUE])) {
      return trim($this->a_rest_args[$param][self::PARAM_VALUE]) == '';
    }

    if (is_int($this->a_rest_args[$param][self::PARAM_VALUE])) {
      return $this->a_rest_args[$param][self::PARAM_VALUE] == 0;
    }

    // Default empty behavour.
    return empty($this->a_rest_args[$param][self::PARAM_VALUE]);
  }

  /**
   * Returns the app_ids param value (always array).
   */
  public function get_param_value_app() {
    return $this->get_param_value(self::APP_ID);
  }

  /**
   * Returns the offset param value.
   */
  public function get_param_value_offset() {
    return $this->get_param_value(self::OFFSET);
  }

  /**
   * Returns the offset param value.
   */
  public function get_param_value_limit() {
    return $this->get_param_value(self::LIMIT);
  }

  /**
   * Returns the order_by param value.
   */
  public function get_param_value_order_by() {
    return $this->get_param_value(self::ORDER_BY);
  }

  /**
   * Returns the order_direction param value.
   */
  public function get_param_value_order_direction() {
    return $this->get_param_value(self::ORDER_DIRECTION);
  }
  // ------------------------------------------------------------------ Functions (protected).
  /**
   * Process the supplied parameters and type validation.
   *
   * @param array $a_var_setup
   */
  protected function process_rest_args(array $a_var_setup) {

    // Clear it.
    $this->a_rest_args = array();
    foreach ($this->{self::URI_PARAMS} as $key_uri => $value_uri) {
      if (!isset($a_var_setup[self::VARS][$key_uri]) && $this->check_for_unspecified) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_uri));
      }

      $this->a_rest_args[$key_uri][self::PARAM_VALUE] = $value_uri;
    }

    // Fill the $a_rest_args array with rest input.
    foreach ($_GET as $key_get => $value_get) {
      if ($key_get == 'q') {
        continue; // Ignore the q variable.
      }

      if (!isset($a_var_setup[self::VARS][$key_get]) && $this->check_for_unspecified) {
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

    // Now do the same to _POST.
    foreach ($_POST as $key_post => $value_post) {
      if (!isset($a_var_setup[self::VARS][$key_post]) && $this->check_for_unspecified) {
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

    // All args are now known. Now walk through the setup and set the defaults.
    foreach ($a_var_setup[self::VARS] as $param => &$a_var) {

      // Lets assert when we forget the description.
      assert(isset($a_var[self::VAR_DESCRIPTION]));

      // Set by default off.
      $this->a_rest_args[$param][self::PARAM_ISSET_DEFAULT] = FALSE; // Set by default

      // If variable is internal value only, then we set the value to default or NULL. Put it on READ ONLY.
      // If REST has tried set then we throw exception.
      if (isset($a_var[self::VAR_IS_INTERNAL_ONLY]) && $a_var[self::VAR_IS_INTERNAL_ONLY] == self::VAR_IS_INTERNAL_ONLY_YES  && !$this->call_is_internal()) {
        if (isset($this->a_rest_args[$param][self::PARAM_VALUE])) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INTERNAL_ONLY, array('@param' => $param, '@type' => $this->a_rest_args[$param][self::PARAM_TYPE]));
        }

      	unset($this->a_rest_args[$param][self::PARAM_VALUE]);
        if (!isset($a_var[self::VAR_IS_READ_ONLY])) {
          $a_var[self::VAR_IS_READ_ONLY] = self::VAR_IS_READ_ONLY_YES;
        }
      }

      // Make sure its set, even when empty.
      if (!isset($this->a_rest_args[$param][self::PARAM_VALUE])) {
        $this->a_rest_args[$param][self::PARAM_VALUE] = NULL; // Default NULL.

        // Defaults.
        if (isset($a_var[self::VAR_DEFAULT_VALUE])) {
          $this->a_rest_args[$param][self::PARAM_VALUE] = $a_var[self::VAR_DEFAULT_VALUE];
          $this->a_rest_args[$param][self::PARAM_ISSET_DEFAULT] = TRUE; // Set by default
        }
      }

      // Set not validated so we can validate when we need it.
      $this->a_rest_args[$param][self::PARAM_VALIDATED] = FALSE;

      // array check.
      if (isset($this->a_rest_args[$param][self::PARAM_VALUE])) {
        if (!isset($a_var[self::VAR_IS_ARRAY]) || $a_var[self::VAR_IS_ARRAY] == self::VAR_IS_ARRAY_NO) {
          if (is_array($this->a_rest_args[$param][self::PARAM_VALUE])) {
            $this->a_rest_args[$param][self::PARAM_VALUE] = reset($this->a_rest_args[$param][self::PARAM_VALUE]);
          }
        }
        elseif (!is_array($this->a_rest_args[$param][self::PARAM_VALUE])) {
          $this->a_rest_args[$param][self::PARAM_VALUE] = $this->a_rest_args[$param][self::PARAM_VALUE] == '' ? array() : array($this->a_rest_args[$param][self::PARAM_VALUE]);
        }
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
      $this->a_rest_args[$param][self::PARAM_IS_READ_ONLY] = $a_var[self::VAR_IS_READ_ONLY];

      // Is required.
      $this->a_rest_args[$param][self::PARAM_IS_REQUIRED] = $a_var[self::VAR_IS_REQUIRED];

      // if param is specified with range and the param is provided during rest call or had defaulft value, then check range.
      $this->a_rest_args[$param][self::PARAM_RANGE_START] = isset($a_var[self::VAR_RANGE_START]) && $this->isset_param($param) ? $a_var[self::VAR_RANGE_START] : NULL;
      $this->a_rest_args[$param][self::PARAM_RANGE_END] = isset($a_var[self::VAR_RANGE_END]) && $this->isset_param($param) ? $a_var[self::VAR_RANGE_END] : NULL;

      // Type.
      $this->a_rest_args[$param][self::PARAM_TYPE] = $a_var[self::VAR_TYPE];

      // Allowed values;
      $this->a_rest_args[$param][self::PARAM_ALLOWED_VALUES] = isset($a_var[self::VAR_ALLOWED_VALUES]) ? $a_var[self::VAR_ALLOWED_VALUES] : array();
    }

    return $a_var_setup;
  }

  /**
   * Validate the supplied parameters and type validation.
   *
   * @param array $a_var_setup
   */
  protected function validate_rest_args(array $a_var_setup) {

    foreach ($this->a_rest_args as $param => $a_param) {
      $this->a_rest_args[$param][self::PARAM_VALUE] = mediamosa_type::check($param, $a_param[self::PARAM_TYPE], $this->a_rest_args[$param][self::PARAM_VALUE], $this->a_rest_args[$param][self::PARAM_IS_REQUIRED] == self::VAR_IS_REQUIRED_YES, $this->a_rest_args[$param][self::PARAM_RANGE_START], $this->a_rest_args[$param][self::PARAM_RANGE_END], isset($this->a_rest_args[$param][self::PARAM_ALLOWED_VALUES]) && count($this->a_rest_args[$param][self::PARAM_ALLOWED_VALUES]) ? $this->a_rest_args[$param][self::PARAM_ALLOWED_VALUES] : NULL);
      $this->a_rest_args[$param][self::PARAM_VALIDATED] = TRUE;
    }
  }
}
