Maybe it is the most quickly codesign alternative for iOS12+, cross-platform  **Linux**, **macOS**, more features.
If this tool can help you, please don't forget to <font color=#FF0000 size=5>🌟**star**🌟</font> [Zhylnn (Original creator of zsign)](https://github.com/zhlynn/zsign) & [Me](https://github.com/turannul/zsign)
## Compiling on macOS & Linux

```bash
./INSTALL.sh
```
## Note: Windows is unsupoorted unless used in WSL.
<br>
  
## zsign usage:
I have already tested on macOS and Linux, but you also need **unzip** and **zip** command installed.

```bash
Usage: zsign [-bnvledfwqvh] [-p privkey.p12/pem] [-m mobile.provision] [-o signed.ipa] unsigned.ipa [-P p12_pass] [-z compression_level]
Options:

-p, --pkey		Path to private key or p12 file. (PEM or DER format)
-m, --prov		Path to provisioning profile.
-o, --output		Path to output ipa file.
-P, --password		Password for private key or p12 file.
-b, --bundle_id		New bundle id to change.
-n, --bundle_name	New bundle name to change.
-v, --bundle_version	New bundle version to change.
-z, --zip_level		Compressed level when output the ipa file. (0-9)
-l, --dylib		Path to inject dylib file.
-E, --entitlements	Path to entitlements file.
-e, --remove_embedded	Remove emmbedded.mobileprovision.
-d, --debug		Generate debug output files. (.zsign_debug folder)
-f, --force		Force sign without cache when signing folder.
-w, --weak		Inject dylib as LC_LOAD_WEAK_DYLIB.
-q, --quiet		Quiet operation.
-V, --version		Show version.
-h, --help		Show this message.
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

