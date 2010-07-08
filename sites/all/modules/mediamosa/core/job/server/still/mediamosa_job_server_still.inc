<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Jobserver still functions.
 */
class mediamosa_job_server_still {

  static function get($jobserver_id, $a_fields = array()) {

    return mediamosa_db::db_select(mediamosa_job_server_still_db::TABLE_NAME, 'jss')
      ->fields('jss', $a_fields)
      ->condition(mediamosa_job_server_still_db::ID, $jobserver_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Return all the colors sorted by dominance.
   *
   * 1.x: _vpx_jobserver_still_colors
   *
   * @param string $image Path to the image
   * @return array All the colors as hexidecimal values ordered by occurance.
   */
  static public function still_colors($image) {
    // Resize the image, we need only the most significant colors
    $size = getimagesize($image);
    $scale = 1;
    if ($size[0]>0) {
      $scale = min(mediamosa_settings::STILL_RESAMPLE_WIDTH / $size[0], mediamosa_settings::STILL_RESAMPLE_HEIGHT / $size[1]);
    }

    if ($scale < 1) {
      $width = floor($scale * $size[0]);
      $height = floor($scale * $size[1]);
    }
    else {
      $width = $size[0];
      $height = $size[1];
    }

    $image_resized = imagecreatetruecolor($width, $height);
    switch ($size[2]) {
      case 1:
        $image_orig = imagecreatefromgif($image);
        break;
      case 2:
        $image_orig = imagecreatefromjpeg($image);
        break;
      case 3:
        $image_orig = imagecreatefrompng($image);
        break;
      default:
        $image_orig = imagecreatefromjpeg($image);
        break;
    }

    // Nearest neightbour as it does not alter the colors
    imagecopyresampled($image_resized, $image_orig, 0, 0, 0, 0, $width, $height, $size[0], $size[1]);

    $im = $image_resized;
    $img_width = imagesx($im);
    $img_height = imagesy($im);

    for ($y=0; $y < $img_height; $y++) {
      for ($x=0; $x < $img_width; $x++) {
        $index = imagecolorat($im,$x,$y);
        $rgb = imagecolorsforindex($im,$index);
        // Round the colors to reduce the amount of them
        $rgb['red']   = intval((($rgb['red']) + 15) / 32) * 32;
        $rgb['green'] = intval((($rgb['green']) + 15)/32) * 32;
        $rgb['blue']  = intval((($rgb['blue']) + 15) / 32) * 32;
        if ($rgb['red'] >= 256) {
          $rgb['red'] = 240;
        }

        if ($rgb['green'] >= 256) {
          $rgb['green'] = 240;
        }

        if ($rgb['blue'] >= 256) {
          $rgb['blue'] = 240;
        }

        $hex_array[] = mediamosa_gd::rgb2hex($rgb);
      }
    }
    $hex_array = array_count_values($hex_array);
    natsort($hex_array);
    $hex_array = array_reverse($hex_array, TRUE);

    return $hex_array;
  }

  /**
   * Calculate the average color by resampling the image to 1px.
   *
   * 1.x: _vpx_jobserver_still_averagecolor
   *
   * @param string $image Image path
   *
   * @return array RGB color
   */
  function still_averagecolor($image) {
    $image_tmp = imagecreatetruecolor(1, 1);
    $size = getimagesize($image);

    switch ($size[2]) {
      case IMAGETYPE_GIF:
        $image_orig = imagecreatefromgif($image);
        break;
      case IMAGETYPE_JPEG:
        $image_orig = imagecreatefromjpeg($image);
        break;
      case IMAGETYPE_PNG:
        $image_orig = imagecreatefrompng($image);
        break;
      default:
        $image_orig = imagecreatefromjpeg($image);
        break;
    }

    imagecopyresampled($image_tmp, $image_orig, 0, 0, 0, 0, 1, 1, $size[0], $size[1]);

    $color = imagecolorat($image_tmp, 0, 0);
    $rgb = imagecolorsforindex($image_tmp, $color);

    return $rgb;
  }

  /**
   * Based on the jobserver id and mediafile_src a number of stills is generated,
   * the still with the most different colors is returned.
   *
   * 1.x: _vpx_jobserver_still_validate
   *
   * @param integer $job_id
   * @param string $mediafile_id_src
   *
   * @return mixed
   *  FALSE when there are no stills or the name of the still
   */
  static public function still_validate($job_id, $mediafile_id_src) {
    // Retrieve the job settings
    $result_row = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_job_server_still} AS jsj
       LEFT JOIN {#mediamosa_job_server} AS jj ON jj.jobserver_job_id = jsj.jobserver_job_id
       WHERE jj.job_id = :job_id',
      array(
        '#mediamosa_job_server_still' => mediamosa_job_server_still_db::TABLE_NAME,
        '#mediamosa_job_server' => mediamosa_job_server_db::TABLE_NAME,
        ':job_id' => $job_id
      )
    )->fetchAssoc();

    if (!$result_row) {
      return $mediafile_id_src;
    }

    $execution_string = "";
    $frametime = $result_row[mediamosa_job_server_still_db::FRAMETIME];
    $size = $result_row[mediamosa_job_server_still_db::SIZE];
    $h_padding = $result_row[mediamosa_job_server_still_db::H_PADDING];
    $v_padding = $result_row[mediamosa_job_server_still_db::V_PADDING];
    $blackstill_check = $result_row[mediamosa_job_server_still_db::BLACKSTILL_CHECK];

    if ($blackstill_check == mediamosa_job_server_still_db::BLACKSTILL_CHECK_FALSE) {
      return $mediafile_id_src;
    }

    $file_duration = mediamosa_db::db_query(
      'SELECT file_duration FROM {#mediamosa_asset_mediafile_metadata} m
       LEFT JOIN {#mediamosa_job} j ON m.mediafile_id = j.mediafile_id
       WHERE job_id = :job_id
       LIMIT 1',
      array(
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
      )
    )->fetchfield();

    $duration_array = explode(':', $file_duration);
    $seconds = $duration_array[0] * 3600;
    $seconds += $duration_array[1] * 60;
    $seconds += $duration_array[2];

    $base_filename = mediamosa_io::get_base_filename($mediafile_id_src);
    $base_path = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::transcode_tmp_location_get() . DIRECTORY_SEPARATOR . $base_filename;

    // Create our extra stills
    $stills = array();

    // Move the original still to safety
    $path = $base_path . sprintf(mediamosa_settings::STILL_EXTENSION, 1) .'.jpeg';
    $new_path = $base_path .'_0.jpeg';
    mediamosa_io::rename($path, $new_path);

    // Add our still to an array
    $stills[0] = array(
      'base_filename' => $base_filename . '_0',
      'path' => $new_path,
    );

    $offset = 0;
    for ($i = 1; $i <= mediamosa_settings::STILLS_AMOUNT; $i++) {
      // Calc the time offsets
      if ($i % 2) {
        $offset = floor($i * mediamosa_settings::STILL_INTERVAL * mediamosa_settings::STILL_INTERVAL_JITTER);
        $time = $frametime + $offset;
      }
      else {
        $time = $frametime - $offset;
      }
      //$time = $i * $seconds / (mediamosa_settings::STILLS_AMOUNT+1);

      // Stay within the movie duration
      $time = ($time <= 0)? 1 : $time;
      $time = ($time >= $seconds)? $seconds-1 : $time;

      $duration = 1;
      $framerate = 1;

      if (mediamosa_settings::get_ffmpeg_pad()) {
        $execution_string = sprintf(
          '%s %s %s jpeg ' . mediamosa_settings::STILL_STRING . ' > /dev/null',
          mediamosa_settings::still_file(),
          mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(),
          $base_filename,
          $size, $h_padding, $h_padding, $v_padding, $v_padding,
          round($duration * $framerate),
          $time, $duration, $framerate);
      }
      else {
        $size_explode = explode('x', $size);
        $size_width = $size_explode[0] + (2 * $v_padding);
        $size_height = $size_explode[1] + (2 * $h_padding);
        $execution_string = sprintf(
          '%s %s %s jpeg ' . mediamosa_settings::STILL_STRING_VFPAD . ' > /dev/null',
          mediamosa_settings::still_file(),
          mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(),
          $base_filename,
          $size, $size_width, $size_height, $v_padding, $h_padding,
          round($duration * $framerate),
          $time, $duration, $framerate);
      }

      // Execute the LUA call
      mediamosa_io::exec($execution_string);

      // Move it
      $path = $base_path . sprintf(mediamosa_settings::STILL_EXTENSION, 1) . '.jpeg';
      $new_path = $base_path .'_'. $i .'.jpeg';
      mediamosa_io::rename($path, $new_path);

      // Add our still to an array
      $stills[$i] = array(
        'base_filename' => $base_filename . '_'. $i,
        'path' => $new_path,
      );
    }

    // Check the colors all the stills
    foreach ($stills as $key => $value) {
      if (file_exists($stills[$key]['path']) && filesize($stills[$key]['path']) > 0) {
        $stills[$key]['colors'] = self::still_colors($stills[$key]['path']);
      }
      else {
        // The still does not exist
        mediamosa_io::unlink($stills[$key]['path']);
      }
    }

    // Determine the still with the most colors
    if (count($stills) > 0) {
      $maxvalue = 0;
      foreach ($stills as $key => $value) {
        if (count($value['colors']) > $maxvalue) {
          $maxvalue = count($value['colors']);
          $maxkey = $key;
        }
      }

      // $stills[$maxkey] should contain the most colors and should be ok.
      // Adjust the filename

      $file = explode('_', $stills[$maxkey]['base_filename']); // That is why there was an _ in the filename
      $still = $file[0];
      $path = $stills[$maxkey]['path'];
      $new_path = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::transcode_tmp_location_get() . DIRECTORY_SEPARATOR . $file[0] . sprintf(mediamosa_settings::STILL_EXTENSION, 1) .'.jpeg';
      mediamosa_io::rename($path, $new_path);

      // Remove everything in $stills since we no longer need it
      foreach ($stills as $key => $value) {
        mediamosa_io::unlink($stills[$key]['path']);
      }

      // Return the best still
      return $still;
    }
    else {

      // @FIXME:
      // Should remove the original still instead of rename, otherwise the still
      // generation will hang for 2 hours.

      return FALSE;
    }
  }
}
