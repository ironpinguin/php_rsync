--TEST--
Test function rsync_patch_file with stream as input and output.
--SKIPIF--
<?php if (!extension_loaded("rsync") || 
          !file_exists("tests/007test.png") || 
          !file_exists("tests/007test.patch")) print "skip"; ?>
--FILE--
<?php 
echo rsync_error(RSYNC_DONE) . "\n";
echo rsync_error(RSYNC_BLOCKED) . "\n";
echo rsync_error(RSYNC_RUNNING) . "\n";
echo rsync_error(RSYNC_TEST_SKIPPED) . "\n"; 
echo rsync_error(RSYNC_IO_ERROR) . "\n";
echo rsync_error(RSYNC_SYNTAX_ERROR) . "\n"; 
echo rsync_error(RSYNC_MEM_ERROR) . "\n";
echo rsync_error(RSYNC_INPUT_ENDED) . "\n";
echo rsync_error(RSYNC_BAD_MAGIC) . "\n";
echo rsync_error(RSYNC_UNIMPLEMENTED) . "\n";
echo rsync_error(RSYNC_CORRUPT) . "\n";
echo rsync_error(RSYNC_INTERNAL_ERROR) . "\n";
echo rsync_error(RSYNC_PARAM_ERROR) . "\n";
echo rsync_error() . "\n";
?>
--EXPECT--
OK
blocked waiting for input or output buffers
still running
unexplained problem
IO error
bad command line syntax
out of memory
unexpected end of input
bad magic number at start of stream
unimplemented case
stream corrupt
library internal error
unexplained problem
OK
