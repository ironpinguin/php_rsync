--TEST--
Test the rsync_generate_signatur function with binary string as input.
--SKIPIF--
<?php 
if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'e6048d9f1ed7367259270db93b43cf74';
$input = file_get_contents('tests/005test.png');
$ret = rsync_generate_signature($input, 'tests/005signatur.sig', 1);
//$ret = rsync_generate_signature('tests/005test.png','tests/005signatur.sig');
$resultMD5 = md5_file('tests/005signatur.sig');
if ($resultMD5 == $targetMD5 && $ret == RSYNC_DONE) {
	echo "SUCCESS\n";
} else {
	echo "FAILED with Return $ret and MD5 $resultMD5\n";
}	
unlink('tests/005signatur.sig');
?>
--EXPECT--
SUCCESS
