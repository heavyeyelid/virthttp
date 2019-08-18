# HeavyEyelid

The purpose of this project is to provide a WEB HTTP based frontend to libvirt. We use beast, a minimal C++ HTTP server that is used to communicate between the WEB and the libvirt daemon. We use JSon to transmit data between the server and the client.

We also created a C++ binding API for libvirt, and using this to communicate with libvirt.

### Few usage examples

#### A cURL request to list all domains

```bash
curl -X GET "http://localhost:8081/libvirt/domains" \
     -H "Content-Type:application/json" \
     -H "X-Auth-Key:1234567893feefc5f0q5000bfo0c38d90bbeb" \
     -H "X-Auth-Username:smith"
```

#### A JSon return listing all domains

```json
{
  "results": [
    {
      "name": "vm1",
      "uuid": "f6ah2js8-dfgv-3f3f-fgs1-d2s09dhjej83",
      "id": -1,
      "status": "Shutoff",
      "os": "hvm",
      "ram": 4194304,
      "ram_max": 4194304,
      "cpu": 4
    },
    {
      "name": "vm2",
      "uuid": "8shfjaif-f8s9-12vg-ger5-e5hbt78n4bhe",
      "id": 2,
      "status": "Running",
      "os": "hvm",
      "ram": 4194304,
      "ram_max": 4194304,
      "cpu": 4
    }
  ],
  "success": true,
  "errors": [],
  "messages": []
}
```

For more examples and usages references, see the [API documentation](https://github.com/ShinoYasx/HeavyEyelid/wiki/API-documentation)

## Building
### Dependencies *(debian packages)*
- libvirt-dev
- libboost-system-dev

### Build steps
#### Getting sources
```
$ git clone https://github.com/ShinoYasx/HeavyEyelid.git
$ git submodule update --init
```
#### Compiling
```
$ cd HeavyEyelid/
$ mkdir build
$ cd build/
$ cmake ..
$ make -j $(nproc)
```
