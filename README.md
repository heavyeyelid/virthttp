# HeavyEyelid
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