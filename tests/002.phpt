--TEST--
Test the rsync_generate_signatur function
--SKIPIF--
<?php 
if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'e6048d9f1ed7367259270db93b43cf74';
$ret = rsync_generate_signature('tests/002test.png','tests/002signatur.sig');
$resultMD5 = md5_file('tests/002signatur.sig');
if ($resultMD5 == $targetMD5 && $ret == RSYNC_DONE) {
	echo "SUCCESS\n";
} else {
	echo "FAILED with Return $ret and MD5 $resultMD5\n";
}	
unlink('tests/002signatur.sig');
?>
--EXPECT--
SUCCESS
