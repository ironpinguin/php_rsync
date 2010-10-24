--TEST--
Test function rsync_generate_delta
--SKIPIF--
<?php if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'b6efe00f2049d221626e63c2a2534361';
$ret = rsync_generate_delta('tests/003signatur.sig', 'tests/003test.png', 'tests/003test.patch');
$resultMD5 = md5_file('tests/003test.patch');
if ($resultMD5 == $targetMD5 && $ret == RSYNC_DONE) {
	echo "SUCCESS\n";
} else {
	echo "FAILED with Return $ret and MD5 $resultMD5\n";
}	
unlink('tests/003test.patch');
?>
--EXPECT--
SUCCESS
