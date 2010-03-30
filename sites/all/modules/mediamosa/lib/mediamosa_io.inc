<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
  * In and out file library layer.
  */

class mediamosa_io {

  /**
   * Log for mediamosa_io.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array()) {
    mediamosa_debug::log_debug($message, $variables, 'mediamosa_io');
  }

  /**
   * Attempts to create the directory specified by pathname.
   * Fails with exception.
   *
   * @param string $pathname
   *  Path of the directory to create.
   * @param bool $recursive
   *  Enable recursive to create the sub maps required to create directory.
   * @param int $mode
   *  Default mode value for created directory, defaults to mode 0777.
   */
  static public function mkdir($pathname, $recursive = TRUE, $mode = 0777) {

    // Already exists? Then don't try to create.
    if (file_exists($pathname) && is_dir($pathname)) {
      return TRUE; // Return TRUE for unit test.
    }

    if (!drupal_mkdir($pathname, $mode, $recursive)) {
      throw new mediamosa_exception_error(
        mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
        array(
          '@location' => $pathname,
          '@reason' => mediamosa::last_php_error_message()
        )
      );
    }

    return TRUE;
  }

  /**
   * Deletes a file.
   *
   * @param string $filename
   *  The file to delete.
   */
  static public function unlink($filename) {
    if (!file_exists($filename)) {
      return TRUE;
    }

    return @unlink($filename);
  }

  /**
   * Clears file status cache.
   */
  static public function clearstatcache() {
    clearstatcache();
  }
  /**
   * Gets file size.
   */
  static public function filesize($filename) {
    return filesize($filename);
  }

  /**
   * Return the filename without extension.
   *
   * @param string $filename
   */
  static public function get_base_filename($filename) {
    $last_dot = strripos($filename, '.'); // 0-index

    return $last_dot === FALSE ? $filename : mediamosa_unicode::substr($filename, 0, $last_dot);
  }

  /**
   * Return the file extension (excluding the dot, like 'jpg' or 'mp4').
   *
   * @param string $filename
   */
  static public function get_ext($filename) {
    $last_dot = strripos($filename, '.'); // 0-index

    return $last_dot === FALSE && ($last_dot + 1) != strlen($filename) ? '' : mediamosa_unicode::substr($filename, $last_dot + 1);
  }

  /**
   * Returns directory name component of path.
   *
   * @param string $path
   */
  static public function dirname($path) {
    return dirname($path);
  }


  /**
   * Move one file to other file (rename).
   *
   * @param string $old_name
   * @param string $new_name
   *
   * @return bool
   */
  static public function rename($old_name, $new_name) {
    $success = FALSE;
    $message = 'cause unknown';
    try {
      // Make sure dest. map exists.
      $dirname = dirname($new_name);
      self::mkdir($dirname, TRUE);

      $success = rename($old_name, $new_name);
    }
    catch (Exception $e) {
      $message = $e->getMessage();
    }

    if (!$success) {
      self::log("Renaming file '@old_name' to '@new_name' failed, @message.", array('@old_name' => $old_name, '@new_name' => $new_name, '@message' => $message));
    }
    else {
      self::log("Successfully renamed file '@old_name' to '@new_name'.", array('@old_name' => $old_name, '@new_name' => $new_name));
    }

    return $success;
  }

  /**
   * Execute an external program
   *
   * @param $str
   */
  static public function exec($command, array &$output = NULL, &$return_var = NULL) {
    self::log("Executing command '@command'.", array('@command' => $command));
    return exec($command, $output, $return_val);
  }

  /**
   * Test if file is exists.
   *
   * @param string $filename
   */
  static public function file_exists($filename) {
    return file_exists($filename);
  }

  /**
   * Test if file is a symbolic link.
   *
   * @param string $filename
   */
  static public function is_link($filename) {
    return is_link($filename);
  }

  /**
   * Test if path is a directory.
   *
   * @param string $path
   */
  static public function is_dir($path) {
    return is_dir($path);
  }

  /**
   * Create a symbolic link.
   *
   * @param string $target
   *  Target file.
   * @param string $link
   *  Link to file to create.
   */
  static public function symlink($target, $link) {
    return symlink($target, $link);
  }

  /**
   * Tells whether a file exists and is readable.
   *
   * @param string $filename
   */
  static public function is_readable($filename) {
    return is_readable($filename);
  }


  /**
   * Tells whether the filename is writable.
   *
   * @param string $filename
   */
  static public function is_writable($filename) {
    return is_writable($filename);
  }

  /**
   * Sets access and modification time of file.
   *
   * @param string $filename
   *  The name of the file being touched.
   * @param integer $time
   *  The touch time. If time  is not supplied, the current system time is used.
   * @param integer $atime
   *  If present, the access time of the given filename is set to the value of atime.
   *  Otherwise, it is set to time.
   */
  static public function touch($filename, $time = NULL, $atime = NULL) {
    if(!isset($time)) {
      $time = time();
    }

    return @touch($filename, $time, $atime);
  }

  /**
   * Makes a copy of the file source to dest.
   *
   * @param string $source
   * @param string $dest
   */
  static public function copy($source, $dest) {
    return @copy($source, $dest);
  }

  /**
   * File scan directory
   *
   * @param dir
   * @param mask
   * @param options
   * @param depth
   */
  static public function file_scan_directory($dir, $mask, $options = array(), $depth = 0) {
    return file_scan_directory($dir, $mask, $options, $depth);
  }
}
