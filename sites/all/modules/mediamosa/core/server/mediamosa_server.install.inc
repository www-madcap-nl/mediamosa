<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
 * Installer for mediamosa_server_download module.
 */

require_once 'mediamosa_server_db.inc';
require_once 'job/mediamosa_server_job.install.inc';
require_once 'streaming/mediamosa_server_streaming.install.inc';
require_once 'tool/mediamosa_server_tool.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_install() {

  // Server node extensions.
  mediamosa::drupal_install_schema(mediamosa_server_db::TABLE_NAME);

  // Children.
  _mediamosa_server_streaming_install();
  _mediamosa_server_tool_install();

  $a_default_servers = array(
    array(1,0,'ON',"streaming","http://localhost/ticket/{TICKET}",
      "asf",
      "{IF_EXTERNAL}
<asx version='3.0'>
<entry>
<ref href='{TICKET_URI}' />
{IF_START}
<starttime value='{START_TIME}' />
{/IF_START}
{IF_DURATION}
<duration value='{DURATION_TIME}' />
{/IF_DURATION}
</entry>
</asx>
{/IF_EXTERNAL}
<object id='{MEDIAFILE_ID}' classid='CLSID:22D6F312-B0F6-11D0-94AB-0080C74C7E95' standby='Loading Media Player components...' type='application/x-oleobject' width='{WIDTH}' height='{HEIGHT}'>
<param name='filename' value='{WMA_TICKET_URI}' />
<param name='autostart' value='{AUTOPLAY}' />
<embed type='application/x-mplayer2' src='{WMA_TICKET_URI}' autostart='{AUTOPLAY_NUM}' name='{MEDIAFILE_ID}' width='{WIDTH}' height='{HEIGHT}'/>
</embed>
</object>"
      ,'','','','',''),
    array(2,0,'ON',"streaming","http://localhost/ticket/{TICKET}",
      "flv",
      "<script type='text/javascript' src='http://145.97.38.12/swfobject.js'></script>
<div id='mediaspace'>This div will be replaced</div>
<script type='text/javascript'>
var s1 = new SWFObject('http://145.97.38.12/player.swf','ply','{WIDTH}','{HEIGHT}','9');
s1.addParam('allowfullscreen','true');
s1.addParam('allowscriptaccess','always');
s1.addParam('wmode','opaque');
s1.addParam('flashvars','streamer={TICKET_URI}&autostart={AUTOPLAY}{IF_START}&start={START_TIME_SECONDS}{/IF_START}{IF_DURATION}&duration={START_PLUS_DURATION_TIME_SECONDS}{/IF_DURATION}');
s1.addVariable('type', 'mp4');
s1.write('mediaspace');
</script>"
      ,'','','','',''),
    array(3,0,'ON',"streaming","http://localhost/ticket/{TICKET}",
      "mov;mp4;m4a;3gp;3g2;mj2i:h264",
      "<script type='text/javascript' src='http://145.97.38.29/swfobject.js'></script>
<div id='mediaspace'>This div will be replaced</div>
<script type='text/javascript'>
var s1 = new SWFObject('http://145.97.38.12/player.swf','ply','{WIDTH}','{HEIGHT_PLUS_20}','9');
s1.addParam('allowfullscreen','true');
s1.addParam('allowscriptaccess','always');
s1.addParam('wmode','opaque');
s1.addParam('flashvars','streamer={TICKET_URI}&autostart={AUTOPLAY}{IF_START}&start={START_TIME_SECONDS}{/IF_START}{IF_DURATION}&duration={START_PLUS_DURATION_TIME_SECONDS}{/IF_DURATION}');
s1.addVariable('type', 'mp4');
s1.write('mediaspace');
</script>"
      ,'','','','',''),
    array(4,0,'ON',"streaming","http://localhost/ticket/{TICKET}",
      "mpeg|mov;mp4;m4a;3gp;3g2;mj2",
      "<object id='{MEDIAFILE_ID}' classid='CLSID:02Bf25D5-8C17-4B23-BC80-D3488ABDDC6B' standby='Loading Quicktime Player components...' width='{WIDTH}' height='{HEIGHT}'>
{IF_START}
<param name='starttime' value='{START_TIME}' />
{/IF_START}
{IF_DURATION}
<param name='endtime' value='{END_TIME}' />
{/IF_DURATION}
<param name='src' value='{TICKET_URI}' />
<param name='autoplay' value='{AUTOPLAY}' />
<embed type='video/quicktime' src='{TICKET_URI}' name='{MEDIAFILE_ID}' width='{WIDTH}' height='{HEIGHT}' autoplay='{AUTOPLAY}' {IF_START}starttime='{START_TIME}'{/IF_START} {IF_DURATION}endtime='{END_TIME}'{/IF_DURATION}/>
</object>"
      ,'','','','',''),
    array(11,2,'ON',"download","http://localhost/download/{TICKET}",'','','','','','',''),
    array(12,0,'OFF',"still","http://localhost/still/{TICKET}",'','','','','','',''),
    array(13,0,'ON',"job_processor",'http::/localhost','','',"localhost","127.0.0.1",10,"ANALYSE|ffmpeg|STILL",''),
    array(14,0,'ON',"upload","http://localhost/mediafile/upload?upload_ticket={TICKET}",'','','','','','',"http://example.org/mediafile/uploadprogress"),
  );


  // Create our node type.
  $server_node_type = array(
    'type' => mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER,
    'name' => t('MediaMosa server'),
    'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER,
    'description' => t('MediaMosa servers execute jobs for transcoding, uploading and other related work.'),
    'custom' => 1,
    'modified' => 1,
    'locked' => 0,
  );

  $server_node_type = node_type_set_defaults($server_node_type);
  node_type_save($server_node_type);

  foreach ($a_default_servers as $a_default_server) {
    $node = new stdClass();
    $node->uid = 1; // Admin
    $node->status = 1; // Published
    $node->type = mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER;

    $node->title = $a_default_server[4];
    $node->{mediamosa_server_db::VERSION} = $a_default_server[1];
    $node->{mediamosa_server_db::SERVER_STATUS} = $a_default_server[2];
    $node->{mediamosa_server_db::SERVER_TYPE} = $a_default_server[3];
    $node->{mediamosa_server_db::URI} = $a_default_server[4];
    $node->{mediamosa_server_db::CONTAINERS} = $a_default_server[5];
    $node->{mediamosa_server_db::OBJECT_CODE} = $a_default_server[6];
    $node->{mediamosa_server_db::SERVER_NAME} = $a_default_server[7];
    $node->{mediamosa_server_db::SLOTS} = (int)$a_default_server[8];
    $node->{mediamosa_server_db::TOOLS} = $a_default_server[9];
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS} = $a_default_server[10];

    if ($a_default_server[3] == 'job_progress') {
      $a_tools = explode('|', $a_default_server[9]);
      foreach ($a_tools as $tool) {
        $node->{mediamosa_server_tool_db::TABLE_NAME}[] = array(mediamosa_server_tool_db::TOOL => $tool);
      }
    }

    if ($a_default_server[3] == 'streaming') {
      $a_containers = explode('|', $a_default_server[5]);
      foreach ($a_containers as $container) {
        $node->{mediamosa_server_streaming_container_db::TABLE_NAME}[] = array(mediamosa_server_streaming_container_db::CONTAINER => $container);
      }
    }

    $node = node_save($node);
  }

  // Server job table is a link table between mediamosa_job and mediamosa_jobserver_job and our server.
  _mediamosa_server_job_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_server_uninstall() {
  // Server job uninstall.
  _mediamosa_server_job_uninstall();

    // Unstall our children..
  _mediamosa_server_streaming_uninstall();
  _mediamosa_server_tool_uninstall();

  // Server node extensions.
  mediamosa::drupal_uninstall_schema(mediamosa_server_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_schema() {
  $a_types = array(
    mediamosa_server_db::SERVER_TYPE_DOWNLOAD,
    mediamosa_server_db::SERVER_TYPE_STILL,
    mediamosa_server_db::SERVER_TYPE_STREAMING,
    mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR,
    mediamosa_server_db::SERVER_TYPE_UPLOAD,
  );

  $a_status = array(
    mediamosa_server_db::SERVER_STATUS_OFF,
    mediamosa_server_db::SERVER_STATUS_ON,
    mediamosa_server_db::SERVER_STATUS_CLOSE,
  );

  $a_schema[mediamosa_server_db::TABLE_NAME] = array(
    'description' => t('The download server table contains settings for the download servers.'),
    'fields' => array(
      mediamosa_server_db::NID => array(
        'description' => t('The Drupal node id.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_db::VID => array(
        'description' => t('The Drupal node revision id.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_db::VERSION => array(
        'description' => t('The version number.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => FALSE,
      ),
      mediamosa_server_db::SERVER_STATUS => array(
        'description' => t('The server status.'),
        'type' => "enum('" . implode("','", $a_status) . "')",
        'mysql_type' => "enum('" . implode("','", $a_status) . "')",
        'not null' => TRUE,
      ),
      mediamosa_server_db::SERVER_TYPE => array(
        'description' => t('The server type.'),
        'type' => "enum('" . implode("','", $a_types) . "')",
        'mysql_type' => "enum('" . implode("','", $a_types) . "')",
        'not null' => TRUE,
      ),
      mediamosa_server_db::URI => array(
        'description' => t('The optional URI of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::URI_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::DESCRIPTION => array(
        'description' => t('The description of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::DESCRIPTION_LENGTH,
        'not null' => FALSE,
        'default' => '',
      ),
      mediamosa_server_db::CONTAINERS => array(
        'description' => t('The optional container names this server can serve.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::CONTAINERS_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::OBJECT_CODE => array(
        'description' => t('The optional container names the server can serve.'),
        'type' => 'text',
        'not null' => FALSE,
      ),
      mediamosa_server_db::SERVER_NAME => array(
        'description' => t('The optional name of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::SERVER_NAME_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::SLOTS => array(
        'description' => t('The maximum number of concurrent transcodes.'),
        'type' => 'int',
        'not null' => FALSE,
      ),
      mediamosa_server_db::TOOLS => array(
        'description' => t('The optional tools this server serves.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::TOOLS_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::URI_UPLOAD_PROGRESS => array(
        'description' => t('The optional uri of the upload progress.'),
        'type' => 'varchar',
        'length' => mediamosa_server_db::URI_UPLOAD_PROGRESS_LENGTH,
        'not null' => FALSE,
      ),
    ),
    'primary key' => array('nid', 'vid'),
    'indexes' => array(
      'idx_servertype_serverstatus' => array(mediamosa_server_db::NID, mediamosa_server_db::SERVER_TYPE, mediamosa_server_db::SERVER_STATUS),
    )
  );

  return $a_schema;
}
