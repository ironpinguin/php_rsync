<?php
/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ATTANTION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * This is a example for using the rsync extension
 * This example don't work with large directories and big files.
 * There get alle changing content in the address space to send them over
 * network, so if the data to transmit is to great to fit in the php max
 * memory usage this client will be case an PHP error.
 *
 * Feel free to implement your own protocol using the librsync to generate
 * signatur files and patch files.
 *
 * This client can only used to get changes from server and patch you local
 * directory with it.
 * There get the patches of changed files, deleted files, new files and new
 * directory.
 * (only the mode will be prevent an no user or group changes will be made)
 */
if (!extension_loaded("rsync")) {
    echo "You need the rsync php extension loaded to use this!";
    exit;
}
// Load the Logging class to log all messages from the extension in a logging file
// over callback registration
require_once "logger.php";

/**
 * The rsync example Client Class
 */
class rsyncClient
{
    /**
     * Target Url of the rsync php extension Server
     *
     * @var string
     */
    public $targetUrl;
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
     * Sync direction
     *  f  for syncing changes from Server to Client.
     *  b  for syncing changes from Client to Server.
     *
     * @var string
     */
    public $direction = 'f';

    /**
     * list of all entries and subentries of the local directory.
     *
     * @var array
     */
    public $strukture = array();

    /**
     * The start and end of the message blocks.
     *
     * @var array
     */
    public $msgblock = array('ERR' =>
                            array('start' => '<h1 style="color:red;">',
                                    'end' => '</h1>'),
                             'ALT' =>
                            array('start' => '<h2 style="color:blue;">',
                                    'end' => '</h2>'),
                             'MSG' =>
                            array('start' => '<p>', 'end' => '</p>')
                       );

    /**
     * Constructor of the Client.
     *
     * @param string $targetUrl Url of the Server
     * @param string $localpath Local directory to sync
     * @param string $direction Direction to sync
     *                          f = client to server
     *                          b = server to client
     * @param string $basepath  Remote base directory to sync if null the
     *                          default will be used at the server
     */
    public function __construct($targetUrl, $localpath, $sapi_type, $direction = 'f',
         $basepath = null)
    {
        // Define the defailt MessageBlock to make output message in the webbrowser.
        // Check if the php is called as cli or not.
        if ($sapi_type == 'cli') {
            foreach($this->msgblock as $type => $block) {
                $this->msgblock[$type]['start'] = '';
                $this->msgblock[$type]['end'] = '\n';
            }
        }
        // check if Target Url is valid
        if (!$this->isValidURL($targetUrl)) {
            $this->msg("Given Url '".$targetUrl."' is not a valid URL", 'ERR');
            rsyncClient::usage($sapi_type);
            throw new Exception("Given Url is not a valid URL", 1);
        }
        // Set TargetUrl
        $this->targetUrl = $targetUrl;
        // Check an set local directory path
        if (!is_dir($localpath) || !is_writable($localpath)) {
            $this->msg("Given local Directory '".$localpath.
                "' is not a directory or/and is not writeable","ERR");
            rsyncClient::usage($sapi_type);
            throw new Exception("Given local Directory '$localpath' is not a".
                "directory or/and is not writeable", 2);
        }
        $this->localpath = $localpath;
        // get and set the direction to sync.
        if ($direction != 'f' && $direction != 'b') {
            $this->msg("No valid Direction given: '".$direction."'","ERR");
            rsyncClient::usage($sapi_type);
            throw new Exception("No valid Direction given: '$direction'", 3);
        }
        $this->direction = $direction;

        // Set Basepath if given.
        if ($basepath !== null) $this->basepath = $basepath;
    }

    /**
     * Print a message out.
     *
     * @param string $message The message to print out
     * @param string $type    The type of message:
     *                         'MSG' = Normal message
     *                         'ALT' = Alert message
     *                         'ERR' = Error message.
     */
    public function msg($message, $type = 'MSG')
    {
        echo    $this->msgblock[$type]['start'].
                    $message.
                $this->msgblock[$type]['end'];
    }

    /**
     * Start der Sync process.
     */
    public function sync()
    {
        // generate a array from the local directory structure.
        $this->getLocalStructur($this->localpath);
        // Prepare the curl request
        $curl = $this->prepareCurl();
        // Prepare the post data for the request.
        $postdata = array();
        $postdata['filelist'] = json_encode($this->structure);
        $postdata['direction'] = $this->direction;
        if (!empty($this->basepath)) $postdata['basepath'] = $this->basepath;

        if ($this->direction == 'f') {
            $postdata['step'] = 1;
            $this->serverToClient($curl, $postdata);
        } else {
            // @TODO Implement the sync from client to server.
        }

    }

    /**
     *  Step by sync files from Server to Client
     *
     */
    public function serverToClient($curl, $postdata)
    {
        $signaturFiles = array();
        foreach($this->structure as $file => $data) {
            if ($data['type'] != 'dir') {
                // Open a file stream to the existing file and a temporary
                // file for the returning signatur file.
                $fin = fopen($this->localpath.'/'.$file, 'rb');
                $tmpname = tempnam(sys_get_temp_dir(), 'sign');
                $fsig = fopen($tmpname, 'wb');
                // Generate the signatur file
                $ret = rsync_generate_signature($fin, $fsig);
                fclose($fin);
                fclose($fsig);
                if ($ret != RSYNC_DONE) {
                    throw new Exception("Signatur generating Failed with ".
                            $ret."! => ".rsync_error($ret), $ret);
                }
                $signaturFiles[$file] = base64_encode(file_get_contents($tmpname));
                unlink($tmpname);
            }
        }
        $postdata['signatures'] = json_encode($signaturFiles);
        curl_setopt($curl, CURLOPT_POSTFIELDS, $postdata);
        $this->msg("Send Data to Server");
        $response = $this->sendCurlRequest($curl);

        if (!array_key_exists('changes', $response)) {
            $this->msg("ERROR from Server", "ERR");
            if (isset($response['ERROR'])) {
                $this->msg($response['ERROR'], 'ERR');
            }
            throw new Exception("Missing the patches in the response".
                    " from Server", 12);
        }
        $this->msg("Work on received data");
        ksort($response['changes'], SORT_LOCALE_STRING);
        foreach ($response['changes'] as $changeFile => $changeData) {
            switch ($changeData['changetype']) {
                case 'newDir':
                    $this->createDirectory($changeFile, $changeData);
                    break;
                case 'patch':
                    $this->patchFile($changeFile, $changeData);
                    break;
                case 'newFile':
                    $this->createFile($changeFile, $changeData);
                    break;
                case 'delete':
                    unlink($changeFile);
                    break;
                default :
                    throw new Exception("Unknow ChangeType ".
                            $changeData['changetype'], 13);
                    break;
            }
        }
    }

    /**
     * Send the Curl Request to the server and make a simple analyse of the response
     *
     * @param resource $curl The curl request resource
     *
     * @return array The Response Array
     */
    public function sendCurlRequest($curl)
    {
        $requestResponse = curl_exec($curl);
        if ($requestResponse === FALSE) {
            $this->msg("Curl Request Errror: ".curl_error($curl), "ERR");
            throw new Exception("Curl Request Error: ".curl_error($curl),
                    curl_errno($curl));
        }
        $response = json_decode($requestResponse, true);
        if ($response === NULL) {
            $this->msg("Response from Server is not understandable","ERR");
            throw new Exception("Response from Server is not understandable",
                    10);
        }
        if (key_exists("ERROR", $response)) {
            $this->msg("Some Error on Server: ".$response["ERROR"], "ERR");
            throw new Exception("Some Error on Server: ".$response["ERROR"], 11);
        }
    return $response;
    }

    /**
     * Set a curl request object.
     *
     * @return resource
     */
    public function prepareCurl()
    {
        $curl = curl_init($this->targetUrl);
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
        return $curl;
    }

    /**
     * Create a new Directory
     *
     * @param string $filename Directoryname
     * @param array  $data     createData
     */
    public function createDirectory($filename, $data)
    {
    	$this->msg("Create directory $filename");
        mkdir($this->localpath.DIRECTORY_SEPARATOR.$filename);
        chmod($this->localpath.DIRECTORY_SEPARATOR.$filename, $data['rights']);
    }

    /**
     * Create a new File
     *
     * @param string $filename
     * @param array  $data
     */
    public function createFile($filename, $data)
    {
        $this->msg("Create file $filename");
        file_put_contents($this->localpath.$filename,
                base64_decode($data['content']));
        chmod($this->localpath.$filename, $data['rights']);
    }

    /**
     * Patch the local file with the new changes from the patch file.
     *
     * @param $filename The file to patch
     * @param $data     The patch data
     *
     */
    public function patchFile($filename, $data)
    {
        $this->msg("Patch existing file $filename");
        $patchFile = tempnam(sys_get_temp_dir(), 'patch_');
        file_put_contents($patchFile, base64_decode($data['patch']));
        $ret = rsync_patch_file($this->localpath.$filename,
                $patchFile,
                $this->localpath.$filename.'-new');
        if ($ret != RSYNC_DONE) {
            throw new Exception("Can not patch file ".$filename.". Message: ".
                rsync_error($ret), $ret);
        }
        unlink($patchFile);
        unlink($this->localpath.DIRECTORY_SEPARATOR.$filename);
        rename($this->localpath.DIRECTORY_SEPARATOR.$filename.'-new',
                $this->localpath.DIRECTORY_SEPARATOR.$filename);
        chmod($this->localpath.$filename, $data['rights']);
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
            if ($dentry != '.' && $dentry != '..') {
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

    /**
     * Check if the given url is valid
     *
     * @param string $url The url to check
     * @return bool
     */
    private function isValidURL($url)
    {
        $r = preg_match(
            '|^http(s)?://[a-z0-9-]+(.[a-z0-9-]+)*(:[0-9]+)?(/.*)?$|i', $url);
        return $r;
    }

    /**
     * Print the usage to the console.
     *
     */
    public static function usage($sapi_type)
    {
        if ($sapi_type == 'cli') {
            echo "Usage: php client.php -t <URL> -s <local> [-b <base>]".
                " [-d <direction]\n".
                "  -t <URL>       The url where the server.php will be".
                " found\n".
                "  -b <base>      The base path where are configured in".
                " server.php to sync.\n".
                "                 Only needed if the server.php is ".
                "configured with multiple \n".
                "                 basedirectories to sync.\n".
                "  -s <local>     The local directory to sync\n".
                "  -d <direction> The direction to sync. Default is 'f'.\n".
                "                   f => from server to client\n".
                "                   b => from client to server\n";
        } else {
            echo "<h2>This Software need follow parameters:</h2>".
                 "<table border=0><tr><th>Parameter</th><th>Description".
                 "</th></tr><tr><td>target</td><td>".
                 "The url where the server.php will be".
                 " found</td></tr>".
                 "<tr><td>base</td>The base path where are configured in".
                 " server.php to sync.<td></td></tr>".
                 "<tr><td>local</td><td>The local directory to sync</td></tr>".
                 "<tr><td>direction</td><td></td></tr>".
                 "<tr><td></td><td>The direction to sync. Default is 'f'.".
                 "<br/> f => from server to client<br/>".
                 " b => from client to server</td></tr>";
        }
    }

}

/**
 * This is the part to get commandline parameters and init the rsyncClient
 * Class to sync with server.
 */

/**
 * Target Url get from given parameters
 *
 * @var string
 */
$targetUrl = '';
/**
 * Base directory on server to sync
 *
 * @var string/null
 */
$base = null;
/**
 * Local directory to sync with server
 *
 * @var string
 */
$local = '';
/**
 * Direction to sync.
 *     f => from server to client
 *     b => from client to server (not implemented)
 *
 * @var string
 */
$direction = 'f';
/**
 * Get string cli for sapi.
 *
 * @var string
 */
$sapi_type = substr(php_sapi_name(), 0, 3);

if ($sapi_type == 'cli') {
    if ($_SERVER['argc'] < 6) {
        rsyncClient::usage($sapi_type);
        exit(1);
    }

    for ($i=1; $i > $argc; $i=$i+2) {
        switch ($_SERVER['argv'][$i]) {
            case '-t':
                $targetUrl = $_SERVER['argv'][$i+1];
                break;
            case '-b':
                $base = $_SERVER['argv'][$i+1];
                break;
            case '-s':
                $local = $_SERVER['argv'][$i+1];
                break;
            case '-d':
                $direction = $_SERVER['argv'][$i+1];
                break;
            case '-h':
                rsyncClient::usage($sapi_type);
                exit(2);
                break;
            default:
                echo "Unknow option ".$_SERVER['argv'][$i]."\n";
                rsyncClient::usage($sapi_type);
                exit(3);
                break;
        }
    }
} else {
    if (isset($_REQUEST['target'])) $targetUrl = $_REQUEST['target'];
    if (isset($_REQUEST['base'])) $base = $_REQUEST['base'];
    if (isset($_REQUEST['local'])) $local = $_REQUEST['local'];
    if (isset($_REQUEST['dir'])) $direction = $_REQUEST['dir'];
    if (empty($targetUrl) || empty($local) ||
         ($direction != 'f' && $direction != 'b') ||
         (empty($base) && !is_null($base))) {
        echo '<h1 style="color:red;">The given parameters are wrong!</h1>';
        rsyncClient::usage($sapi_type);
        exit(4);
    }
}

// Init the logger class
try {
    $logger = rsyncLogger::getInstance();
    $logger->setLogfile("client.log");
    // Register the log method as callback for the rsync extension.
    rsync_set_log_callback(array(&$logger,"log"));
} catch (Exception $e){
    echo "Can't init logger class!\n".$e->getMessage()."\n";
    exit(5);
}

$rClient = new rsyncClient($targetUrl, $local, $sapi_type, $direction, $base);

$rClient->sync();
