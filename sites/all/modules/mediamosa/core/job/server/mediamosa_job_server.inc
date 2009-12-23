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
  * The job server module.
  */


class mediamosa_job_server {

  /**
   * Log for job_server.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa::log($message, $variables, $severity, 'job_server');
  }

  /**
   * Retrieve a listing of the jobs.
   *
   * This function may only be called from jobserver (via internal).
   *
   * 1.x: vpx_jobserver_get_jobs_status (not exact port(!)).
   *
   */
  static public function search() {

    return mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->fields('jsa')
      ->leftJoin(mediamosa_job_server_analyse_db::TABLE_NAME, 'jsa', strtr('js.#jobserver_job_id = jsa.#jobserver_job_id', array('#jobserver_job_id' => mediamosa_job_server_db::ID)))
      ->condition(mediamosa_job_server_db::INSTALL_ID, MEDIAMOSA_INSTALL_ID)
      ->execute();
  }

  /**
   * Get the job.
   *
   * @param integer $job_id
   */
  static public function get($job_server_id) {
    return mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->condition(mediamosa_job_server_db::ID, $job_server_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Set the job status on a jobserver table.
   *
   * _vpx_jobserver_set_job_status
   *
   * @param $job_id
   * @param $job_status
   * @param $progress
   * @param $error_description
   */
  static public function set_job_status($job_id, $job_status, $progress, $error_description = '') {

    $a_fields = array(
      mediamosa_job_server_db::JOB_STATUS => $job_status,
      mediamosa_job_server_db::PROGRESS => $progress,
    );

    switch ($job_status) {
      case mediamosa_job_server_db::JOB_STATUS_FINISHED:
      case mediamosa_job_server_db::JOB_STATUS_FAILED:
      case mediamosa_job_server_db::JOB_STATUS_CANCELLED:
        $a_fields[mediamosa_job_server_db::FINISHED] = mediamosa::db_current_timestamp_now();
        break;
    }

    // Check if its started.
    $a_job = self::get($job_id);
    if (!$a_job) {
      assert(0);
      return;
    }

    // Set status.
    if ($a_job[mediamosa_job_server_db::JOB_STATUS] == mediamosa_job_server_db::JOB_STATUS_WAITING && $job_status == mediamosa_job_server_db::JOB_STATUS_INPROGRESS) {
      $a_fields[mediamosa_job_server_db::STARTED] = mediamosa::db_current_timestamp_now();
    }

    if (!empty($error_description)) {
      $a_fields[mediamosa_job_server_db::ERROR_DESCRIPTION] = $error_description;
    }

    // Update.
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->execute();
  }

  /**
   * Create a link to the parent asset belonging to a given job id.
   *
   * @param int $job_id
   *
   * @return string
   *  Link to an asset.
   */
  static public function get_asset_link($job_id) {

    // Get the job.
    $a_job = mediamosa_job::get($job_id);

    // Get asset ID from job.
    $asset_id = $a_job[mediamosa_job_db::ASSET_ID];

    // Get the asset.
    $a_asset = mediamosa_asset::get($a_job[mediamosa_job_db::ASSET_ID]);

    // Check parent.
    if (!empty($a_asset[mediamosa_asset_db::PARENT_ID])) {
      $asset_id = $a_asset[mediamosa_asset_db::PARENT_ID];
    }

    // Return link.
    return l(mediamosa::t('Go to asset @asset_id', array('@asset_id' => $asset_id)), strtr(mediamosa_settings::URL_ASSET, array('@asset_id' => $asset_id)));
  }

  /**
   * Create the string send with the vpx-analyse script.
   *
   * 1.x: _vpx_jobserver_get_analyse_string
   *
   * @param string $mediafile_src
   * @param $job_id
   */
  function get_analyse_string($mediafile_id, $job_id) {

    // Get the job.
    $a_app = mediamosa_job::get($job_id, array(mediamosa_job_db::APP_ID));
    $app_id = $a_app[mediamosa_app_db::APP_ID];

    // Get the app.
    $a_mediamosa_app = mediamosa_app::get_by_appid($app_id, array(mediamosa_app_db::ALWAYS_HINT_MP4, mediamosa_app_db::ALWAYS_INSERT_MD));

    $a_options = array();

    if ($a_mediamosa_app[mediamosa_app_db::ALWAYS_HINT_MP4] == mediamosa_app_db::ALWAYS_HINT_MP4_TRUE) {
      $a_options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
    }

    if ($a_mediamosa_app[mediamosa_app_db::ALWAYS_INSERT_MD] == mediamosa_app_db::ALWAYS_INSERT_MD_TRUE) {
      $a_options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
    }

    $execution_string = sprintf('%s %s %s',
      mediamosa_settings::analyse_file(),
      mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(),
      $mediafile_id
    );

    $execution_string .= (count($a_options) ? ' ' . implode(' ', $a_options) : '');

    return $execution_string;
  }

  /**
   * Generate the string that is used for the vpx_transcode script.
   *
   * 1.x: _vpx_jobserver_get_transcode_string
   *
   * @param string $jobserver_id
   * @param string $mediafile_id
   */
  function get_transcode_string($jobserver_id, $mediafile_id) {

    // Get it.
    $a_job_server_transcode = mediamosa_job_server_transcode::get($jobserver_id);

    $execution_string = '';

    if ($a_job_server_transcode) {
      $tool = $a_job_server_transcode[mediamosa_job_server_transcode_db::TOOL];
      $file_extension = $a_job_server_transcode[mediamosa_job_server_transcode_db::FILE_EXTENSION];
      $command = $a_job_server_transcode[mediamosa_job_server_transcode_db::COMMAND];
    }
    else {
      self::log('Transcode job not found, jobserver_id: @jobserver_id', array('@jobserver_id' => $jobserver_id));
      return $execution_string;
    }

    // bewerk de parameters zodat er een mooie string ontstaat.
    $parameters = mediamosa_lib::create_named_array($command, ';', ':');
    $parameter_string = "'";
    foreach ($parameters as $name => $value) {
      $parameter_string .= sprintf('%s %s ', $name, $value);
    }

    $parameter_string .= "'";
    $parameter_string = trim($parameter_string);
    if ($parameter_string == "''") {
      $parameter_string = "";
    }

    // combineer alles op basis van de gekozen tool.
    switch ($tool) {
      case 'ffmpeg':
        $execution_string = sprintf("%s %s %s %s %s > /dev/null &", mediamosa_settings::ffmpeg_transcode_file(), mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(), $mediafile_id, $file_extension, $parameter_string);
        break;
      case 'windows':
        $execution_string = sprintf("%s %s %s %s", mediamosa_settings::windows_transcode_file(), $mediafile_id, $file_extension, $parameter_string);
        break;
      default:
        return '';
    }

    return $execution_string;
  }

  /**
   * Generate the still execute string.
   *
   * 1.x: _vpx_jobserver_get_still_string
   *
   * @param string $jobserver_id
   * @param string $mediafile_id
   * @return string|string|string
   */
  static public function get_still_string($jobserver_id, $mediafile_id) {

    $a_job_server_still = mediamosa_job_server_still::get($jobserver_id);
    $a_job_server = mediamosa_job_server::get($jobserver_id);
    $job_id = $a_job_server[mediamosa_job_server_db::JOB_ID];

    $execution_string = "";

    if ($a_job_server_still) {
      $frametime = $a_job_server_still[mediamosa_job_server_still_db::FRAMETIME];
      $size = $a_job_server_still[mediamosa_job_server_still_db::SIZE];
      $h_padding = $a_job_server_still[mediamosa_job_server_still_db::H_PADDING];
      $v_padding = $a_job_server_still[mediamosa_job_server_still_db::V_PADDING];

      $still_parameters = unserialize($a_job_server_still[mediamosa_job_server_still_db::STILL_PARAMETERS]);
      $video_duration = $still_parameters['video_duration'];
      $fps = $still_parameters['fps'];
      $start_frame = $still_parameters['start_frame'];
      $end_frame = $still_parameters['end_frame'];
      if (!$start_frame || !is_numeric($start_frame) || $start_frame < 0) {
        $start_frame = 0;
      }
      //$ef = $video_duration * $fps;
      if (!$end_frame || !is_numeric($end_frame) || $end_frame > $video_duration-1) {
        $end_frame = $video_duration-1;
      }
      if ($start_frame > $video_duration-1) {
        $start_frame = $video_duration-1;
      }
      if ($end_frame < 0) {
        $end_frame = 0;
      }
      if (!$video_duration || !is_numeric($video_duration)) {
        // An error, falls back to the original behaviour
        $still_parameters['still_type'] = 'NONE';
      }
      if ($start_frame > $end_frame) {
        $tmp = $start_frame;
        $start_frame = $end_frame;
        $end_frame = $tmp;
      }

      // Data check
      switch ($still_parameters['still_type']) {
        case 'NORMAL':
          if (!is_numeric($still_parameters['still_per_mediafile']) || $still_parameters['still_per_mediafile'] < 1) {
            $still_parameters['still_type'] = 'NONE';
          }
          break;
        case 'SECOND':
          if (!is_numeric($still_parameters['still_every_second']) || $still_parameters['still_every_second'] < 0) {
            $still_parameters['still_type'] = 'NONE';
          }
          break;
        case 'SCENE':
          if (!$fps || !is_numeric($fps)) {
            // An error, falls back to the original behaviour
            $still_parameters['still_type'] = 'NONE';
          }
          break;
        default:
          break;
      }

      $frametime = $start_frame;
      $duration = 1;
      $framerate = 1;
      switch ($still_parameters['still_type']) {
        case 'NORMAL':
          $still_per_mediafile = $still_parameters['still_per_mediafile'];
          $frametime = $start_frame;
          $duration = ($end_frame - $start_frame);
          // |--------- video length -----------|
          // Stills per mediafile = 4
          // |------S------S------S------S------|
          // $framesecond = duration / (4 + 1)
          // .......|-- video length -----------|
          // frametime += $framesecond
          // duration -= $framesecond
          $framesecond = $duration / ($still_per_mediafile + 1);
          $frametime += $framesecond;
          $duration -= $framesecond;
          // Frames per second
          $framerate = 1/$framesecond;
          // Safety check
          if ($duration / $framesecond > mediamosa_settings::STILL_MAXIMUM) {
            $duration = mediamosa_settings::STILL_MAXIMUM * $framesecond;
          }
          break;
        case 'SECOND':
          $still_every_second = $still_parameters['still_every_second'];
          $frametime = $start_frame;
          $duration = ($end_frame - $start_frame);
          //
          //$frametime += $still_every_second;
          //$duration -= $still_every_second;
          $framerate = 1/$still_every_second;
          // Safety check
          if ($duration / $still_every_second > mediamosa_settings::STILL_MAXIMUM) {
            $duration = mediamosa_settings::STILL_MAXIMUM * $still_every_second;
          }
          break;
        case 'SCENE':
          watchdog('server', 'Scene detection starts');
          $destination_path = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . DATA_LOCATION . DIRECTORY_SEPARATOR . 'transcode' . DIRECTORY_SEPARATOR;
          //watchdog('server', sprintf('Scene: destination path = %s', $destination_path));
          $input = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id);

          // Clean up I.
          $order = sprintf('rm %s', $destination_path . $mediafile_id .'_scene.avi');
          //watchdog('server', $order);
          exec($order);
          $order = sprintf('rm %s', $destination_path . $mediafile_id .'_list.el');
          //watchdog('server', $order);
          exec($order);

  /*
  // Old code
          $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $input, $destination_path . $mediafile_id .'_scene.avi');
          //watchdog('server', $order);
          exec($order);
  */

  // Hot fix
          $order = sprintf('ffmpeg -i %s %s', $input, $destination_path . $mediafile_id .'_scene.wmv');
          //watchdog('server', $order);
          exec($order);

          $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $destination_path . $mediafile_id . '_scene.wmv', $destination_path . $mediafile_id .'_scene.avi');
          //watchdog('server', $order);
          exec($order);

          $order = sprintf('rm %s', $destination_path . $mediafile_id . '_scene.wmv');
          //watchdog('server', $order);
          exec($order);
  // Hot fix - end

          $order = sprintf('lav2yuv -S %s %s', $destination_path . $mediafile_id .'_list.el', $destination_path . $mediafile_id .'_scene.avi');
          watchdog('server', $order);
          exec($order);

          $output = NULL;
          $order = sprintf('cat %s', $destination_path . $mediafile_id .'_list.el');
          //watchdog('server', $order);
          exec($order, $output);
          //watchdog('server', sprintf('output = %s', print_r($output, TRUE)));

          // Clean up II.
          $order = sprintf('rm %s', $destination_path . $mediafile_id .'_scene.avi');
          //watchdog('server', $order);
          exec($order);
          $order = sprintf('rm %s', $destination_path . $mediafile_id .'_list.el');
          //watchdog('server', $order);
          exec($order);

          // Analyze
          if ($output && is_array($output) && $output != array()) {
            //watchdog('server', sprintf('Output is array with data'));

            // tbr & map from the best stream
            $map = NULL;
            $tbr = NULL;
            $kbs = NULL;
            $order = sprintf('ffmpeg -i %s 2>&1', $input);
            //watchdog('server', $order);
            exec($order, $details);
            //watchdog('server', 'Details: '. print_r($details, TRUE));
            foreach($details as $line) {
              if (stripos($line, ' Stream ') !== FALSE && stripos($line, ' Video: ') !== FALSE) {
                //FIXME DO NOT USE EREG, IT WILL DEPRECRATED IN PHP.
                //ereg:This function has been DEPRECATED as of PHP 5.3.0 and REMOVED as of PHP 6.0.0. Relying on this feature is highly discouraged.
                if (ereg('Stream #[0-9]*\.([0-9]*)', $line, $reg_map) && ereg(' ([0-9]*) kb/s', $line, $reg_kbs)) {
                  if (!$map || !$kbs || $kbs < $reg_kbs[1]) {
                    $map = $reg_map[1];
                    $kbs = $reg_kbs[1];
                  }
                  if (ereg(' ([0-9]*) tb', $line, $regs)) {
                    $tbr = $regs[1];
                  }
                }
                elseif (!$map && ereg(' ([0-9]*) tb', $line, $regs)) {
                  $tbr = $regs[1];
                }
              }
            }
            //watchdog('server', 'tbr: '. print_r($tbr, TRUE) .', map: '. print_r($map, TRUE) .', kbs: '. print_r($kbs, TRUE));
            if (!$tbr) {
              // This is the base value
              $tbr = 25;
            }
            // Go further, if we have tbr
            if ($tbr) {

              $scene_frame = NULL;
              $i = 1;
              $scenes = array();
              foreach ($output as $line) {
                //watchdog('server', sprintf('Line: %s', print_r($line, TRUE)));
                $line_args = explode(' ', $line);
                //watchdog('server', sprintf('Line args: %s', print_r($line_args, TRUE)));
                if ($line_args && is_array($line_args) && $line_args != array()) {
                  //watchdog('server', sprintf('Line args is array with data'));
                  if (isset($line_args[0]) && isset($line_args[1]) && isset($line_args[2]) && !isset($line_args[3]) && is_numeric($line_args[0]) && is_numeric($line_args[1]) && is_numeric($line_args[2])) {
                    //watchdog('server', sprintf('Line args has 3 numbers'));
                    // $line_args[1] + num = a little bit after the change
                    $change_frame = $line_args[1] + mediamosa_settings::STILL_SCENE_AFTER;
                    //watchdog('server', 'change_frame: '. print_r($change_frame, TRUE) .', start_frame: '. print_r($start_frame, TRUE) .', end_frame: '. print_r($end_frame, TRUE));
                    if ($change_frame >= ($start_frame*$fps) && $change_frame <= ($end_frame*$fps)) {
                      // $scene_frame + num = minimal distance with two changes
                      if (!$scene_frame || $scene_frame + mediamosa_settings::STILL_SCENE_DISTANCE < $change_frame) {
                        $scene_frame = $change_frame;
                        //watchdog('server', sprintf('New scene frame: %s', $scene_frame));
                        $scene_sec = (int)($scene_frame/$tbr);

                        // VPX_STILL_SCENE_STRING = ffmpeg -i %s -s %s -padtop %d -padbottom %d -padleft %d -padright %d -an -deinterlace -y -ss %d -t 1 -r 1 %s -f image2 %s
                        $order = sprintf(mediamosa_settings::STILL_SCENE_STRING, $input, $size, $h_padding, $h_padding, $v_padding, $v_padding, $scene_sec, ($map ? '-map 0.'. $map : ''), $destination_path . $mediafile_id . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg");
                        //watchdog('server', $order);
                        exec($order, $details);
                        //watchdog('server', 'Details: '. print_r($details, TRUE));

                        $scenes[] = $scene_sec;

                        $i++;
                        if ($i > mediamosa_settings::STILL_MAXIMUM) {
                          // Emergency break
                          break;
                        }
                      }
                    }
                  }
                }
              }

            }

          }

          // Save the sec data (see: vpx_jobhandler.module)
          if (is_array($scenes) && $scenes != array()) {
            $my_file = $destination_path . $job_id .'_scene.txt';
            $fh = fopen($my_file, 'w') or $fh = NULL;
            if ($fh) {
              foreach ($scenes as $scene) {
                fwrite($fh, $scene ."\n");
              }
              fclose($fh);
            }

            // Do nothing after this
            return 'echo "Scene stills are rock"';
          }

          // We reached this point, so something went wrong in the creation of scene stills
          // So we are creating still type NONE (original behaviour)
          break;
        default:
          break;
      }

    }
    else {
      self::log('Still job not found, jobserver_id: @jobserver_id', array('@jobserver_id' => $jobserver_id));
      return $execution_string;
    }

    $still_parameters['frametime'] = $frametime;
    $still_parameters['duration'] = $duration;
    $still_parameters['framerate'] = $framerate;

    $a_fields = array(
      mediamosa_job_db::STILL_PARAMETERS => serialize($still_parameters),
    );

    // Enrich with changed/created.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_db::JOB_ID)
      ->execute();

    // combineer de string (alleen voor linux, windows genereert geen stills)
    $execution_string = sprintf('%s %s %s jpeg ' . mediamosa_settings::STILL_STRING, mediamosa_settings::still_file(), mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(), $mediafile_id, $size, $h_padding, $h_padding, $v_padding, $v_padding, $frametime, $duration, $framerate);
    $execution_string .= ' > /dev/null &';

    return $execution_string;
  }

  /**
   * Create an array of the status file.
   *
   * 1.x: _vpx_jobserver_get_status
   *
   * @param $filename
   * @param $orig
   */
  static public function get_status_contents($filename, $orig = FALSE) {
    $statusfile = mediamosa_configuration_storage::status_file_location_get($filename);
    if (!file_exists($statusfile)) {
      return array();
    }

    $a_result = $a_lines = array();

    $handle = fopen($statusfile, "r");
    while (!feof($handle)) {
      $a_lines[] = fgets($handle);
    }
    fclose($handle);

    // strip the garbage from the file
    foreach ($a_lines as $line) {
      list($name, $value) = explode(":", $line, 2);
      if ($name == "Progress" || $name == "Status" || $name == "Errors") {
        $a_result[$name] = trim($value);
      }
      elseif ($name == 'ffmpeg-output') {
        $a_result[$name] = implode("\n", explode('}-{', trim($value)));
      }
    }

    return ($orig ? implode('', $a_lines) : $a_result);
  }

  /**
   * Check the created still and save it if everything is ok.
   *
   * 1.x: _vpx_jobserver_store_new_still
   *
   * @param string $job_id
   *  Current job id.
   * @param string $mediafile_src
   *  Contains a file path to the mediafile
   * @return string
   *  Contains the error message
   */
  function store_new_still($job_id, $mediafile_src) {

    $base_filename = mediamosa_io::get_base_filename($mediafile_src);

    // Is black still working?
    if (@filesize(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename .'_0.jpeg')) {
      return JOBSTATUS_INPROGRESS;
    }

    // Check if there really is an image ($file_size > 0)
    if (filesize(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, 1) . ".jpeg") == 0) {
      // Something failed, very likely the frametime was too high. Remove the files and fail the job.
      $still_error = mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_JOB_FRAMETIME_GREATER_THEN_DURATION);

      // Update status.
      self::set_job_status($job_id, mediamosa_job_db::JOB_STATUS_FAILED, '1.000', $still_error);

      // Remove all of the still images
      $i = 1;
      while (file_exists(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg") && $i <= mediamosa_settings::STILL_MAXIMUM) {
        unlink(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg");
        $i++;
      }
      unlink(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename .".status");

      # FIX ME. Remove error notice from table.
      self::log($still_error);
      return JOBSTATUS_FAILED;
    }

    // Check if the frame has any usefull content. We do this by checking the amount of dominant colors.
    $mediafile_src = mediamosa_job_server_still::still_validate($job_id, $mediafile_src);

    if ($mediafile_src === FALSE) {
      self::log("Job failed, are there any colors in this movie?");
      self::set_job_status($job_id, mediamosa_job_db::JOB_STATUS_FAILED, '1.000', "Job failed, are there any colors in this movie?");
      return JOBSTATUS_FAILED;
    }

    $i = 1;
    $mediafile_dest = array();
    while (file_exists(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg")) {
      if ($i <= mediamosa_settings::STILL_MAXIMUM) {
        // Generate new hash
        $filename = mediamosa_db::uuid(rand(1, 9999));

        // Everything went ok, move the still and remove other files
        rename(
          mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg",
          mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . STILL_LOCATION . DIRECTORY_SEPARATOR . $filename{0} . DIRECTORY_SEPARATOR . $filename
        );
        $mediafile_dest[] = $filename;
      }
      else {
        // Reached the maximum, just delete the remain stills
        unlink(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) .".jpeg");
      }
      $i++;
    }
    mediamosa_io::unlink(mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR . $base_filename .".status");

    // Data to update.
    $a_fields = array(
      mediamosa_job_server_db::MEDIAFILE_DEST => serialize($mediafile_dest),
    );

    // Add changed.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    // Update mediafile_dest of the job
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->execute();

    self::log('Job finished: Still saved as e.g.: ' . mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . STILL_LOCATION . DIRECTORY_SEPARATOR . $filename{0} . DIRECTORY_SEPARATOR . $filename, NULL);

    return JOBSTATUS_FINISHED;
  }
}
