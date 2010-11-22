<?php

if (!extension_loaded("rsync")) {
    echo "You need the rsync php extension loaded to use this!";
    exit;
}

/**
 * The rsync example Client Class
 */
class rsyncServer
{
    /**
     * Basis Path at the rsync php extension Server 
     * 
     * @var string
     */
    public $basepath;
    
    /**
     * Local directory path to sync
     * 
     * @var string 
     */
    public $localpath;
    
    /**
     * Sync direction f for syncing changes from Client to Server and 
     * b for syncing changes from Server to Client.
     * 
     * @var string
     */
    public $direction = 'f';
    
    /**
     * list of all entries and subentries of the local directory.
     * 
     * @var array
     */
    public $structure = array();
    
    public $result = array();

    /**
     * Constructor of the Client.
     * 
     * @param string $localpath Local directory to sync
     * @param string $direction Direction to sync 
     *                          f = server to client
     *                          b = client to server
     */
    public function __construct($localpath, $direction)
    {
        $this->localpath = $localpath;
        $this->getLocalStructur($localpath);
        $this->direction = $direction;
    }

    /**
     *
     * @param type $remoteStructure
     * @param type $signatures 
     */
    public function step1($remoteStructure, $signatures)
    {
        foreach ($remoteStructure as $name => $data) {
            if (array_key_exists($name, $this->structure)) {
                $patch = $this->createPatch($name, $signatures[$name]);
                if ($patch === false) return json_encode("ERROR");
                $this->result['changes'][$name] = $this->structure[$name];
                $this->result['changes'][$name]['patch'] = $patch;
                $this->result['changes'][$name]['changetype'] = 'patch';
            }
        }
        foreach($this->structure as $name => $data) {
            if (!array_key_exists($name, $remoteStructure)) {
                $this->result['changes'][$name] = $data;
                if ($data['type'] == 'dir') {
                    $this->result['changes'][$name]['changetype'] = 'newDir';
                } else {
                    $this->result['changes'][$name]['changetype'] = 'newFile';
                    $this->result['changes'][$name]['content'] = 
                            file_get_contents($this->localpath.
                                    DIRECTORY_SEPARATOR.$name);
                }
            }
        }
        return json_encode($this->result);
    }
    
    /**
     *
     * @param type $name
     * @param type $signature
     * @return type 
     */
    public function createPatch($name, $signature) {
        $patchfile = tempnam(sys_get_temp_dir(), 'patch');
        $sighandle = fopen('data://text/plain;base64,'.
                base64_encode($signature), 'rb');
        $ret = rsync_generate_delta($sighandle, 
                $this->localpath.DIRECTORY_SEPARATOR.$name, $patchfile);
        fclose($sighandle);
        if ($ret != RSYNC_DONE) {
            return false;
        }
        $patch = file_get_contents($patchfile);
        unlink($patchfile);
        return $patch;
    }


    /**
     * Get the Local Directory Structure to check against the remote.
     * This Method is working recursive to step deeper in the directory.
     *
     * @param string $dir    Aktual working directory
     * @param string $prefix Prefix to make relative path to the initial 
     *                       directory
     */
    public function getLocalStructur($dir, $prefix = '')
    {
        $actualDirContent = scandir($dir);
        foreach( $actualDirContent as $dentry) {
            if ($dentry != '.' || $dentry != '..') {
                $type = filetype($dir."/".$dentry);
                if ($type != 'dir' && $type != 'file') continue;
                $stats = stat($dir."/".$dentry);
                if ($stats === FALSE) {
                    throw new Exception("Filestats for ".$dir."/".$dentry.
                            " ist not readable!", 9);
                }
                $this->structure[$prefix.'/'.$dentry] = array(
                    'name' => $prefix.'/'.$dentry,
                    'type' => $type, 'rights' => $stats['mode'],
                    'mtime' => $stats['mtime'], 'uid' => $stats['uid'],
                    'gid' => $stats['gid']);
                if ($type == 'dir') {
                    $this->getLocalStructur($dir.'/'.$dentry, 
                            $prefix.'/'.$dentry);
                }
            }
        }
    }
}

$syncpathes = array('/testdir1', '/testdir2');
$default = 0;

if (!isset($_REQUEST['step'])) {
    echo json_encode("ERROR");
}

$remoteStructure = json_decode($_REQUEST['filelist']);
if (isset($_REQUEST['basepath'])) {
    if (!in_array($_REQUEST['basepath'], $syncpathes)) {
        echo json_encode("ERROR");
        exit;
    }
    $localpath = $_REQUEST['basepath'];
} else {
    $localpath = $syncpathes[$default];
}

if ($_REQUEST['direction'] != 'f' && $_REQUEST['direction'] != 'b') {
    echo json_encode("ERROR");
    exit;
}

$direction = $_REQUEST['direction'];

if (!isset($_REQUEST['filelist']) && empty($_REQUEST['filelist'])) {
    echo json_encode("ERROR");
    exit;
}

$signatures = null;
$remoteStructure = json_decode($_REQUEST['filelist']);
if ($direction == 'f') {
    if (!isset($_REQUEST['signatures'])) {
        echo json_encode("ERROR");
        exit;
    }
    $signatures = json_decode($_REQUEST['signatures']);
} else {
    // Not implemented jet.
    echo json_encode("ERROR");
    exit;
}

try {
    $server = new rsyncServer($localpath, $direction);
} catch (Exception $e) {
    echo json_encode("ERROR");
    exit;
}
switch ($_REQUEST['step']) {
    case '1':
        echo $server->step1($remoteStructure, $signatures);
        break;
    case '2':
        // Not impelemented jet.
        echo json_encode("ERROR");
        break;
    default:
        echo json_encode("ERROR");
        break;
}