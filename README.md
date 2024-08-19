Maybe it is the most quickly codesign alternative for iOS12+, cross-platform  **Linux**, **macOS** & **Windows** , more features.
If this tool can help you, please don't forget to <font color=#FF0000 size=5>🌟**star**🌟</font> [Me](https://github.com/zhlynn).
## Compile on macOS & Linux (Debian {apt} based):

```bash
./install.sh
```
``` Windows is not supported i don't care, install linux. ```
<br>
  
## zsign usage:
I have already tested on macOS and Linux, but you also need **unzip** and **zip** command installed.

```bash
Usage: zsign [-options] [-k privkey.pem] [-m dev.prov] [-o output.ipa] file|folder

options:
-k, --pkey           Path to private key or p12 file. (PEM or DER format)
-m, --prov           Path to mobile provisioning profile.
-c, --cert           Path to certificate file. (PEM or DER format)
-d, --debug          Generate debug output files. (.zsign_debug folder)
-f, --force          Force sign without cache when signing folder.
-o, --output         Path to output ipa file.
-p, --password       Password for private key or p12 file.
-b, --bundle_id      New bundle id to change.
-n, --bundle_name    New bundle name to change.
-r, --bundle_version New bundle version to change.
-e, --entitlements   New entitlements to change.
-z, --zip_level      Compressed level when output the ipa file. (0-9)
-l, --dylib          Path to inject dylib file.
-w, --weak           Inject dylib as LC_LOAD_WEAK_DYLIB.
-i, --install        Install ipa file using ideviceinstaller command for test.
-q, --quiet          Quiet operation.
-v, --version        Show version.
-h, --help           Show help.
```

1. Show mach-o and codesignature segment info.
```bash
./zsign demo.app/execute
```

2. Sign ipa with private key and mobileprovisioning file.
```bash
./zsign -k privkey.pem -m dev.prov -o output.ipa -z 9 demo.ipa
```

3. Sign folder with p12 and mobileprovisioning file (using cache).
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.app
```

4. Sign folder with p12 and mobileprovisioning file (without cache).
```bash
./zsign -f -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.app
```

5. Inject dylib into ipa and re-sign.
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -o output.ipa -l demo.dylib demo.ipa
```

6. Change bundle id and bundle name
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -o output.ipa -b 'com.tree.new.bee' -n 'TreeNewBee' demo.ipa
```

7. Inject dylib(LC_LOAD_DYLIB) into mach-o file.
```bash
./zsign -l "@executable_path/demo.dylib" demo.app/execute
```

8. Inject dylib(LC_LOAD_WEAK_DYLIB) into mach-o file.
```bash
./zsign -w -l "@executable_path/demo.dylib" demo.app/execute
```
## How to sign quickly?

You can unzip the ipa file at first, and then using zsign to sign folder with assets.
At the first time of sign, zsign will perform the complete signing and cache the signed info into *.zsign_cache* dir at the current path.
When you re-sign the folder with other assets next time, zsign will use the cache to accelerate the operation. Extremely fast! You can have a try!


## License

zsign is licensed under the terms of  BSD-3-Clause license. See the [LICENSE](LICENSE) file.

> THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

