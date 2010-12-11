--TEST--
Test function rsync_patch_file
--SKIPIF--
<?php if (!extension_loaded("rsync") || 
          !file_exists("tests/004test.png") || 
          !file_exists("tests/004test.patch")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'e869400efb68a96bafc33bda9a940111';
$ret = rsync_patch_file('tests/004test.png', 'tests/004test.patch', 'tests/004test2.png');
$resultMD5 = md5_file('tests/004test2.png');

if ($resultMD5 == $targetMD5 && $ret == RSYNC_DONE) {
	echo "SUCCESS\n";
} else {
	echo "FAILED with Return $ret and MD5 $resultMD5\n";
}	
unlink('tests/004test2.png');
?>
--EXPECT--
SUCCESS
