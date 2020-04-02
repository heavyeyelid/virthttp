# Stage 0
FROM alpine:edge AS build
ENV PREFIX /usr/local/
COPY . /tmp/virthttp
RUN apk --no-cache add clang gcc g++ make cmake binutils libvirt-dev boost-dev
RUN wget -q -O - https://github.com/ebiggers/libdeflate/archive/v1.5.tar.gz | tar -C /tmp -zxf - && \
    CC=clang PREFIX=${PREFIX} make -C /tmp/libdeflate-1.5 -j $(nproc) install
RUN if [ -e /tmp/virthttp/build ]; then rm -rf /tmp/virthttp/build; fi && \
    mkdir -p /tmp/virthttp/build && \
    cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_CXX_FLAGS=-I${PREFIX}/include -DCMAKE_EXE_LINKER_FLAGS=-L${PREFIX}/lib \
        -S /tmp/virthttp -B /tmp/virthttp/build && \
    cmake --build /tmp/virthttp/build --target install -- -j $(nproc)

# Stage 1
FROM alpine:edge AS virthttp
ENV PREFIX /usr/local/
RUN apk --no-cache upgrade && apk --no-cache add boost-system libvirt-libs libstdc++
COPY --from=build ["${PREFIX}","${PREFIX}/"]
CMD ["virthttp"]
EXPOSE 8081