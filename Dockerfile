FROM alpine:edge
COPY . /usr/src/virthttp
RUN apk add gcc g++ clang cmake make binutils libvirt-dev boost-dev wget tar
RUN mkdir -p /usr/src
WORKDIR /usr/src
RUN wget -O libdeflate-1.3.tar.gz https://github.com/ebiggers/libdeflate/archive/v1.3.tar.gz
RUN tar -xvf /usr/src/libdeflate-1.3.tar.gz
RUN rm libdeflate-1.3.tar.gz
WORKDIR /usr/src/libdeflate-1.3
RUN make -j $(nproc) install
RUN if [ -e /usr/src/virthttp/build ]; then rm -rf /usr/src/virthttp/build; fi
RUN mkdir -p /usr/src/virthttp/build
WORKDIR /usr/src/virthttp/build
RUN cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
RUN cmake --build . --target install
CMD ["/usr/bin/virthttp"]
EXPOSE 8081