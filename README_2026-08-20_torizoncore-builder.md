# torizoncore-builder

## Links

* [github](https://github.com/torizon/torizoncore-builder/)
* [arguments](https://developer.toradex.com/torizon/os-customization/torizoncore-builder-tool-commands-manual/)

## Update target via ssh

```bash
docker run --rm -i \
    -v "$(pwd):/workdir:ro" \
    -v tcb-storage:/storage \
    torizon/torizoncore-builder:latest \
    images \
    --remove-storage \
    unpack \
    yocto-workdir/build-torizon/deploy/images/verdin-imx8mp/torizon-docker-verdin-imx8mp-Tezi.tar
```

```bash
docker run --rm -i \
    -v tcb-storage:/storage \
    --net=host \
    -v /var/run/docker.sock:/var/run/docker.sock \
    torizon/torizoncore-builder:latest \
    deploy \
    --remote-host verdin-imx8mp-08910183 \
    --remote-username torizon \
    --remote-password 'torizon' \
    --reboot \
    base
```

Optional parameters
* `--verbose`
* `--log-level LOG_LEVEL`
* `--log-file LOG_FILE`

## Torizon Cloud

* https://developer.toradex.com/torizon/torizon-platform/torizon-updates/signing-and-pushing-torizoncore-packages-to-torizon-ota/

### credential.zip

Requires developer plan. Download from dashboard.

### Upload yocto image

```bash
docker run --rm -i \
    -v "$(pwd):/workdir:ro" \
    -v tcb-storage:/storage \
    torizon/torizoncore-builder:latest \
    images \
    --remove-storage \
    unpack \
    yocto-workdir/build-torizon/deploy/images/verdin-imx8mp/torizon-docker-verdin-imx8mp-Tezi.tar
```


The `/deploy` mount is required because `platform push` creates `/deploy/tuf-repo`
during signing. Mounting the user-owned `tuf/` directory keeps that operation
writable while the container runs under the current user's UID/GID.

```bash
cp ~/work_beampilot/austausch/2026-08-21_torizon_cloud_credentials/credentials.zip .

rm -rf tuf
mkdir tuf

docker run --rm -i \
    --user "$(id -u):$(id -g)" \
    -v "$(pwd):/workdir:ro" \
    -v "$(pwd)/tuf:/deploy" \
    -v tcb-storage:/storage \
    torizon/torizoncore-builder:latest \
    platform push \
    --credentials credentials.zip \
    --package-name maerki_demo \
    --package-version v0.1.2 \
    base
```

TODO: I have to `rm -r tuf` everytime...
```
Signing OSTree package maerki_demo (commit checksum c36fa238d81cbed536ee206c58978d421093ec866dee3b4e78678f6a2d83cff5) for Hardware Id(s) "verdin-imx8mp".
An error occurred
com.advancedtelematic.tuf.cli.repo.TufRepo$RepoAlreadyInitialized: Repository at /deploy/tuf-repo was already initialized
Error (3) running uptane command "uptane-sign" with arguments "['init', '--credentials', '/workdir/credentials.zip', '--repo', '/deploy/tuf-repo']"
```

## Ostree

```bash
$ ostree admin status
* torizon c36fa238d81cbed536ee206c58978d421093ec866dee3b4e78678f6a2d83cff5.1
    Version: 7.7.0-devel-20260821082328+build.0
    origin refspec: c36fa238d81cbed536ee206c58978d421093ec866dee3b4e78678f6a2d83cff5
  torizon 2e53aba101b012d63a7d90c955891e2269a51e2f92ac75362652a40320e5c467.2 (rollback)
    Version: 7.7.0-devel-20260820145747+build.0
    origin refspec: 2e53aba101b012d63a7d90c955891e2269a51e2f92ac75362652a40320e5c467
```

```bash
$ cat /proc/cmdline
```

```bash
$ ostree refs --list --revision
ostree/1/1/0	c36fa238d81cbed536ee206c58978d421093ec866dee3b4e78678f6a2d83cff5
ostree/1/1/1	2e53aba101b012d63a7d90c955891e2269a51e2f92ac75362652a40320e5c467
torizon:7/verdin-imx8mp/torizon/torizon-docker/testing	68f58c580b18e96d3b98c993529eebcf28e945d545c66887ced728c69b60bb69
```

```bash
$ ostree remote list
aktualizr-remote
tcbuilder

$ ostree remote refs tcbuilder    
error: While fetching http://localhost:41003/summary.sig: [7] Couldn't connect to server

$ ostree remote refs aktualizr-remote
error: While fetching https://dgw.torizon.io/treehub/summary.sig: [58] Problem with the local SSL certificate
```