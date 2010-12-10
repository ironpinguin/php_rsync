--TEST--
Test function rsync_patch_file with stream as input and output.
--SKIPIF--
<?php if (!extension_loaded("rsync") || 
          !file_exists("tests/007test.png") || 
          !file_exists("tests/007test.patch")) print "skip"; ?>
--FILE--
<?php 
rsync_set_log_level(RSYNC_LOG_EMERG);
?>
--EXPECT--
