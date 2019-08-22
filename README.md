# HeavyEyelid

The purpose of this project is to provide a web HTTP based frontend to libvirt. We use Boost.Beast, a minimal C++ HTTP server that is used to communicate between the web and the libvirt daemon. We use JSON to transmit data between the server and the client.

We also created a C++ binding for libvirt, and use it to interface with libvirt.

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

For more examples and a usage reference, see the [API documentation](https://github.com/HeavyEyelid/virthttp/wiki/API-documentation)

## Building
### Dependencies *(debian packages)*
- libvirt-dev
- libboost-system-dev

### Build steps
#### Getting sources
```
$ git clone https://github.com/HeavyEyelid/virthttp.git
$ git submodule update --init
```
#### Compiling
```
$ cd virthttp/
$ mkdir build
$ cd build/
$ cmake ..
$ make -j $(nproc)
```
