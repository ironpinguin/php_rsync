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
    
    public $remoteStructure = array();

    /**
     * Constructor of the Client.
     * 
     * @param string $targetUrl Url of the Server
     * @param string $localpath Local directory to sync
     * @param string $basepath  Remote base directory to sync if null the 
     *                          default will be used at the server
     * @param string $direction Direction to sync 
     *                          f = client to server
     *                          b = server to client
     */
    public function __construct($remoteStructure, $localpath, $direction, 
            $request) 
    {
        $this->localpath = $localpath;
        $this->remoteStructure = $remoteStructure;
        $this->direction = $direction;
    }

    public function sync()
    {
        $postdata = array();
        $this->getLocalStructur($this->localpath);
        $curl = curl_init($this->targetUrl);
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
        
        $postdata['filelist'] = json_encode($this->structure);
        $postdata['direction'] = $this->direction;
        if (!empty($this->basepath)) $postdata['basepath'] = $this->basepath;
        
        if ($this->direction == 'f') {
            $signaturFiles = array();
            foreach($this->structure as $filedata) {
                if ($filedata['type'] != 'dir') {
                    $fin = fopen($this->localpath.'/'.$filedata['name'], 'rb');
                    $tmpname = tempnam(sys_get_temp_dir(), 'sign');
                    $fsig = fopen($tmpname, 'wb');
                    $ret = rsync_generate_signature($fin, $fsig);
                    fclose($fin);
                    fclose($fsig);
                    if ($ret != RSYNC_DONE) {
                        throw new Exception("Signatur generating Failed with $ret !", $ret);
                    }
                    $signaturFiles[$filedata['name']] = file_get_contents($tmpname);
                    unlink($tmpname);
                }
            }
            $postdata['signatures'] = json_encode($signaturFiles);
        }
        $response = curl_errno($curl);
        var_dump($response);
        exit;
    }
    
    public function getLocalStructur($dir, $prefix = '')
    {
        $actualDirContent = scandir($dir);
        foreach( $actualDirContent as $dentry) {
            if ($dentry != '.' || $dentry != '..') {
                $type = filetype($dir.$dentry);
                $this->structure[] = array('name' => $prefix.'/'.$dentry,
                                           'type' => $type);
                if ($type == 'dir') {
                    $this->getLocalStructur($dir.$dentry, $prefix.'/'.$dentry);
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
$targetUrl = '';
$base = null;
$local = '';
$direction = 'f';

if (count($args) < 8) {
    rsyncClient::usage();
    exit(1);
}

for ($i=1; $i > count($args); $i=$i+2) {
    switch ($args[$i]) {
        case '-t':
            $targetUrl = $args[$i+1];
            break;
        case '-b':
            $base = $args[$i+1];
            break;
        case '-s':
            $local = $args[$i+1];
            break;
        case '-d':
            $direction = $args[$i+1];
            break;
        case '-h':
            rsyncClient::usage();
            exit;
            break;
        default:
            echo "Unknow option ".$args[$i]."\n";
            usage();
            break;
    }
}