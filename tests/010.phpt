--TEST--
Test function rsync_patch_file with stream as input and output.
--SKIPIF--
<?php if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
rsync_set_log_level(RSYNC_LOG_INFO);
/* TODO implement this*/ 
?>
--EXPECT--
