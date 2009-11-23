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
 * Our DB layer between MediaMosa and Drupal DB.
 */


class mediamosa_db {
  // ------------------------------------------------------------------ Consts.
  const LIMIT = 'limit';
  const OFFSET = 'offset';
  const LIMIT_MAX = 1000;
  const ORDER_BY = 'order_by';
  const ORDER_DIRECTION = 'order_direction';
  const ORDER_DIRECTION_ASC = 'asc';
  const ORDER_DIRECTION_DESC = 'desc';

  // The varchar length of the used HASH ids.
  const HASH_ID_LENGTH = 32;

  // ------------------------------------------------------------------ Functions (static).
  static public function db_delete(/*$table, $options = NULL*/) {
    $a_args = func_get_args(); // Must copy into $a_args, cant use it directly in call_user_func_array

    if (mediamosa::is_db_logging()) {
      $f_start_time = microtime(TRUE);
      $result = call_user_func_array('db_delete', $a_args);
      mediamosa::log_db(round(microtime(TRUE) - $f_start_time, 4), $result && isset($result->queryString) ? $result->queryString : $a_args[0]);
      return $result;
    }

    return call_user_func_array('db_delete', $a_args);
  }

  /**
   * Wrapper around query functions.
   *
   * @param string $func
   *  String value of the db_query[_*] function.
   * @param string $query
   *  SQL query.
   * @param array $a_args
   *  The arguments for $query.
   * @return result
   *  DB result set object.
   */
  static private function query($func = 'db_query', array $a_args = array()) {

    // Because the order can change of the a_args per func, lets create pointers to them.
    switch ($func) {
      case 'db_query_range':
       // By default, specify we want array's and not objects.
        $a_args[4] += array(
          'fetch' => PDO::FETCH_ASSOC,
        );
        $query = &$a_args[0]; // The Query
        $args = &$a_args[3];  // The Arguments.
        break;

      case 'db_query_temporary':
      case 'db_query':
        // By default, specify we want array's and not objects.
        $a_args[2] += array(
          'fetch' => PDO::FETCH_ASSOC,
        );

        $query = &$a_args[0]; // The Query.
        $args = &$a_args[1];  // The Arguments.
        break;

      default:
        throw new mediamosa_exception_program_error('not expected.');
    }

    // Make sure {} are inside the query, this will prevent problems later.
    if (stripos($query, 'FROM') !== FALSE && stripos($query, ' found_rows()') !== FALSE) {
      if (strpos($query, '{') === FALSE || strpos($query, '}') === FALSE) {
        error_log($query);
        throw new mediamosa_exception_program_error('Missing {} your query; @query.', array('@query' => $query));
      }
    }

    // Define '#vars' for inserting values that are not quoted in the result.
    if (isset($args)) {
      $a_extras = array();
      foreach ($args as $key => $value) {
        if ($key{0} != '#') {
          continue;
        }

        $a_extras[$key] = $value;
      }

      if (count($a_extras)) {
        $query = strtr($query, $a_extras);
        foreach ($a_extras as $key => $value) {
          unset($args[$key]);
        }
      }
    }

    // Loggin on.
    if (mediamosa::is_db_logging()) {
      $f_start_time = microtime(TRUE);
    }

    // Do the query.
    $result = call_user_func_array($func, $a_args);

    // Loggin on.
    if (mediamosa::is_db_logging()) {
      mediamosa::log_db(round(microtime(TRUE) - $f_start_time, 4), $result && isset($result->queryString) ? $result->queryString : $query);
    }

    return $result;
  }

  /**
   * Same as db_query, except for adding an range for query.
   *
   * @return DB result
   *  DB result set object.
   */
  static public function db_query_range(/*$query, $from, $count, array $args = array(), array $options = array()*/) {
    $a_args = func_get_args();
    return self::query('db_query_range', $a_args);
  }

  /**
   * Execute DB query.
   *
   * @return DB result
   *  DB result set object.
   */
  static public function db_query(/*$query, $args = array(), $options = array()*/) {
    $a_args = func_get_args();

    /**
     * Start Query hack fix for old 6.x drupal queries.
     */
    if (isset($a_args[1]) && !is_array($a_args[1])) {
      $query = &$a_args[0]; // The Query.
      $a_args_2 = array();

      $y = 1;
      for ($x = 0; $x < strlen($query); $x++) {
        if ($query{$x} != '%' || $x + 1 == strlen($query)) {
          continue;
        }

        // We only replace the specifiers which are not escaped and in our array.
        $char = $query{++$x};
        if (in_array($char, array('n', 'd', 'f', 'b', 's'))) {
          $query = substr($query, 0, $x - 1) . '#mediamosa_var_' . $y. substr($query, $x + 1);

          $prefix = '#';

          // If we had '%s', then this has become '#mediamosa_var_x', now replace that.
          if ( ($x - 2) >= 0 && substr($query, $x - 2, strlen("'#mediamosa_var_" . $y . "'")) == "'#mediamosa_var_" . $y . "'") {
            $query = substr($query, 0, $x - 2) . ':mediamosa_var_' . $y . substr($query, $x + strlen("#mediamosa_var_" . $y));
            $prefix = ':';
          }

          // lets skip the inserted stuff a bit.
          $x += strlen('#mediamosa_var_' . $y) - 2;

          switch ($char) {
            case 'd':
              $a_args_2[$prefix . 'mediamosa_var_' . $y] = (int)$a_args[$y];
              break;
            case 'f':
              $a_args_2[$prefix . 'mediamosa_var_' . $y] = (float)$a_args[$y];
              break;
            default:
              $a_args_2[$prefix . 'mediamosa_var_' . $y] = (string)$a_args[$y];
              break;
          }

          // Remove the numbered index version.
          unset($a_args[$y++]);
        }
      }

      // Copy the new array into args[1]
      $a_args[1] = $a_args_2;
    }
    /**
     * End Query hack fix for old 6.x drupal queries.
     */

    if (!isset($a_args[1])) {
      $a_args[1] = array();
    }
    if (!isset($a_args[2])) {
      $a_args[2] = array();
    }

    // Lets make sure we return the inserted ID with insert into!
    if (strtoupper(substr($a_args[0], 0, strlen('INSERT INTO '))) == 'INSERT INTO ') {
      $a_args[2] += array(
        'return' => Database::RETURN_INSERT_ID,
      );
    }

    return self::query('db_query', $a_args);
  }

  /**
   * Execute DB query in an temporary table.
   *
   * @return DB result
   *  DB result set object.
   */
  static public function db_query_temporary(/*$query, array $args = array(), array $options = array()*/) {
    $a_args = func_get_args();
    return self::query('db_query_temporary', $a_args);
  }

  /**
   * Create an db_select object.
   *
   * @return object
   *  DB Select object.
   */
  static public function db_select(/*$table, $alias = NULL, array $options = array()*/) {
    $a_args = func_get_args(); // Must copy into $a_args, cant use it directly in call_user_func_array

    // Lets return array's by default and not objects.
    $a_args += array(2 =>
      array(
        'fetch' => PDO::FETCH_ASSOC,
      ),
    );

    if (mediamosa::is_db_logging()) {
      $f_start_time = microtime(TRUE);
      $result = call_user_func_array('db_select', $a_args);
      mediamosa::log_db(round(microtime(TRUE) - $f_start_time, 4), $result && isset($result->queryString) ? $result->queryString : '-failure-');
      return $result;
    }

    // Call db_select function and attach ouy select extender.
    return call_user_func_array('db_select', $a_args);
  }

  /**
   * Create an db_insert object.
   *
   * @return object
   *  DB Insert object.
   */
  static public function db_insert(/*$table, array $options = array()*/) {
    $a_args = func_get_args(); // Must copy into $a_args, cant use it directly in call_user_func_array

    if (mediamosa::is_db_logging()) {
      $f_start_time = microtime(TRUE);
      $result = call_user_func_array('db_insert', $a_args);
      mediamosa::log_db(round(microtime(TRUE) - $f_start_time, 4), $result && isset($result->queryString) ? $result->queryString : '-failure-');
      return $result;
    }

    return call_user_func_array('db_insert', $a_args);
  }

  /**
   * Create an db_update object.
   *
   * @return object
   *  DB Update object.
   */
  static public function db_update(/*$table, array $options = array()*/) {
    $a_args = func_get_args(); // Must copy into $a_args, cant use it directly in call_user_func_array

    if (mediamosa::is_db_logging()) {
      $f_start_time = microtime(TRUE);
      $result = call_user_func_array('db_update', $a_args);
      mediamosa::log_db(round(microtime(TRUE) - $f_start_time, 4), $result && isset($result->queryString) ? $result->queryString : '-failure-');
      return $result;
    }

    return call_user_func_array('db_update', $a_args);
  }

  /**
   * Escape the value for database
   * 'db_escape_string' is removed from drupal 7, D7 uses prepare instead. For now
   * we will use PDO::quote.
   */
  static public function escape_string($string) {
    // Hmm well kinda hack here, but dont see any other way atm
    // than calling the getconnection for PDO and using quote...
    // But \' must be escaped as '' in the PDO layer, quote seems to
    // make mistakes here by escaping with \'. Anyway, hotfix here. RBL.
    return str_replace('\\\'', "''", mediamosa_unicode::substr(Database::getConnection()->quote($string), 1, -1)); // Remove the quotes, only escape.
  }

  /**
   * Returns a valid escaped string when using like
   *
   * @param string $str_like
   * @return string
   */
  static public function escape_string_like($string_like) {
    return str_replace(array('%', '_'), array('\%', '\_'), self::escape_string($string_like));
  }

  /**
   * Returns a valid escaped string when for fulltext.
   *
   * @param string $str_like
   * @return string
   */
  static public function escape_string_fulltext($string_fulltext) {
    return self::escape_string($string_fulltext);
  }

  /**
   * Set current active database wrapper.
   *
   * @param name of database $key
   * @return previous selected database.
   */
  static public function db_set_active($key = 'default') {
    return db_set_active($key);
  }
  /**
   * Get current active database wrapper.
   *
   * @return selected database.
   */
  static public function db_get_active() {
    $current_key = db_set_active();
    db_set_active($current_key);

    return $current_key;
  }

  /**
   * Install the foreign keys.
   * Drupal 7 at this time only describes how it wants to implement foreign keys in
   * their schema layer. For now we will add foreign keys ourselfs after creation of
   * the table.
   *
   * @param array $schema
   */
  static public function db_install_schema_foreign_keys($module_name) {
    $a_schema = drupal_get_schema_unprocessed($module_name);
    _drupal_schema_initialize($module_name, $a_schema);

    if (!isset($a_schema[$module_name]) || !isset($a_schema[$module_name]['foreign keys'])) {
      return;
    }

    foreach ($a_schema[$module_name]['foreign keys'] AS $column => $a_foreign_key) {
      foreach ($a_foreign_key as $foreign_table_name => $foreign_column) {
        $query = strtr("ALTER TABLE #table_name ADD CONSTRAINT `fk_#table_name_#column` FOREIGN KEY (`#column`) REFERENCES `#foreign_table_name` (`#foreign_column`)",
          array(
            '#table_name' => $module_name,
            '#column' => $column,
            '#foreign_table_name' => $foreign_table_name,
            '#foreign_column' => $foreign_column,
          )
        );

        try {
          db_query($query);
        }
        catch(PDOException $e) {
          assert($e);
          assert(0);// so we know.
          mediamosa::log(
            "Failed to create foreign key 'fk_#table_name_#column' on table '#table_name', query; '#query'.",
            array(
              '#table_name' => $module_name,
              '#column' => $column,
              '#query' => $query,
            )
          );
        }
        // only the first is allowed.
        break;
      }
    }
  }

  /**
   * Returns the number of rows.
   *
   * @param string $table_name
   *  The table name.
   * @param array $a_columns_and
   *  column => value.
   * @return integer
   *    returns number of rows.
   */
  static public function db_count_rows($table_name, $a_columns_and = array()) {

    // Create select object.
    $query = self::db_select($table_name);
    foreach($a_columns_and as $column => $value) {
      $query->condition($column, $value);
    }

    return $query->countQuery()->execute()->fetchField();
  }

  /**
   * Will throw exception if no rows where found.
   *
   * @param string $table_name
   *  The table name.
   * @param array $a_columns_and
   *  column => value.
   *
   */
  static function db_must_exists($table_name, $a_columns_and = array()) {

    if (self::db_count_rows($table_name, $a_columns_and)) {
      return; // exists
    }

    //
    switch ($table_name) {
      case mediamosa_asset_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, array("@asset_id" => $a_columns_and[mediamosa_asset_db::ID]));
      case mediamosa_asset_mediafile_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, array("@mediafile_id" => $a_columns_and[mediamosa_asset_media_db::ID]));
      case mediamosa_aut_group_db::TABLE_NAME:
        throw new mediamosa_exception_error(ERRORCODE_AUT_GROUP_NOT_FOUND, array("@group" => $a_columns_and[mediamosa_aut_group_db::AUT_GROUP_NAME]));
/*
      case 'collection':
        throw new vpx_exception_error(ERRORCODE_COLLECTION_NOT_FOUND, array("@coll_id" => $a_columns_and['coll_id']));
      case 'ftp_batch':
        throw new vpx_exception_error(ERRORCODE_FTP_BATCH_NOT_FOUND, array("@batch_id" => $a_columns_and['batch_id']));
      case 'job':
        throw new vpx_exception_error_job_not_found(array("@job_id" => $a_columns_and['job_id']));
      case 'ftpuser':
        throw new vpx_exception_error(ERRORCODE_FTP_UNKNOWN_USER);
      case 'quota_user':
        throw new vpx_exception_error(ERRORCODE_USERMAN_INVALID_USER, array("@user_id" => $a_columns_and["name"]));
      case 'quota_group':
        throw new vpx_exception_error(ERRORCODE_USERMAN_INVALID_GROUP, array("@group_id" => $a_columns_and["group_id"]));
*/
    }

    assert(0);
    throw new mediamosa_exception_error(ERRORCODE_UNEXPECTED_ERROR);
  }

  /**
   * Will throw exception if no rows where found.
   *
   * @param string $table_name
   *  The table name.
   * @param array $a_columns_and
   *  column => value.
   * @return boolean
   *  Return true or false on exists one or more row match.
   */
  static function db_exists($table_name, $a_columns_and = array()) {
    try {
      self::db_must_exists($table_name, $a_columns_and);
    }
    catch (mediamosa_exception $e) {
      assert($e);
      return FALSE;
    }

    return TRUE;
  }
}
